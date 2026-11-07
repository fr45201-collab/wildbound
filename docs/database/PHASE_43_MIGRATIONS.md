# Wildbound — Phase 43: Database Migrations System

**Tool:** `wildbound-migrate` (`backend/cmd/migrate/main.go`)  
**Package:** `backend/internal/database/migrations.go`  
**Tracking Table:** `schema_migrations`

---

## 1. Migration System Architecture

Wildbound uses a self-contained, lightweight, dependency-free database migration runner embedded directly into the Go backend and exposed as a standalone CLI tool.

### Key Architectural Properties:
1. **Advisory Locking**: Prevents race conditions during simultaneous deployment of multiple backend instances using `pg_try_advisory_lock(987654321)`.
2. **Transactional Application**: Every migration script (`.up.sql` or `.down.sql`) runs inside a dedicated ACID transaction. If any statement fails, the entire migration rolls back cleanly.
3. **Tracking Table**: Maintains a record of applied migrations with version number, script name, and exact timestamp.
4. **Idempotence**: Table creations and index declarations leverage `IF NOT EXISTS` and `ON CONFLICT` clauses where applicable.

---

## 2. CLI Usage Reference

The CLI binary is compiled at `backend/bin/wildbound-migrate.exe`.

### 2.1 Viewing Migration Status
```powershell
./bin/wildbound-migrate.exe status
```
Output:
```
VERSION   NAME                                  STATUS    APPLIED AT
-------   ----                                  ------    ----------
000001    000001_initial_schema                 APPLIED   2026-11-08 00:05:37 UTC
000002    000002_seed_cosmetics                 APPLIED   2026-11-08 00:05:37 UTC
000003    000003_add_last_login_and_inventory   APPLIED   2026-11-08 00:05:37 UTC
```

### 2.2 Applying Pending Migrations
```powershell
./bin/wildbound-migrate.exe up
```

### 2.3 Rolling Back Last Migration
```powershell
./bin/wildbound-migrate.exe down
```

### 2.4 Checking Current Version
```powershell
./bin/wildbound-migrate.exe version
```
Output:
```
Current schema version: 000003
```

---

## 3. Migration Inventory

| Version | Name | Target Changes |
|---|---|---|
| `000001` | `000001_initial_schema` | Core schema: `users`, `player_profiles`, `refresh_tokens`, `cosmetic_definitions`, `player_cosmetics`, `match_records`, `match_participants`, `player_statistics`. |
| `000002` | `000002_seed_cosmetics` | Seeds default asset catalog matching Wildbound client (avatar, tunics, trousers, boots, hairstyles, skin complexions). |
| `000003` | `000003_add_last_login_and_inventory` | Adds `last_login_at` tracking to `users` and creates persistent `player_inventory` table with positive check constraint. |

