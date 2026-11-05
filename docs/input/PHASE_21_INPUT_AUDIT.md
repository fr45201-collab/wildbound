# Phase 21: Advanced Input Management System — Pre-Implementation Audit

**Project**: Wildbound — 2D Top-Down Fantasy Battle Royale  
**Engineering Discipline**: Senior C++20 Gameplay Systems Engineer  
**Date**: November 2026  
**Status**: Pre-Implementation Audit Complete  

---

## 1. Executive Summary & Purpose

The objective of Phase 21 is to design and implement a centralized, extensible input management system for Wildbound. Prior to this phase, physical hardware inputs (keyboard and mouse) were queried directly by gameplay classes, controller scripts, and UI systems via raw raylib API calls (`IsKeyPressed`, `IsKeyDown`, `GetMousePosition`, `IsMouseButtonDown`).

This audit inspects the current repository state, catalogues all direct input dependencies, analyzes coordinate transformation pipelines, identifies missing capabilities, highlights architectural risks, and outlines an incremental migration plan to ensure zero gameplay regressions.

---

## 2. Verification of Existing Subsystems

Before designing the input architecture, previous phases were verified against the live source tree:
* **C++20 & Build Environment**: MSVC 19.44, CMake 3.20+, CTest with 22/22 active test suites passing in both Release and Debug mode.
* **Phase 19 Integration Status**: Verified. Full inventory (`Inventory`, `ItemInstance`), dual melee/ranged combat (`ProjectileSystem`, `CombatSystem`), equipment mitigation (`EquipmentSystem`), and player-facing HUD (`UIManager`) are implemented and active in `Game.cpp`.
* **Phase 20 Physics Status**: Verified. `CollisionManager` with 10 logical layers, bilateral mask filtering, stateful `TriggerVolume`, deferred mutations, and DevUI Tab 13 are operational and verified by `PhysicsCollisionTest`.
* **Input Directory (`src/input/`)**: The directory `src/input/` exists in the filesystem but is currently empty (0 files). No `InputManager`, `InputAction`, or context management classes exist.

---

## 3. Direct Input Polling Audit

An exhaustive scan for raylib input calls (`IsKey*`, `IsMouse*`, `GetMouse*`, `IsGamepad*`) reveals significant input fragmentation across multiple gameplay classes:

### 3.1 `src/core/Game.cpp`
* **Direct Keyboard Checks**:
  - `KEY_ESCAPE` (lines 326, 436): Dual polling for pause menu toggles and exit conditions.
  - `KEY_F1` – `KEY_F7` (lines 349–391): Developer diagnostic and debug overlays.
  - `KEY_R` (line 398): Player and camera respawn trigger.
  - `KEY_I`, `KEY_TAB` (line 414): Inventory toggle.
  - `KEY_C` (line 425): Wardrobe/Cosmetics toggle.
  - `KEY_ONE` – `KEY_FOUR` (lines 467–470): Hotbar slot usage.
  - `KEY_E`, `KEY_F` (line 474): Building entrance/exit and chest looting.
  - `KEY_W`, `KEY_S`, `KEY_A`, `KEY_D` (lines 506–509): Free camera panning.
  - `KEY_Q`, `KEY_E` (lines 517, 520): Camera zoom in/out.
* **Direct Mouse Checks**:
  - `GetMousePosition()` (lines 534, 702): Screen-space pointer coordinates.
  - `IsMouseButtonPressed(MOUSE_BUTTON_LEFT)` (line 536): UI click detection.
  - `IsMouseButtonDown(MOUSE_BUTTON_LEFT)` (line 537): UI drag/down state.

### 3.2 `src/entities/CharacterController.cpp`
* **Direct Keyboard Checks**:
  - `KEY_W`, `KEY_UP`, `KEY_S`, `KEY_DOWN`, `KEY_A`, `KEY_LEFT`, `KEY_D`, `KEY_RIGHT` (lines 33–36): Movement direction accumulation.
  - `KEY_LEFT_SHIFT`, `KEY_RIGHT_SHIFT` (line 39): Sprint toggle.
  - `KEY_SPACE` (line 42): Melee/ranged attack request.
  - `KEY_ESCAPE` (line 45): Pause request.
* **Direct Mouse Checks**:
  - `IsMouseButtonPressed(MOUSE_BUTTON_LEFT)` (line 42): Attack request trigger.

### 3.3 `src/ui/UIManager.cpp`
* **Direct Keyboard Checks**:
  - `KEY_ESCAPE` (line 65): Screen dismissal / pause menu navigation.
  - `KEY_I`, `KEY_TAB` (line 78): Inventory screen toggle.
  - `KEY_C` (line 88): Wardrobe screen toggle.
* **Direct Mouse Handling**:
  - Relies on parameters passed from `Game.cpp`: `mouseVirtual`, `mouseLeftPressed`, `mouseLeftDown`.

---

## 4. Mouse Coordinate Transformation Audit

Wildbound renders internally to a fixed virtual canvas ($640 \times 360$) and presents it scaled point-filtered (typically $2\times$ to $1280 \times 720$ with letterboxing if resized).

Currently, mouse coordinates are converted through two distinct stages:
1. **Screen (OS Window) $\to$ Virtual Canvas**:
   Handled by `graphics::Viewport::ScreenToVirtual(Vector2 screenPos)`:
   $$\text{virtualX} = (\text{screenX} - \text{destRect.x}) \times \frac{\text{virtualWidth}}{\text{destRect.width}}$$
   $$\text{virtualY} = (\text{screenY} - \text{destRect.y}) \times \frac{\text{virtualHeight}}{\text{destRect.height}}$$
   This correctly accounts for pillarboxing/letterboxing aspect ratio offsets.
