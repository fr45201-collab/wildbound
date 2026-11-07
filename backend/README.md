# Wildbound Backend Services

High-performance Go backend service providing REST API, player authentication, persistent profiles, match records, cosmetic ownership, statistics, and persistent inventory for Wildbound.

---

## 1. Quick Start

### 1.1 Prerequisites
- **Go 1.22+**
- **PostgreSQL 16+** (Create database `wildbound_db` manually via pgAdmin 4)

### 1.2 Configuration
Copy the template configuration file:
```powershell
cp .env.example .env
```
Edit `.env` to configure your PostgreSQL credentials:
```env
APP_ENV=development
DATABASE_URL=postgres://wildbound_user:your_password@localhost:5432/wildbound_db?sslmode=disable
DATABASE_MAX_CONNECTIONS=20
DATABASE_MIN_CONNECTIONS=2
DATABASE_MAX_CONNECTION_LIFETIME=30m
DATABASE_MAX_CONNECTION_IDLE_TIME=5m
LOG_LEVEL=debug
```

### 1.3 Running Migrations
Build and run the migration CLI:
```powershell
# Build the migration tool
go build -o bin/wildbound-migrate.exe ./cmd/migrate

# View migration status
./bin/wildbound-migrate.exe status

# Apply pending migrations
./bin/wildbound-migrate.exe up
```

### 1.4 Starting the Server
```powershell
# Build the server binary
go build -o bin/wildbound-backend.exe ./cmd/api

# Run the backend
./bin/wildbound-backend.exe
```

The server will listen at `http://127.0.0.1:8080`.

---

## 2. Running Tests

```powershell
go test -v ./tests
```

All 11 integration test suites run against the configured test database.

---

## 3. Directory Layout

```
backend/
├── cmd/
│   ├── api/              # API server entrypoint (main.go)
│   └── migrate/          # Standalone migration CLI (main.go)
├── configs/              # Environment config JSONs (development.json, etc.)
├── internal/
│   ├── auth/             # Contender authentication, JWT & bcrypt
│   ├── config/           # Config loader with .env support
│   ├── cosmetics/        # Cosmetic definitions & armory unlocks
│   ├── database/         # Connection pooling, health probes, tx helpers & migrations
│   ├── inventory/        # Persistent out-of-match player inventory
│   ├── matches/          # Match records & participant ingest
│   ├── middleware/       # JWT auth, CORS, rate limiting, logging
│   ├── observability/    # Structured logger (slog)
│   ├── profiles/         # Contender public & private profiles
│   ├── repository/       # Database access layer (clean SQL abstractions)
│   ├── response/         # Standardized JSON response envelope
│   ├── server/           # Chi router & HTTP server wiring
│   ├── statistics/       # Contender career progression & stats
│   └── validation/       # Contender username, email, password validators
├── migrations/           # SQL migration files (.up.sql / .down.sql)
├── tests/                # Automated integration test suite
├── .env.example          # Environment variable template
└── go.mod
```

---

## 4. Documentation

Full architectural and operational guides are available in `docs/database/`:
- `PHASE_43_ARCHITECTURE.md`: Database and backend architectural design.
- `PHASE_43_SCHEMA.md`: Relational table definitions and constraints.
- `PHASE_43_ER_DIAGRAM.md`: Entity-relationship Mermaid diagram.
- `PHASE_43_MIGRATIONS.md`: Migration guide and CLI reference.
- `PHASE_43_CONNECTION_POOLING.md`: Pool sizing, timeouts, and health monitoring.
- `PHASE_43_TRANSACTIONS.md`: Atomic transactions, isolation levels, and rollback semantics.
- `PHASE_43_INDEXING_PERFORMANCE.md`: Query indexes and execution plan benchmarks.
- `PHASE_43_SECURITY.md`: SQL injection defense and least privilege enforcement.
- `PHASE_43_REDIS_EVALUATION.md`: In-depth analysis of why Redis is not needed.
- `PHASE_43_BACKUP_RECOVERY.md`: Backup scripts, restore procedures, and retention policies.
- `PHASE_43_TESTING.md`: Test suite documentation and execution reports.
- `PHASE_43_IMPLEMENTATION_REPORT.md`: Comprehensive completion report for Phase 43.

