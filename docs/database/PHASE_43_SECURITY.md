# Wildbound — Phase 43: Database Security & Boundaries

---

## 1. Principle of Least Privilege

1. **Dedicated Application Role (`wildbound_user`)**:
   - The Go backend connects using a restricted user account (`wildbound_user`).
   - `wildbound_user` is granted `CONNECT`, `SELECT`, `INSERT`, `UPDATE`, `DELETE` on application tables.
   - `wildbound_user` is strictly **NOT** a PostgreSQL superuser and cannot create/drop databases or alter server roles.
2. **Database Owner Management**:
   - The project owner creates and configures `wildbound_db` in pgAdmin 4.
   - Schema modifications are managed via tracked migration scripts rather than dynamic DDL queries.

---

## 2. Secrets & Environment Isolation

1. **Zero Committed Credentials**:
   - Database passwords, JWT secret keys, and internal server tokens are stored exclusively in `.env`.
   - Both root `.gitignore` and `backend/.gitignore` strictly ignore `.env` and `*.env`.
   - `.env.example` provides sanitized template values with explicit placeholders.
2. **Strict Sanitization**:
   - Connection strings and credentials are redacted from log outputs.
   - Database error details in HTTP responses are sanitized to avoid leaking database schema names or query fragments to clients.

---

## 3. SQL Injection Defense

All queries in `backend/internal/repository/` and `backend/internal/database/` use **parameterized queries** via `jackc/pgx/v5`:
```go
// Safe: Query parameters are transmitted out-of-band via binary protocol
r.pool.QueryRow(ctx, "SELECT id FROM users WHERE username = $1", username)
```
String concatenation or formatting (`fmt.Sprintf`) into SQL statements is prohibited across all repository methods.
Verified by automated test `TestSecurityAndValidationBoundaries` rejecting `' OR '1'='1` injection payloads.

---

## 4. Constraint-Level Security Enforcements

1. **Positive Quantity Enforcement**:
   - `CHECK (quantity > 0)` constraint on `player_inventory` ensures that negative or zero balances can never exist in the database even if application bugs occur.
2. **Foreign Key Cascades**:
   - `ON DELETE CASCADE` on `player_profiles`, `player_inventory`, `player_cosmetics`, and `refresh_tokens` ensures clean GDPR compliance and zero orphaned player records on user deletion.
3. **Winner Foreign Key Softening**:
   - `ON DELETE SET NULL` on `match_records.winner_user_id` preserves immutable match history even if a winner's account is subsequently deleted.

