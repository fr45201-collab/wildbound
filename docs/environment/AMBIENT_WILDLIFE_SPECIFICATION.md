# WILDBOUND — AMBIENT WILDLIFE SPECIFICATION

## 1. Scope & Philosophy

Ambient wildlife creatures are non-combat entities designed to breathe life into Wildbound's biomes. They are lightweight, procedural wanderers that reuse existing Phase 9 creature assets while executing specialized ambient behavior state machines.

Ambient entities do not require combat components, health records, or complex pathfinding grids. They rely on local territory bounding boxes (`wanderBounds`), procedural timers, and vertical altitude offsets.

---

## 2. Supported Species

### 2.1 Small Bird (`small_bird`)
- **Visual Asset**: Reuses `sheet_small_bird` (Phase 9 wildlife).
- **Behavior State Machine**:
  1. **Flying (`state = 0`)**: Moves toward `targetPosition` with speed between 45–70 px/s at an elevation altitude of $18–25$ px. Flaps wings at 12 fps.
  2. **Perched / Resting (`state = 2`)**: Settles on tree branch, fence post, or ground. Velocity drops to zero. Altitude drops to $0–4$ px. Plays stationary breathing/perch animation.
- **Shadow**: Renders an elliptical ground shadow directly beneath the bird's world coordinate ($x, y$), scaling subtly with altitude.

### 2.2 Butterflies (`forest_butterfly`, `blue_butterfly`, `golden_butterfly`)
- **Visual Assets**: Reuses `sheet_forest_butterfly`, `sheet_blue_butterfly`, `sheet_golden_butterfly`.
- **Behavior State Machine**:
  1. **Fluttering / Gliding (`state = 0`)**: Erratic, sinusoidal flight vectors mimicking natural butterfly movement. Speed between 15–30 px/s at altitude $6–12$ px.
  2. **Hovering (`state = 1`)**: Pauses over floral bushes and grass patches with high-frequency wing flutter (8–10 fps).
  3. **Resting (`state = 2`)**: Lands on foliage with folded wings for 2.0–4.0 seconds.

---

## 3. Data Structures & State Machine

```cpp
struct AmbientCreature {
    int id = 0;
    AmbientType type = AmbientType::Bird;
    std::string species;
    Vector2 position{ 0.0f, 0.0f };
    Vector2 velocity{ 0.0f, 0.0f };
    Vector2 targetPosition{ 0.0f, 0.0f };
    float altitude = 0.0f;
    float stateTimer = 0.0f;
    int state = 0; // 0=Flying, 1=Hovering, 2=Resting
    std::string spritesheetId;
    std::string currentClip;
    float animTimer = 0.0f;
    int currentFrame = 0;
    Rectangle wanderBounds{ 0.0f, 0.0f, 1200.0f, 1200.0f };
};
```

---

## 4. Bounding Box & Territory Constraints

- Each creature is bound to a `Rectangle wanderBounds`.
- When an entity nears the boundary edge, `targetPosition` is procedurally reflected toward the center of the territory:
```cpp
if (c.position.x < c.wanderBounds.x) c.targetPosition.x = c.wanderBounds.x + 50.0f;
if (c.position.x > c.wanderBounds.x + c.wanderBounds.width) c.targetPosition.x = c.wanderBounds.x + c.wanderBounds.width - 50.0f;
```
- This ensures ambient wildlife flocks and swarms remain localized around meadows, groves, and ponds without escaping across the world map.

