# Wardrobe & Store Interface Specification (Phase 26)

## 1. Canvas Layout & Virtual Geometry

The player-facing interface is rendered through `UIManager::RenderWardrobe` and `UIManager::RenderStore` within the canonical **640x360 virtual canvas**, scaling cleanly to any target window resolution.

```
+-------------------------------------------------------------------------------+
| [BACK]    COSMETICS STORE & WARDROBE                [ STORE ]  [ WARDROBE ]   |
+-------------------------------------------------------------------------------+
| [Featured] [All] [Hair] [Clothing] [Armor] [Head] [Face] [Packs] [Boots] [Loadouts] |
+-------------------------------------------------------+-----------------------+
|  CATALOGUE GRID / LIST (350x252)                      | CHARACTER PREVIEW     |
|                                                       |        (222x252)      |
|  +-------------------------------------------------+  |   [<]  DOWN  [>]      |
|  | [Epic] Royal Cloak                [LOCKED]     |  |   [ANIM: IDLE]        |
|  +-------------------------------------------------+  |                       |
|  | [Rare] Iron Cuirass               [OWNED]      |  |      (Character       |
|  +-------------------------------------------------+  |      Doll Model)      |
|  | [Uncommon] Forest Tunic           [EQUIPPED]   |  |                       |
|  +-------------------------------------------------+  | --------------------- |
|  | [Common] Leather Boots            [OWNED]      |  | Royal Cloak (Epic)    |
|  +-------------------------------------------------+  | Color: [< Crimson >]  |
|  | ...                                             |  | [PREVIEW] [UNLOCK]    |
|                                                       | --------------------- |
|  [^ Page Up]                       [v Page Down]     | [CONFIRM]  [RESET]    |
+-------------------------------------------------------+-----------------------+
```

---

## 2. Visual Component Styles

### 2.1 Outer Border & Frame
- Renders with double-beveled gold corner studs and parchment-style panels (`DrawPanel(..., "main")`).
- Left list and right character box render with inset dark slate panels (`DrawPanel(..., "dark")`).

### 2.2 Item Card Presentation
- **Dimensions**: `300 x 24` pixels.
- **Background**: Slate charcoal (`#161C2A`) with hover brightening.
- **Rarity Border**: Highlighted with `RarityToBorderColor(item->rarity)`.
- **Status Pills**:
  - `[EQUIPPED]`: Radiant gold (`#F5D75F`), confirm style.
  - `[OWNED]`: Mint green (`#46D264`).
  - `[LOCKED]`: Amber orange (`#E69628`).

### 2.3 Character Preview Viewport
- **Viewport Dimension**: `222 x 252` pixels.
- **Controls**:
  - `Rotate Left (<)`: Rotates character 90 degrees counter-clockwise (Down $\to$ Right $\to$ Up $\to$ Left).
  - `Rotate Right (>)`: Rotates character 90 degrees clockwise.
  - `Animation Toggle`: Switches between `ANIM: IDLE` and `ANIM: WALK`.
- **Selected Item Details & Actions**:
  - Displays cosmetic name in its rarity color.
  - Shows slot category and rarity name.
  - If locked: displays `[PREVIEW]` and `[UNLOCK (PROTOTYPE)]` buttons.
  - If owned and not equipped: displays `[EQUIP]` button.
  - If owned and equipped: displays `[UNEQUIP]` button.
- **Bottom Lifecycle Controls**:
  - `[CONFIRM]`: Commits previewed appearance to authoritative player character and saves loadouts.
  - `[RESET]`: Reverts preview character to original appearance.
  - `[BACK]`: Cancels unconfirmed preview changes and closes interface.

