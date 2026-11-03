# Environmental Collision Metadata — Wildbound Engine

## 1. Overview
The collision system in Phase 3 provides spatial indexing and query functionality for environmental barriers, water hazards, and map boundaries. No dynamic player movement physics or combat resolution is executed in Phase 3.

---

## 2. Collision Classification (`CollisionType`)

```cpp
enum class CollisionType {
    None = 0,
    Solid,       // Blocks entity movement (trees, rocks, fences, buildings)
    Impassable,  // Impassable deep terrain
    Water,       // Water surfaces (requires swimming or boat)
    Walkable,    // Pathway, bridge, open ground
    Trigger      // Region trigger
};
```

---

## 3. Spatial Indexing
When `World::LoadMap()` completes:
1. `World::RebuildCollisionCache()` iterates through all environmental objects and extracts active `CollisionBox` structs.
2. It processes the dedicated `"Collision"` tilelayer to mark water courses and boundary edges.
3. The resulting spatial cache supports fast broad-phase collision tests:
   * `World::GetCollisionBoxes(queryArea)`: Returns only boxes that intersect the queried rectangle.
   * `World::CheckCollision(queryBox)`: Fast boolean test for solid collision.

