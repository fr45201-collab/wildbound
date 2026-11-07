-- 000005_cosmetics_loadouts.down.sql
DROP TABLE IF EXISTS player_loadout_items;
DROP TABLE IF EXISTS player_loadouts;
ALTER TABLE cosmetic_definitions DROP COLUMN IF EXISTS equipment_slot;
ALTER TABLE cosmetic_definitions DROP COLUMN IF EXISTS asset_id;
