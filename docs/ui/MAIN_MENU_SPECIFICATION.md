# Main Menu Specification

## 1. Overview & Purpose

The Main Menu (`wildbound::ui::MainMenu`) is the primary landing screen for players launching Wildbound. It establishes the fantasy battle royale aesthetic, provides seamless entry into gameplay matches, facilitates character customization via the Wardrobe, offers access to settings, and displays game credits.

---

## 2. Visual Layout & Screen Geometry

The layout is positioned within the virtual $640 \times 360$ canvas, centered around an asymmetric two-column composition:
- **Left Column**: Title logo, game branding, and primary action menu buttons.
- **Right Column**: Hero character preview / battle royale silhouette and patch notes/version banner.
- **Footer**: Copyright notice, build hash, engine version (`Wildbound v0.27.0-dev`).

```
+-----------------------------------------------------------------------------------+
|  [VIRTUAL CANVAS: 640 x 360]                                                      |
|                                                                                   |
|    # WILDBOUND #                                  +--------------------------+    |
|    Battle Royale                                  |                          |    |
|                                                   |   Hero Mannequin /       |    |
|    +-----------------------------+                |   Atmospheric Silhouette |    |
|    | [ Play Match ]              |                |                          |    |
|    | [ Character Wardrobe ]      |                |   Season 1: The Wild     |    |
|    | [ Cosmetics Store ]         |                |                          |    |
|    | [ Settings ]                |                +--------------------------+    |
|    | [ Credits ]                 |                                                |
|    | [ Exit Game ]               |                                                |
|    +-----------------------------+                                                |
|                                                                                   |
|  Wildbound Engine (C++20/raylib)                                Build: 2026.11.05 |
+-----------------------------------------------------------------------------------+
```

### Metrics & Coordinates
- **Menu Container**:
  - Origin: $X = 60, Y = 120$
  - Width: $200\text{ px}$
  - Item Height: $28\text{ px}$
  - Item Spacing: $6\text{ px}$
- **Title Block**:
  - Text: `"WILDBOUND"` (Gold `#D4AF37`, Font Size: $32\text{ px}$, Beveled Drop Shadow)
  - Subtitle: `"A FANTASY BATTLE ROYALE"` (Light Gray `#C8C8C8`, Font Size: $12\text{ px}$)
  - Position: $X = 60, Y = 50$

---

## 3. Menu Options & Functionality

| Menu Item | Action / Destination | Description |
|---|---|---|
| **Play Match** | `UIManager::SetScreenState(UIScreenState::None)` | Transitions directly into the active match loop, unfreezes simulation, and activates HUD. |
| **Character Wardrobe** | `UIManager::SetScreenState(UIScreenState::CosmeticsStore)` | Opens the Phase 26 Wardrobe interface for cosmetic customization. |
| **Cosmetics Store** | `UIManager::SetScreenState(UIScreenState::CosmeticsStore)` | Direct link to cosmetic unlocks and equipment previews. |
| **Settings** | `UIManager::SetScreenState(UIScreenState::SettingsMenu)` | Opens the unified Settings modal dialog. |
| **Credits** | `m_showCredits = true` | Toggles the in-engine scrolling credits modal window. |
| **Exit Game** | `Game::RequestShutdown()` | Terminates GLFW window and Raylib context cleanly with return code 0. |

---

## 4. Navigation & State Handling

### 4.1 Focus Management
- Focus is tracked via `m_selectedItemIndex` $\in [0..5]$.
- Directional Input:
  - `UI_UP` / `Key_Up` / `Gamepad_DpadUp`: `m_selectedItemIndex = (m_selectedItemIndex - 1 + count) % count`
  - `UI_DOWN` / `Key_Down` / `Gamepad_DpadDown`: `m_selectedItemIndex = (m_selectedItemIndex + 1) % count`
- Primary Action:
  - `UI_SELECT` / `Key_Enter` / `Key_Space` / `Gamepad_A`: Triggers selected item action.
  - Dispatches audio event `ui.button.click`.

### 4.2 Mouse Interaction
- Each button bounds rectangle is tested against virtual mouse coordinates:
  ```cpp
  bool hovered = CheckCollisionPointRec(mousePos, btnRect);
  if (hovered) {
      m_selectedItemIndex = i;
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          TriggerAction(i);
      }
  }
  ```
- Hover state renders a subtle gold left border accent (`#D4AF37`, 3px wide) and dark beveled background highlight.

---

## 5. Credits Modal Specification

When `Credits` is activated, an overlay dialog is rendered:
- **Dialog Rect**: Centered at $(X = 140, Y = 50)$, Dimensions: $360 \times 260\text{ px}$.
- **Background**: Dark charcoal `#1A1A1E` with beveled border `#3A3A42`.
- **Content**:
  - Development Team: Gameplay Architecture, Combat Systems, UI/UX Systems.
  - Technology: raylib 5.5, Dear ImGui, sol2, Lua 5.4, nlohmann/json.
  - Close Prompt: `[Press ESC or B to Close]`.
- Pressing `Escape` or clicking outside dismisses the modal and restores menu focus.

