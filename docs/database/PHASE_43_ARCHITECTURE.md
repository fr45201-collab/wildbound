# Wildbound — Phase 43: Database Architecture

**Project:** Wildbound  
**Phase:** 43 — Database Infrastructure  
**Primary Database:** PostgreSQL 16+  
**Application Database:** `wildbound_db`  
**Driver:** `pgx/v5` (`github.com/jackc/pgx/v5/pgxpool`)

---

## 1. Architectural Principles

1. **PostgreSQL as Primary Source of Truth**: All persistent entities (accounts, profiles, cosmetics, persistent inventory, match records, contender stats) reside in PostgreSQL with full ACID guarantees.
2. **Dedicated Headless Server Gameplay Authority**: Active match simulation (combat, movement, storm, physics, projectile trajectories, ephemeral in-match loot) runs strictly in C++ memory on `WildboundServer` instances.
3. **Coordinator Pre-Match In-Memory State**: Active parties, queues, and lobby invitations are managed in-process by `WildboundCoordinator` (Phase 41).
4. **Repository Pattern**: All database interactions are encapsulated behind strict repository interfaces (`internal/repository`), decoupling domain logic and HTTP handlers from SQL execution details.
5. **Connection Pooling**: Managed via `pgxpool.Pool` with bounded connection counts, idle reclamation, health monitoring, and timeout contexts.
6. **Separation of Persistent vs Ephemeral Inventory**: Contender persistent items (crafting resources, arena tokens, cosmetics) are maintained in PostgreSQL (`player_inventory`). Active in-match ephemeral items (looted weapons, consumables) reside exclusively in C++ match simulation memory.

---

## 2. Component Diagram

```
+-------------------------------------------------------------+
|                      Wildbound Client                       |
|           (raylib / C++20 Contender Graphical Client)       |
+------------------------------+------------------------------+
                               | REST / HTTPS (Bearer JWT)
                               v
+-------------------------------------------------------------+
|                     Wildbound Backend                       |
|               (Go REST API / Service Layer)                 |
|                                                             |
|   +-------------------+  +--------------------+             |
|   |   Auth Service    |  |  Profile Service   |             |
|   +-------------------+  +--------------------+             |
|   | Inventory Service |  |  Cosmetics Service |             |
|   +-------------------+  +--------------------+             |
|   |  Matches Service  |  | Statistics Service |             |
|   +-------------------+  +--------------------+             |
|                                                             |
|   +-----------------------------------------------+         |
|   |               Repository Layer                |         |
|   | (Users, Profiles, Inventory, Cosmetics, etc.) |         |
|   +-----------------------+-----------------------+         |
|                           |                                 |
|   +-----------------------v-----------------------+         |
|   |             database.DB (pgxpool)             |         |
|   |  - Pool Sizing & Idle Timeout Management     |         |
|   |  - Health & Latency Probes                   |         |
|   |  - Transaction Management (WithTransaction)  |         |
|   +-----------------------+-----------------------+         |
+---------------------------|---------------------------------+
                            |
                            | TCP (Port 5432 / 5433)
                            v
+-------------------------------------------------------------+
|                     PostgreSQL Database                     |
|                 (wildbound_db - ACID Store)                 |
|                                                             |
|   [users]              [player_profiles]                    |
|   [player_inventory]   [cosmetic_definitions]               |
|   [player_cosmetics]   [match_records]                      |
|   [match_participants] [player_statistics]                 |
|   [refresh_tokens]     [schema_migrations]                  |
+-------------------------------------------------------------+
```

---

## 3. Data Flow

### 3.1 Contender Authentication & Profile
1. Client submits credentials to `POST /api/v1/auth/login`.
2. Backend queries `users` table via `UsersRepository.GetByUsernameOrEmail`.
3. Verifies bcrypt password hash, generates signed JWT access token and cryptographic refresh token.
4. Asynchronously records `last_login_at = NOW()` on the user record.
5. Client uses Bearer token for subsequent authenticated operations.

### 3.2 Persistent Inventory Consumption
1. Client requests `POST /api/v1/players/me/inventory/consume`.
2. `InventoryRepository` initiates a transaction (`WithTransaction`).
3. Acquires row lock using `SELECT ... FOR UPDATE` on `(user_id, item_definition_id)`.
4. Checks that current quantity is sufficient.
5. If remaining balance reaches 0, atomically executes `DELETE`. Otherwise, executes `UPDATE quantity = quantity - $1`.
6. Commits transaction; database check constraint `CHECK (quantity > 0)` guarantees no invalid states.

### 3.3 Match Ingestion
1. Match ends on `WildboundServer`. Outbox persists match payload to local disk.
2. Server POSTs payload to `POST /api/v1/internal/matches/results` with `X-Server-Token`.
3. Backend atomically inserts `match_records`, bulk inserts `match_participants`, and increments contender stats in `player_statistics` within a single ACID transaction.

