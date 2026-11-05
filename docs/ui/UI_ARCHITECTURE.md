# Wildbound UI Architecture Specification

## 1. Architectural Overview

Wildbound utilizes a unified, layered, virtual-canvas user interface architecture built on top of C++20 and raylib 5.5. The system decouples the logical presentation of UI elements from underlying window and render target resolutions through a fixed $640 \times 360$ virtual canvas projection.

The architecture strictly segregates player-facing UI (driven by `wildbound::ui::UIManager` and rendered directly via raylib primitives and textured quads) from internal developer diagnostics (driven by Dear ImGui inside `wildbound::ui::DevUI`).

```
+-----------------------------------------------------------------------------------+
|                                  Game Loop                                        |
|                          (src/core/Game.cpp)                                      |
+-----------------------------------------------------------------------------------+
         |                                                 |
         | Updates Simulation & UI Input                   | Renders World & Virtual Canvas
         v                                                 v
+-----------------------------+                 +-----------------------------------+
|   wildbound::ui::UIManager  |                 |    graphics::Renderer (Canvas)    |
+-----------------------------+                 +-----------------------------------+
  |           |          |                        |  Fixed Virtual: 640 x 360       |
  | State     | Event    | Subsystem Render       |  Window Target: 1280 x 720 (x2) |
  v           v          v                        +---------------------------------+
[Screen]  [AudioSys]   [Subsystems]                                |
  |                      |-- HUDManager (Vitals, Ammo, Radar)      |
  |                      |-- InventoryUI (24 slots, Inspect)       |
  |                      |-- MainMenu (Play, Wardrobe, Settings)   |
  |                      |-- SettingsMenu (Audio, Video, Controls) |
  |                      |-- PauseMenu (Modal game pause)          |
  |                      \-- NotificationQueue (Toasts)            v
  +---------------------------------------------------> [RenderTexture2D / Window Frame]
```

---

## 2. Coordinate Systems & Virtual Canvas Scaling

### 2.1 Virtual Canvas Dimensions
- **Reference Resolution**: $640 \times 360$ pixels ($16:9$ aspect ratio).
- **Default Display Window**: $1280 \times 720$ pixels (an exact $2\times$ integer scaling).
- **Pixel Density & Filtering**: Point/Nearest-Neighbor sampling ensures crisp pixel art rendering without blur or bilinear bleeding across UI widgets.

### 2.2 Screen-to-Virtual Coordinate Mapping
Mouse input coordinates from the operating system / GLFW window layer ($[0..W_{window}], [0..H_{window}]$) are normalized and mapped to virtual canvas space:

$$\mathbf{p}_{virtual} = \left( \frac{\mathbf{p}_{window}.x - \text{offset}_x}{\text{scale}}, \frac{\mathbf{p}_{window}.y - \text{offset}_y}{\text{scale}} \right)$$

where $\text{scale} = \min(W_{window} / 640, H_{window} / 360)$, and offsets account for letterboxing/pillarboxing black bars when non-$16:9$ aspect ratios are used.

`wildbound::ui::UITheme` provides centralized access to screen anchors, dimensions, and layout constants:

```cpp
namespace wildbound::ui {
    constexpr float VIRTUAL_WIDTH = 640.0f;
    constexpr float VIRTUAL_HEIGHT = 360.0f;
}
```

---

## 3. UI State Machine & Screen Management

The top-level user interface lifecycle is governed by `wildbound::ui::UIScreenState`:

| Screen State | Description | Simulation Active | Visible Components |
|---|---|---|---|
| `None` | Pure gameplay immersion | Yes | HUDManager, Toast Notifications |
| `MainMenu` | Title screen, match launch, wardrobe entrance | No | MainMenu background, menu list, modal dialogs |
| `PauseMenu` | Gameplay paused, modal overlay | No | Frozen game background, HUD (dimmed), Pause dialog |
| `SettingsMenu` | Configuration panel (Video, Audio, Keys, Game) | No | Settings tabs, sliders, keybind table |
| `Inventory` | Player inventory & equipment inspection | Yes / Contextual | HUD, 24-slot inventory grid, item tooltip |
| `CosmeticsStore` | Wardrobe and character customization interface | No | 3D/2D preview mannequin, item catalogue |
| `Credits` | Attribution dialog modal | No | Scrolled credits panel |

### State Transitions and Previous State Stack
The UI manager maintains state transition history via `m_previousState`:
- Opening `SettingsMenu` from `PauseMenu` stores `PauseMenu` as the return destination.
- Opening `SettingsMenu` from `MainMenu` stores `MainMenu` as the return destination.
- Pressing `Escape` or the gamepad B button calls `NavigateBack()`, cleanly restoring previous focus and context without hardcoding screen relationships.

---

## 4. UI Layering and Composition Hierarchy

UI rendering occurs within the final pass of the game frame, drawn strictly on top of tilemaps, particles, and entity sprites:

```
[Layer 0] World Tilemaps & Ground Loot
[Layer 1] Entity Sprites (Player, Enemies, Projectiles)
[Layer 2] Weather & Lighting Overlay
[Layer 3] Gameplay HUD (Minimap, Vitals, Ammo, Match State)
[Layer 4] Modal Menus (PauseMenu, SettingsMenu, InventoryUI)
[Layer 5] Notification Queue (Non-blocking Toast Overlays)
[Layer 6] Cursor / Reticle
```

When modal menus are opened (e.g., `PauseMenu`), a dimming backdrop rectangle is rendered over Layer 3:
```cpp
DrawRectangle(0, 0, (int)VIRTUAL_WIDTH, (int)VIRTUAL_HEIGHT, Color{ 0, 0, 0, 160 });
```
This ensures high visual contrast and guides user focus to active dialogue windows.

---

## 5. Input Routing Architecture

The `wildbound::ui::UIManager` serves as an input filter that intercepts navigation, keyboard keystrokes, and gamepad events:

1. **Context Interception**: If `m_currentState != UIScreenState::None`, the UI consumes directional inputs (`UI_UP`, `UI_DOWN`, `UI_LEFT`, `UI_RIGHT`), primary action (`UI_SELECT` / Enter / Space / Gamepad A), and back (`UI_CANCEL` / Escape / Gamepad B).
2. **Focus Management**: Active menus track `m_focusedIndex` or grid coordinates `(slotX, slotY)`. Directional inputs cleanly increment or decrement focus with bounds clamping.
3. **Mouse / Touch Parity**: Hovering over interactive widgets automatically synchronizes `m_focusedIndex`, preventing discrepancies between mouse and gamepad navigation.

---

## 6. Decoupling from Gameplay Logic

The UI system follows a **Data-Consumer** architectural pattern:
- **Zero Simulation Ownership**: The UI neither mutates combat values nor owns item state directly.
- **Read-Only Inspection**: Health, armor mitigation, ammo counts, and cooldowns are polled from `HealthComponent`, `EquipmentSystem`, and `WeaponManager`.
- **Transactional Actions**: Operations like dropping items or using potions call explicit transactional APIs:
  ```cpp
  player->GetInventory().DropItem(slotIndex, quantity, *world, playerPos, playerFacing, audioSys);
  ```
  The UI reflects the return status via Toast notifications rather than predicting mutations locally.

