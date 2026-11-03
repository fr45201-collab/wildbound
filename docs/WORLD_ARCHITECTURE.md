# World Subsystem Architecture — Wildbound Engine

## 1. Overview & Purpose
The **World Subsystem** in Wildbound manages 2D data-driven tile environments, coordinate transformations, environmental object placement, depth sorting, and spatial collision indexing. It bridges static level authored in Tiled Map Editor (`.tmx`/`.json`) with the real-time raylib renderer and game loop.

```
                              ┌────────────────────────┐
                              │    Tiled Map Editor    │
                              │ (.tmx XML / .json TMJ) │
                              └───────────┬────────────┘
                                          │
                                          ▼
                              ┌────────────────────────┐
                              │   world::MapLoader     │
                              │ (Parses CSV, GIDs,     │
                              │  Objects & Properties) │
                              └───────────┬────────────┘
                                          │
                     ┌────────────────────┴────────────────────┐
                     ▼                                         ▼
         ┌────────────────────────┐                ┌────────────────────────┐
         │     world::Tilemap     │                │ resources::AssetManager│
         │  - Tilesets & GIDs     │                │  - Cached Textures     │
         │  - Tile Layers (2D)    │                │  - Spritesheets        │
         │  - Environment Props   │                │  - Nearest-Neighbor pt │
         └───────────┬────────────┘                └───────────┬────────────┘
                     │                                         │
                     ▼                                         │
         ┌────────────────────────┐                            │
         │      world::World      │                            │
         │  - Spatial Collision   │                            │
         │  - Visible Queries     │                            │
         └───────────┬────────────┘                            │
                     │                                         │
                     ▼                                         ▼
         ┌─────────────────────────────────────────────────────────┐
         │                  world::WorldRenderer                   │
         │  - Camera Bounding Box Visibility Culling               │
         │  - Visible Tile Range Clamping                          │
         │  - Y-Sorted Object Depth Ordering                       │
         │  - Debug Overlays (Grid, Bounds, Collision, Origins)    │
         └─────────────────────────────────────────────────────────┘
```

---

## 2. Component Structure

| Component | Header / Source | Description |
|---|---|---|
| `Tileset` | `src/world/Tileset.hpp/.cpp` | Manages tileset dimensions, GID range calculations, margins, spacing, and UV source rectangles. |
| `TileLayer` | `src/world/TileLayer.hpp/.cpp` | Flat contiguous 2D grid of global tile IDs (GIDs), visibility state, and alpha opacity. |
| `EnvironmentObject` | `src/world/EnvironmentObject.hpp/.cpp` | Data-driven environmental prop with origin anchor, bounds, texture key, and Y-depth sorting. |
| `Tilemap` | `src/world/Tilemap.hpp/.cpp` | Master map container holding layers, tilesets, object groups, and coordinate conversions. |
| `MapLoader` | `src/world/MapLoader.hpp/.cpp` | Robust parser for Tiled JSON (`.json`/`.tmj`) and TMX XML (`.tmx`) with CSV tile decompression. |
| `MapValidator` | `src/world/MapValidator.hpp/.cpp` | Audits tilemaps for invalid GIDs, orphaned tilesets, negative dimensions, and collision inconsistencies. |
| `World` | `src/world/World.hpp/.cpp` | High-level world manager responsible for map lifecycle, spatial collision index, and visibility queries. |
| `WorldRenderer` | `src/world/WorldRenderer.hpp/.cpp` | High-performance camera-culled renderer employing visible boundary clamping and depth sorting. |

---

## 3. High-Performance Camera Culling Pipeline
On an 80×60 tilemap (4,800 tiles per layer across 5 layers = 24,000 tiles), iterating over every tile each frame causes severe CPU overhead. `WorldRenderer` uses camera-aware frustum bounds culling:

1. **Calculate Visible World Area**:
   ```cpp
   float visibleWidth = viewport.GetVirtualWidth() / camera.GetZoom();
   float visibleHeight = viewport.GetVirtualHeight() / camera.GetZoom();
   Vector2 target = camera.GetTarget();
   Rectangle visibleRect = {
       target.x - (visibleWidth * 0.5f) - marginX,
       target.y - (visibleHeight * 0.5f) - marginY,
       visibleWidth + (marginX * 2.0f),
       visibleHeight + (marginY * 2.0f)
   };
   ```

2. **Clamp to Visible Matrix Indices**:
   ```cpp
   int startCol = std::max(0, static_cast<int>(std::floor(visibleRect.x / tw)));
   int endCol   = std::min(mapWidth - 1, static_cast<int>(std::ceil((visibleRect.x + visibleRect.width) / tw)));
   int startRow = std::max(0, static_cast<int>(std::floor(visibleRect.y / th)));
   int endRow   = std::min(mapHeight - 1, static_cast<int>(std::ceil((visibleRect.y + visibleRect.height) / th)));
   ```
   At zoom `1.0x` on a 640×360 virtual canvas with 32×32 tiles, only ~22×13 tiles (286 tiles) are rendered per layer, reducing draw calls by **over 94%**.

---

## 4. Entity Depth & Y-Sorting
Top-down perspective requires environmental objects (trees, buildings, bushes, rocks) to render in depth order according to their vertical baseline:
* Entities with smaller `GetZOrder()` (higher up on screen) render first (background).
* Entities with larger `GetZOrder()` (lower down on screen) render later (foreground).
* The anchor origin is defined at the entity's feet (bottom-center: `x = width * 0.5f, y = height`), ensuring correct visual overlap when a player or prop walks behind tree canopies or cabin facades.
