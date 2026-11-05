# WILDBOUND — PHASE 25 REPOSITORY AUDIT REPORT
## Character Customization & Modular Appearance Architecture

**Date:** 2026-11-04  
**Status:** Audit Completed — Architecture Approved for Implementation  
**Auditor:** Senior Gameplay & Systems Architect  

---

## 1. Executive Summary

This audit evaluates the codebase of Wildbound in preparation for **Phase 25: Character Customization System**. The goal is to provide a complete, modular, data-driven character customization architecture that connects existing modular character assets to actual gameplay, enabling players to customize appearance, preview changes in real time, equip and unequip cosmetics, and manage persistent appearance loadouts.

The repository was inspected across all relevant domains:
1. **Modular Character Rendering** (`ModularCharacter`, `CharacterEquipment`, `VisualLayer`, `AnimationController`, directional layering).
2. **Cosmetic Assets & Metadata** (Phase 8 production cosmetics [32 items], Phase 17 costume catalog [50 items], base skin variations, AMS registration).
3. **Gameplay Equipment & Inventory** (`EquipmentSystem`, `Inventory`, item definitions, stat calculations).
4. **UI Architecture** (`UIManager`, custom raylib 640x360 virtual canvas, `DevUI` Dear ImGui overlay).
5. **Input Contexts** (`InputManager`, `InputContext`, `InputContextType::Wardrobe`).
6. **Audio System** (`AudioSystem`, `AudioManager`, UI sound event dispatches).
7. **Persistence** (`nlohmann/json`, file I/O, config serialization).

---

## 2. Existing Architecture & Reused Systems

### 2.1 Modular Character Rendering (`src/entities/ModularCharacter.hpp`, `ModularCharacter.cpp`)
- **Layer Architecture:** `ModularCharacter` constructs a directional sorted layer stack via `BuildLayerStack(Direction facing)`. It renders layers (Body, Skin, Hair, Shirt, Armor, Pants, Shoes, Headwear, Face, Goggles, Mask, Back, Waist) in lockstep with `AnimationController` and `Direction`.
- **CharacterEquipment:** Manages a slot-based map `std::unordered_map<CosmeticCategory, CosmeticItem> m_equipped` with compatibility checking (`CheckCompatibility`) and occlusion flag queries (`HasOcclusionFlag`, e.g., `hair_hide_all`, `hair_hide_top`, `mask_hide_goggles`).
- **Directional Rendering:** Supports all 4 cardinal directions (`Up`, `Down`, `Left`, `Right`) with correct depth sorting (e.g. back accessories render behind body when facing Down, in front when facing Up).
- **Current State:** Fully functional, stable, and tested in `test_modular_character.exe` and `test_production_cosmetics.exe`.

### 2.2 Gameplay Equipment Architecture (`src/entities/EquipmentSystem.hpp`, `EquipmentSystem.cpp`)
- **Authority:** `EquipmentSystem` owns gameplay equipment (`EquipmentSlot::Head`, `Chest`, `Legs`, `Feet`, `Face`, `Back`, `Weapon`), derives combat modifiers (`totalDefense`, `damageReduction`, `healthBonus`, `speedModifier`, `attackSpeedModifier`), and switches active weapons.
- **Separation:** Crucially, gameplay armor stats must remain strictly decoupled from cosmetic appearance. When gameplay equipment changes, it triggers visual synchronization via `CharacterAppearance`, but pure cosmetic customization must never alter `EquipmentStatModifiers` or `HealthComponent`.

### 2.3 Asset Management System (AMS)
- **Manifest:** `assets/manifests/asset_manifest.json` registers over 185 spritesheets and corresponding textures, including all Phase 8 and Phase 17 cosmetic assets.
- **Dynamic Caching:** `AssetManager` and `TextureManager` properly cache loaded textures and spritesheets. Recent fixes ensured fallback textures are cached, preventing repetitive disk queries.

### 2.4 User Interface Architecture (`src/ui/UIManager.hpp`, `UIManager.cpp`)
- **Canvas:** Renders to a 640x360 virtual resolution canvas scaled to the window.
- **Screens:** `UIScreen::Wardrobe` and `UIScreen::CharacterSelect` already exist as enum states.
- **Components:** `DrawPanel`, `DrawButton`, `DrawProgressBar`, `DrawIcon` provide consistent pixel-art aesthetic.
- **Previous Implementation:** `RenderWardrobe` previously contained a preliminary prototype that directly modified the player's equipment without preview isolation, loadout management, or category filtering.

### 2.5 Input Management (`src/input/InputContext.hpp`, `InputManager.hpp`)
- `InputContextType::Wardrobe` is already defined in `InputContext.hpp`. It is configured with `ActionCategory::UI` and `ActionCategory::Menu`, blocking gameplay movement and combat actions while the customization screen is active.

---

## 3. Available Assets vs Missing Gaps

