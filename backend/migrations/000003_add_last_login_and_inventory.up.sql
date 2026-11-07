-- ============================================================================
-- Migration: 000003_add_last_login_and_inventory.up.sql
-- Description: Adds last_login_at tracking to users and introduces persistent
--              player inventory with positive quantity checks.
-- ============================================================================

-- 1. Add last_login_at to users table
ALTER TABLE users ADD COLUMN IF NOT EXISTS last_login_at TIMESTAMPTZ;

-- 2. Create persistent player inventory table
CREATE TABLE IF NOT EXISTS player_inventory (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    item_definition_id VARCHAR(64) NOT NULL,
    quantity INT NOT NULL DEFAULT 1 CHECK (quantity > 0),
    acquired_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT uq_player_inventory_user_item UNIQUE (user_id, item_definition_id)
);

-- 3. Indexes for fast inventory retrieval
CREATE INDEX IF NOT EXISTS idx_player_inventory_user_id ON player_inventory(user_id);
CREATE INDEX IF NOT EXISTS idx_player_inventory_item_def ON player_inventory(item_definition_id);

