# Phase 26 Mandatory Repository Audit: Cosmetics Store & Wardrobe

## 1. Executive Summary
This audit inspects the *Wildbound* repository in preparation for **Phase 26: Cosmetics Store & Wardrobe**. The audit examines the modular character rendering architecture (Phase 7), cosmetic assets (Phase 8 & 17), UI architecture (Phase 18), gameplay integration (Phase 19), input contexts (Phase 21), inventory (Phase 23), equipment (Phase 24), and character customization (Phase 25).

---

## 2. Existing Systems and Source Locations

| Subsystem | Source Location | Status | Phase 26 Reusability |
| :--- | :--- | :--- | :--- |
| **Modular Character** | `src/entities/ModularCharacter.hpp/.cpp` | Complete | Authoritative 9-layer sprite renderer; supports directional frames and sockets. Directly reused by preview. |
| **Cosmetic Catalogue** | `data/cosmetics/costume_catalog.json` | Complete | 50 Phase 17 items with complete metadata (slot, anchors, directions, occlusion). |
| **Cosmetic Registry** | `src/customization/CosmeticRegistry.hpp/.cpp` | Complete | Indexes 98 cosmetics (50 Phase 17 + 32 Phase 8 + skin tints). Provides fast lookup by ID and slot. |
| **Character Appearance** | `src/customization/CharacterAppearance.hpp/.cpp` | Complete | Authoritative appearance struct (9 slots + skin/hair tints). JSON serializable. |
| **Appearance Preview** | `src/customization/AppearancePreview.hpp/.cpp` | Complete | Isolated sandbox preview with independent `ModularCharacter`, 4-direction rotation, Idle/Walk cycle. Zero gameplay mutation until committed. |
| **Appearance Loadouts** | `src/customization/AppearanceLoadout.hpp`, `AppearanceSerializer.hpp/.cpp` | Complete | Named loadout presets persisted locally to `user_data/character/appearance_loadouts.json`. |
| **Customization Manager** | `src/customization/CharacterCustomizationManager.hpp/.cpp` | Complete | Central coordinator connecting registry, preview, appearance, loadouts, and player entity. |
| **Equipment System** | `src/entities/EquipmentSystem.hpp/.cpp` | Complete | Sole authority on combat stats (defense, movement penalties, damage reduction). Decoupled from vanity cosmetics. |
| **Input Contexts** | `src/input/InputManager.hpp/.cpp` | Complete | `InputContextType::Wardrobe` provides modal input isolation, blocking gameplay combat/movement while enabling UI. |
| **UI Framework** | `src/ui/UIManager.hpp/.cpp` | Complete | Raylib native rendering with 2D virtual resolution scaling (640x360 -> window). |
| **Dev Tools** | `src/ui/DevUI.hpp/.cpp` | Complete | Dear ImGui + rlImGui overlay with debugging windows. |
| **Audio System** | `src/audio/AudioSystem.hpp/.cpp` | Complete | Event-driven audio dispatcher with volume controls and sound effects. |

---

## 3. Reusable Components vs. Missing Components

### Reusable Components
1. **`CosmeticDefinition` & `CosmeticRegistry`**: Already parses and indexes 98 items across all slots.
2. **`AppearancePreview`**: Complete isolated staging lifecycle (`BeginPreview`, `ApplySlotChange`, `ResetPreview`, `ConfirmChanges`, `CancelChanges`).
3. **`CharacterAppearance` & `AppearanceLoadout`**: Complete appearance data models and loadout persistence.
4. **`ModularCharacter`**: Multi-layer directional sprite rendering with socket offsets.
5. **`UIManager`**: Scaled button, text, and container rendering helpers.
6. **`AudioSystem`**: Dispatches UI audio cues.

### Missing Components (To Be Implemented in Phase 26)
1. **Cosmetic Ownership Subsystem (`src/cosmetics/CosmeticsOwnershipManager.hpp/.cpp`)**:
   - Currently, all 98 registered cosmetics are treated as unlocked.
   - Missing explicit `Owned` vs. `Locked` state representation.
   - Missing local persistence file `user_data/cosmetics/cosmetic_ownership.json`.
   - Missing transactional prototype unlock mechanism (`Unlock (Prototype)`).
2. **Cosmetic Rarity Model (`src/cosmetics/CosmeticsRarity.hpp`)**:
   - Formalization of the 6 rarity tiers: Common, Uncommon, Rare, Epic, Legendary, Mythic.
   - Presentation metadata: badge colors, display names, sort weights.
3. **Cosmetic Color Variations (`src/cosmetics/CosmeticsColorVariant.hpp`)**:
   - Explicit data model mapping cosmetic IDs to supported color palettes and variants.
   - Support for previewing and applying color variations.
4. **Search, Multi-Filtering & Sorting Engine (`src/cosmetics/CosmeticsSearchFilter.hpp/.cpp`)**:
   - Case-insensitive search by name, description, category, rarity, ID.
   - Filter chaining: Category + Rarity + Ownership state.
   - Sorting: Alphabetical, Rarity, Owned-first, Locked-first.
