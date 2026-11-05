# Footstep Audio System Specification

## 1. Overview & Mechanics

The Player Footstep System (`wildbound::entities::Player`) provides realistic acoustic feedback based on:
1. **The physical ground surface under the player's feet.**
2. **The movement state (Walking vs. Running vs. Stationary).**
3. **Multi-variation cycling to prevent auditory fatigue.**

---

## 2. Supported Surface Types

The system detects and supports 8 distinct terrain surfaces:

| Surface Type | Associated Tile / Biome Characteristics | Audio Event ID | Variations |
|---|---|---|---|
| **Grass** | Wild grass, meadows, temperate ground | `player.footstep.grass` | `001`, `002`, `003` |
| **Dirt** | Packed earth, cart roads, barren trails | `player.footstep.dirt` | `001`, `002`, `003` |
| **Stone** | Cobblestones, masonry, crag rock, ruins | `player.footstep.stone` | `001`, `002`, `003` |
| **Snow** | Deep snowdrifts, frozen tundra | `player.footstep.snow` | `001`, `002`, `003` |
| **Sand** | Dunes, arid waste, dry clay | `player.footstep.sand` | `001`, `002`, `003` |
| **Mud** | Marshlands, wetlands, soggy mire | `player.footstep.mud` | `001`, `002`, `003` |
| **Wood** | Bridges, wooden docks, tavern floors | `player.footstep.wood` | `001`, `002`, `003` |
| **Water** | Shallows, rivers, marsh puddle edges | `player.footstep.water` | `001`, `002`, `003` |

---

## 3. Surface Detection Algorithm

In `Player::DetectSurfaceUnderFoot(const world::World& world)`:
1. Convert player world coordinates $(x, y)$ to tile coordinates $(col, row)$ via `Tilemap::WorldToTile`.
2. Inspect the `"Ground"` tile layer (or bottom-most layer).
3. Read the tile GID at $(col, row)$. If 0, check secondary terrain layers.
4. If the corresponding tileset defines a `"surface"` custom property (e.g. `surface = "wood"`), use it directly.
5. If no explicit property is attached, evaluate tileset heuristics based on active biome mapping:
   - *Desert tileset*: GID $33..48 \to$ `stone`, else $\to$ `sand`.
   - *Snow tileset*: GID $25..32 \to$ `water`, else $\to$ `snow`.
   - *Swamp tileset*: GID $25..32 \to$ `water`, else $\to$ `mud`.
   - *Mountain tileset*: GID $9..16 \to$ `dirt`, else $\to$ `stone`.
   - *Village / Interior*: GID $1..16 \to$ `wood`, else $\to$ `stone`.
   - *Forest default*: GID $1..8 \to$ `grass`, $9..16 \to$ `dirt`, $17..24 \to$ `stone`, $25..32 \to$ `water`, $33..40 \to$ `wood`.

---

## 4. Cadence & Animation Synchronization

```
Stationary (Speed <= 5.0 px/s)  ---> No footsteps played, timer primed to 0.15s
Walking (Speed > 5.0 px/s)     ---> Step interval = 0.40s
Running (Speed > 100.0 px/s)    ---> Step interval = 0.26s
```

### Footstep Update Routine
```cpp
float moveSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
if (moveSpeed > 5.0f && m_health.IsAlive()) {
    float stepInterval = isRunning ? 0.26f : 0.40f;
    m_footstepTimer += dt;
    if (m_footstepTimer >= stepInterval) {
        m_footstepTimer = 0.0f;
        if (audioSys) {
            std::string surface = DetectSurfaceUnderFoot(world);
            audioSys->DispatchEvent("player.footstep." + surface);
        }
    }
} else {
    m_footstepTimer = 0.15f; // Ready to step when movement begins
}
```

### Acoustic Variations
Each step cycles through 3 distinct procedural WAV assets:
- `_001`: Standard mid-weight heel strike.
- `_002`: Subtle toe drag / weight shift.
- `_003`: Slightly firmer tread.
This prevents repetitive rhythms and creates a natural walking experience.

