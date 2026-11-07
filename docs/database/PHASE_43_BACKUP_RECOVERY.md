# Wildbound — Phase 43: Backup and Recovery Strategy

**Project:** Wildbound  
**Phase:** 43 — Database Infrastructure  
**Database Name:** `wildbound_db`  
**Administration Tool:** pgAdmin 4 / pg_dump / psql  
**Engine:** PostgreSQL 16+

---

## 1. Overview & RPO / RTO Objectives

Wildbound's persistent data (user accounts, player credentials, inventory ownership, cosmetics, match history, and contender progression) requires a robust, automated backup strategy.

| Metric | Target | Description |
|---|---|---|
| **Recovery Point Objective (RPO)** | 1 hour | Maximum allowable data loss in the event of hardware or storage failure. |
| **Recovery Time Objective (RTO)** | 15 minutes | Maximum allowable downtime required to restore the database to operational status. |

---

## 2. Backup Methods

Wildbound utilizes two primary backup types:
1. **Logical Backups (`pg_dump`)**: Nightly and pre-deployment full database dumps stored in custom compressed format (`.dump`) and plain SQL format.
2. **Physical / WAL Archiving**: Continuous Write-Ahead Logging (WAL) for Point-In-Time Recovery (PITR) in production environments.

### 2.1 Logical Backup Command (`pg_dump`)

#### Custom Compressed Format (Recommended for Production & Staging):
```powershell
# Windows PowerShell
$TIMESTAMP = (Get-Date).ToString("yyyyMMdd_HHmmss")
$BACKUP_FILE = "backups/wildbound_db_${TIMESTAMP}.dump"

& "C:\Program Files\PostgreSQL\16\bin\pg_dump.exe" `
    -h localhost -p 5432 -U wildbound_user `
    -F c -b -v -f $BACKUP_FILE wildbound_db
```

#### Plain SQL Text Format (Readable for Schema Verification):
```powershell
& "C:\Program Files\PostgreSQL\16\bin\pg_dump.exe" `
    -h localhost -p 5432 -U wildbound_user `
    --schema-only -f "backups/wildbound_schema_${TIMESTAMP}.sql" wildbound_db
```

---

## 3. Restoration Procedures

### 3.1 Restoring from Custom Format (`pg_restore`)

```powershell
# 1. Stop backend services to terminate active pool connections
# 2. Re-create target database if restoring to a clean instance
& "C:\Program Files\PostgreSQL\16\bin\dropdb.exe" -h localhost -p 5432 -U postgres --if-exists wildbound_db
& "C:\Program Files\PostgreSQL\16\bin\createdb.exe" -h localhost -p 5432 -U postgres -O wildbound_user wildbound_db

# 3. Restore dump
& "C:\Program Files\PostgreSQL\16\bin\pg_restore.exe" `
    -h localhost -p 5432 -U wildbound_user `
    -d wildbound_db -v -c "backups/wildbound_db_20261108_000000.dump"
```

### 3.2 Restoring from Plain SQL Script (`psql`)

```powershell
& "C:\Program Files\PostgreSQL\16\bin\psql.exe" `
    -h localhost -p 5432 -U wildbound_user `
    -d wildbound_db -f "backups/wildbound_backup.sql"
```

---

## 4. Disaster Recovery Playbook

1. **Detection**: Alerts triggered by backend `/ready` endpoint failing or health checks logging connection refused.
2. **Isolation**: Stop `wildbound-backend.exe` to prevent partial writes.
3. **Cluster Health Check**: Check disk space, PostgreSQL service status, and corrupt data directories.
4. **Restore Database**: Execute restoration script targeting latest verified backup.
5. **Verify Migrations**: Run `./bin/wildbound-migrate.exe status` to verify schema version matches expected code release.
6. **Integrity Validation**: Execute sample read queries against `users`, `player_inventory`, and `match_records`.
7. **Service Startup**: Relaunch `wildbound-backend.exe` and confirm `/health` reports `healthy`.

---

## 5. Retention & Rotation Policy

| Schedule | Frequency | Retention Period | Storage Location |
|---|---|---|---|
| **Hourly Dumps** | Every 1 hour | 48 hours | Local high-speed NVMe scratch storage |
| **Daily Dumps** | Daily at 03:00 UTC | 30 days | Encrypted secondary object storage (S3/GCS) |
| **Weekly Dumps** | Every Sunday | 12 weeks | Off-site immutable archive |
| **Monthly Dumps** | 1st of each month | 12 months | Long-term glacier storage |

