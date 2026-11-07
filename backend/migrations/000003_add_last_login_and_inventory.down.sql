-- ============================================================================
-- Migration: 000003_add_last_login_and_inventory.down.sql
-- Description: Reverses persistent player inventory and last_login_at.
-- ============================================================================

DROP TABLE IF EXISTS player_inventory CASCADE;

ALTER TABLE users DROP COLUMN IF EXISTS last_login_at;

