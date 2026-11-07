# Wildbound — Phase 42: Database Migrations & Seeding Guide

## 1. Migration Architecture
Database schema changes are managed via numbered, forward and backward compatible SQL migration files located in `backend/migrations/`.

Each migration has:
- `XXXXXX_<description>.up.sql`: Applies the new schema changes.
- `XXXXXX_<description>.down.sql`: Safely rolls back the schema changes in reverse dependency order.

## 2. Inventory of Migrations

| Migration File | Description | Impacted Entities |
|---|---|---|
| `000001_initial_schema.up.sql` | Base schema creation | Creates `users`, `player_profiles`, `refresh_tokens`, `cosmetic_definitions`, `player_cosmetics`, `match_records`, `match_participants`, and `player_statistics`. Sets up unique constraints and indexes. |
| `000001_initial_schema.down.sql` | Safe rollback | Drops tables in reverse foreign key order (`DROP TABLE IF EXISTS ... CASCADE`). |
| `000002_seed_cosmetics.up.sql` | Cosmetic items seed | Seeds 12 cosmetic definitions matching the Wildbound catalog (Adventurer Crest, Explorer Fedora, Ranger Rucksack, etc.). |
| `000002_seed_cosmetics.down.sql` | Cosmetic seed rollback | Deletes seeded cosmetic definitions (`DELETE FROM cosmetic_definitions WHERE id IN (...)`). |

## 3. Applying Migrations

### Using `psql` (Windows / Linux):
```bash
# Apply initial schema
psql -h 127.0.0.1 -p 5433 -U wildbound_user -d wildbound -f backend/migrations/000001_initial_schema.up.sql

# Seed cosmetics catalog
psql -h 127.0.0.1 -p 5433 -U wildbound_user -d wildbound -f backend/migrations/000002_seed_cosmetics.up.sql
```

### Verification Query:
```sql
SELECT count(*) FROM cosmetic_definitions; -- Output: 12
SELECT table_name FROM information_schema.tables WHERE table_schema='public'; -- Output: 8 tables
```

