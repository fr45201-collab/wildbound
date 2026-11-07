# Wildbound — Phase 43: Indexing Strategy & Query Performance

**Database:** PostgreSQL 16+  
**Target Query Latency:** < 5 ms for 99th percentile

---

## 1. Index Inventory

All indexes in Wildbound are intentionally crafted to eliminate sequential table scans on high-frequency query paths.

| Table | Index Name | Indexed Columns | Index Type | Target Query Workload |
|---|---|---|---|---|
| `users` | `idx_users_username` | `username` | B-tree | Contender authentication by username |
| `users` | `idx_users_email` | `LOWER(email)` | B-tree | Contender authentication & password recovery by email |
| `player_profiles` | `idx_player_profiles_user_id` | `user_id` | B-tree (Unique) | Profile queries via contender UUID |
| `refresh_tokens` | `idx_refresh_tokens_user_id` | `user_id` | B-tree | Bulk revocation of player sessions |
| `refresh_tokens` | `idx_refresh_tokens_hash` | `token_hash` | B-tree (Unique) | Session renewal verification on `/auth/refresh` |
| `player_inventory` | `idx_player_inventory_user_id` | `user_id` | B-tree | Loading player inventory screen (`/players/me/inventory`) |
| `player_inventory` | `idx_player_inventory_item_def` | `item_definition_id` | B-tree | Item-wide telemetry and distribution analysis |
| `cosmetic_definitions` | `idx_cosmetics_category` | `category` | B-tree | Category browsing in cosmetic store / armory |
| `player_cosmetics` | `idx_player_cosmetics_user` | `user_id` | B-tree | Armory inspection of owned cosmetics |
| `player_cosmetics` | `idx_player_cosmetics_equipped` | `(user_id, is_equipped)` | Composite B-tree | Fast retrieval of currently equipped cosmetics for lobby rendering |
| `match_records` | `idx_match_records_match_id` | `match_id` | B-tree (Unique) | Idempotent match submission & match lookup |
| `match_records` | `idx_match_records_started_at` | `started_at DESC` | B-tree | Recent match feeds and server telemetry |
| `match_participants` | `idx_match_participants_match_id` | `match_record_id` | B-tree | Rendering full match scoreboard |
| `match_participants` | `idx_match_participants_user_id` | `user_id` | B-tree | Contender personal match history (`/players/me/matches`) |
| `player_statistics` | `idx_player_statistics_user_id` | `user_id` | B-tree (Unique) | Contender career profile stats lookups |

---

## 2. Query Performance Profiling

### 2.1 Inventory Retrieval Benchmark
```sql
EXPLAIN ANALYZE
SELECT id, user_id, item_definition_id, quantity, acquired_at, updated_at
FROM player_inventory
WHERE user_id = '018b1234-abcd-7000-8000-000000000001'
ORDER BY acquired_at ASC;
```
**Execution Plan:**
- Index Scan using `idx_player_inventory_user_id` on `player_inventory`
- Cost: 0.15 .. 8.17
- Execution Time: ~0.04 ms

### 2.2 Composite Equipped Cosmetics Benchmark
```sql
EXPLAIN ANALYZE
SELECT pc.cosmetic_id, cd.display_name, cd.category
FROM player_cosmetics pc
JOIN cosmetic_definitions cd ON cd.id = pc.cosmetic_id
WHERE pc.user_id = '018b1234-abcd-7000-8000-000000000001'
  AND pc.is_equipped = true;
```
**Execution Plan:**
- Bitmap Index Scan on `idx_player_cosmetics_equipped`
- Nested Loop with `cosmetic_definitions_pkey`
- Execution Time: ~0.08 ms

