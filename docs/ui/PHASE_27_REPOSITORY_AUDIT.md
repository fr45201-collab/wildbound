# Wildbound — Phase 27 Repository Audit
## HUD, Menus & Game UI

**Date**: 2026-11-05  
**Auditor**: Senior C++20 Gameplay Systems Architect  
**Subsystem**: Player-Facing UI, HUD, Menus & Navigation

---

## 1. Executive Summary

This repository audit evaluates the state of user interface components, gameplay data sources, input handling, and audio dispatch across Phases 18 through 26. The objective is to identify existing functional systems, reusable components, and architectural gaps to implement a unified, responsive, and visually consistent UI system in Phase 27.

The Wildbound engine features a custom pixel-art UI system built directly with C++20 and raylib 5.5, designed for an internal virtual canvas of **640x360** scaled with letterboxing to the window resolution (default 1280x720).

---

## 2. Audit of Existing Subsystems

### 2.1 Phase 18 — UI Foundation & Audio Architecture
* **`UIManager` (`src/ui/UIManager.hpp/.cpp`)**:
  - Implements the central `UIManager` class managing `UIScreen` modes (`None`, `MainMenu`, `CharacterSelect`, `Wardrobe`, `Store`, `Inventory`, `Settings`, `PauseMenu`, `MatchResults`, `GameOver`).
  - Provides primitive draw routines: `DrawPanel` (gold beveled border with corner studs), `DrawButton` (supporting primary, confirm, destructive, and tab styles with hover/pressed states), `DrawProgressBar`, and `DrawIcon`.
  - Integrated with `AudioSystem` via event dispatches (`UI.ButtonClick`, `UI.ButtonHover`, `UI.PanelOpen`, `UI.PanelClose`, `UI.TabSwitch`).
* **Asset Availability (`assets/ui/`)**:
  - Backgrounds: `bg_main_menu.png`, `bg_pause_menu.png`, `bg_settings.png`, `bg_inventory.png`, `bg_wardrobe.png`, `bg_character_select.png`, `bg_match_results.png`, `bg_game_over.png`.
  - 25 bespoke pixel icons: `icon_health.png`, `icon_stamina.png`, `icon_armor.png`, `icon_weapons.png`, `icon_inventory.png`, `icon_map.png`, `icon_controls.png`, `icon_graphics.png`, `icon_audio.png`, etc.
  - Sliceable panels and 9-patch frame textures: `panel_main_64.png`, `panel_dark_64.png`, `panel_hud_bar.png`, `panel_card_48.png`.

### 2.2 Phase 19 — Gameplay Integration & World Map
* **`World` (`src/world/World.hpp/.cpp`)**:
  - Holds active `Tilemap` instance with width, height, tile width (32px), tile height (32px), layers, and environment objects.
  - Exposes `GetWorldBounds()` returning the full pixel rectangle of the map (e.g., $100 \times 100 \times 32 = 3200 \times 3200$ px for Forest).
  - Exposes `GetVisibleObjects()` and `GetWorldItems()`.
* **Biome Indicators**: Active biome tracking in `BiomeManager` and `AudioSystem::GetCurrentBiome()`.

### 2.3 Phase 20 — Physics & Spatial Coordinates
* **`CollisionManager` (`src/physics/CollisionManager.hpp`)**:
  - Maintains spatial hashing grid for static world colliders and dynamic character colliders.
  - World coordinates: Origin $(0, 0)$ at top-left of the tilemap, $+X$ right, $+Y$ down.
  - Player position queryable via `player->GetPosition()`.

### 2.4 Phase 21 — Input Management
* **`InputManager` (`src/input/InputManager.hpp/.cpp`)**:
  - Centralized logical action mappings (`InputAction::MoveUp`, `MoveDown`, `MoveLeft`, `MoveRight`, `Attack`, `Interact`, `OpenInventory`, `OpenWardrobe`, `Pause`, `Confirm`, `Cancel`).
  - Context stack (`InputContextType::Gameplay`, `Menu`, `Inventory`, `Wardrobe`, `Settings`, `Paused`).
  - `InputSettings` with key/button rebinding and JSON persistence (`config/input_bindings.json`).
  - Full Gamepad support with analog sticks and buttons (`GamepadInput`).

### 2.5 Phase 22 — World Interaction System
* **`InteractionManager` (`src/interaction/InteractionManager.hpp/.cpp`)**:
  - Exposes `GetCurrentPrompt()` returning `InteractionPrompt` with `visible`, `keyGlyph`, `verb`, `targetName`, `feedbackMessage`, and `feedbackTimer`.
  - Already rendered in `UIManager::RenderGameplayHUD` above the hotbar.

### 2.6 Phase 23 — Loot & Inventory System
* **`Inventory` (`src/items/Inventory.hpp/.cpp`)**:
  - Fixed-slot container (default 24 slots) with `ItemInstance` structures.
  - Transactional operations: `SwapSlots`, `SplitStack`, `MergeStacks`, `MoveSlot`, `DropItem`, `DropAll`.
