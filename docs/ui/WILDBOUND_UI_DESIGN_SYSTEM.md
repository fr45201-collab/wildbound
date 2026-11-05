# WILDBOUND — UI DESIGN SYSTEM SPECIFICATION

## 1. System Vision & Visual Direction

Wildbound uses a dedicated, retro-fantasy pixel-art UI design system tailored specifically for a top-down Battle Royale. The design harmonizes pixel-art visual aesthetics with modern user experience (UX) responsiveness, accessibility, and high readability.

### Virtual Resolution & Canvas Scaling
* **Internal / Virtual Canvas Resolution**: $640 \times 360$ pixels ($16:9$ aspect ratio).
* **Target Presentation Window Resolution**: $1280 \times 720$ pixels (default $2\times$ integer scale).
* **Window Scaling & Filtering**: Pixel-perfect point sampling managed by `graphics::Viewport` with integer-scaling letterbox preservation on non-standard aspect ratios.
* **Separation of Concerns**:
  * **Player-Facing UI**: Rendered entirely through raylib and `wildbound::ui::UIManager` at the $640 \times 360$ virtual canvas resolution.
  * **Developer Tooling (DevUI)**: Managed strictly via Dear ImGui and `rlImGui` on the overlay display plane for inspection, debugging, and live tweaking.

---

## 2. Color Palette & Typography

The UI color palette utilizes rich, earthy fantasy tones inspired by iron, worn leather, weathered parchment, polished gold trim, and celestial magic.

| Token | Hex / RGB | Description | Usage |
|---|---|---|---|
| `UI_BG_DARK` | `#0A0D12` / `(10, 13, 18)` | Deep obsidian night | Panel background fill, modal backdrops |
| `UI_BG_PANEL` | `#121622` / `(18, 22, 34)` | Midnight slate blue | Standard frame content background |
| `UI_BORDER_GOLD` | `#B49141` / `(180, 145, 65)` | Antique gilded gold | Primary panel borders, ornate frames |
| `UI_HIGHLIGHT_GOLD` | `#FFE68C` / `(255, 230, 140)` | Radiant leaf gold | Top-left bevel highlights, corner studs |
| `UI_BUTTON_NORMAL` | `#20283C` / `(32, 40, 60)` | Tempered steel slate | Default interactive button background |
| `UI_BUTTON_HOVER` | `#39486A` / `(57, 72, 106)` | Elevated slate sheen | Mouse hover highlight |
| `UI_BUTTON_PRESSED` | `#10141E` / `(16, 20, 30)` | Depressed inset tone | Mouse pressed active state |
| `UI_TEXT_PRIMARY` | `#F5F5FF` / `(245, 245, 255)` | Pure starlight white | Main labels, dialog titles, prominent text |
| `UI_TEXT_MUTED` | `#A0B4D2` / `(160, 180, 210)` | Weathered parchment dusk | Tooltips, category headers, sub-text |
| `UI_BAR_HEALTH` | `#DC2D37` / `(220, 45, 55)` | Crimson vitality | HP progress bar fill |
| `UI_BAR_STAMINA` | `#EBC34B` / `(235, 195, 75)` | Amber vigor | Stamina progress bar fill |
| `UI_CONFIRM_GREEN` | `#193C23` / `(25, 60, 35)` | Emerald forest moss | Confirm / Positive actions |
| `UI_DESTRUCT_RED` | `#3C191E` / `(60, 25, 30)` | Bloodiron maroon | Destructive / Cancel / Exit actions |

---

## 3. UI Primitives & Component Hierarchy

### Nine-Slice & Scalable Panels
The panel framework supports five standard archetypes:
1. `panel_main_64.png` ($64 \times 64$ px, 16px corner margins): Used for prominent root dialog windows (Main Menu, Character Select, Wardrobe, Inventory).
2. `panel_dark_64.png` ($64 \times 64$ px, 16px corner margins): Inset containers for character pedestals, inventory grids, and stat cards.
3. `panel_tooltip_32.png` ($32 \times 32$ px, 8px corner margins): Lightweight floating overlays for item descriptions and controls hints.
4. `panel_card_48.png` ($48 \times 48$ px, 12px corner margins): Modular display cards for cosmetic slots and item details.
5. `panel_hud_bar.png`: Streamlined banner panel for combat status tracking (HP, SP, active biome).

### Button States & Interactive Transitions
All buttons implement six consistent interactive states:
* **Normal**: Base slate fill with muted antique gold border.
* **Hover**: Brightened interior fill (+25 RGB values), glowing top-left golden bevel line, and dispatches `"UI.ButtonHover"` sound.
* **Pressed**: Darkened interior fill (-20 RGB values), 1px coordinate offset toward bottom-right to simulate mechanical physical depth.
* **Disabled**: Desaturated charcoal fill with dimmed border, ignoring pointer clicks.
* **Selected**: Vivid gold border ($2.0\text{px}$ line weight) denoting active tab or item selection.
* **Focused**: Subtle gold corner indicator highlighting keyboard/gamepad focus.

---

## 4. Player-Facing Screen Layouts

1. **Main Menu (`UIScreen::MainMenu`)**:
   * Centered game logo with drop-shadow effect.
   * Action column featuring: Play Game, Character Select, Wardrobe, Inventory, Settings, Exit.
   * Plays `"mus_main_menu"` in a procedural loop.
2. **Character Select (`UIScreen::CharacterSelect`)**:
   * Central dark pedestal preview with arrow navigators.
   * Modular character rendering showing current appearance and active costume layers.
   * "Confirm Select" button triggering transition to battle.
3. **Wardrobe (`UIScreen::Wardrobe`)**:
   * Left category tabs covering 8 Phase 17 cosmetic types: Clothing, Armor, Pants, Shoes, Hats, Face, Back, Variations.
   * Scrollable cosmetic selection list with instant real-time live preview on player model.
4. **Inventory (`UIScreen::Inventory`)**:
   * 6 Category tabs filtering the 40 Phase 16 items: All, Collectibles, Consumables, Crafting Materials, Resources, Special Items.
   * $6 \times 4$ slot inventory grid with item icons and selection indicators.
   * Right details panel showing item name, rarity tier, icon preview, and lore description.
5. **Settings (`UIScreen::Settings`)**:
   * Interactive volume sliders for 6 audio buses: Master, Music, SFX, UI, Environment, Voice.
   * Instant slider updates with live audio feedback.
   * "Mute All" toggle and "Save & Back" button writing to disk.
6. **Pause Menu (`UIScreen::PauseMenu`)**:
   * Frosted semi-transparent dark overlay preserving combat situation backdrop.
   * Quick resume, settings access, and return to main menu.
7. **Gameplay HUD (`UIScreen::None`)**:
   * Top-left: Health bar ($90\%$ default) and Stamina bar ($75\%$ default).
   * Top-right: Active Biome indicator displaying dynamic environment status.
   * Bottom-center: 4-slot quickbar with slot numbers.
8. **Match Results & Game Over (`UIScreen::MatchResults` & `UIScreen::GameOver`)**:
   * Stat breakdown: eliminations, damage dealt, survival time, final placement.
   * Play Again / Return to Menu buttons.

