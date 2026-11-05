# Wildbound — Environment Placement & Collision Specification (Phase 10)

## 1. Spatial Placement Principles

In Wildbound's top-down $2.5\text{D}$ orthographic coordinate space, every environmental object must balance visual presentation with physical gameplay boundaries.

### 1.1 Decoupled Visual and Physical Footprints
A foundational rule established in the [Art Bible](file:///c:/dev/Wildbound/docs/art/WILDBOUND_ART_BIBLE.md) and [World Architecture](file:///c:/dev/Wildbound/docs/WORLD_ARCHITECTURE.md) is that **visual dimensions and collision dimensions are strictly independent**:

```
Visual Sprite Rectangle (e.g. 80 x 96 px Oak Tree)
+-------------------------------------------------------------+
|                                                             |
|                       Overhanging Canopy                    |  <- Walkable BEHIND
|                       (No Collision)                        |     (Player occluded)
|                                                             |
+------------------------------+------------------------------+
                               |
                        Trunk  |  <- Ground Anchor (40, 90)
                       +-------+-------+
                       | Physical AABB |  <- Solid Collision (24 x 14 px)
                       +---------------+
```

1. **Canopies and Eaves are Walkable**: Players can walk directly behind tree canopies, under building roof overhangs, and beneath hanging lanterns.
2. **Ground Contact Governs Collision**: Collision volumes are restricted to the physical ground base where an object meets the soil (e.g. tree trunks, building foundation walls, rock bases, fence posts).
3. **Traversable Infrastructure**: Footbridges, stepping stones, and ground carpets have collision disabled or marked as traversable surfaces, allowing fluid crossing over water and ditches.

---

## 2. Ground Anchor & Depth Sorting ($Y$-Sorting)

The ground anchor $(x_{\text{anchor}}, y_{\text{anchor}})$ is the mathematical pivot used by the engine for:
1. **World Placement**: The world position $(x_{\text{world}}, y_{\text{world}})$ assigned to the object in Tiled or C++ aligns with the ground anchor.
2. **Dynamic $Y$-Sorting**: [`WorldRenderer`](file:///c:/dev/Wildbound/src/world/WorldRenderer.cpp) computes the object's depth order as:
   $$z_{\text{order}} = y_{\text{world}}$$
   When an entity's ground contact $y_{\text{entity}} < y_{\text{object}}$, the entity is drawn first (rendering behind the environmental object). When $y_{\text{entity}} \ge y_{\text{object}}$, the entity is drawn second (rendering in front).

---

## 3. Tiled Map Editor Integration Standards

When authoring maps in Tiled, environmental objects are placed in the `objects` layer as **Tile Objects** or **Insert Objects**:

### 3.1 Standard Tiled Object Properties
Every solid environmental prop includes the following custom properties:

| Property Name | Tiled Type | Example Value | Description |
| :--- | :---: | :---: | :--- |
| `collision` | `bool` | `true` | Enables or disables physical collision in `EnvironmentObject`. |
| `collision_type` | `string` | `"solid"` | Collision classification (`solid`, `water`, `hazard`, `trigger`). |
| `collision_width` | `float` | `24.0` | Width of the physical collision AABB in pixels. |
| `collision_height` | `float` | `14.0` | Height of the physical collision AABB in pixels. |
| `facing` | `string` | `"down"` | Optional orientation or alignment flag. |

### 3.2 Tiled Object JSON Mapping
[`MapLoader::LoadFromJson`](file:///c:/dev/Wildbound/src/world/MapLoader.cpp) automatically interprets object types and assigns texture IDs and collision parameters:

```json
{
  "id": 105,
  "name": "Oak_Tree_Forest_North",
  "type": "tree_oak_003",
  "x": 320.0,
  "y": 480.0,
  "width": 80.0,
  "height": 96.0,
  "properties": [
    { "name": "collision", "type": "bool", "value": true },
    { "name": "collision_type", "type": "string", "value": "solid" },
    { "name": "collision_width", "type": "float", "value": 24.0 },
    { "name": "collision_height", "type": "float", "value": 14.0 }
  ]
}
```

---

## 4. Collision Footprint Matrix by Category

### 4.1 Nature
| Asset ID | Dimensions | Collision $(W \times H)$ | Ground Offset $(X, Y)$ | Notes |
| :--- | :---: | :---: | :---: | :--- |
| `tree_oak_001` | $48 \times 64$ px | $16 \times 10$ px | $[16, 52]$ | Trunk footprint base |
| `tree_oak_002` | $64 \times 80$ px | $20 \times 12$ px | $[22, 64]$ | Trunk footprint base |
| `tree_oak_003` | $80 \times 96$ px | $24 \times 14$ px | $[28, 78]$ | Trunk footprint base |
| `tree_oak_004` | $112 \times 128$ px | $36 \times 18$ px | $[38, 104]$ | Gnarled root base |
| `tree_pine_001` | $32 \times 64$ px | $12 \times 10$ px | $[10, 52]$ | Pine trunk base |
| `tree_pine_002` | $48 \times 80$ px | $16 \times 12$ px | $[16, 66]$ | Pine trunk base |
| `tree_pine_003` | $64 \times 96$ px | $20 \times 14$ px | $[22, 78]$ | Pine trunk base |
| `bush_round_001` | $32 \times 32$ px | $16 \times 10$ px | $[8, 20]$ | Dense core obstruction |
| `bush_dense_001` | $40 \times 32$ px | $24 \times 12$ px | $[8, 18]$ | Thick foliage obstruction |
| `bush_large_001` | $48 \times 36$ px | $30 \times 14$ px | $[9, 20]$ | Wide hedge obstruction |
| `rock_medium_001`| $24 \times 20$ px | $16 \times 9$ px | $[4, 10]$ | Solid stone core |
| `rock_boulder_001`| $48 \times 40$ px | $36 \times 18$ px | $[6, 18]$ | Massive granite obstruction |
| `rock_mossy_001` | $32 \times 28$ px | $22 \times 12$ px | $[5, 14]$ | Slippery rock base |
| `log_medium_001` | $48 \times 24$ px | $40 \times 10$ px | $[4, 12]$ | Horizontal log barrier |
| `log_trunk_001` | $64 \times 32$ px | $54 \times 14$ px | $[5, 14]$ | Fallen redwood trunk |

### 4.2 Buildings
| Asset ID | Dimensions | Collision $(W \times H)$ | Ground Offset $(X, Y)$ | Doorway Status |
| :--- | :---: | :---: | :---: | :--- |
| `house_cottage_001` | $80 \times 80$ px | $64 \times 40$ px | $[8, 36]$ | Walkable entrance gap |
| `house_stone_small_001`| $80 \times 80$ px | $64 \times 40$ px | $[8, 36]$ | Walkable entrance gap |
| `house_village_001` | $96 \times 88$ px | $76 \times 44$ px | $[10, 40]$ | Walkable entrance gap |
| `house_residence_large_001`| $112 \times 96$ px | $92 \times 48$ px | $[10, 44]$ | Double entrance gap |
| `house_twostory_001`| $120 \times 112$ px | $96 \times 54$ px | $[12, 54]$ | Main entryway gap |
| `cabin_forest_001` | $72 \times 72$ px | $56 \times 36$ px | $[8, 32]$ | Porch step access |
| `cabin_hunting_001` | $72 \times 72$ px | $56 \times 36$ px | $[8, 32]$ | Porch step access |
| `cabin_woodland_001`| $80 \times 76$ px | $64 \times 38$ px | $[8, 34]$ | Porch step access |
| `shop_general_001` | $96 \times 88$ px | $76 \times 44$ px | $[10, 40]$ | Storefront threshold |
| `shop_market_001` | $88 \times 80$ px | $70 \times 40$ px | $[9, 36]$ | Stall counter gap |
| `blacksmith_forge_001`| $112 \times 96$ px | $90 \times 48$ px | $[11, 44]$ | Forge entrance gap |
| `storage_shed_001` | $64 \times 64$ px | $50 \times 32$ px | $[7, 28]$ | Shed door access |
| `storage_warehouse_001`| $96 \times 88$ px | $78 \times 44$ px | $[9, 40]$ | Wide cargo threshold |
| `watchtower_wood_001`| $64 \times 112$ px | $40 \times 36$ px | $[12, 72]$ | Walkable perimeter |
| `watchtower_stone_001`| $64 \times 120$ px | $44 \times 40$ px | $[10, 76]$ | Walkable perimeter |
| `ruin_wall_broken_001`| $64 \times 40$ px | $52 \times 16$ px | $[6, 20]$ | Traversal through gaps |
| `ruin_building_abandoned_001`| $96 \times 80$ px | $74 \times 40$ px | $[11, 36]$ | Broken wall breaches |
| `ruin_tower_small_001`| $64 \times 88$ px | $44 \times 36$ px | $[10, 48]$ | Rubble perimeter |

### 4.3 Props
| Asset ID | Dimensions | Collision $(W \times H)$ | Ground Offset $(X, Y)$ | Notes |
| :--- | :---: | :---: | :---: | :--- |
| `fence_straight_001` | $32 \times 24$ px | $30 \times 8$ px | $[1, 14]$ | Modular fence line |
| `fence_corner_001` | $32 \times 32$ px | $24 \times 24$ px | $[4, 4]$ | Corner junction |
| `fence_gate_001` | $32 \times 24$ px | $30 \times 8$ px | $[1, 14]$ | Gate segment |
| `fence_broken_001` | $32 \times 24$ px | $18 \times 8$ px | $[1, 14]$ | Passable opening |
| `barrel_standard_001`| $20 \times 24$ px | $14 \times 12$ px | $[3, 10]$ | Solid container |
| `barrel_small_001` | $16 \times 18$ px | $12 \times 10$ px | $[2, 6]$ | Compact keg |
| `barrel_banded_001` | $22 \times 26$ px | $16 \times 12$ px | $[3, 12]$ | Reinforced barrel |
| `crate_small_001` | $20 \times 20$ px | $16 \times 14$ px | $[2, 4]$ | Stacking crate |
| `crate_large_001` | $32 \times 32$ px | $26 \times 20$ px | $[3, 8]$ | Bulk cargo block |
| `crate_reinforced_001`| $28 \times 28$ px | $22 \times 18$ px | $[3, 7]$ | Heavy storage crate |
| `well_stone_001` | $36 \times 40$ px | $26 \times 22$ px | $[5, 14]$ | Circular stone well |
| `well_wooden_001` | $36 \times 44$ px | $26 \times 22$ px | $[5, 18]$ | Roofed well |
| `lamp_post_001` | $20 \times 48$ px | $10 \times 10$ px | $[5, 36]$ | Post footprint |
| `sign_direction_001`| $24 \times 36$ px | $8 \times 8$ px | $[8, 26]$ | Post footprint |
| `sign_noticeboard_001`| $36 \times 36$ px | $28 \times 10$ px | $[4, 24]$ | Dual post board |
| `sign_warning_001` | $20 \times 32$ px | $8 \times 8$ px | $[6, 22]$ | Post footprint |
| `bench_wooden_001` | $32 \times 20$ px | $26 \times 10$ px | $[3, 8]$ | Village park bench |
| `bench_stone_001` | $32 \times 20$ px | $26 \times 10$ px | $[3, 8]$ | Plaza stone bench |

