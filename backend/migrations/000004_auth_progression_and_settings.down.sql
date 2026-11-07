-- ============================================================================
-- Migration: 000004_auth_progression_and_settings.down.sql
-- Description: Reverses Phase 44 schema additions cleanly.
-- ============================================================================

DROP TABLE IF EXISTS player_achievements CASCADE;
DROP TABLE IF EXISTS achievement_definitions CASCADE;
DROP TABLE IF EXISTS progression_events CASCADE;
DROP TABLE IF EXISTS player_progression CASCADE;
DROP TABLE IF EXISTS player_settings CASCADE;
DROP TABLE IF EXISTS user_sessions CASCADE;

