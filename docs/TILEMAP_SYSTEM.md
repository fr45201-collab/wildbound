# Tilemap System Specification — Wildbound Engine

## 1. Grid Standards
* **Tile Dimensions**: 32 × 32 pixels.
* **Internal Resolution**: 640 × 360 pixels (~20 columns × 11.25 rows visible at zoom 1.0x).
* **Sample Forest Map Dimensions**: 80 × 60 tiles (2560 × 1920 pixels world space).

---

## 2. Global Tile ID (GID) Mapping
Tiles in Tiled are identified by Global Tile IDs (GIDs):
* `GID = 0`: Empty / transparent cell (no tile drawn).
* `GID >= firstGid`: Valid tile in tileset starting at `firstGid`.

### Source Rectangle Math
Given GID `g`, tileset starting GID `firstGid`, column count `C`, tile width `W`, tile height `H`, spacing `S`, and margin `M`:
```
localId = g - firstGid
col = localId % C
row = localId / C
u = M + col * (W + S)
v = M + row * (H + S)
sourceRect = Rectangle{ u, v, W, H }
```

---

## 3. Layer Ordering Standards
Wildbound maps follow a standardized 6-layer structure rendered from bottom to top:

1. **`Ground`** (tilelayer): Base terrain (lush grass, variant grasses with wildflowers, dark mossy grass). Fully populated across all cells.
2. **`Terrain`** (tilelayer): Dirt pathways, cobblestone paving, elevation transitions.
3. **`Water`** (tilelayer): Flowing rivers, ponds, shorelines, water lilies, and ripple animations.
4. **`Ground Decorations`** (tilelayer): Mushrooms, pebbles, fallen leaves, tall grass tufts, campfires, crates, barrels.
5. **`Environmental Objects`** (objectgroup): Y-sorted standalone entities (oak trees, pine trees, bushes, granite boulders, wooden cabin).
6. **`Collision`** (tilelayer / metadata): Solid walls, map boundary limits, and water barriers. Rendered only in debug mode.

---

## 4. Coordinate Conversions
```cpp
// World to Tile (clamped to grid coordinates)
Vector2 tileCoords = tilemap.WorldToTile(worldPos);

// Tile to World (returns top-left corner of tile cell in world pixels)
Vector2 worldPos = tilemap.TileToWorld(col, row);
```
Out-of-bounds tile lookups via `TileLayer::GetTileGid(col, row)` return `0` safely without throwing exceptions or causing memory corruption.
