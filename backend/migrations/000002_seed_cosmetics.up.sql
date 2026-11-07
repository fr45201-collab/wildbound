-- 000002_seed_cosmetics.up.sql
-- Seed standard cosmetic definitions matching Wildbound asset catalog

INSERT INTO cosmetic_definitions (id, cosmetic_key, display_name, category, rarity, description, is_available)
VALUES
    ('cosmetic_avatar_default', 'avatar_default', 'Adventurer Crest', 'Avatar', 'common', 'The standard insignia of a Wildbound contender.', true),
    ('cosmetic_tunic_adventurer', 'tunic_adventurer', 'Adventurer Tunic', 'Chest', 'common', 'A rugged leather tunic built for wilderness survival.', true),
    ('cosmetic_pants_leather', 'pants_leather', 'Leather Trousers', 'Legs', 'common', 'Reinforced leather leggings resistant to bramble tears.', true),
    ('cosmetic_boots_leather', 'boots_leather', 'Traveler Boots', 'Feet', 'common', 'Sturdy boots engineered for marsh and crag exploration.', true),
    ('cosmetic_hair_short', 'hair_short', 'Crop Cut', 'Hair', 'common', 'A practical low-maintenance haircut for contenders.', true),
    ('cosmetic_hair_medium', 'hair_medium', 'Wild Mane', 'Hair', 'uncommon', 'Mid-length windswept hair styled by the storm.', true),
    ('cosmetic_hair_long', 'hair_long', 'Braided Locks', 'Hair', 'uncommon', 'Elaborately braided hair favored by valley tribes.', true),
    ('cosmetic_skin_fair', 'skin_fair', 'Fair Complexion', 'Skin', 'common', 'Default fair skin tone.', true),
    ('cosmetic_skin_medium', 'skin_medium', 'Sun-Kissed Complexion', 'Skin', 'common', 'Sun-bronzed skin tone.', true),
    ('cosmetic_skin_dark', 'skin_dark', 'Dusk Complexion', 'Skin', 'common', 'Deep rich skin tone.', true),
    ('cosmetic_hat_adventurer', 'hat_adventurer', 'Explorer Fedora', 'Headwear', 'rare', 'A wide-brimmed weathered hat guarding against storm fallout.', true),
    ('cosmetic_backpack_traveler', 'backpack_traveler', 'Ranger Rucksack', 'Back', 'rare', 'A tactical survival backpack with bedroll mounts.', true)
ON CONFLICT (id) DO NOTHING;

