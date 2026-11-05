# Storm Minimap Integration Specification

## 1. Overview and Coordinate Projections

Wildbound features a circular radar minimap located at the top-right corner of the player viewport (`ui::Minimap`). The minimap visualizes terrain, structures, player orientation, and the active storm boundaries.

```
                           +----------------------+
                           |   Circular Minimap   |
                           |       (R = 54px)     |
                           |                      |
                           |        (Target)      |
                           |       /   *   \      |
                           |      (  Player )     |
                           |       \   *   /      |
                           |        \     /       |
                           |   -----(     )-----  |
                           |  (     Current     ) |
                           |   \   Safe Zone   /  |
                           +----------------------+
```

---

## 2. Projection Mathematics

Let:
- $P_{\text{player}} = (X_p, Y_p)$ be the player world coordinate.
- $C_{\text{radar}} = (M_x, M_y)$ be the screen center coordinate of the circular minimap widget.
- $R_{\text{radar}} = 54.0\text{px}$ be the minimap radar display radius.
- $W_{\text{world}}$ be the world width dimension.
- $Z$ be the minimap zoom level (default $1.0$).

The coordinate transformation scale factor $S$ (pixels on radar per world pixel) is:
$$S = \frac{R_{\text{radar}}}{\frac{W_{\text{world}} \times 0.25}{Z}}$$

For any world coordinate $C_{\text{world}} = (X_w, Y_w)$:
1. Calculate delta relative to player:
   $$\Delta X = (X_w - X_p) \cdot S$$
   $$\Delta Y = (Y_w - Y_p) \cdot S$$
2. Radar screen center:
   $$C_{\text{screen}} = (M_x + \Delta X, M_y + \Delta Y)$$
3. Projected radius:
   $$R_{\text{screen}} = R_{\text{world}} \cdot S$$

---

## 3. Minimap Boundary Render States

1. **Current Safe Zone**:
   - Drawn as a solid bright cyan / neon blue circle (`#00FFFF`) of radius $R_{\text{screen}}$ centered at $C_{\text{screen}}$.
   - Ring line thickness: 2.0px.
2. **Next Target Safe Zone**:
   - Drawn during the Warning / Hold phase as a pulsating yellow/gold circle (`#FFD700`).
   - Pulsates at 2.0 Hz to distinguish it from the current perimeter.
3. **Clipping Guarantee**:
   - Boundary rings are masked to stay within the circular minimap window, preventing graphical artifact bleed into surrounding UI elements.