5. **Dedicated Cosmetics Store & Wardrobe UI (`src/ui/UIManager.cpp` & `src/cosmetics/CosmeticsStoreManager.hpp/.cpp`)**:
   - Left category navigation: Featured, All, Hairstyles, Clothing, Armor, Headwear, Face, Backpacks, Footwear, Special.
   - Catalogue grid with item cards, icons, rarity badges, and ownership pills.
   - Interactive Details Panel with item stats, color variation swatches, Preview, Equip, and Prototype Unlock buttons.
   - Live character preview viewport reusing `AppearancePreview`.
   - Dedicated Wardrobe filter showing only owned cosmetics.
6. **Loadout Ownership Validation**:
   - Integration enforcing that loading a loadout only equips owned cosmetics. If a loadout references an unowned or missing cosmetic, it gracefully falls back without granting unowned items.
7. **Dear ImGui Developer Tools Integration**:
   - "Cosmetics Store & Wardrobe Debugger" tab in `DevUI` allowing inspection of ownership statistics, granting/revoking items, resetting ownership, and testing color variants.

---

## 4. Asset Compatibility and Limitations

- **Spritesheets**: Phase 17 provides 50 standalone spritesheets for clothing, armor, helmets, pants, boots, face accessories, and backpacks. Phase 8 provides 32 modular character spritesheets.
- **Color Variants**: Some assets possess dedicated variant spritesheets (e.g. hair tints, tunic shades); others utilize raylib RGBA modulation tinting.
- **Directional Rendering**: All 50 Phase 17 items support the 4 canonical directions: `up`, `down`, `left`, `right`.
- **Occlusion**: Occlusion flags (e.g. `hair_hide_all` on full helmets) are supported by `ModularCharacter`.

---

## 5. Architectural Separation Rules

```
┌────────────────────────────────────────────────────────┐
│               CosmeticsStoreManager                    │
│    (Coordinates catalogue, ownership, and filtering)   │
└───────────────┬────────────────────────┬───────────────┘
                │                        │
                ▼                        ▼
┌───────────────────────────────┐ ┌───────────────────────────────┐
│   CosmeticsOwnershipManager   │ │  CharacterCustomizationManager│
│  (Authority on Owned vs Locked│ │ (Authority on Appearance &    │
│   local prototype persistence)│ │  Preview Staging Lifecycle)   │
└───────────────────────────────┘ └───────────────┬───────────────┘
                                                  │
                                                  ▼
                                       ┌────────────────────────┐
                                       │    EquipmentSystem     │
                                       │  (Sole authority over  │
                                       │   combat & armor stats)│
                                       └────────────────────────┘
```
1. **Cosmetic Definition**: Defines metadata, slot, and spritesheet key (`CosmeticDefinition`).
2. **Cosmetic Ownership**: Governs whether player owns the cosmetic (`CosmeticsOwnershipManager`).
3. **Character Appearance**: Governs what the character visually displays (`CharacterAppearance`).
4. **Loadout**: Saved combination of cosmetic IDs and color variants (`AppearanceLoadout`).
5. **Equipment System**: Governs combat defense, speed penalties, and durability (`EquipmentSystem`).

---

## 6. Recommended Implementation Sequence

1. **Core Data Models (`src/cosmetics/`)**:
   - `CosmeticsRarity.hpp`: 6 rarity tiers, UI colors, sort weights.
   - `CosmeticsColorVariant.hpp`: Variant definitions, palette options.
   - `CosmeticsOwnership.hpp`: Ownership record structure, JSON serialization.
   - `CosmeticsOwnershipManager.hpp/.cpp`: Ownership query, prototype unlock, local persistence to `user_data/cosmetics/cosmetic_ownership.json`.
   - `CosmeticsSearchFilter.hpp/.cpp`: Filtering, text search, sorting algorithms.
   - `CosmeticsStoreManager.hpp/.cpp`: High-level store coordinator tying ownership, catalogue, preview, and loadouts together.
2. **Wardrobe & Customization Integration**:
   - Connect `CosmeticsOwnershipManager` to `CharacterCustomizationManager` so `EquipCosmetic` enforces ownership.
   - Add loadout ownership validation so loading presets skips/defaults unowned items.
3. **Player-Facing UI (`UIManager`)**:
   - Dedicated Store & Wardrobe screens with category navigation, catalogue grid, preview viewport, and detail panel.
   - Prototype unlock button and color variation selector.
   - Audio event dispatches on navigation and store interactions.
4. **Developer Tools (`DevUI`)**:
   - Cosmetics Store & Wardrobe Debugger in ImGui.
5. **Automated Testing (`tests/cosmetics/test_cosmetics_store.cpp`)**:
   - Complete 6-test suite covering ownership, catalogue search/filter, color variants, wardrobe equip rules, loadout ownership validation, and integration.
6. **Documentation & Verification**:
   - Complete all 13 documentation files in `docs/cosmetics/`.
   - Verify Debug and Release builds pass 28/28 CTest suites.

