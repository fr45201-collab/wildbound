# Wildbound — Phase 43: Connection Pooling & Management

**Package:** `backend/internal/database/pool.go`  
**Underlying Pool:** `jackc/pgx/v5/pgxpool`

---

## 1. Connection Pool Sizing Rationale

In high-concurrency multiplayer backend systems, uncontrolled connection spikes exhaust PostgreSQL server memory, CPU thread scheduling, and socket limits. Connection pooling bounds the number of simultaneous active sockets between the Go REST backend and PostgreSQL.

### Configuration Parameters

| Environment Variable | Config Struct Field | Default | Rationale |
|---|---|---|---|
| `DATABASE_MAX_CONNECTIONS` | `DatabaseConfig.MaxConns` | `20` | Caps maximum concurrent active database connections per API instance. Prevents pool-induced PostgreSQL OOM errors. |
| `DATABASE_MIN_CONNECTIONS` | `DatabaseConfig.MinConns` | `2` | Keeps a warm baseline of connections ready to eliminate connection handshake latency during quiet periods. |
| `DATABASE_MAX_CONNECTION_LIFETIME` | `DatabaseConfig.MaxConnLifetimeMinutes` | `30m` | Retires connections after 30 minutes, preventing stale socket memory accumulation or broken network state behind proxies. |
| `DATABASE_MAX_CONNECTION_IDLE_TIME` | `DatabaseConfig.MaxConnIdleMinutes` | `5m` | Reclaims inactive connections exceeding `MinConns` after 5 minutes of inactivity. |

---

## 2. Health Monitoring & Pool Telemetry

The connection pool provides real-time statistics queryable through `db.Stats()`:
- `TotalConns`: Current count of established connections in the pool.
- `AcquiredConns`: Number of connections currently executing queries or transactions.
- `IdleConns`: Number of established connections idle and available for immediate acquisition.
- `MaxConns`: Configured maximum pool capacity.

### Integration with Probes
The `/health` and `/ready` endpoints query `db.CheckHealth(ctx)`:
```json
{
  "success": true,
  "data": {
    "status": "healthy",
    "service": "wildbound-backend",
    "version": "1.0.0",
    "database": {
      "status": "healthy",
      "latency_ms": 0.517,
      "pool_stats": {
        "total_conns": 3,
        "acquired_conns": 0,
        "idle_conns": 3,
        "max_conns": 20,
        "constructing_conns": 0
      }
    }
  }
}
```
If PostgreSQL becomes unresponsive, `/ready` returns HTTP 503 `SERVICE_UNAVAILABLE` to orchestrators (e.g. Kubernetes/systemd), preventing bad traffic routing.

