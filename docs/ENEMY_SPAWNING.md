# Wildbound Enemy Spawning Subsystem

The Enemy Spawner (`wildbound::world::EnemySpawner`) coordinates data-driven entity generation, dynamic density control, collision-aware placement, and clean deferred lifecycle disposal.

---

## 1. Spawner Configuration

Configured in `data/spawning/enemy_spawning.json`:

```json
{
  "spawning": {
    "enabled": true,
    "max_active_enemies": 6,
    "spawn_interval": 6.0,
    "initial_spawn_count": 3,
    "min_distance_from_player": 180.0,
    "max_distance_from_player": 450.0,
    "enemy_types": [
      {
        "type": "goblin",
        "weight": 70,
        "config_file": "data/characters/enemies/goblin.json"
      },
      {
        "type": "skeleton",
        "weight": 30,
        "config_file": "data/characters/enemies/skeleton.json"
      }
    ],
    "spawn_zones": [
      { "name": "NorthClearing", "x": 640.0, "y": 480.0, "radius": 150.0 },
      { "name": "EastWoods", "x": 1600.0, "y": 900.0, "radius": 200.0 }
    ]
  }
}
```

---

## 2. Spawn Position Validation Pipeline

Before instantiating an entity in the game world, candidate positions undergo strict multi-pass validation:

```mermaid
flowchart TD
    Candidate[Candidate Position] --> BoundsCheck{Inside World Bounds?}
    BoundsCheck -->|No| Reject[Reject Location]
    BoundsCheck -->|Yes| EdgeCheck{Margin >= 32px from edge?}
    EdgeCheck -->|No| Reject
    EdgeCheck -->|Yes| CollisionCheck{Footprint 16x12 intersects solid tile / object?}
    CollisionCheck -->|Yes| Reject
    CollisionCheck -->|No| DistanceCheck{Within [minDist, maxDist] from player?}
    DistanceCheck -->|No| Reject
    DistanceCheck -->|Yes| Valid[Accept Spawn Position]
```

### Footprint Collision Check
The spawner queries the `World` collision spatial cache against a $16 \times 12$ px bounding box centered at the candidate position's feet anchor:
```cpp
bool EnemySpawner::IsValidSpawnLocation(Vector2 pos, const World& world) const {
    // 1. World boundary check
    Rectangle bounds = world.GetWorldBounds();
    if (pos.x < bounds.x + 32.0f || pos.x > bounds.x + bounds.width - 32.0f ||
        pos.y < bounds.y + 48.0f || pos.y > bounds.y + bounds.height - 32.0f) {
        return false;
    }

    // 2. Collision check
    Rectangle feetBox{ pos.x - 8.0f, pos.y - 12.0f, 16.0f, 12.0f };
    return !world.CheckCollision(feetBox);
}
```

---

## 3. Weighted Entity Selection

Spawn archetypes are selected probabilistically using cumulative weight distribution:
$$\text{Weight}_{\text{total}} = \sum w_i$$
A roll in $[1, \text{Weight}_{\text{total}}]$ determines the spawned type (e.g., 70% Goblin, 30% Skeleton).

---

## 4. Spawner Controls in Developer UI

The DevUI "Combat & AI" tab surfaces full real-time spawner controls:
- **Toggle Spawner**: Enable/disable automated interval spawning (**`F7`** hotkey).
- **Manual Spawn Buttons**: One-click instant spawning for Goblin and Skeleton entities near the player.
- **Clear All Enemies**: Instantly removes all active enemies and cleans up `CombatSystem` registrations.
- **Active Entity Table**: Displays live ID, type, state, health, position, and target of every active enemy.

