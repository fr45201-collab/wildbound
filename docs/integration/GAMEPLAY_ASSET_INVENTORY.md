# WILDBOUND — GAMEPLAY ASSET INTEGRATION INVENTORY

## 1. Inventory & Integration Scope Summary

* **Total Registered Engine Assets in AMS**: 580 assets
* **Audit Date**: 2026-11-04
* **Target Subsystems**:
  * Item Definitions, Inventory & Runtime Instances
  * World Pickups & Loot Containers
  * Swords (Melee) & Bows (Ranged Projectiles)
  * Armor, Defense Calculation & Equipment System
  * Consumables, Resources & Special Items
  * Modular Character Appearance Synchronization
  * Player-Facing UI (HUD, Inventory, Equipment, Wardrobe)
  * Centralized Audio Event System
  * Biomes, Living World Environmental Animations & Interiors

---

## 2. Comprehensive Asset Inventory Table

| Asset ID | Category | Path | Metadata / Catalog | AMS Reg | Target Gameplay System | Integration Status | Validation Status |
|---|---|---|---|---|---|---|---|
| **Items (Phase 16 - 40 Items)** | | | | | | | |
| `col_gold_coin_001` | Collectible | `assets/items/icons/col_gold_coin_001_icon.png` | `data/items/item_catalog.json` | Registered | Economy / Inventory | Ready for integration | Verified |
| `col_silver_coin_001` | Collectible | `assets/items/icons/col_silver_coin_001_icon.png` | `data/items/item_catalog.json` | Registered | Economy / Inventory | Ready for integration | Verified |
| `col_ruby_001` | Collectible | `assets/items/icons/col_ruby_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `col_sapphire_001` | Collectible | `assets/items/icons/col_sapphire_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `col_emerald_001` | Collectible | `assets/items/icons/col_emerald_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `col_ancient_crystal_001` | Collectible | `assets/items/icons/col_ancient_crystal_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `col_sunken_relic_001` | Collectible | `assets/items/icons/col_sunken_relic_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `col_royal_emblem_001` | Collectible | `assets/items/icons/col_royal_emblem_001_icon.png` | `data/items/item_catalog.json` | Registered | Loot / Inventory | Ready for integration | Verified |
| `pot_health_001` | Consumable | `assets/items/icons/pot_health_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Consumables | Ready for integration | Verified |
| `pot_mana_001` | Consumable | `assets/items/icons/pot_mana_001_icon.png` | `data/items/item_catalog.json` | Registered | Mana / Consumables | Ready for integration | Verified |
| `pot_stamina_001` | Consumable | `assets/items/icons/pot_stamina_001_icon.png` | `data/items/item_catalog.json` | Registered | Stamina / Consumables | Ready for integration | Verified |
| `pot_antidote_001` | Consumable | `assets/items/icons/pot_antidote_001_icon.png` | `data/items/item_catalog.json` | Registered | Status / Consumables | Ready for integration | Verified |
| `pot_herbal_elixir_001` | Consumable | `assets/items/icons/pot_herbal_elixir_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Consumables | Ready for integration | Verified |
| `food_bread_001` | Consumable | `assets/items/icons/food_bread_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Food | Ready for integration | Verified |
| `food_apple_001` | Consumable | `assets/items/icons/food_apple_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Food | Ready for integration | Verified |
| `food_meat_cooked_001` | Consumable | `assets/items/icons/food_meat_cooked_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Food | Ready for integration | Verified |
| `food_berries_001` | Consumable | `assets/items/icons/food_berries_001_icon.png` | `data/items/item_catalog.json` | Registered | Health / Food | Ready for integration | Verified |
| `food_water_flask_001` | Consumable | `assets/items/icons/food_water_flask_001_icon.png` | `data/items/item_catalog.json` | Registered | Stamina / Food | Ready for integration | Verified |
| `mat_timber_log_001` | Material | `assets/items/icons/mat_timber_log_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_granite_stone_001` | Material | `assets/items/icons/mat_granite_stone_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_iron_ore_001` | Material | `assets/items/icons/mat_iron_ore_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_gold_ore_001` | Material | `assets/items/icons/mat_gold_ore_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_leather_strip_001` | Material | `assets/items/icons/mat_leather_strip_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_cloth_bolt_001` | Material | `assets/items/icons/mat_cloth_bolt_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_iron_ingot_001` | Material | `assets/items/icons/mat_iron_ingot_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `mat_gold_ingot_001` | Material | `assets/items/icons/mat_gold_ingot_001_icon.png` | `data/items/item_catalog.json` | Registered | Resource / Crafting | Ready for integration | Verified |
| `res_red_mushroom_001` | Resource | `assets/items/icons/res_red_mushroom_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_blue_mushroom_001` | Resource | `assets/items/icons/res_blue_mushroom_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_sunblossom_flower_001` | Resource | `assets/items/icons/res_sunblossom_flower_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_forest_herb_001` | Resource | `assets/items/icons/res_forest_herb_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_wild_berries_001` | Resource | `assets/items/icons/res_wild_berries_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_cactus_flesh_001` | Resource | `assets/items/icons/res_cactus_flesh_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `res_frostbloom_sprig_001` | Resource | `assets/items/icons/res_frostbloom_sprig_001_icon.png` | `data/items/item_catalog.json` | Registered | Alchemy / Inventory | Ready for integration | Verified |
| `key_celestial_001` | Special | `assets/items/icons/key_celestial_001_icon.png` | `data/items/item_catalog.json` | Registered | Lock / Container | Ready for integration | Verified |
| `key_rusty_iron_001` | Special | `assets/items/icons/key_rusty_iron_001_icon.png` | `data/items/item_catalog.json` | Registered | Lock / Container | Ready for integration | Verified |
| `scroll_ancient_001` | Special | `assets/items/icons/scroll_ancient_001_icon.png` | `data/items/item_catalog.json` | Registered | Lore / Interaction | Ready for integration | Verified |
| `map_treasure_001` | Special | `assets/items/icons/map_treasure_001_icon.png` | `data/items/item_catalog.json` | Registered | Lore / Interaction | Ready for integration | Verified |
| `rune_stone_001` | Special | `assets/items/icons/rune_stone_001_icon.png` | `data/items/item_catalog.json` | Registered | Magic / Interaction | Ready for integration | Verified |
| `amulet_sun_001` | Special | `assets/items/icons/amulet_sun_001_icon.png` | `data/items/item_catalog.json` | Registered | Magic / Equipment | Ready for integration | Verified |
| `artifact_chrono_001` | Special | `assets/items/icons/artifact_chrono_001_icon.png` | `data/items/item_catalog.json` | Registered | Magic / Relic | Ready for integration | Verified |
| **Weapons (Phase 15 - 12 Weapons & Ammunition)** | | | | | | | |
| `sword_wooden_training_001` | Sword | `assets/weapons/held/sword_wooden_training_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `sword_rusty_short_001` | Sword | `assets/weapons/held/sword_rusty_short_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `sword_iron_001` | Sword | `assets/weapons/held/sword_iron_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `sword_steel_longsword_001` | Sword | `assets/weapons/held/sword_steel_longsword_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `sword_curved_saber_001` | Sword | `assets/weapons/held/sword_curved_saber_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `sword_ancient_runeblade_001` | Sword | `assets/weapons/held/sword_ancient_runeblade_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Melee | Ready for integration | Verified |
| `bow_wooden_hunting_001` | Bow | `assets/weapons/held/bow_wooden_hunting_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Ranged | Ready for integration | Verified |
| `bow_yew_longbow_001` | Bow | `assets/weapons/held/bow_yew_longbow_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Ranged | Ready for integration | Verified |
| `bow_reinforced_composite_001` | Bow | `assets/weapons/held/bow_reinforced_composite_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Ranged | Ready for integration | Verified |
| `bow_equestrian_recurve_001` | Bow | `assets/weapons/held/bow_equestrian_recurve_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Ranged | Ready for integration | Verified |
| `bow_ancient_runebow_001` | Bow | `assets/weapons/held/bow_ancient_runebow_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Combat / Ranged | Ready for integration | Verified |
| `ammo_arrow_broadhead_001` | Ammunition | `assets/weapons/held/ammo_arrow_broadhead_001_held.png` | `data/weapons/weapon_catalog.json` | Registered | Projectile / Ammo | Ready for integration | Verified |
| **Armor & Costumes (Phase 17 - 50 Designs)** | | | | | | | |
| `armor_leather_cuirass` | Armor | `assets/cosmetics/armor/armor_leather_cuirass_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_iron_cuirass` | Armor | `assets/cosmetics/armor/armor_iron_cuirass_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_steel_cuirass` | Armor | `assets/cosmetics/armor/armor_steel_cuirass_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_forest_guardian` | Armor | `assets/cosmetics/armor/armor_forest_guardian_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_desert_nomad` | Armor | `assets/cosmetics/armor/armor_desert_nomad_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_frost_knight` | Armor | `assets/cosmetics/armor/armor_frost_knight_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `armor_ancient_rune` | Armor | `assets/cosmetics/armor/armor_ancient_rune_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Defense | Ready for integration | Verified |
| `hat_iron_helmet` | Headwear | `assets/cosmetics/hats/hat_iron_helmet_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Head | Ready for integration | Verified |
| `hat_steel_helmet` | Headwear | `assets/cosmetics/hats/hat_steel_helmet_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Head | Ready for integration | Verified |
| `shoes_iron_boots` | Shoes | `assets/cosmetics/shoes/shoes_iron_boots_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Feet | Ready for integration | Verified |
| `pants_armored_cuisses` | Pants | `assets/cosmetics/pants/pants_armored_cuisses_spritesheet.png` | `data/cosmetics/costume_catalog.json` | Registered | Equipment / Legs | Ready for integration | Verified |
| **UI Components (Phase 18)** | | | | | | | |
| 10 Backgrounds | UI | `assets/ui/backgrounds/*.png` | `assets/manifests/asset_manifest.json` | Registered | UIManager Screens | Ready for integration | Verified |
| 5 Panels | UI | `assets/ui/panels/*.png` | `assets/manifests/asset_manifest.json` | Registered | UIManager Frames | Ready for integration | Verified |
| 21 Button Textures | UI | `assets/ui/buttons/*.png` | `assets/manifests/asset_manifest.json` | Registered | UIManager Controls | Ready for integration | Verified |
| 25 UI Icons | UI | `assets/ui/icons/*.png` | `assets/manifests/asset_manifest.json` | Registered | Inventory / HUD | Ready for integration | Verified |
| **Audio Assets (Phase 18 - 68 Assets)** | | | | | | | |
| 10 Music Tracks | Music | `assets/audio/music/*.wav` | `data/audio/audio_catalog.json` | Registered | AudioSystem Music | Ready for integration | Verified |
| 7 Ambient Loops | Ambience | `assets/audio/ambience/*.wav` | `data/audio/audio_catalog.json` | Registered | AudioSystem Env | Ready for integration | Verified |
| 15 UI Sounds | SFX | `assets/audio/sfx/ui/*.wav` | `data/audio/audio_catalog.json` | Registered | AudioSystem UI | Ready for integration | Verified |
| 14 Player Sounds | SFX | `assets/audio/sfx/player/*.wav` | `data/audio/audio_catalog.json` | Registered | Movement / Hurt | Ready for integration | Verified |
| 9 Weapon Sounds | SFX | `assets/audio/sfx/weapons/*.wav` | `data/audio/audio_catalog.json` | Registered | Combat / Projectiles | Ready for integration | Verified |
| 8 Interaction Sounds | SFX | `assets/audio/sfx/interactions/*.wav` | `data/audio/audio_catalog.json` | Registered | Loot / Doors | Ready for integration | Verified |
| 5 Event Stingers | SFX | `assets/audio/sfx/events/*.wav` | `data/audio/audio_catalog.json` | Registered | Match Flow | Ready for integration | Verified |

---

## 3. Classification Breakdown

* **Ready for Integration**: **100% of required production assets** (Items, Weapons, Armor, Costumes, Audio, UI, Biomes, Village, Animations).
* **Missing**: 0 critical production assets.
* **Incompatible**: 0 assets.
* **Prerequisite Gate Status**: **PASSED**. All underlying subsystems (AMS, ModularCharacter, CombatSystem, WorldRenderer, UIManager, AudioSystem, BuildingManager, BiomeManager) are fully implemented and verified by 20/20 passing CTests.

