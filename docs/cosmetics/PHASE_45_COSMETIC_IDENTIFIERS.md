# Wildbound — Cosmetic Identification Architecture & Schema Standards

## 1. Overview

Phase 45 unifies cosmetic identifiers across the asset pipeline, C++ game client, dedicated server, network serialization, Go backend, and PostgreSQL database.

---

## 2. Identifier Principles

1. **Deterministic & Stable**: Cosmetic IDs are fixed strings independent of display names, localized text, or file directory hierarchies. Renaming or moving an asset file does not invalidate ownership or loadout records.
2. **Category & Slot Segregation**: Each cosmetic belongs to an overarching `category` (e.g. `clothing`, `armor`, `pants`, `footwear`, `headwear`, `face_accessories`, `backpacks`, `character_variations`) and explicitly binds to an `equipment_slot`:
   - `Skin`
   - `Hair`
   - `Headwear`
   - `Helmet`
   - `Face`
   - `Chest`
   - `Legs`
   - `Feet`
   - `Back`
3. **Asset Resolution via AMS**: Rather than storing raw file paths, definitions reference asset keys (`asset_id`), which the client's Asset Management System (AMS) resolves to loaded textures and spritesheets (e.g. `sheet_clo_basic_traveler_001`, `tex_hat_iron_helmet_001`).

---

## 3. Database Schema (Migration `000005`)

### 3.1 `cosmetic_definitions`
```sql
ALTER TABLE cosmetic_definitions
    ADD COLUMN IF NOT EXISTS equipment_slot VARCHAR(32) NOT NULL DEFAULT 'Chest',
    ADD COLUMN IF NOT EXISTS asset_id VARCHAR(128) NOT NULL DEFAULT '';
```

### 3.2 `player_loadouts`
Tracks the active loadout version for optimistic concurrency control:
```sql
CREATE TABLE player_loadouts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    loadout_version INT NOT NULL DEFAULT 1,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_loadouts_user UNIQUE (user_id)
);
```

### 3.3 `player_loadout_items`
Normalizes equipped items per slot:
```sql
CREATE TABLE player_loadout_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    loadout_id UUID NOT NULL REFERENCES player_loadouts(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cosmetic_id VARCHAR(64) NOT NULL REFERENCES cosmetic_definitions(id) ON DELETE CASCADE,
    equipment_slot VARCHAR(32) NOT NULL,
    equipped_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_loadout_slot UNIQUE (user_id, equipment_slot)
);
```

---

## 4. Canonical Cosmetic ID Samples

| Cosmetic ID | Display Name | Category | Equipment Slot | Asset ID | Rarity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `clo_basic_traveler_001` | Basic Traveler Shirt | clothing | Chest | `sheet_clo_basic_traveler_001` | common |
| `arm_iron_armor_001` | Iron Armor | armor | Chest | `sheet_arm_iron_armor_001` | uncommon |
| `arm_ancient_rune_001` | Ancient Rune Armor | armor | Chest | `sheet_arm_ancient_rune_001` | legendary |
| `clo_p_traveler_pants_001` | Traveler Pants | pants | Legs | `sheet_clo_p_traveler_pants_001` | common |
| `sho_leather_boots_001` | Leather Boots | footwear | Feet | `sheet_sho_leather_boots_001` | common |
| `hat_iron_helmet_001` | Iron Helmet | headwear | Headwear | `sheet_hat_iron_helmet_001` | uncommon |
| `fac_adventurer_goggles_001`| Adventurer Goggles | face_accessories | Face | `sheet_fac_adventurer_goggles_001` | common |
| `bag_adventurer_backpack_001`| Adventurer Backpack | backpacks | Back | `sheet_bag_adventurer_backpack_001` | rare |
| `var_skin_dark_001` | Dark Complexion | character_variations | Skin | `sheet_var_skin_dark_001` | common |
| `var_hair_braided_red_001` | Braided Red Hair | character_variations | Hair | `sheet_var_hair_braided_red_001` | common |

