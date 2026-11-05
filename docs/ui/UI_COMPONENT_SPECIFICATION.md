# WILDBOUND — UI COMPONENT SPECIFICATION

## 1. Architecture & Rendering Pipeline

Player-facing user interface rendering in Wildbound is decoupled from developer overlays:
* **Player UI**: Implemented in C++ via `wildbound::ui::UIManager`. Rendered strictly inside `Renderer::BeginUI()` / `Renderer::EndUI()` targeting the $640 \times 360$ virtual canvas.
* **Coordinate Mapping**: Mouse coordinates from raylib `GetMousePosition()` (window space) are projected into canvas virtual space via `Viewport::ScreenToVirtual(mouseScreen)` before passing into `UIManager::Update()`.
* **Zero Input Bleed**: When `UIManager::IsScreenActive()` is true, gameplay movement and attacks are bypassed so mouse clicks and hotkeys navigate interface menus without triggering character movement or weapon attacks.

---

## 2. Component API Reference

### `DrawPanel(Rectangle bounds, const std::string& style = "main")`
Renders an ornate pixel-art beveled frame with gilded border highlights, corner studs, and semi-transparent dark backplate.
* **`bounds`**: Coordinates and size within the $640 \times 360$ canvas.
* **`style`**:
  * `"main"`: Midnight slate blue fill (`#121622`) with antique gold border (`#B49141`) and golden corner studs.
  * `"dark"`: Deep obsidian fill (`#0A0C12`) with muted steel-slate border (`#283041`) for inset containers.

### `DrawButton(Rectangle bounds, const std::string& text, const std::string& style = "primary", bool selected = false) -> bool`
Renders a button with 6-state interactivity and audio dispatch on hover and click.
* **Returns**: `true` on the single frame mouse button 0 is clicked over the button rectangle.
* **Audio Dispatch**: Automatically triggers `"UI.ButtonHover"` on initial hover and `"UI.ButtonClick"` on mouse release.
* **Styles**:
  * `"primary"`: Default dark slate with gold border.
  * `"confirm"`: Forest green with emerald border.
  * `"destructive"`: Maroon bloodiron with crimson border.
  * `"tab"`: Slim horizontal category tab; displays prominent gold line when `selected == true`.

### `DrawProgressBar(Rectangle bounds, float progress, Color fillColor, Color bgColor)`
Renders a health/stamina/volume progress meter with 1px border frame and clamped fill percentage ($[0.0, 1.0]$).

### `DrawIcon(const std::string& iconId, Vector2 pos, float size = 24.0f, Color tint = WHITE)`
Renders an interface icon glyph at the specified coordinates. Supports crisp vector-rendered glyphs or AMS texture sprites.

---

## 3. Screen State Machine

```
              [Launch Game]
                    │
                    ▼
          ┌──────────────────┐
          │  UIScreen::None  │◄────────────┐
          │  (Gameplay HUD)  │             │
          └────────┬─────────┘             │
                   │                       │
      [ESC]        │ [I/Tab]               │ [C]
        ┌──────────┼──────────────┐        │
        ▼          ▼              ▼        │
   ┌─────────┐ ┌───────────┐ ┌──────────┐  │
   │PauseMenu│ │ Inventory │ │ Wardrobe │  │
   └────┬────┘ └─────┬─────┘ └────┬─────┘  │
        │            │            │        │
        │ [Settings] │ [Back]     │ [Back] │
        ▼            └────────────┼────────┘
   ┌─────────┐                    │
   │Settings │────────────────────┘
   └─────────┘
```

* **Hotkey Bindings**:
  * `ESC`: Toggles Pause Menu when in-game, or returns to previous screen.
  * `I` / `Tab`: Toggles Inventory & Collectibles Screen.
  * `C`: Toggles Modular Wardrobe & Appearance Screen.
  * `F1`: Toggles DevUI Suite (Developer Tooling).

