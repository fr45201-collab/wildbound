# Minimap Architecture Specification

## 1. Overview & Mechanics

The Minimap Radar (`wildbound::ui::Minimap`) provides real-time spatial awareness of the battle royale terrain, player orientation, ground loot drops, and world boundaries.

It is positioned in the Upper-Right region $(X = 516, Y = 60)$, rendering within a circular/beveled square viewport of $108 \times 108\text{ pixels}$.

---

## 2. World-to-Minimap Coordinate Transformation

The minimap operates in an egocentric (player-centered) frame of reference with configurable zoom scaling:

$$\mathbf{p}_{minimap} = \mathbf{c}_{center} + \left( \frac{\mathbf{p}_{world} - \mathbf{p}_{player}}{R_{world}} \cdot R_{radar} \right)$$

where:
- $\mathbf{c}_{center} = (X_{center}, Y_{center})$ is the visual center of the minimap box ($(570, 114)$ in virtual coordinates).
- $R_{radar} = 50.0\text{ px}$ (the visual radius of the minimap radar circle).
- $R_{world}$ is the visible world radius corresponding to the radar edge:
  $$R_{world} = \frac{R_{base}}{\text{Zoom}}$$
  - Zoom $1.0\times \implies R_{world} = 1200\text{ units}$.
  - Zoom $1.5\times \implies R_{world} = 800\text{ units}$.
  - Zoom $2.0\times \implies R_{world} = 600\text{ units}$.

### Boundary Clamping & Blip Culling
Any world item or point $\mathbf{p}_{world}$ whose projected distance $\|\mathbf{p}_{minimap} - \mathbf{c}_{center}\| > R_{radar}$ is either culled or clamped to the perimeter edge with an arrow indicator depending on entity priority.

```
       N (0, -1)
          |
   +-------------+
   |      *      |  * = Ground Loot Drop (Gold Blip)
W -|      ^      |- E
   |             |  ^ = Player Position & Facing Direction
   +-------------+
          |
       S (0, 1)
```

---

## 3. Rendered Elements & Visual Styling

### 3.1 Terrain & World Boundaries
- **World Bounds**: World edges $(X_{min}, Y_{min}, X_{max}, Y_{max})$ are projected onto the radar as a bold crimson border line when within range.
- **Storm Circle**: The shrinking battle royale safe zone is drawn as a white circle; the toxic storm wall is drawn in translucent violet (`Color{ 140, 20, 200, 120 }`).

### 3.2 Ground Loot Blips
Ground loot instances from `wildbound::world::World` are iterated:
- Active uncollected items are plotted as $2.5\text{ px}$ radius dots.
- Rarity tinting:
  - Common: Gray (`#BDC3C7`)
  - Uncommon: Green (`#2ECC71`)
  - Rare: Blue (`#3498DB`)
  - Epic: Purple (`#9B59B6`)
  - Legendary: Gold (`#F1C40F`)

### 3.3 Player Indicator
- The local player is anchored at the exact center $\mathbf{c}_{center}$.
- A bright green blip ($3\text{ px}$) marks position.
- A directional chevron/arrow extends $7\text{ px}$ from the blip indicating the player's facing angle $\theta$:
  $$\mathbf{p}_{arrow} = \mathbf{c}_{center} + 7.0 \times (\cos \theta, \sin \theta)$$

### 3.4 Compass & Controls
- Compass points (`N`, `S`, `E`, `W`) are inscribed on the outer rim.
- Current zoom indicator is printed in the lower corner (e.g., `[1.5x]`).
- Pressing `M` toggles zoom cycling ($1.0\times \to 1.5\times \to 2.0\times \to 1.0\times$).