2. **Virtual Canvas $\to$ World Space**:
   Handled by `graphics::Camera2DSystem::ScreenToWorld(Vector2 screenPos, const Viewport& viewport)`:
   Converts screen to virtual coordinates first, then invokes raylib's `GetScreenToWorld2D(virtualPos, m_camera)`.

### Findings:
* UI elements (`UIManager`, `HUD`, inventory grid) require **Virtual Canvas coordinates**.
* In-world interactions (entity picking, ground targeting, directional aiming) require **World Space coordinates**.
* The new `InputManager` must centrally cache and expose all three coordinate spaces (`Screen`, `Virtual`, and `World`) each frame, eliminating redundant transformations across callers.

---

## 5. Gamepad Support Status

* **Current Implementation**: Zero gamepad support exists. Neither `CharacterController`, `Game`, nor `UIManager` query raylib gamepad functions.
* **Raylib Gamepad Capabilities**: raylib 5.5 exposes complete gamepad APIs:
  - Detection: `IsGamepadAvailable(int gamepad)`, `GetGamepadName(int gamepad)`.
  - Buttons: `IsGamepadButtonPressed`, `IsGamepadButtonDown`, `IsGamepadButtonReleased`.
  - Axes: `GetGamepadAxisMovement`, `GetGamepadAxisCount`.
* **Requirements**:
  - Support standard Xbox / DualShock / generic HID controller layouts.
  - Implement radial deadzones with normalization to prevent stick drift.
  - Support simultaneous keyboard/mouse and gamepad input with active device auto-detection.

---

## 6. Gap Analysis & Missing Capabilities

| Capability | Current State | Target State in Phase 21 |
| :--- | :--- | :--- |
| **Input Architecture** | Fragmented direct raylib calls | Centralized `InputManager` singleton or engine-owned subsystem |
| **Action Abstraction** | Hardcoded key codes (`KEY_W`, etc.) | Logical `InputAction` enum with multi-device bindings |
| **Input Contexts** | Ad-hoc `uiBlocksGameplay` boolean | Formal `InputContext` stack (`Gameplay`, `Menu`, `Inventory`, `Wardrobe`, `Settings`, `Paused`) |
| **Input Buffering** | None (missed frames drop attacks) | Configurable `InputBuffer` (e.g. 120ms attack window, 150ms interact) |
| **Rebinding** | None (hardcoded) | Runtime rebinding API with conflict detection |
| **Persistence** | None | JSON serialization to `config/input_bindings.json` |
| **Analog Processing**| Digital only | Analog stick normalization, radial deadzones, trigger thresholds |
| **Developer Tools** | None | Dear ImGui DevUI Tab 14 ("Input Debugger") |

---

## 7. Architectural Risks & Mitigation

1. **Double-Polling & Race Conditions**:
   - *Risk*: `KEY_ESCAPE` is polled independently in `Game::ProcessInput` and `UIManager::Update`, creating fragile ordering dependencies where one system consumes or resets state unexpectedly.
   - *Mitigation*: Only `InputManager` polls raylib. Subsystems query `InputManager`, and the active `InputContext` strictly regulates action consumption.
2. **Input Bleed Across Contexts**:
   - *Risk*: Clicking on an inventory slot or pressing `Space` in a modal screen could trigger an in-game sword slash.
   - *Mitigation*: When an exclusive context (e.g. `Inventory` or `Menu`) is at the top of the context stack, `Gameplay` actions are blocked, and input buffer queues for gameplay actions are cleared on transition.
3. **Alt-Tab / Focus Loss**:
   - *Risk*: Releasing a key while the game window is unfocused can cause the held state to remain permanently `true`.
   - *Mitigation*: Implement `InputManager::Reset()` triggered when window focus is lost (`IsWindowFocused() == false`).
4. **Performance & Allocations**:
   - *Risk*: Allocating strings or dynamic maps every frame during action polling.
   - *Mitigation*: Use flat arrays indexed by strongly-typed `InputAction` enums with pre-allocated binding structures.

---

## 8. Migration Plan

```
Phase 21 Implementation Roadmap:
  [Step 1: Core Action & Binding Definitions] (InputAction.hpp, InputBinding.hpp)
                          │
                          ▼
  [Step 2: Low-Level Device Handlers] (Keyboard, Mouse, Gamepad with deadzones)
                          │
                          ▼
  [Step 3: Coordinate Conversion Integration] (Screen <-> Virtual <-> World)
                          │
                          ▼
  [Step 4: Input Buffering Subsystem] (Configurable action queues & timeouts)
                          │
                          ▼
  [Step 5: Context Management Architecture] (Priority stack, exclusivity rules)
                          │
                          ▼
  [Step 6: Central InputManager Implementation] (Central tick, device auto-detect)
                          │
                          ▼
  [Step 7: Rebinding & JSON Persistence] (config/input_bindings.json)
                          │
                          ▼
  [Step 8: Gameplay & UI Integration] (CharacterController, Game, UIManager)
                          │
                          ▼
  [Step 9: DevUI Tab 14 — Input Debugger] (Live key states, axes, contexts, buffer)
                          │
                          ▼
  [Step 10: Automated CTest & Verification] (23 test suites, 100% pass)
```

**Pre-Implementation Audit Sign-Off**: PASSED. No blockers identified. Architecture is ready for implementation.

