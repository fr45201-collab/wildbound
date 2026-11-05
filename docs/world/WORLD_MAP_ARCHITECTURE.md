# Wildbound — World Map Architecture Specification

## 1. Map Architecture Overview
The **Wildbound** Battle Royale island map is authored as an expansive multi-layer, multi-tileset Tiled JSON map (`maps/world/battle_royale_island.json`) spanning $320 \times 320$ tiles ($10,240 \times 10,240$ world pixels). The design adheres to raylib 5.5 and C++20 conventions, using data-driven indexing to achieve seamless multi-biome rendering.

```
+------------------------------------------------------------------------------------+
|                         MAP FILE: battle_royale_island.json                        |
|                                                                                    |
| Dimensions: 320x320 tiles | Tile Size: 32x32 px | World Units: 10,240x10,240 px    |
|                                                                                    |
| [Tilesets (5)]                                                                     |
|  - forest_tileset   (firstgid: 1)   - snow_tileset     (firstgid: 129)             |
|  - desert_tileset   (firstgid: 65)  - swamp_tileset    (firstgid: 193)             |
|                                     - mountain_tileset (firstgid: 257)             |
|                                                                                    |
| [Layers (7)]                                                                       |
|  1. Deep Water            (id: 1, type: tilelayer, solid ocean/coastal border)    |
|  2. Shallow Water         (id: 2, type: tilelayer, rivers, oasis, fords)          |
|  3. Ground Base           (id: 3, type: tilelayer, grass, sand, snow, mud, stone)  |
|  4. Terrain Details       (id: 4, type: tilelayer, flora, dirt patches, scree)    |
|  5. Paths and Roads       (id: 5, type: tilelayer, highways, trails, bridges)     |
|  6. Collision Layer       (id: 6, type: tilelayer, impassable water/cliffs)       |
|  7. Environmental Objects (id: 7, type: objectgroup, 448 buildings/trees/rocks)   |
+------------------------------------------------------------------------------------+
```

---

## 2. Multi-Tileset GID Indexing Architecture

To support 5 distinct visual biomes on a single continuous map without texture blending seams, the engine's `MapLoader` and `Tilemap` employ an offset first-GID addressing scheme:

| Tileset Name | First GID | Tile Count | Columns | Asset Key | Texture Source File |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`forest_tileset`** | 1 | 64 | 8 | `forest_tileset` | `assets/tilesets/forest/forest_tileset.png` |
| **`desert_tileset`** | 65 | 64 | 8 | `desert_tileset` | `assets/tilesets/desert/desert_tileset.png` |
| **`snow_tileset`** | 129 | 64 | 8 | `snow_tileset` | `assets/tilesets/snow/snow_tileset.png` |
| **`swamp_tileset`** | 193 | 64 | 8 | `swamp_tileset` | `assets/tilesets/swamp/swamp_tileset.png` |
| **`mountain_tileset`** | 257 | 64 | 8 | `mountain_tileset` | `assets/tilesets/mountain/mountain_tileset.png` |

Each tile's texture coordinates are computed via:
```cpp
const Tileset* ts = tilemap->GetTilesetForGid(gid);
int localId = gid - ts->GetFirstGid();
int col = localId % ts->GetColumns();
int row = localId / ts->GetColumns();
Rectangle srcRect{ col * tw, row * th, tw, th };
```

---

## 3. Layer Stack & Rendering Order

1. **Layer 1: Deep Water** (`tilelayer`, z=0)
   * Encompasses the entire perimeter ocean from tile 0 to 24 on all borders. Provides seamless dark blue visual backdrop.
2. **Layer 2: Shallow Water** (`tilelayer`, z=1)
   * Riverbeds, inland oasis, shoreline transitions, and marsh ponds.
3. **Layer 3: Ground Base** (`tilelayer`, z=2)
   * Continuous biome foundation: lush green grass, golden desert sand, crisp taiga snow, dark peat mud, and gray mountain slate.
4. **Layer 4: Terrain Details** (`tilelayer`, z=3)
   * Dense foliage patches, rocky scree, wild desert scrub, tundra frost patches, and marsh reeds.
5. **Layer 5: Paths and Roads** (`tilelayer`, z=4)
   * High-traffic transit networks: packed earth roads, stone bridges, mountain stairways, and wooden causeways.
6. **Layer 6: Collision Layer** (`tilelayer`, invisible in game, parsed by engine)
   * Explicit solid tile flags for deep ocean water, unfordable river stretches, and sheer mountain cliff edges.
7. **Layer 7: Environmental Objects** (`objectgroup`, y-sorted with entities)
   * 448 structured environmental instances: cabins, ruined temples, stone walls, watchtowers, oak trees, pine trees, and large boulders.

---

## 4. Object Group Schema & Custom Properties
Environmental objects are placed with standard Tiled object properties parsed into `EnvironmentObject` instances:
* `name`: Functional identifier (e.g., `cabin_01`, `fortress_keep`, `oak_tree_04`).
* `type`: Asset type tag (e.g., `cabin`, `pine_tree`, `rock_large`). Automatically resolved to texture ID `tex_<type>`.
* `x`, `y`, `width`, `height`: World position and dimensions.
* `properties`:
  * `collision` (`bool`): Enables physics collision box.
  * `collision_width` (`float`): Custom collision box width (default: object width).
  * `collision_height` (`float`): Custom collision box height (default: 40% height for top-down perspective depth).
  * `collision_offset_x`, `collision_offset_y`: Positional offsets aligning collision to the object's base/footprint.