### 3.1 Available Cosmetic Assets
1. **Phase 8 Production Cosmetics (32 items):**
   - **Hair (5):** `hair_short_001`, `hair_long_001`, `hair_messy_001`, `hair_ponytail_001`, `hair_curly_001`
   - **Shirts (5):** `shirt_tunic_001`, `shirt_leather_001`, `shirt_armor_001`, `shirt_traveler_001`, `shirt_mage_001`
   - **Pants (5):** `pants_traveler_001`, `pants_leather_001`, `pants_armored_001`, `pants_short_001`, `pants_mage_001`
   - **Shoes (3):** `shoes_boots_001`, `shoes_armored_001`, `shoes_light_001`
   - **Hats (5):** `hat_adventurer_001`, `hat_hood_001`, `hat_helmet_001`, `hat_wizard_001`, `hat_cap_001`
   - **Goggles (3):** `goggles_adventure_001`, `goggles_steampunk_001`, `goggles_snow_001`
   - **Masks (3):** `mask_cloth_001`, `mask_leather_001`, `mask_mystic_001`
   - **Backpacks (3):** `backpack_traveler_001`, `backpack_leather_001`, `backpack_adventure_001`

2. **Phase 17 Costume Catalog (50 items in `data/cosmetics/costume_catalog.json`):**
   - **Clothing (8):** `clo_basic_traveler_001` to `clo_dark_adventurer_001`
   - **Armor (8):** `arm_leather_armor_001` to `arm_ancient_rune_001`
   - **Pants (6):** `clo_p_traveler_pants_001` to `clo_p_mystic_pants_001`
   - **Footwear (5):** `sho_leather_boots_001` to `sho_desert_sandals_001`
   - **Headwear (6):** `hat_adventurer_hat_001` to `hat_royal_crown_001`
   - **Face (5):** `fac_cloth_mask_001` to `fac_mystic_blindfold_001`
   - **Backpacks (5):** `pak_leather_backpack_001` to `pak_explorer_pack_001`
   - **Variations (7):** `var_skin_pale_001` to `var_skin_orc_001`

3. **Core Base Cosmetics:**
   - `skin_default`, `skin_tan`, `hair_short_brown`, `hair_ponytail_blonde`, `shirt_forest_tunic`, `pants_traveler_trousers`, `shoes_leather_boots`, `back_traveler_backpack`.

### 3.2 Gaps Identified Before Phase 25
- **No Centralized Cosmetic Registry:** Cosmetic data was split between hardcoded structs, JSON catalogs, and test fixtures without a unified C++ registry class.
- **No Isolated Character Preview:** Modifying wardrobe immediately mutated the active player's equipped visual items. Cancellation was not possible.
- **No Directional/Animation Preview Controls:** Players could not rotate the character or test animations inside the wardrobe.
- **No Appearance Loadout System:** No ability to save, name, load, duplicate, or delete cosmetic presets.
- **No Local Persistence:** User appearance loadouts were not persisted to `user_data/character/appearance_loadouts.json`.
- **UI Screen Limitations:** The prototype wardrobe screen lacked pagination/scrolling, loadout UI, confirm/cancel buttons, and full category coverage.

---

## 4. Technical Risks & Mitigation Strategies

| Risk | Impact | Mitigation Strategy |
| :--- | :--- | :--- |
| **Cosmetic & Equipment Stat Coupling** | Player equipping cosmetic armor accidentally gains defense, or equipping decorative shirt wipes armor bonus. | Maintain strict boundary: `EquipmentSystem` is the sole authority on stats. `CharacterCustomizationManager` manages visual presentation. If gameplay armor is equipped, it updates the visual appearance slot; changing cosmetics does NOT modify `EquipmentSystem` stat values. |
| **Preview State Mutation Leaks** | Temporary preview selections mutate player in the active world before confirmation. | Implement `AppearancePreview` with isolated appearance copies. State is only applied to `Player` upon explicit `ConfirmPreview()` invocation. |
| **Missing Asset Fallbacks** | A missing or corrupted cosmetic asset crashes the game or spams missing texture logs. | Validate all cosmetic definitions against `AssetRegistry` upon loading. If an asset is missing, safely fall back to the slot default or null layer without throwing or spamming logs. |
| **Heap Churn during Preview** | Rotating character or switching tabs causes texture allocations every frame. | Reuse existing `ModularCharacter::Render` and cached AMS textures; do not allocate render textures or re-parse JSON during runtime preview updates. |

---

## 5. Recommended Implementation Strategy

1. **Subsystem Namespace:** Place all customization components under `src/customization/` (`namespace wildbound::customization`).
2. **Core Components:**
   - `CosmeticTypes.hpp`: Enums and slot mappings (`CosmeticSlot`, `CosmeticCategory`).
   - `CosmeticDefinition.hpp/.cpp`: Comprehensive metadata structure.
   - `CosmeticRegistry.hpp/.cpp`: Centralized database indexing all 80+ available cosmetics.
   - `CharacterAppearance.hpp/.cpp`: Authoritative appearance state representation.
   - `AppearancePreview.hpp/.cpp`: Sandboxed preview session with rotation and animation.
   - `AppearanceLoadout.hpp`: Multi-slot loadout structure.
   - `AppearanceSerializer.hpp/.cpp`: Robust JSON persistence with schema validation and fallback.
   - `CharacterCustomizationManager.hpp/.cpp`: Central façade coordinating customization, preview, player sync, and persistence.
3. **UI Integration:** Modernize `UIManager::RenderWardrobe` to feature category tabs, item card grids/lists, interactive 4-direction preview, and loadout management.
4. **DevUI Integration:** Upgrade `DevUI::RenderWardrobe` into a comprehensive Character Customization Debugger.
5. **Testing & Verification:** Add `tests/customization/test_customization.cpp` covering all acceptance criteria.
6. **Documentation:** Produce all 12 required markdown documents in `docs/customization/`.

