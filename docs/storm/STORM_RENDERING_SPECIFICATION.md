# Storm Rendering & Visual Effects Specification

## 1. Multi-Layer Visual Pipeline

The `StormBoundaryRenderer` renders three distinct visual tiers:
1. **World Space Layer**: Atmospheric haze, boundary barrier rings, and particle emitters.
2. **Screen Space Layer**: Hazard vignette overlay, directional navigation pointer, and screen edge corruption creeping.
3. **Minimap Layer**: Coordinate projection onto the circular minimap radar.

```
+-------------------------------------------------------------------------------+
|                             Rendering Hierarchy                               |
|                                                                               |
|  1. World Layer (RenderWorldBoundary)                                         |
|     |-- Screen-space bounds check vs Camera viewport                          |
|     |-- Atmospheric Corruption Haze outside safe perimeter                    |
|     |-- Primary Boundary Energy Wall (thickness: 24px, pulsing alpha)         |
|     |-- Secondary Boundary Wall (thickness: 12px, high-frequency oscillation)  |
|     |-- Target Zone Projected Ring (dashed indicator during hold/warning)     |
|     \-- Perimeter Particle Simulation (sparks, swirling haze, energy flares)  |
|                                                                               |
|  2. Minimap Layer (RenderMinimap)                                             |
|     |-- Transform world safe zone center to player-relative radar coords      |
|     |-- Draw safe zone circle (solid neon blue / turquoise boundary)          |
|     \-- Draw next target circle (dashed pulsating yellow boundary)            |
|                                                                               |
|  3. Screen Layer (RenderScreenEffects)                                        |
|     |-- Post-processing hazard vignette (deep purple/magenta rim fade)        |
|     |-- Throbbing heartbeat pulsation when outside zone                       |
|     \-- Directional Pointer Compass Arrow pointing towards closest safe point |
+-------------------------------------------------------------------------------+
```

---

## 2. World Space Boundary Effects

### 2.1 Multi-Ring Energy Barrier
The storm boundary is rendered using concentric rings with variable radii, frequencies, and alphas:
- **Base Ring**: Radius $R$, Thickness $W_1 = 20\text{px}$, Color: $(140, 20, 220, 180 + 50 \cdot \sin(3t))$.
- **Outer Glow Ring**: Radius $R + 8$, Thickness $W_2 = 12\text{px}$, Color: $(180, 50, 255, 100 + 40 \cdot \cos(2t))$.
- **Core Energy Stream**: Radius $R - 4$, Thickness $W_3 = 6\text{px}$, Color: $(230, 160, 255, 220 + 35 \cdot \sin(5t))$.

### 2.2 Particle Dynamics
A pool of 128 active particles simulates storm emissions along the boundary:
- **Emission Rule**: Spawned randomly on the boundary circumference $(C_x + R \cos\theta, C_y + R \sin\theta)$ with initial outward or tangential velocity.
- **Particle Types**:
  - `part_energy_spark`: High velocity, short lifetime ($0.4\text{s}$), intense violet brilliance.
  - `part_corruption_medium`: Drifting corruption haze smoke ($1.2\text{s}$ lifetime, scaling up as it fades).
  - `part_storm_debris`: Erratic floating debris particles along the edge.

---

## 3. Screen Space Overlays

### 3.1 Hazard Vignette
When the player is outside the safe zone ($d > 0$), a screen-space vignette is drawn:
- Vignette alpha scales dynamically with penetration depth:
  $$\alpha = \min\left(0.75, 0.20 + \frac{d}{2000.0} \cdot 0.55\right)$$
- An additional sine modulation $(1.0 + 0.2 \cdot \sin(4\pi t))$ simulates an adrenaline pulse.

### 3.2 Directional Safety Pointer
- If the player is outside the safe zone, a high-contrast directional arrow is rendered around the player's screen anchor.
- The arrow angle $\theta = \operatorname{atan2}(S_y - P_y, S_x - P_x)$, pointing straight toward the closest safe perimeter coordinate $S$.
- Displays distance to boundary in meters ($1\text{m} = 32\text{px}$).

