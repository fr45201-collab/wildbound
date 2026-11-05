# Wildbound — World Collision & Navigation Architecture

## 1. Overview
The collision and navigation subsystem for the Battle Royale island bridges static environmental world geometry, runtime raylib physics, and high-performance A* pathfinding. 

```
+------------------------------------------------------------------------------------+
|                         COLLISION & NAVIGATION PIPELINE                            |
|                                                                                    |
| [Tiled Map JSON]                                                                   |
|   ├── Tile Collision Layer (Water / Cliffs)                                       |
|   └── Object Group (Buildings / Rocks / Trees)                                    |
|            │                                                                       |
|            ▼                                                                       |
| [World Static Collision Cache]                                                     |
|   └── 41,503 CollisionBox primitives stored in spatial index                       |
|            │                                                                       |
|            ├────────────────────────────────────────┐                              |
|            ▼                                        ▼                              |
| [CollisionManager (Physics)]             [NavigationGrid (AI Pathfinding)]        |
|   • Player vs Environment swept AABB       • 640x640 cells (16px per cell)        |
|   • Projectile & spell raycasting          • 166,559 blocked cells (40.7%)        |
|   • Spatial broadphase partitioning        • Cross-biome A* pathfinding (<10ms)    |
+------------------------------------------------------------------------------------+
```

---

## 2. Static Collision Cache & Spatial Partitioning
`wildbound::world::World` manages collision for the island through `m_staticCollisionCache`:
1. **Object Collision**: Extracted from 448 `EnvironmentObject` instances. Each building, tree, rock, or barricade defines an AABB relative to its base anchor.
2. **Tile Collision**: Extracted from layer `"Collision Layer"`. All 32x32px deep ocean border tiles, steep mountain rockfaces, and deep river channels generate solid collision boxes.
3. **Spatial Query Acceleration**: Queries such as `World::GetCollisionBoxes(const Rectangle& queryArea)` test against local bounding limits, filtering down from 41,503 total boxes to only the ~10-30 boxes overlapping active entities or viewport rectangles.

---

## 3. Navigation Grid Rasterization
To serve creature AI, bots, and pathfinding queries across the $10,240 \times 10,240$ world, `wildbound::ai::NavigationGrid` rasterizes the collision cache into a discrete 2D grid:
* **Grid Resolution**: $640 \times 640$ cells ($409,600$ total cells).
* **Cell Size**: $16.0$ world units ($0.5$ map tiles).
* **Memory Footprint**: $400$ KB raw cell buffer (`std::vector<uint8_t>`), easily fitting within modern CPU L2/L3 cache.
* **Rasterization Time**: **$99.8$ milliseconds** to process and rasterize all 41,503 collision boxes across the entire island.
* **Blocked Cell Count**: $166,559$ cells blocked ($40.7\%$ island water/obstacle density, leaving $59.3\%$ walkable landmass).

---

## 4. Cross-Biome Pathfinding Performance
Pathfinding across biomes utilizes `wildbound::ai::Pathfinding`:
* **Algorithm**: Optimized A* with octile distance heuristic ($c_{\text{diag}} = \sqrt{2}$, $c_{\text{ortho}} = 1.0$).
* **Corner Cutting Protection**: Prevents entities from clipping through diagonal building corners and rock vertices.
* **Smoothing**: Post-search line-of-sight raycasting reduces jagged grid steps into straight-line traversal vectors.
* **Benchmark Metrics**:
  * Greenwood Village (Forest) to Ancient Fortress (Center): **$8.73$ ms search time**, exploring $1,971$ nodes to produce $79$ smooth waypoints over $1,592.6$ world units.

