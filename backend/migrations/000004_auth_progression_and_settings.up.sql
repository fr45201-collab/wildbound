-- ============================================================================
-- Migration: 000004_auth_progression_and_settings.up.sql
-- Description: Adds user_sessions, player_settings, player_progression,
--              progression_events, achievement_definitions, and player_achievements.
-- ============================================================================

-- 1. User Sessions table
CREATE TABLE IF NOT EXISTS user_sessions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    refresh_token_hash VARCHAR(64) NOT NULL,
    device_identifier VARCHAR(128) NOT NULL DEFAULT 'unknown',
    device_name VARCHAR(128) NOT NULL DEFAULT 'PC Client',
    ip_address VARCHAR(45) NOT NULL DEFAULT '127.0.0.1',
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_used_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMPTZ NOT NULL,
    revoked_at TIMESTAMPTZ,
    revocation_reason VARCHAR(128),
    CONSTRAINT uq_user_sessions_hash UNIQUE (refresh_token_hash)
);

CREATE INDEX IF NOT EXISTS idx_user_sessions_user_id ON user_sessions(user_id);
CREATE INDEX IF NOT EXISTS idx_user_sessions_hash ON user_sessions(refresh_token_hash);
CREATE INDEX IF NOT EXISTS idx_user_sessions_active ON user_sessions(user_id, revoked_at);

-- 2. Player Settings table
CREATE TABLE IF NOT EXISTS player_settings (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    language VARCHAR(16) NOT NULL DEFAULT 'en',
    region VARCHAR(32) NOT NULL DEFAULT 'us-east',
    preferred_game_mode VARCHAR(32) NOT NULL DEFAULT 'solo',
    privacy_profile_visibility VARCHAR(32) NOT NULL DEFAULT 'public',
    privacy_match_history VARCHAR(32) NOT NULL DEFAULT 'public',
    allow_friend_requests BOOLEAN NOT NULL DEFAULT true,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_player_settings_user_id ON player_settings(user_id);

-- 3. Player Progression table
CREATE TABLE IF NOT EXISTS player_progression (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    account_level INT NOT NULL DEFAULT 1 CHECK (account_level >= 1),
    total_xp INT NOT NULL DEFAULT 0 CHECK (total_xp >= 0),
    available_reward_points INT NOT NULL DEFAULT 0 CHECK (available_reward_points >= 0),
    matches_completed INT NOT NULL DEFAULT 0 CHECK (matches_completed >= 0),
    progression_version INT NOT NULL DEFAULT 1,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_player_progression_level ON player_progression(account_level);

-- 4. Progression Events table (auditable history & duplicate reward prevention)
CREATE TABLE IF NOT EXISTS progression_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    source_type VARCHAR(32) NOT NULL,
    source_id VARCHAR(128) NOT NULL,
    xp_delta INT NOT NULL DEFAULT 0,
    reward_points_delta INT NOT NULL DEFAULT 0,
    reason VARCHAR(255) NOT NULL DEFAULT '',
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT uq_progression_event_source UNIQUE (user_id, source_type, source_id)
);

CREATE INDEX IF NOT EXISTS idx_progression_events_user_id ON progression_events(user_id);
CREATE INDEX IF NOT EXISTS idx_progression_events_created ON progression_events(user_id, created_at DESC);

-- 5. Achievement Definitions table
CREATE TABLE IF NOT EXISTS achievement_definitions (
    id VARCHAR(64) PRIMARY KEY,
    achievement_key VARCHAR(64) NOT NULL,
    display_name VARCHAR(128) NOT NULL,
    description TEXT NOT NULL DEFAULT '',
    category VARCHAR(32) NOT NULL DEFAULT 'general',
    target_value INT NOT NULL DEFAULT 1,
    reward_xp INT NOT NULL DEFAULT 0,
    reward_points INT NOT NULL DEFAULT 0,
    is_active BOOLEAN NOT NULL DEFAULT true,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT uq_achievement_key UNIQUE (achievement_key)
);

-- 6. Player Achievements table
CREATE TABLE IF NOT EXISTS player_achievements (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    achievement_id VARCHAR(64) NOT NULL REFERENCES achievement_definitions(id) ON DELETE CASCADE,
    progress INT NOT NULL DEFAULT 0,
    target INT NOT NULL DEFAULT 1,
    is_unlocked BOOLEAN NOT NULL DEFAULT false,
    unlocked_at TIMESTAMPTZ,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT uq_player_achievements_user_ach UNIQUE (user_id, achievement_id)
);

CREATE INDEX IF NOT EXISTS idx_player_achievements_user ON player_achievements(user_id);

-- 7. Seed foundational achievement definitions
INSERT INTO achievement_definitions (id, achievement_key, display_name, description, category, target_value, reward_xp, reward_points)
VALUES
    ('ach_first_blood', 'first_blood', 'First Blood', 'Eliminate your first contender in an arena match.', 'combat', 1, 150, 10),
    ('ach_arena_victor', 'arena_victor', 'Arena Victor', 'Claim victory as the last contender standing in a match.', 'mastery', 1, 500, 50),
    ('ach_seasoned_survivor', 'seasoned_survivor', 'Seasoned Survivor', 'Survive for a total of 1,000 seconds in the storm arena.', 'survival', 1000, 300, 25),
    ('ach_sharpshooter', 'sharpshooter', 'Sharpshooter', 'Deal 2,500 total damage to enemy contenders.', 'combat', 2500, 350, 30),
    ('ach_veteran_contender', 'veteran_contender', 'Veteran Contender', 'Successfully complete 10 Battle Royale matches.', 'progression', 10, 400, 40)
ON CONFLICT (id) DO NOTHING;

-- 8. Backfill existing users with default settings and progression
INSERT INTO player_settings (user_id)
SELECT id FROM users
ON CONFLICT (user_id) DO NOTHING;

INSERT INTO player_progression (user_id)
SELECT id FROM users
ON CONFLICT (user_id) DO NOTHING;

