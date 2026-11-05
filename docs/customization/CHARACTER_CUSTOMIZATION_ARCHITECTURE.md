# WILDBOUND — CHARACTER CUSTOMIZATION ARCHITECTURE
## Phase 25 Architecture & Subsystem Specification

---

## 1. Subsystem Overview

The **Character Customization System** provides an extensible, modular, data-driven architecture that allows players to personalize their visual character appearance, test changes in an isolated real-time preview viewport, manage appearance loadouts, and persist aesthetic choices locally.

The architecture maintains strict boundaries:
- **Cosmetic Appearance** (`customization::CharacterAppearance`, `customization::CharacterCustomizationManager`) is authoritative over visual sprites and cosmetics.
- **Gameplay Equipment** (`entities::EquipmentSystem`) is authoritative over combat stats, defense modifiers, weapon damage, and inventory slots.
- **Preview Isolation** (`customization::AppearancePreview`) allows non-destructive trial of cosmetics, animations, and directions without mutating player gameplay state until explicit confirmation.

```
+-------------------------------------------------------------+
|                     Player Entity                           |
|  +--------------------+             +--------------------+  |
|  |  EquipmentSystem   |             |  ModularCharacter  |  |
|  | (Combat & Defense) |             | (Directional Paper)|  |
|  +---------+----------+             +---------^----------+  |
+------------|----------------------------------|-------------+
             | Sync on Equip                    | Sync Visuals
             v                                  |
+-----------------------------------------------+-------------+
|              CharacterCustomizationManager                  |
|  +---------------------+             +-------------------+  |
|  | CharacterAppearance |             | AppearancePreview |  |
|  | (Authoritative)     |             | (Isolated Trial)  |  |
|  +----------+----------+             +---------^---------+  |
|             |                                  |            |
|             v                                  |            |
|  +---------------------+             +---------+---------+  |
|  |  CosmeticRegistry   |             | AppearanceLoadout |  |
|  | (Indexed Metadata)  |             |  & Serializer     |  |
|  +---------------------+             +-------------------+  |
+-------------------------------------------------------------+
```

---

## 2. Core Architectural Components

### 2.1 `CharacterCustomizationManager`
Coordinates all customization activities:
- Binds to the player entity and synchronizes appearance with `ModularCharacter`.
- Provides transactional operations: `EquipCosmetic`, `UnequipCosmetic`, `ReplaceCosmetic`, `ResetCosmeticSlot`, `ResetToDefault`.
- Manages preview session transitions: `BeginPreview`, `ApplyPreviewCosmetic`, `ResetPreview`, `ConfirmPreview`, `CancelPreview`.
- Manages loadout persistence: `CreateLoadout`, `SaveCurrentToLoadout`, `LoadLoadout`, `DuplicateLoadout`, `DeleteLoadout`.
- Synchronizes with `EquipmentSystem::OnEquipmentChanged` without altering gameplay stats.

### 2.2 `CosmeticRegistry`
Central catalog storing and indexing all available cosmetic definitions:
- Fast lookups by unique ID (`GetCosmetic(id)`).
- Filtered lookups by slot (`GetCosmeticsBySlot(slot)`).
- Automatic catalog ingestion from `data/cosmetics/costume_catalog.json` (Phase 17).
- Built-in registration of Phase 8 production cosmetics (32 items) and default character variations.

### 2.3 `CharacterAppearance`
Authoritative visual configuration:
- Maps each `CustomizationSlot` (`Skin`, `Hair`, `Headwear`, `Helmet`, `Face`, `Chest`, `Legs`, `Feet`, `Back`) to its selected cosmetic ID.
- Stores base body skin spritesheet.
- Decoupled from entity simulation: translates into `ModularCharacter` layer stacks without coupling to world physics or hitboxes.

### 2.4 `AppearancePreview`
Isolated sandbox preview session:
- Clones appearance state upon `BeginPreview()`.
- Renders through an independent `ModularCharacter` and `AnimationController` instance.
- Supports 4 cardinal directions (`Up`, `Down`, `Left`, `Right`) and interactive clockwise/counter-clockwise rotation.
- Supports animation state toggling (`Idle` vs `Walk`) to verify sprite synchronization during motion.
- Guarantees zero leakage into authoritative gameplay state upon `CancelPreview()`.

### 2.5 `AppearanceLoadout` & `AppearanceSerializer`
- Represents named presets with schema versioning and timestamps.
- Persists loadouts to `user_data/character/appearance_loadouts.json` using atomic file operations.
- Gracefully handles missing files, malformed JSON, and provides default starter presets.

---

## 3. Directional Layer Ordering & Occlusion Rules

The customization system relies on `ModularCharacter::BuildLayerStack` for depth sorting:
1. **Down Facing (South):** Back Accessory -> Shadow -> Base Body -> Skin -> Pants -> Shoes -> Shirt -> Armor -> Face -> Hair -> Headwear/Helmet -> Weapon.
2. **Up Facing (North):** Shadow -> Hair -> Headwear/Helmet -> Base Body -> Skin -> Pants -> Shoes -> Shirt -> Armor -> Back Accessory -> Face -> Weapon.
3. **Occlusion Suppression:**
   - Helmets with flag `hair_hide_all` suppress the entire hair rendering layer.
   - Hoods with flag `hair_hide_top` suppress top hair volume while keeping back locks visible.
   - Masks with flag `mask_hide_goggles` suppress overlapping eyewear.

