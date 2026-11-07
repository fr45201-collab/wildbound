-- 000005_cosmetics_loadouts.up.sql
-- Phase 45: Cosmetic Ownership, Store Catalogue, and Loadouts

-- 1. Extend cosmetic_definitions with equipment_slot and asset_id
ALTER TABLE cosmetic_definitions
    ADD COLUMN IF NOT EXISTS equipment_slot VARCHAR(32) NOT NULL DEFAULT 'Chest',
    ADD COLUMN IF NOT EXISTS asset_id VARCHAR(128) NOT NULL DEFAULT '';

-- 2. Update existing Phase 42 seeded cosmetics with correct slots
UPDATE cosmetic_definitions SET equipment_slot = 'Chest', asset_id = 'sheet_tunic_adventurer' WHERE id = 'cosmetic_tunic_adventurer';
UPDATE cosmetic_definitions SET equipment_slot = 'Legs', asset_id = 'sheet_pants_leather' WHERE id = 'cosmetic_pants_leather';
UPDATE cosmetic_definitions SET equipment_slot = 'Feet', asset_id = 'sheet_boots_leather' WHERE id = 'cosmetic_boots_leather';
UPDATE cosmetic_definitions SET equipment_slot = 'Hair', asset_id = 'sheet_hair_short' WHERE id = 'cosmetic_hair_short';
UPDATE cosmetic_definitions SET equipment_slot = 'Hair', asset_id = 'sheet_hair_medium' WHERE id = 'cosmetic_hair_medium';
UPDATE cosmetic_definitions SET equipment_slot = 'Hair', asset_id = 'sheet_hair_long' WHERE id = 'cosmetic_hair_long';
UPDATE cosmetic_definitions SET equipment_slot = 'Skin', asset_id = 'sheet_skin_fair' WHERE id = 'cosmetic_skin_fair';
UPDATE cosmetic_definitions SET equipment_slot = 'Skin', asset_id = 'sheet_skin_medium' WHERE id = 'cosmetic_skin_medium';
UPDATE cosmetic_definitions SET equipment_slot = 'Skin', asset_id = 'sheet_skin_dark' WHERE id = 'cosmetic_skin_dark';
UPDATE cosmetic_definitions SET equipment_slot = 'Headwear', asset_id = 'sheet_hat_adventurer' WHERE id = 'cosmetic_hat_adventurer';
UPDATE cosmetic_definitions SET equipment_slot = 'Back', asset_id = 'sheet_backpack_traveler' WHERE id = 'cosmetic_backpack_traveler';

