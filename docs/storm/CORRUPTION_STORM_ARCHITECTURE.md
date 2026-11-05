# Corruption Storm & Safe Zone System — Architecture

## 1. Executive Summary

The Corruption Storm system is Wildbound's core Battle Royale territorial boundary mechanic. It bounds play within a progressively contracting circular safe zone, penalizing entities positioned outside the current safe perimeter with continuous damage over time, distinct visual hazard effects, directional guidance toward safety, and synchronized acoustic feedback.

```
+--------------------------------------------------------------------------------+
|                                  Game Loop                                     |
|                                                                                |
|   +-----------------------+              +---------------------------------+   |
|   |  WorldManager         |              |  Player Entity                  |   |
|   |  - PlayableBounds     |              |  - Transform2D (World Pos)      |   |
|   +-----------+-----------+              |  - HealthComponent              |   |
|               |                          +----------------+----------------+   |
|               v                                           |                    |
|   +-------------------------------------------------------v----------------+   |
|   |                       wildbound::storm::StormManager                   |   |
|   |                                                                        |   |
|   |  +-----------------------+  Tick DT   +-----------------------------+  |   |
|   |  | SafeZone              | ---------> | StormDamageSystem           |  |   |
|   |  | - Current Circle      |            | - Frame-Rate Indep. Accum.  |  |   |
|   |  | - Target Circle       |            | - Safety Check              |  |   |
|   |  | - Interpolation       |            | - Damage Source Tagging     |  |   |
|   |  +-----------------------+            +--------------+--------------+  |   |
|   |              |                                       |                 |   |
|   |              v                                       v                 |   |
|   |  +------------------------------------------------------------------+  |   |
|   |  | StormBoundaryRenderer                                            |  |   |
|   |  | - World Haze Quad & Multi-Ring Edge Shader                       |  |   |
|   |  | - Particle Emitters (Sparks, Haze, Dust)                         |  |   |
|   |  | - Fullscreen Vignette & Screen Distortion                        |  |   |
|   |  | - Directional Guidance Compass Arrow                             |  |   |
|   |  | - Minimap Radar Arc & Projection                                 |  |   |
|   |  +------------------------------------------------------------------+  |   |
|   +------------------------------------------------------------------------+   |
+--------------------------------------------------------------------------------+
```

---

## 2. Component Breakdown

### 2.1 SafeZone (`SafeZone.hpp` / `SafeZone.cpp`)
- Encapsulates geometric representation of 2D circular boundary: center $(X, Y)$ and radius $R$.
- Provides containment queries (`Contains(x, y)`), distance computations (`DistanceToEdge(x, y)`), and closest safe point projection (`GetClosestSafePoint(x, y)`).
- Implements `CalculateNextCenter(...)` with drift bounded to $(R_{\text{current}} - R_{\text{next}}) \times \text{driftRatio}$ and strict clamping within `playableBounds` to guarantee the safe zone never manifests off-island.

### 2.2 StormDamageSystem (`StormDamageSystem.hpp` / `StormDamageSystem.cpp`)
- Authoritative execution of outside-zone damage against `combat::HealthComponent`.
- Uses high-precision fractional damage accumulator (`float m_damageAccumulator`) to ensure consistent DPS across variable frame rates (30 FPS vs 60 FPS vs 120 FPS vs 240 FPS).
- Employs unique damage source identifier `"DAMAGE_SOURCE_CORRUPTION_STORM"` to distinguish storm fatalities in logs and death events.
- Immediately stops applying damage upon crossing the safe perimeter or entity expiration.

### 2.3 StormBoundaryRenderer (`StormBoundaryRenderer.hpp` / `StormBoundaryRenderer.cpp`)
- **World Space Rendering**:
  - Viewport-culled world haze quad outside the safe zone.
  - Multi-layered pulsating boundary rings with alpha modulation and scrolling UVs.
  - Particle simulation emitting sparks, swirling corruption haze, and energy flares along the active perimeter.
- **Screen Space Rendering**:
  - Hazard vignette with intensity proportional to outside penetration depth.
  - Directional guidance indicator arrow oriented toward the closest point on the safe perimeter.
- **Minimap Radar Rendering**:
  - Scaled coordinate projection onto the minimap radar circle ($R_{\text{radar}} = 54.0$ px), drawing the active safe zone and pulsating target destination ring.

### 2.4 StormManager (`StormManager.hpp` / `StormManager.cpp`)
- Master coordinator orchestrating match lifecycle across 6 distinct phases.
- Manages state machine: `Waiting` $\rightarrow$ `Warning` $\rightarrow$ `Shrinking` $\rightarrow$ `Holding` $\rightarrow$ `Finished`.
- Dispatches sound events (`sfx_storm_warning`, `sfx_storm_shrink`, `sfx_storm_damage`, `amb_storm_rumble`).
- Exposes complete introspection and manual control hooks for developer tools.

