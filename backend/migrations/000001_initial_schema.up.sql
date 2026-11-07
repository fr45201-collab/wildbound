-- 000001_initial_schema.up.sql
-- Wildbound Phase 42 Core Schema

-- 1. Users table
CREATE TABLE IF NOT EXISTS users (
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

CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);

-- 2. Player Profiles table
CREATE TABLE IF NOT EXISTS player_profiles (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    display_name VARCHAR(64) NOT NULL,
    biography TEXT NOT NULL DEFAULT '',
    avatar_cosmetic_id VARCHAR(64) NOT NULL DEFAULT 'cosmetic_avatar_default',
    preferred_game_mode VARCHAR(32) NOT NULL DEFAULT 'solo',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_profiles_user_id UNIQUE (user_id)
);

CREATE INDEX IF NOT EXISTS idx_player_profiles_user_id ON player_profiles(user_id);

-- 3. Refresh Tokens table
CREATE TABLE IF NOT EXISTS refresh_tokens (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token_hash VARCHAR(64) NOT NULL,
    expires_at TIMESTAMPTZ NOT NULL,
    revoked_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_refresh_tokens_hash UNIQUE (token_hash)
);

CREATE INDEX IF NOT EXISTS idx_refresh_tokens_user_id ON refresh_tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_refresh_tokens_hash ON refresh_tokens(token_hash);

-- 4. Cosmetic Definitions table
CREATE TABLE IF NOT EXISTS cosmetic_definitions (
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

CREATE INDEX IF NOT EXISTS idx_cosmetics_category ON cosmetic_definitions(category);

-- 5. Player Cosmetics table
CREATE TABLE IF NOT EXISTS player_cosmetics (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cosmetic_id VARCHAR(64) NOT NULL REFERENCES cosmetic_definitions(id) ON DELETE CASCADE,
    acquired_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    acquisition_source VARCHAR(64) NOT NULL DEFAULT 'starter',
    is_equipped BOOLEAN NOT NULL DEFAULT false,
    CONSTRAINT uq_player_cosmetics_user_item UNIQUE (user_id, cosmetic_id)
);

CREATE INDEX IF NOT EXISTS idx_player_cosmetics_user ON player_cosmetics(user_id);
CREATE INDEX IF NOT EXISTS idx_player_cosmetics_equipped ON player_cosmetics(user_id, is_equipped);

-- 6. Match Records table
CREATE TABLE IF NOT EXISTS match_records (
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

CREATE INDEX IF NOT EXISTS idx_match_records_match_id ON match_records(match_id);
CREATE INDEX IF NOT EXISTS idx_match_records_started_at ON match_records(started_at DESC);

-- 7. Match Participants table
CREATE TABLE IF NOT EXISTS match_participants (
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

CREATE INDEX IF NOT EXISTS idx_match_participants_match_id ON match_participants(match_record_id);
CREATE INDEX IF NOT EXISTS idx_match_participants_user_id ON match_participants(user_id);

-- 8. Player Statistics table
CREATE TABLE IF NOT EXISTS player_statistics (
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

CREATE INDEX IF NOT EXISTS idx_player_statistics_user_id ON player_statistics(user_id);