-- 3. Seed 50 production cosmetics from costume_catalog.json
INSERT INTO cosmetic_definitions (
    id, cosmetic_key, display_name, category, equipment_slot, rarity, description, asset_id, is_available
)
VALUES
    ('clo_basic_traveler_001', 'clo_basic_traveler_001', 'Basic Traveler Shirt', 'clothing', 'Chest', 'common', 'Linen traveler shirt with laced neck and rolled brown cuffs.', 'sheet_clo_basic_traveler_001', true),
    ('clo_forest_tunic_001', 'clo_forest_tunic_001', 'Forest Tunic', 'clothing', 'Chest', 'common', 'Deep woodland green tunic with reinforced hem and brass buckle.', 'sheet_clo_forest_tunic_001', true),
    ('clo_desert_robe_001', 'clo_desert_robe_001', 'Desert Robe', 'clothing', 'Chest', 'common', 'Flowing light linen desert robe offering sun and sand protection.', 'sheet_clo_desert_robe_001', true),
    ('clo_snow_jacket_001', 'clo_snow_jacket_001', 'Snow Jacket', 'clothing', 'Chest', 'common', 'Heavy insulated winter parka with plush white fur collar trim.', 'sheet_clo_snow_jacket_001', true),
    ('clo_leather_jacket_001', 'clo_leather_jacket_001', 'Leather Jacket', 'clothing', 'Chest', 'common', 'Durable boiled cowhide jacket with brass buttons and lapel.', 'sheet_clo_leather_jacket_001', true),
    ('clo_mystic_robe_001', 'clo_mystic_robe_001', 'Mystic Robe', 'clothing', 'Chest', 'epic', 'Arcane indigo vestment embroidered with gold runic script.', 'sheet_clo_mystic_robe_001', true),
    ('clo_royal_outfit_001', 'clo_royal_outfit_001', 'Royal Outfit', 'clothing', 'Chest', 'legendary', 'Velvet imperial tunic trimmed with ermine and gold thread.', 'sheet_clo_royal_outfit_001', true),
    ('clo_dark_adventurer_001', 'clo_dark_adventurer_001', 'Dark Adventurer Outfit', 'clothing', 'Chest', 'common', 'Muted stealth adventurer tunic designed for night scouting.', 'sheet_clo_dark_adventurer_001', true),
    ('arm_leather_armor_001', 'arm_leather_armor_001', 'Leather Armor', 'armor', 'Chest', 'common', 'Toughened boiled leather cuirass with riveted shoulder straps.', 'sheet_arm_leather_armor_001', true),
    ('arm_reinforced_leather_001', 'arm_reinforced_leather_001', 'Reinforced Leather Armor', 'armor', 'Chest', 'uncommon', 'Studded leather breastplate reinforced with forged steel plates.', 'sheet_arm_reinforced_leather_001', true),
    ('arm_iron_armor_001', 'arm_iron_armor_001', 'Iron Armor', 'armor', 'Chest', 'uncommon', 'Solid cast iron breastplate with flared neck guard and arming cops.', 'sheet_arm_iron_armor_001', true),
    ('arm_steel_armor_001', 'arm_steel_armor_001', 'Steel Armor', 'armor', 'Chest', 'rare', 'Folded high-polish steel cuirass with central deflecting ridge.', 'sheet_arm_steel_armor_001', true),
    ('arm_forest_guardian_001', 'arm_forest_guardian_001', 'Forest Guardian Armor', 'armor', 'Chest', 'epic', 'Living heartwood scale armor enchanted by ancient forest spirits.', 'sheet_arm_forest_guardian_001', true),
    ('arm_desert_armor_001', 'arm_desert_armor_001', 'Desert Armor', 'armor', 'Chest', 'common', 'Bronze-gilded lamellar cuirass tailored for searing dune combat.', 'sheet_arm_desert_armor_001', true),
    ('arm_frost_armor_001', 'arm_frost_armor_001', 'Frost Armor', 'armor', 'Chest', 'epic', 'Glacial steel cuirass radiating a crisp frost aura with fur rim.', 'sheet_arm_frost_armor_001', true),
    ('arm_ancient_rune_001', 'arm_ancient_rune_001', 'Ancient Rune Armor', 'armor', 'Chest', 'legendary', 'Celestial meteoric plate armor pulsating with luminous azure runes.', 'sheet_arm_ancient_rune_001', true),
    ('clo_p_traveler_pants_001', 'clo_p_traveler_pants_001', 'Traveler Pants', 'pants', 'Legs', 'common', 'Rugged canvas trousers built for long journeys across the wild.', 'sheet_clo_p_traveler_pants_001', true),
    ('clo_p_leather_pants_001', 'clo_p_leather_pants_001', 'Leather Pants', 'pants', 'Legs', 'common', 'Fitted dark leather riding breeches with reinforced knees.', 'sheet_clo_p_leather_pants_001', true),
    ('clo_p_armored_pants_001', 'clo_p_armored_pants_001', 'Armored Pants', 'pants', 'Legs', 'common', 'Padded chausses fitted with steel poleyns and knee guards.', 'sheet_clo_p_armored_pants_001', true),
    ('clo_p_desert_trousers_001', 'clo_p_desert_trousers_001', 'Desert Trousers', 'pants', 'Legs', 'common', 'Lightweight loose-cut pleated trousers ideal for arid deserts.', 'sheet_clo_p_desert_trousers_001', true),
    ('clo_p_snow_trousers_001', 'clo_p_snow_trousers_001', 'Snow Trousers', 'pants', 'Legs', 'common', 'Quilted winter trousers stuffed with down insulation.', 'sheet_clo_p_snow_trousers_001', true),
    ('clo_p_mystic_pants_001', 'clo_p_mystic_pants_001', 'Mystic Pants', 'pants', 'Legs', 'epic', 'Midnight violet scholar''s breeches hemmed with gold silk.', 'sheet_clo_p_mystic_pants_001', true),
    ('sho_leather_boots_001', 'sho_leather_boots_001', 'Leather Boots', 'footwear', 'Feet', 'common', 'Mid-calf brown leather explorer boots with folded cuffs.', 'sheet_sho_leather_boots_001', true),
    ('sho_iron_boots_001', 'sho_iron_boots_001', 'Iron Boots', 'footwear', 'Feet', 'uncommon', 'Heavy forged iron sabatons offering substantial foot protection.', 'sheet_sho_iron_boots_001', true),
    ('sho_traveler_shoes_001', 'sho_traveler_shoes_001', 'Traveler Shoes', 'footwear', 'Feet', 'common', 'Supple low-profile walking shoes tailored for swift travel.', 'sheet_sho_traveler_shoes_001', true),
    ('sho_snow_boots_001', 'sho_snow_boots_001', 'Snow Boots', 'footwear', 'Feet', 'common', 'Thick snow boots lined with fleece for arctic conditions.', 'sheet_sho_snow_boots_001', true),
    ('sho_desert_sandals_001', 'sho_desert_sandals_001', 'Desert Sandals', 'footwear', 'Feet', 'common', 'Sturdy leather desert sandals with cross-straps and toe loops.', 'sheet_sho_desert_sandals_001', true),
    ('hat_adventurer_hat_001', 'hat_adventurer_hat_001', 'Adventurer Hat', 'headwear', 'Headwear', 'common', 'Wide-brimmed felt hat with a dark band and traveling feather.', 'sheet_hat_adventurer_hat_001', true),
    ('hat_explorer_hood_001', 'hat_explorer_hood_001', 'Explorer Hood', 'headwear', 'Headwear', 'rare', 'Woodland cowl hood draping smoothly over the shoulders.', 'sheet_hat_explorer_hood_001', true),
    ('hat_iron_helmet_001', 'hat_iron_helmet_001', 'Iron Helmet', 'headwear', 'Headwear', 'uncommon', 'Forged iron kettle helmet with nose guard protecting the skull.', 'sheet_hat_iron_helmet_001', true),
    ('hat_steel_helmet_001', 'hat_steel_helmet_001', 'Steel Helmet', 'headwear', 'Headwear', 'rare', 'Full-face polished knight''s bascinet with eye slits.', 'sheet_hat_steel_helmet_001', true),
    ('hat_mystic_hat_001', 'hat_mystic_hat_001', 'Mystic Hat', 'headwear', 'Headwear', 'epic', 'Pointed indigo wizard hat adorned with celestial star embroidery.', 'sheet_hat_mystic_hat_001', true),
    ('hat_forest_ranger_hood_001', 'hat_forest_ranger_hood_001', 'Forest Ranger Hood', 'headwear', 'Headwear', 'common', 'Hunter''s green hood fitted with a golden oak-leaf clasp.', 'sheet_hat_forest_ranger_hood_001', true),
    ('fac_cloth_mask_001', 'fac_cloth_mask_001', 'Cloth Mask', 'face_accessories', 'Face', 'common', 'Black assassin cloth bandana concealing mouth and nose.', 'sheet_fac_cloth_mask_001', true),
    ('fac_leather_mask_001', 'fac_leather_mask_001', 'Leather Mask', 'face_accessories', 'Face', 'common', 'Hardened leather combat respirator mask with brass rivets.', 'sheet_fac_leather_mask_001', true),
    ('fac_mystic_mask_001', 'fac_mystic_mask_001', 'Mystic Mask', 'face_accessories', 'Face', 'epic', 'Ornate porcelain mask painted with arcane sigils.', 'sheet_fac_mystic_mask_001', true),
    ('fac_adventurer_goggles_001', 'fac_adventurer_goggles_001', 'Adventurer Goggles', 'face_accessories', 'Face', 'common', 'Brass explorer goggles with amber-tinted lenses.', 'sheet_fac_adventurer_goggles_001', true),
    ('fac_mechanical_goggles_001', 'fac_mechanical_goggles_001', 'Mechanical Goggles', 'face_accessories', 'Face', 'common', 'Complex steampunk multi-lens goggles with optical gears.', 'sheet_fac_mechanical_goggles_001', true),
    ('bag_small_traveler_001', 'bag_small_traveler_001', 'Small Traveler Backpack', 'backpacks', 'Back', 'common', 'Compact canvas daypack with single brass-buckled flap.', 'sheet_bag_small_traveler_001', true),
    ('bag_leather_backpack_001', 'bag_leather_backpack_001', 'Leather Backpack', 'backpacks', 'Back', 'common', 'Sturdy saddle-leather expedition backpack with utility pouches.', 'sheet_bag_leather_backpack_001', true),
    ('bag_adventurer_backpack_001', 'bag_adventurer_backpack_001', 'Adventurer Backpack', 'backpacks', 'Back', 'common', 'Traveler pack with rolled woolen bedroll strapped on top.', 'sheet_bag_adventurer_backpack_001', true),
    ('bag_explorer_backpack_001', 'bag_explorer_backpack_001', 'Explorer Backpack', 'backpacks', 'Back', 'rare', 'Large field pack fitted with map tube and brass compass pouch.', 'sheet_bag_explorer_backpack_001', true),
    ('bag_military_backpack_001', 'bag_military_backpack_001', 'Military-Style Backpack', 'backpacks', 'Back', 'rare', 'Heavy-duty reinforced tactical ruck with modular gear straps.', 'sheet_bag_military_backpack_001', true),
    ('var_skin_pale_001', 'var_skin_pale_001', 'Pale Skin Appearance', 'character_variations', 'Skin', 'common', 'Fair alabaster skin tone option for player customization.', 'sheet_var_skin_pale_001', true),
    ('var_skin_dark_001', 'var_skin_dark_001', 'Umber Skin Appearance', 'character_variations', 'Skin', 'common', 'Deep umber rich skin tone option for player customization.', 'sheet_var_skin_dark_001', true),
    ('var_skin_olive_001', 'var_skin_olive_001', 'Olive Skin Appearance', 'character_variations', 'Skin', 'common', 'Warm sun-kissed olive complexion for player customization.', 'sheet_var_skin_olive_001', true),
    ('var_hair_braided_red_001', 'var_hair_braided_red_001', 'Braided Crimson Hair', 'character_variations', 'Hair', 'common', 'Long warrior braids woven in vibrant flame-red hair.', 'sheet_var_hair_braided_red_001', true),
    ('var_hair_spiky_black_001', 'var_hair_spiky_black_001', 'Spiky Obsidian Hair', 'character_variations', 'Hair', 'common', 'Dynamic stylized spiky adventurer haircut in deep obsidian.', 'sheet_var_hair_spiky_black_001', true),
    ('var_face_warpaint_001', 'var_face_warpaint_001', 'Azure Hunter Warpaint', 'character_variations', 'Chest', 'common', 'Tribal azure warpaint stripes painted across cheeks and nose.', 'sheet_var_face_warpaint_001', true),
    ('var_face_eyepatch_001', 'var_face_eyepatch_001', 'Veteran Leather Eyepatch', 'character_variations', 'Chest', 'common', 'Weathered black leather eyepatch covering the right eye.', 'sheet_var_face_eyepatch_001', true)
