# Wildbound — Phase 43: Database Schema Specification

**Database Name:** `wildbound_db`  
**Current Schema Version:** `000003`

---

## 1. Relational Schema Summary

| Table | Primary Key | Foreign Keys | Key Constraints | Description |
|---|---|---|---|---|
| `users` | `id` (UUID) | None | Unique `username`, Unique `email` | Contender authentication, account status, and timestamps |
| `player_profiles` | `id` (UUID) | `user_id` -> `users(id)` | Unique `user_id` | Public display name, biography, active avatar |
| `refresh_tokens` | `id` (UUID) | `user_id` -> `users(id)` | Unique `token_hash` | SHA-256 hashed refresh tokens for session rotation |
| `player_inventory` | `id` (UUID) | `user_id` -> `users(id)` | Unique `(user_id, item_definition_id)`, `quantity > 0` | Contender persistent out-of-match items |
| `cosmetic_definitions` | `id` (VARCHAR) | None | Unique `cosmetic_key` | Catalog of avatar, armor, hair, skin cosmetics |
| `player_cosmetics` | `id` (UUID) | `user_id` -> `users(id)`, `cosmetic_id` -> `cosmetic_definitions(id)` | Unique `(user_id, cosmetic_id)` | Contender-owned cosmetic unlocks and equipped flags |
| `match_records` | `id` (UUID) | `winner_user_id` -> `users(id)` | Unique `match_id` | Durable history of completed battle royale matches |
| `match_participants` | `id` (UUID) | `match_record_id` -> `match_records(id)`, `user_id` -> `users(id)` | None | Contender per-match statistics and placements |
| `player_statistics` | `id` (UUID) | `user_id` -> `users(id)` | Unique `user_id` | Aggregated career totals (kills, wins, damage) |
| `schema_migrations` | `version` (INT) | None | Primary `version` | Migration version tracking and execution history |

---

## 2. Table Definitions

### 2.1 `users`
```sql
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    username VARCHAR(32) NOT NULL,
    email VARCHAR(255) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    account_status VARCHAR(32) NOT NULL DEFAULT 'active',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_login_at TIMESTAMPTZ,
    CONSTRAINT uq_users_username UNIQUE (username),
    CONSTRAINT uq_users_email UNIQUE (email)
);
```

### 2.2 `player_inventory` (New in Phase 43)
```sql
CREATE TABLE player_inventory (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    item_definition_id VARCHAR(64) NOT NULL,
    quantity INT NOT NULL DEFAULT 1 CHECK (quantity > 0),
    acquired_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT uq_player_inventory_user_item UNIQUE (user_id, item_definition_id)
);
```

### 2.3 `cosmetic_definitions`
```sql
CREATE TABLE cosmetic_definitions (
    id VARCHAR(64) PRIMARY KEY,
    cosmetic_key VARCHAR(64) NOT NULL,
    display_name VARCHAR(128) NOT NULL,
    category VARCHAR(32) NOT NULL,
    rarity VARCHAR(32) NOT NULL DEFAULT 'common',
    description TEXT NOT NULL DEFAULT '',
    is_available BOOLEAN NOT NULL DEFAULT true,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_cosmetic_key UNIQUE (cosmetic_key)
);
```

### 2.4 `player_cosmetics`
```sql
CREATE TABLE player_cosmetics (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cosmetic_id VARCHAR(64) NOT NULL REFERENCES cosmetic_definitions(id) ON DELETE CASCADE,
    acquired_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    acquisition_source VARCHAR(64) NOT NULL DEFAULT 'starter',
    is_equipped BOOLEAN NOT NULL DEFAULT false,
    CONSTRAINT uq_player_cosmetics_user_item UNIQUE (user_id, cosmetic_id)
);
```

### 2.5 `match_records`
```sql
CREATE TABLE match_records (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    match_id VARCHAR(128) NOT NULL,
    game_mode VARCHAR(32) NOT NULL,
    map_id VARCHAR(128) NOT NULL,
    started_at TIMESTAMPTZ NOT NULL,
    ended_at TIMESTAMPTZ NOT NULL,
    duration_seconds INTEGER NOT NULL,
    participant_count INTEGER NOT NULL,
    winner_user_id UUID REFERENCES users(id) ON DELETE SET NULL,
    winner_display_name VARCHAR(64) NOT NULL DEFAULT 'No Winner',
    result_status VARCHAR(32) NOT NULL DEFAULT 'completed',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_match_records_match_id UNIQUE (match_id)
);
```

### 2.6 `match_participants`
```sql
CREATE TABLE match_participants (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    match_record_id UUID NOT NULL REFERENCES match_records(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE SET NULL,
    participant_display_name VARCHAR(64) NOT NULL,
    team_id INTEGER NOT NULL DEFAULT 0,
    placement INTEGER NOT NULL,
    eliminations INTEGER NOT NULL DEFAULT 0,
    deaths INTEGER NOT NULL DEFAULT 0,
    assists INTEGER NOT NULL DEFAULT 0,
    damage_dealt REAL NOT NULL DEFAULT 0.0,
    damage_received REAL NOT NULL DEFAULT 0.0,
    survival_seconds REAL NOT NULL DEFAULT 0.0,
    result_status VARCHAR(32) NOT NULL DEFAULT 'eliminated'
);
```

### 2.7 `player_statistics`
```sql
CREATE TABLE player_statistics (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    matches_played INTEGER NOT NULL DEFAULT 0,
    matches_won INTEGER NOT NULL DEFAULT 0,
    eliminations INTEGER NOT NULL DEFAULT 0,
    deaths INTEGER NOT NULL DEFAULT 0,
    assists INTEGER NOT NULL DEFAULT 0,
    total_damage REAL NOT NULL DEFAULT 0.0,
    total_damage_received REAL NOT NULL DEFAULT 0.0,
    total_survival_seconds REAL NOT NULL DEFAULT 0.0,
    highest_placement INTEGER NOT NULL DEFAULT 0,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_statistics_user_id UNIQUE (user_id)
);
```

