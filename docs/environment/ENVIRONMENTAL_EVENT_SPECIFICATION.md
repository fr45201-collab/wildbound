# WILDBOUND — SCRIPTED ENVIRONMENTAL EVENT SPECIFICATION

## 1. Overview & Narrative Purpose

Scripted environmental events introduce world-scale narrative and environmental tension to Wildbound's fantasy battle royale landscape.

The initial event implementation is the **Recon Airplane Flyover** (`airplane_recon_001`). It provides an atmospheric precursor to aerial airdrops, map shrinking boundaries, or reconnaissance alerts without requiring network synchronization in Phase 11.

---

## 2. Recon Airplane Specification

### 2.1 Visual Design
- **Airframe**: Single-engine low-wing reconnaissance aircraft rendered in 16-bit top-down pixel art.
- **Propeller Animation**: 4-frame continuous loop at 16.7 fps (0.06s frame time) rendering motion blur discs on the nose.
- **Glass Glare**: High-contrast specular line shifting across the cockpit canopy.

### 2.2 Ground Shadow Projection
- **Shadow Offset**: Rendered at world position $(x + 25, y + 25)$ with low alpha ($0.35$) and neutral dark tint `Color{ 15, 20, 30, 90 }`.
- **Airframe Elevation**: Aircraft sprite rendered at $(x, y - 45)$, establishing a perceived cruising elevation of $45$ pixels above the ground plane.

---

## 3. Flight Path & Trajectory Calculation

When triggered via `TriggerAirplaneFlyover(startPos, endPos, speed)`:
1. **Heading Angle**:
   $$\theta = \text{atan2}(y_{\text{end}} - y_{\text{start}}, x_{\text{end}} - x_{\text{start}}) \times \frac{180}{\pi} + 90^\circ$$
2. **Translation**: Linear step along normalized direction vector at `speed` (default: 130–150 px/s).
3. **Completion & Deactivation**: When the aircraft arrives within step distance of `endPos`, `m_airplaneEvent.active` is set to `false`, freeing resources cleanly.

---

## 4. Developer UI Controls

Developers can trigger, monitor, and reset flyover events on demand via DevUI Tab 9 ("Living World"):
- **Flight Status**: Real-time display of flight coordinates, altitude, heading angle, and speed.
- **One-Click Dispatch**: Generates a diagonal traverse across the player's active camera view.