ON CONFLICT (id) DO UPDATE SET
    equipment_slot = EXCLUDED.equipment_slot,
    asset_id = EXCLUDED.asset_id,
    display_name = EXCLUDED.display_name,
    category = EXCLUDED.category,
    rarity = EXCLUDED.rarity,
    description = EXCLUDED.description,
    is_available = EXCLUDED.is_available;

-- 4. Player Loadouts table
CREATE TABLE IF NOT EXISTS player_loadouts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    loadout_version INT NOT NULL DEFAULT 1,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_loadouts_user UNIQUE (user_id)
);

CREATE INDEX IF NOT EXISTS idx_player_loadouts_user ON player_loadouts(user_id);

-- 5. Player Loadout Items table
CREATE TABLE IF NOT EXISTS player_loadout_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    loadout_id UUID NOT NULL REFERENCES player_loadouts(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cosmetic_id VARCHAR(64) NOT NULL REFERENCES cosmetic_definitions(id) ON DELETE CASCADE,
    equipment_slot VARCHAR(32) NOT NULL,
    equipped_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_loadout_slot UNIQUE (user_id, equipment_slot)
);

CREATE INDEX IF NOT EXISTS idx_loadout_items_user ON player_loadout_items(user_id);
CREATE INDEX IF NOT EXISTS idx_loadout_items_slot ON player_loadout_items(equipment_slot);

-- 6. Backfill existing users with default loadouts
INSERT INTO player_loadouts (user_id, loadout_version)
SELECT id, 1 FROM users
ON CONFLICT (user_id) DO NOTHING;