* **`ConsumableSystem` (`src/items/ConsumableSystem.hpp/.cpp`)**:
  - Enforces healing, stamina, and status constraints (`UseConsumable`).

### 2.7 Phase 24 — Weapons & Equipment System
* **`EquipmentSystem` (`src/entities/EquipmentSystem.hpp/.cpp`)**:
  - 8 canonical equipment slots: `MainHand`, `OffHand`, `Head`, `Chest`, `Legs`, `Feet`, `Face`, `Back`.
  - Active weapon switching (`SetActiveWeaponSlot`, `ToggleWeaponSlot`).
  - Aggregate defense rating (`GetTotalDefense()`) and damage mitigation (`GetDamageReduction()`).
* **`WeaponController` (`src/weapons/WeaponController.hpp/.cpp`)**:
  - Attack state machine: `Idle`, `Windup`, `Active`, `Recovery`, `Cooldown`.
  - Bow draw ratio tracking `[0.0 .. 1.0]`, dynamic charge timing.

### 2.8 Phase 25 & 26 — Character Customization, Store & Wardrobe
* **`CharacterCustomizationManager` & `CosmeticsStoreManager`**:
  - 98 catalog items across 6 rarity tiers.
  - Color variations, prototype unlocks, saved appearance presets (`AppearanceLoadout`).
  - `AppearancePreview` live doll rendering.

---

## 3. Gap Analysis & Missing Functionality

| HUD / Menu Feature | Current Repository State | Phase 27 Requirement |
|---|---|---|
| **Health Bar** | Instantaneous bar rendering in top-left | Add smooth damage lag indicator (delayed health drain) and low-health pulsing alert. |
| **Armor Indicator** | Numerical string in top-right | Create dedicated armor bar/shield gauge with mitigation percentage and equipped armor name. |
| **Weapon Interface** | Basic text box in bottom-left | Formalize dual weapon HUD slots (Primary/Secondary), category icons, active highlight, and bow draw charge bar. |
| **Ammunition Counter** | Simple string in weapon box | Dedicated arrow counter with reserve stack readout from inventory (`current / reserve`). |
| **Minimap** | **Missing** (No minimap component exists) | Implement high-performance minimap with world-to-minimap transform, terrain overview, player arrow, landmark icons, and zoom levels. |
| **Match Information** | Hardcoded victory screen only | Create match info HUD widget (match timer, alive count, eliminations) with clean prototype interface. |
| **Player Status** | Only stamina bar exists | Implement status effect indicator row (Normal, Low HP, Healing, Sprinting, Exhausted). |
| **Settings Menu** | Partial audio sliders only | Expand into multi-tab suite: Graphics (resolution, fullscreen, VSync), Audio (5 buses), Controls (rebinding table), Gameplay (toggles). |
| **Pause Menu** | Basic 4-button menu | Add Inventory, Character, and Settings sub-navigation with full input context preservation and gameplay simulation freeze. |
| **Main Menu** | Basic vertical button list | Add animated background support, navigation for Play, Character, Wardrobe, Store, Settings, Credits, and Exit. |
| **Inventory UI** | Basic 4-slot hotbar + grid mockup | Implement full 24-slot transactional inventory panel with tooltip inspections, split stack, merge, drop, and consumable use. |
| **Notification Queue** | Single prompt feedback string | Implement non-blocking notification toast queue with auto-fade, priority sorting, and audio triggers. |
| **Keyboard / Gamepad Navigation** | Mouse-only button clicking in UI | Implement focus manager allowing full menu navigation via Arrow Keys / WASD / Gamepad D-pad + Confirm (`Enter`/`A`) / Back (`Esc`/`B`). |
| **UI Debugger** | No UI debugging in `DevUI` | Add UI inspection tab in `DevUI` displaying screen state, focus, HUD data, and minimap coordinates. |

---

## 4. Dependencies & Implementation Risks

1. **Minimap Performance**:
   - *Risk*: Re-rendering world tiles every frame within the minimap viewport would degrade frame rates.
   - *Mitigation*: Cache the terrain map into a lightweight offscreen `RenderTexture` or generate a downsampled pixel representation, rendering only dynamic entities (player marker, landmarks) per frame.
2. **Dual-Input Synchronization**:
   - *Risk*: Conflicts between mouse hover and keyboard/gamepad focus.
   - *Mitigation*: Maintain an explicit `m_focusedElementIndex`. When mouse moves, focus follows mouse; when navigation keys are pressed, mouse hover is suppressed.
3. **Data Mutation Separation**:
   - *Risk*: UI accidentally modifying gameplay variables.
   - *Mitigation*: UI strictly consumes `const Player*`, `const World*`, `const ItemManager*`. All user actions execute via transactional gameplay APIs (`UseConsumable`, `DropItem`, `SwitchWeapon`).

---

## 5. Audit Conclusion

The repository provides all necessary backend systems, math primitives, and pixel-art assets. Phase 27 will establish a clean, modular UI component framework in `src/ui/` with dedicated modules for Core, Menus, HUD, Minimap, and Inventory, while preserving the existing 640x360 virtual canvas and retro pixel aesthetic.

