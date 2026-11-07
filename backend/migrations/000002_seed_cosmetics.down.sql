-- 000002_seed_cosmetics.down.sql
DELETE FROM cosmetic_definitions WHERE id IN (
    'cosmetic_avatar_default',
    'cosmetic_tunic_adventurer',
    'cosmetic_pants_leather',
    'cosmetic_boots_leather',
    'cosmetic_hair_short',
    'cosmetic_hair_medium',
    'cosmetic_hair_long',
    'cosmetic_skin_fair',
    'cosmetic_skin_medium',
    'cosmetic_skin_dark',
    'cosmetic_hat_adventurer',
    'cosmetic_backpack_traveler'
);

