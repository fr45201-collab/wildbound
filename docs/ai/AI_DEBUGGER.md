# Wildbound — AI Debugger & DevUI Integration

## 1. Overview & Interface Location

The AI debugging and visualization suite is integrated directly into the developer interface in `src/ui/DevUI.cpp`.

Located on **Tab 2 ("Combat & AI")**, Section 4 provides deep telemetry, real-time control, and graphical debug overlays for all active AI agents.

---

## 2. DevUI Controls & Telemetry

### 2.1 Simulation Controls
- **Pause AI Simulation**: Freezes all AI controllers and movement while keeping the player and rendering active.
- **Single-Step AI (1 Frame)**: Steps AI forward by a single discrete simulation tick ($1/60\text{s}$) to observe frame-by-frame state transitions.
- **Clear & Re-rasterize Grid**: Forces an immediate re-rasterization of the `NavigationGrid` from `World::GetCollisionBoxes()`.

### 2.2 Visualization Overlays
- **Draw Navigation Grid**:
  - Green wireframe squares for walkable cells.
  - Red shaded tiles for blocked cells.
  - Blue tiles for dynamic agent reservations.
- **Draw Active Paths**:
  - Yellow polyline showing raw A* node sequences.
  - Cyan smoothed line showing active `AIMovementController` waypoints.
- **Draw Perception Cones**:
  - Arc representing field-of-view angle (e.g. $90^\circ$ Goblin, $180^\circ$ Bat).
  - Green dashed line for active line-of-sight raycasts to targets.
  - Red dashed line for occluded / blocked raycasts.
- **Draw Avoidance Vectors**:
  - Orange directional arrows showing local entity separation repulsive forces.

### 2.3 Per-Entity Live Inspector
- Dropdown selector listing all currently alive enemies by ID and archetype.
- Displays:
  - Current `AIState` (e.g. `Chase`, `Attack`, `Retreat`).
  - Active target pointer & target ID.
  - Threat evaluation score.
  - Stuck detector timer ($t_{\text{stuck}}$) and retry counter.
  - Remaining waypoints in queue.
  - Health fraction & retreat threshold.

