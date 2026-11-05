# Collision Debugging & Visualization Guide

## 1. Overview

Wildbound provides real-time in-game collision visualization and inspection tools designed to verify collision boundaries, layer filtering, trigger zones, and event processing without interrupting gameplay.

The debugging architecture consists of two primary systems:
1. **In-World Debug Visualizer (`CollisionDebugRenderer`)**: Renders color-coded bounding boxes, active overlap highlights, and trigger volumes directly into the game world view via raylib.
2. **Developer UI Inspector (DevUI Tab 13: Physics & Collision)**: Provides real-time metrics, interactive bilateral layer filter testing, and a live active overlap monitor via Dear ImGui / rlImGui.

---

## 2. Keybindings & Controls

| Shortcut / Control | Target Subsystem | Description |
| :--- | :--- | :--- |
| **`F1`** | `DevUI` | Toggles the entire Developer UI overlay. |
| **`F3`** | `Game::m_showDebugOverlay` | Toggles collision geometry rendering in the world viewport. |
| **`Tab 13`** | `DevUI::RenderPhysicsTab` | Navigates to the "Physics & Collision" inspector panel. |
| **`--test-run`** | Command Line | Executes an automated 60-frame headless smoke test and exits cleanly. |

---

## 3. Layer Color Taxonomy

To distinguish collision objects instantly during visual inspection, each logical layer is assigned a distinct color in `CollisionDebugRenderer`:

| Layer | Color | RGBA | Typical Usage |
| :--- | :--- | :--- | :--- |
| **`World`** | Gray | `(128, 128, 128, 255)` | Static map geometry, boundary cliffs, deep water. |
| **`Player`** | Forest Green | `(46, 204, 113, 255)` | Player character collision footprint. |
| **`Enemy`** | Crimson Red | `(231, 76, 60, 255)` | Hostile enemy units and combatants. |
| **`Wildlife`** | Amber Gold | `(243, 156, 18, 255)` | Neutral or passive fauna (deer, boar, birds). |
| **`Projectile`** | Electric Yellow | `(241, 196, 15, 255)` | Arrows, magic bolts, thrown weapons. |
| **`Pickup`** | Bright Cyan | `(52, 152, 219, 255)` | Dropped items, weapons, health potions. |
| **`Trigger`** | Vibrant Orange | `(230, 126, 34, 255)` | Doorways, biome transition zones, capture points. |
| **`Building`** | Saddle Brown | `(160, 82, 45, 255)` | Village houses, walls, fences, blacksmith structures. |
| **`Hazard`** | Dark Purple | `(142, 68, 173, 255)` | Spike traps, poison clouds, lava pits, battle royale zone. |
| **`Interactable`**| Sky Blue | `(52, 73, 94, 255)` | Chests, levers, campfire cooking stations, shrines. |

### Visual Conventions
* **Solid Fill**: Colliders render with a subtle translucent fill (`Alpha ~ 0.20-0.25`) to indicate coverage area without obscuring underlying terrain pixel art.
* **Border Line**: Crisp 1-pixel border showing exact bounding limits.
* **Trigger Boundary**: Triggers render with dashed or translucent orange borders.
* **Active Overlap Highlight**: When two colliders intersect and pass bilateral filtering, their bounding boxes render with an accentuated **Bright Red outline (`#FF0055`)** and a crosshair at the overlap center.

---

## 4. Developer UI Panel (Tab 13: Physics & Collision)

The Developer UI provides three functional sections:

### 4.1 System Overview & Metrics
* **Total Registered Colliders**: Live count of active colliders partitioned into Static vs Dynamic.
* **Narrow-Phase Tests**: Number of geometric overlap tests performed in the current frame.
* **Active Overlaps**: Instantaneous number of overlapping entity pairs.
* **Tick Duration**: Microsecond measurement of the collision simulation stage.

### 4.2 Interactive Bilateral Layer Validator
Allows designers and engineers to test mask configurations on the fly:
* Select **Entity A Layer** and **Entity A Mask**.
* Select **Entity B Layer** and **Entity B Mask**.
* Real-time result indicator shows:
  * `[PASS] Bilateral Consent Granted` (colliders interact).
  * `[FAIL] Blocked by A Mask` or `[FAIL] Blocked by B Mask`.

### 4.3 Active Overlaps Inspector
A live table listing all currently intersecting pairs:
* **Collider A ID** & Entity Association.
* **Collider B ID** & Entity Association.
* **Layer A vs Layer B**.
* **Intersection Rectangle** `(x, y, w, h)`.
* **Penetration Depth & Normal** (Minimum Translation Vector).

---

## 5. Automated Verification via CTest

All collision mechanics are backed by comprehensive automated test suites:

```bash
# Run all automated tests including PhysicsCollisionTest
ctest --test-dir build -C Release --output-on-failure

# Run only the collision test suite
ctest --test-dir build -C Release -R PhysicsCollisionTest --output-on-failure
```

### Test Coverage (`tests/physics/test_collision.cpp`)
1. **Layer Bitfield & Bilateral Masking**: Verifies 10 layers, self-exclusion, and bilateral consent logic.
2. **AABB & Overlap Geometry**: Tests point containment, strict overlap, touching edges (zero overlap), and MTV calculation.
3. **Trigger Volume Lifecycle**: Validates exact sequence of `Enter -> Stay -> Exit` transitions.
4. **Deferred Mutation Safety**: Validates that adding, deleting, and modifying colliders inside callbacks does not invalidate iterators or crash the simulation.
5. **Deterministic Event Sequence**: Confirms identical event dispatch order across multiple shuffled iterations.
6. **Benchmark & Scalability**: Simulates 120 dynamic colliders across 100 ticks ensuring frame times well within the 16.6ms budget.

