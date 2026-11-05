# Wildbound — Biome Architecture & Configuration Specification

## 1. System Overview

The **Biome System** in Wildbound provides a scalable, modular framework for authoring, configuring, and dynamically loading distinct geographic environments across the world. Built upon Wildbound's C++20 engine architecture, the biome pipeline coordinates:

* **Terrain & Tile Palettes**: Biome-specific tilesets registered in the Asset Management System (AMS).
* **Atmospheric Parameters**: Weather effects, fog opacity, cloud drifting speeds, and ambient lighting tints.
* **Environmental Entities & Animations**: Biome-appropriate foliage rustle, water shaders/ripples, smoke, and wildlife entities.
* **Map Lifecycle & Camera Framing**: Coordinate transitions between maps, camera world bounds clamping, and spawn resolution.
* **Settlement Compatibility**: Seamless coexistence with Greenwood Village (Phase 13) and future interior settlements.

---

## 2. Core C++ Components

### 2.1 Biome Data Structures (`src/world/BiomeData.hpp`)

* `TerrainType`: Specifies tile ID (`gid`), semantic classification (`name`), and whether the terrain is walkable (`walkable`).
* `AtmosphereConfig`: Stores weather typology, fog opacity, cloud speed, cloud layer toggles, and ambient RGBA tint.
* `BiomeConfig`: Central metadata container:
  * `id`: Unique identifier (e.g., `"forest"`, `"desert"`, `"snow"`, `"swamp"`, `"mountain"`).
  * `displayName`: User-facing localized title.
  * `tileset` / `tilesetImage`: AMS asset key and relative file path.
  * `terrain`: List of supported terrain tile types.
  * `vegetation`: Approved vegetation asset keys.
  * `environmentObjects`: Approved static and interactive prop keys.
  * `animations`: Registered environmental animation keys.
  * `atmosphere`: Atmospheric visual parameters.
  * `ambientEntities`: Species list for ambient wildlife spawns.
  * `maps`: Filepaths to maps associated with this biome.
  * `defaultSpawn`: Primary default character spawn coordinate (`Vector2`).

### 2.2 Biome Manager (`src/world/BiomeManager.hpp`, `src/world/BiomeManager.cpp`)

* **Catalog Initialization**: Discovers and parses biome JSON definitions from `data/biomes/`.
* **Dynamic Biome Switching**: `SwitchBiome(biomeId, world, player, camera, assetMgr, envAnim, mapIndex)`:
  1. Validates biome and map availability.
  2. Loads target Tiled JSON map via `World::LoadMap`.
  3. Relocates player to designated biome spawn point.
  4. Recalibrates 2D camera viewport limits to match map boundaries.
  5. Reconfigures `EnvironmentalAnimationSystem` with biome atmospheric properties.
  6. Rebuilds collision spatial index.

---

## 3. Data Schema (`data/biomes/<biome>.json`)

```json
{
  "id": "desert",
  "display_name": "Sunscorched Wastes",
  "description": "An arid expanse of shifting dunes, cracked clay arroyos, sandstone ruins, and vital freshwater oases.",
  "tileset": "desert_tileset",
  "tileset_image": "tilesets/desert/desert_tileset.png",
  "terrain": [
    { "name": "Sand Dune", "gid": 1, "walkable": true },
    { "name": "Cracked Clay", "gid": 9, "walkable": true },
    { "name": "Caravan Trail", "gid": 17, "walkable": true },
    { "name": "Oasis Deep Water", "gid": 25, "walkable": false },
    { "name": "Sandstone Masonry", "gid": 33, "walkable": false },
    { "name": "Pillar Shaft", "gid": 41, "walkable": false },
    { "name": "Thorny Scrub", "gid": 49, "walkable": true }
  ],
  "vegetation": ["bush_small_001", "rock_boulder_001", "rock_flat_001"],
  "environment_objects": ["ruin_pillar_stone_001", "ruin_arch_stone_001", "crate_reinforced_001", "fire_small_anim"],
  "animations": ["water_ripple_anim", "smoke_plume_anim", "fire_small_anim"],
  "atmosphere": {
    "weather": "arid_heat",
    "fog_density": 0.05,
    "cloud_speed": 12.0,
    "cloud_layer_enabled": true,
    "ambient_tint": { "r": 255, "g": 245, "b": 220, "a": 255 }
  },
  "ambient_entities": ["ambient_bird"],
  "maps": ["maps/desert/desert_01.json"],
  "default_spawn": { "x": 320.0, "y": 720.0 }
}
```

---

## 4. Developer Suite (DevUI) Integration

A dedicated **Biome Selector & Multi-Biome World Explorer** section is embedded in **Tab 3 ("World & Map")** of the Developer Suite (`src/ui/DevUI.cpp`). Developers can:
* Inspect active biome metadata (tileset, weather, fog density, cloud status).
* Browse all registered biomes in the engine catalog.
* One-click hot-swap the active world map, reload camera bounds, and teleport the player directly into the new biome.

