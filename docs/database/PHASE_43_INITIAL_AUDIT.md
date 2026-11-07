# Wildbound — Phase 43: Initial Repository & Database Audit

**Phase:** 43 — Database Infrastructure  
**Audit Date:** 2026-11-07  
**Auditor:** Antigravity  

---

## 1. Executive Summary

This audit assesses the state of the Wildbound codebase following Phase 42 (Go Backend: API, Authentication, Player Profiles, Match Records, Cosmetic Ownership & Statistics). The audit specifically investigates the PostgreSQL database integration, schema design, transactional boundaries, data isolation, Coordinator and Dedicated Server data flows, and migration mechanics to establish the roadmap for Phase 43.

---

## 2. Audit Findings by Subsystem

### 2.1 PostgreSQL Integration & Driver
- **Driver:** `github.com/jackc/pgx/v5` and `pgxpool` are installed in `backend/go.mod` and active.
- **Connection Management:** Basic connection pooling was initialized in `backend/internal/database/postgres.go` using `pgxpool.ParseConfig` with DSN string formatting.
- **Gaps Identified:**
  - Standard connection URL format (`DATABASE_URL=postgres://...`) should be supported as the primary connection string, with fallback to individual host/port/user fields.
  - Idle connection timeout (`MaxConnIdleTime`) is not currently configurable.
  - Active pool utilization statistics (acquired connections, idle connections, total connections) are not exposed via diagnostics.
  - Transaction lifecycle handling is ad-hoc within services rather than unified via structured helpers (`WithTransaction`).

### 2.2 Database Schema & Entity Design
- **Migrations Applied in Phase 42:**
  - `000001_initial_schema.up.sql` / `.down.sql`: Created `users`, `player_profiles`, `refresh_tokens`, `cosmetic_definitions`, `player_cosmetics`, `match_records`, `match_participants`, `player_statistics`.
  - `000002_seed_cosmetics.up.sql` / `.down.sql`: Seeded 12 cosmetic definitions matching the Wildbound catalog.
- **Gaps & Structural Improvements Identified:**
  1. **Account Audit Trail:** The `users` table lacks `last_login_at TIMESTAMPTZ`, which is required for security tracking and inactive account management.
  2. **Persistent Player Inventory:** While `player_cosmetics` tracks visual customization items, there is currently no table for persistent gameplay items, crafting materials, or currency rewards (`player_inventory`).
  3. **Match vs Account Inventory Separation:** Must ensure persistent account inventory (`player_inventory`) is strictly isolated from ephemeral match inventory (`wildbound::items::Inventory` in C++ simulation).
  4. **Strict Check Constraints:** Need explicit check constraints (e.g. `quantity > 0` on inventory items) to enforce data validity at the database level.

### 2.3 Go Backend Codebase & Repositories
- **Current Architecture:** Services in `backend/internal/` currently execute SQL queries directly against `*pgxpool.Pool`.
- **Gaps Identified:**
  - No dedicated repository abstraction layer (`backend/internal/repository/`). Extracting queries into `UserRepository`, `ProfileRepository`, `InventoryRepository`, `CosmeticsRepository`, `MatchesRepository`, and `StatisticsRepository` will improve testability, isolation, and query maintainability.
  - Database error handling directly surfaces raw errors in some paths; structured domain error mapping is required.
  - A unified migration CLI (`migrate status`, `migrate up`, `migrate down`, `migrate version`) is needed so migrations can be run independently of application boot.

### 2.4 WildboundCoordinator Data Flow Audit
- **Temporary In-Memory State:**
  - Ephemeral client connections (`network::ClientId`).
  - Active lobbies (`LobbyId`) and parties (`PartyId`, member lists, ready status).
  - Matchmaking queues (`MatchmakingQueue`, wait times, party sizes).
  - Dedicated server registration and real-time heartbeats (`DedicatedServerRegistry`).
- **Persistent State Requirements:**
  - Player authentication verification (validated against `WildboundBackend`).
  - Player account binding (`persistentUserId` bound to `SessionId`).
- **Verdict:** Coordinator correctly keeps ephemeral pre-match state in memory without persisting abandoned parties or queue tickets to disk.

### 2.5 WildboundServer Match Result Reporting Audit
- **Match Result Reporting Flow:**
  - When a match concludes, `DedicatedServer::ReportMatchCompletedToCoordinator` sends `MsgMatchCompletedNotification` to Coordinator.
  - In Phase 42, `DedicatedServer` was enhanced with `MatchResultsOutbox` which formats the full match scoreboard into JSON and transmits it to `POST /api/v1/internal/matches/results` with `X-Server-Token`.
  - On failure or network disconnect, results are spooled to `user_data/outbox/match_<match_id>.json` on disk and retried on subsequent ticks.
  - The backend ingests results idempotently based on `match_records.match_id`, preventing duplicate statistics increments.
- **Verdict:** Robust. The outbox and idempotent ingestion mechanisms survive backend/database downtime without data loss.

---

## 3. Data Integrity & Performance Risks

| Risk Area | Severity | Description | Phase 43 Mitigation |
|---|---|---|---|
| **Connection Exhaustion** | High | Unbounded or misconfigured pool under high concurrency exhausts PostgreSQL backend slots | Implement explicit `DATABASE_MAX_CONNECTIONS`, `DATABASE_MIN_CONNECTIONS`, `DATABASE_MAX_CONNECTION_LIFETIME`, and `DATABASE_MAX_CONNECTION_IDLE_TIME` |
| **Partial Transaction State** | Medium | A failure in registration or cosmetic equipping leaves orphaned records | Implement atomic `WithTransaction` closures and enforce foreign key cascades |
| **Negative Inventory Quantities** | High | Concurrent deduction requests could drop item counts below zero | Add database check constraint `CHECK (quantity > 0)` and atomic updates `quantity = quantity - $1 WHERE quantity >= $1` |
| **Missing Query Indexes** | Medium | As match records grow, unindexed lookups for match history or inventory will cause table scans | Audit query patterns and ensure composite indexes on `(user_id, ended_at DESC)` and `(user_id, item_definition_id)` |
| **Premature Redis Dependency** | Medium | Adding Redis prematurely increases operational complexity and introduces cache invalidation bugs | Conduct a thorough evaluation and justify whether Redis is needed |

---

## 4. Recommended Phase 43 Plan

1. **Environment & Security:** Prepare `.env.example`, `.env` (with placeholders), and ensure `.env` is ignored in Git.
2. **Database Modularization:** Refactor `backend/internal/database/` into `connection.go`, `pool.go`, `health.go`, and `transactions.go`.
3. **Repository Layer:** Create `backend/internal/repository/` with dedicated domain repositories.
4. **Schema Enhancements (Migration `000003`):** Add `last_login_at` to `users` and create `player_inventory` with positive quantity check constraint and unique index.
5. **Migration CLI:** Build a standalone, robust migration runner supporting `status`, `up`, `down`, and `version`.
6. **Query & Index Optimization:** Document query plans and add missing indexes.
7. **Redis Evaluation:** Document architectural decision in `docs/database/PHASE_43_REDIS_EVALUATION.md`.
8. **Backup & Disaster Recovery:** Author `docs/database/PHASE_43_BACKUP_RECOVERY.md`.
9. **Testing:** Unit, integration, concurrency, failure, and regression testing.

