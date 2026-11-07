# Wildbound — Phase 43: Transaction Management & Isolation

**Package:** `backend/internal/database/transactions.go`  
**Isolation Level:** Read Committed (Default), Serializable (Optional for high-conflict financial/trading operations)

---

## 1. Design & Error Handling Strategy

All multi-statement database modifications in Wildbound must be executed atomically. The `database.WithTransaction` helper enforces standard transaction lifecycle semantics:

```go
func WithTransaction(ctx context.Context, pool *pgxpool.Pool, fn func(tx pgx.Tx) error) error
```

### Safety Guarantees:
1. **Automatic Rollback on Error**: If `fn` returns an error, `tx.Rollback(ctx)` is immediately called and the error is returned to the caller.
2. **Automatic Rollback on Panic**: Uses `defer` with panic detection. If a runtime panic occurs inside `fn`, the transaction is rolled back before the panic unwinds the call stack.
3. **Clean Commit**: If `fn` returns `nil`, `tx.Commit(ctx)` is executed. If the commit fails (e.g. serialization conflict or constraint violation), the error is returned.

---

## 2. Transactional Workflows in Wildbound

### 2.1 Contender Registration
In `auth.Service.Register`:
1. Check uniqueness of `username` and `email`.
2. Insert new user into `users`.
3. Insert default profile into `player_profiles`.
4. Initialize career stats in `player_statistics`.
5. Grant and equip starter cosmetics in `player_cosmetics`.
6. Insert session refresh token in `refresh_tokens`.
7. **Commit**: If any insertion fails, zero orphaned records remain.

### 2.2 Inventory Item Consumption
In `repository.inventoryRepo.ConsumeItem`:
1. Acquire exclusive lock on target row via `SELECT ... FOR UPDATE`.
2. Verify available quantity >= requested quantity.
3. If quantity depleted to 0, execute `DELETE`. Otherwise execute `UPDATE quantity = quantity - $1`.
4. **Commit**: Guarantees protection against double-spend races.

### 2.3 Contender Inventory Transfer
In `repository.inventoryRepo.TransferItem`:
1. Lock sender row via `SELECT ... FOR UPDATE`.
2. Verify sender quantity.
3. Decrement or remove sender item.
4. Atomically credit recipient item using upsert (`ON CONFLICT (user_id, item_definition_id) DO UPDATE`).
5. **Commit**: Total resource count remains strictly conserved across players.

### 2.4 Match Results Ingestion
In `matches.Service.SubmitResult`:
1. Check idempotent uniqueness of `match_id`.
2. Insert `match_records` row.
3. Bulk insert `match_participants`.
4. For each human participant, update career stats in `player_statistics`.
5. **Commit**: Ensures that incomplete matches are never partially recorded.

