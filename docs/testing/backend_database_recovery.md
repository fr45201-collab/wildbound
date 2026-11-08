# Wildbound — Go Backend & PostgreSQL Database Recovery

## 1. Objective

Wildbound relies on a persistent Go REST API and PostgreSQL database (`wildbound_db`) for accounts, authentication, persistent cosmetics, player progression, and durable match history.

This document evaluates the recovery mechanisms across the backend and database stack when experiencing process restart, database disconnection, connection pool exhaustion, or service failover.

---

## 2. Multi-Tier Persistence Architecture

```
+-------------------------------------------------------------------------------+
|                            Dedicated Game Server                              |
|  - Generates Match Results & Player Statistics                                |
|  - MatchResultsOutbox (Durable disk-backed SQLite/JSON queue)                 |
+-------------------------------------------------------------------------------+
                                       |
                           HTTP / REST POST /api/matches
                                       v
+-------------------------------------------------------------------------------+
|                              Go Backend Service                               |
|  - JWT Stateless Authentication                                               |
|  - SQL Database Transaction Boundary                                          |
|  - Connection Pool (pgx / database/sql with MaxOpenConns & MaxIdleConns)      |
+-------------------------------------------------------------------------------+
                                       |
                              PostgreSQL Wire Protocol
                                       v
+-------------------------------------------------------------------------------+
|                           PostgreSQL (wildbound_db)                           |
|  - Write-Ahead Logging (WAL) for Crash Consistency                            |
|  - Foreign Key Constraints & ACID Transactions                               |
|  - Migration Tracking (schema_migrations)                                     |
+-------------------------------------------------------------------------------+
```

---

## 3. Failure & Recovery Modes

### 3.1 Database Network Outage / PostgreSQL Restart
- **Go Backend Health Check**: The Go backend utilizes connection pool ping verification (`db.PingContext()`). If PostgreSQL restarts, the connection pool automatically re-establishes TCP sockets without requiring a backend service restart.
- **Dedicated Server Durable Outbox**: If a match completes while the Go API or PostgreSQL is unreachable, `MatchResultsOutbox` holds match results in durable local disk storage (`data/outbox/`). The server retries submission with exponential backoff until HTTP 200/201 is confirmed. Zero match records are lost.

### 3.2 Backend Service Crash & Stateless Sessions
- **Stateless JWT Tokens**: Player sessions use cryptographic JWT tokens signed with a secret key. If the Go backend restarts, authenticated players with valid unexpired tokens continue making API requests seamlessly without having to re-login.
- **No In-Memory Session Dependency**: All player inventory, loadouts, and statistics are stored in PostgreSQL tables (`player_profiles`, `player_cosmetics`, `matches`), ensuring instant recovery upon service restart.

### 3.3 Database Crash Consistency & WAL
- PostgreSQL's Write-Ahead Logging (WAL) guarantees that all committed transactions (account registration, cosmetic purchases, progression XP) are fully recovered upon database server reboot.
- Database migrations (`001_initial_schema.sql` through `003_cosmetics_and_stats.sql`) execute idempotently using transactional DDL.

