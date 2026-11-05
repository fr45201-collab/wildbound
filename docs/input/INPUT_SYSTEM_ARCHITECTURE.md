# Wildbound — Advanced Input System Architecture

## 1. System Overview

The **Input Management System** provides a centralized, hardware-abstracted, and context-aware control pipeline for Wildbound. It bridges physical hardware primitives (Keyboard, Mouse, Gamepad via raylib) with gameplay simulation logic through logical action mapping, buffering, modal context switching, and persistent user configuration.

```
+---------------------------------------------------------------------------------------+
|                                    HARDWARE LAYER                                     |
|  [Keyboard (Scan Codes)]     [Mouse (Buttons & Wheel)]     [Gamepad (Sticks & Triggers)]|
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
|                                     INPUT MANAGER                                     |
|  * Hardware Polling & Device Activity Tracking                                        |
|  * Screen -> Virtual Canvas (640x360) -> World Space Coordinate Transformations        |
|  * Gamepad Radial Deadzone Filtering & Normalization                                  |
|  * Analog Movement Vector Generation                                                  |
+------------------------------------------+--------------------------------------------+
                                           |
                    +----------------------+----------------------+
                    |                                             |
                    v                                             v
+---------------------------------------+     +-----------------------------------------+
|        INPUT CONTEXT MANAGER          |     |           INPUT BUFFER QUEUE            |
|  * Priority Context Stack             |     |  * Frame-independent Timestamping       |
|  * Modal Exclusivity & Bleed Blocking |     |  * Configurable Expiration Windows      |
|  * Active Mask Evaluation             |     |  * Single-consumption & Deduplication   |
+-------------------+-------------------+     +--------------------+--------------------+
                    |                                              |
                    +----------------------+-----------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
|                                LOGICAL ACTION LAYER                                   |
|   WasActionPressed()       IsActionHeld()       WasActionReleased()    GetActionValue() |
+------------------------------------------+--------------------------------------------+
                                           |
            +------------------------------+-----------------------------+
            |                              |                             |
            v                              v                             v
+-----------------------+      +-----------------------+     +-------------------------+
|  CharacterController  |      |   UIManager / Menus   |     |    DevUI / Tooling      |
| (Movement & Combat)   |      |  (Navigation & Modal) |     |  (Tab 14 Inspector)     |
+-----------------------+      +-----------------------+     +-------------------------+
```

---

## 2. Core Architectural Components

### 2.1 `InputManager` (`src/input/InputManager.hpp`, `src/input/InputManager.cpp`)
* **Role**: Primary singleton/facade instantiated in `Game`. Owns frame lifecycle, active device detection, mouse coordinate transformations, and simulated test adapters.
* **Hardware Interrogation**: Queries raylib's physical polling routines (`IsKeyPressed`, `IsKeyDown`, `GetGamepadAxisMovement`, etc.) exactly once per frame during `PollHardware()`.
* **Coordinate Conversion**:
  - `Screen Position`: Native OS window coordinates $[0 \dots \text{windowWidth}, 0 \dots \text{windowHeight}]$.
  - `Virtual Canvas`: Letterbox/pillarbox-adjusted virtual resolution $[0 \dots 640, 0 \dots 360]$ via `graphics::Viewport::ScreenToVirtual()`.
  - `World Position`: Virtual-to-world conversion via `graphics::Camera2DSystem::ScreenToWorld()`.

### 2.2 `InputAction` (`src/input/InputAction.hpp`)
* **Role**: Enumeration of all logical game actions categorized into `Movement`, `Combat`, `Inventory`, `Character`, `General`, `UI`, and `DevUI`.
* **Abstraction**: Systems never query physical keys (e.g. `KEY_W` or `KEY_SPACE`) directly; they query abstract actions (e.g. `InputAction::MoveUp`, `InputAction::Dodge`).

### 2.3 `InputBinding` (`src/input/InputBinding.hpp`)
* **Role**: Data structures defining the mapping between physical input devices (`Keyboard`, `Mouse`, `Gamepad`), binding source types (`ButtonOrKey`, `Axis`), keycodes/axes, and modifier masks (`Ctrl`, `Shift`, `Alt`).
* **Display Formatting**: `BindingToString()` translates bindings into localized UI glyph tags (e.g. `"W"`, `"Shift+Tab"`, `"MouseLeft"`, `"LS_Y-"`, `"A / Cross"`).

### 2.4 `InputSettings` (`src/input/InputSettings.hpp`, `src/input/InputSettings.cpp`)
* **Role**: Persistent configuration manager supporting primary and alternate bindings, runtime remapping state machine, conflict detection, and JSON serialization (`config/input_bindings.json`).

### 2.5 `InputBuffer` (`src/input/InputBuffer.hpp`)
* **Role**: Buffering queue that caches high-priority player commands (e.g., Attack, Heavy Attack, Dodge, Interact) across frame boundaries with duration windows (100–250 ms), allowing responsive combat responsiveness even under non-deterministic framerates.

### 2.6 `InputContext` (`src/input/InputContext.hpp`)
* **Role**: Priority-stacked input contexts (`Gameplay`, `Menu`, `Inventory`, `Wardrobe`, `Settings`, `Paused`) with modal exclusivity flags to prevent action bleed between game states (e.g. closing an inventory window with Escape will not trigger game pause).

### 2.7 `GamepadInput` (`src/input/GamepadInput.hpp`, `src/input/GamepadInput.cpp`)
* **Role**: Controller subsystem that normalizes twin-stick analog signals using continuous radial deadzones and remaps analog triggers with configurable deadband thresholds.

---

## 3. Data Flow & Integration Points

1. **`Game::ProcessInput()`**:
   - Updates `InputManager` with delta time $\Delta t$.
   - Synchronizes active `InputContext` based on current `UIManager` screen state.
   - Updates camera target and triggers pause/menu transitions via logical actions.
2. **`CharacterController::Update()`**:
   - Queries `inputMgr->GetMovementVector()` (combining analog sticks and digital directional bindings).
   - Consumes buffered combat inputs (`ConsumeActionIfBuffered(InputAction::Attack)`).
   - Evaluates sprint, dodge, and interaction states.
3. **`UIManager::Update()`**:
   - Consumes menu navigation actions (`InputAction::UINavUp`, `InputAction::UIConfirm`, `InputAction::UICancel`).
   - Uses virtual mouse coordinates to perform button and slot hover tests.
4. **`DevUI::RenderDevUI()`**:
   - Hosts Tab 14 ("Input & Controls"), providing live telemetry, deadzone tuning, and interactive rebinding tools.

