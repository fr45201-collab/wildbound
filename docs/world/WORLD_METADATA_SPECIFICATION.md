# Wildbound — World Metadata Specification

## 1. Directory Structure & File Manifest
All high-level world design parameters, regional geometries, points of interest, and spawn configurations reside in data-driven JSON files located in `data/world/`:

```
data/world/
  ├── battle_royale_island.json  # Root world definitions & playable dimensions
  ├── biome_regions.json         # Biome bounding rectangles, tilesets, audio profiles
  ├── settlements.json           # Major villages, centers, radii, building allotments
  ├── points_of_interest.json    # 14 strategic landmarks & gameplay functions
  ├── spawn_regions.json         # 12 deployment zones & 48 safe spawn markers
  ├── loot_locations.json        # 45 container markers & loot tier categories
  ├── traversal_routes.json      # 5 primary highways & 3 strategic bridge chokepoints
  └── world_landmarks.json       # Visual monuments & historical geography
```

---

## 2. JSON Schema Specifications

### A. Root World Definition (`battle_royale_island.json`)
```json
{
  "worldId": "battle_royale_island",
  "name": "Isle of Wildbound",
  "author": "Wildbound World Design Team",
  "version": "1.0.0",
  "dimensions": {
    "widthTiles": 320,
    "heightTiles": 320,
    "tileWidth": 32,
    "tileHeight": 32,
    "worldWidth": 10240.0,
    "worldHeight": 10240.0
  },
  "playableBoundary": {
    "x": 768.0,
    "y": 768.0,
    "width": 8704.0,
    "height": 8704.0
  },
  "defaultSpawn": {
    "x": 4160.0,
    "y": 5920.0
  },
  "counts": {
    "biomes": 5,
    "settlements": 5,
    "pointsOfInterest": 14,
    "spawnRegions": 12,
    "spawnMarkers": 48,
    "lootLocations": 45,
    "primaryRoutes": 5,
    "chokepoints": 3
  }
}
```

### B. Biome Regions (`biome_regions.json`)
```json
{
  "biomes": [
    {
      "id": "biome_verdant_forest",
      "name": "Verdant Forest",
      "tileset": "forest_tileset",
      "bounds": { "x": 0.0, "y": 4800.0, "width": 5120.0, "height": 5440.0 },
      "ambientAudio": "amb_forest_river",
      "footstepMaterial": "grass",
      "dominantPalette": ["#2e6b33", "#438a3d", "#5ba34f"],
      "description": "Lush temperate forest featuring dense oak canopies, rolling hills, and clean river tributaries."
    }
  ]
}
```

### C. Settlements (`settlements.json`)
```json
{
  "settlements": [
    {
      "id": "settlement_greenwood",
      "name": "Greenwood Village",
      "biomeId": "biome_verdant_forest",
      "center": { "x": 4160.0, "y": 5920.0 },
      "radius": 480.0,
      "buildingCount": 12,
      "settlementType": "Major Village",
      "description": "Thriving woodland agricultural hamlet and primary western settlement."
    }
  ]
}
```

### D. Points of Interest (`points_of_interest.json`)
```json
{
  "pointsOfInterest": [
    {
      "id": "poi_ancient_fortress",
      "name": "Ancient Fortress",
      "biomeId": "biome_iron_crag",
      "position": { "x": 5120.0, "y": 4800.0 },
      "radius": 384.0,
      "lootTier": "Tier 4 (Legendary)",
      "strategicImportance": "Highest - Central geographical hub, primary rotation chokepoint",
      "hasFastTravelBeacon": false
    }
  ]
}
```

### E. Traversal Routes & Chokepoints (`traversal_routes.json`)
```json
{
  "routes": [
    {
      "id": "route_forest_fortress",
      "name": "King's Forest Highway",
      "startPoi": "settlement_greenwood",
      "endPoi": "poi_ancient_fortress",
      "surface": "Cobblestone / Compact Dirt",
      "widthTiles": 4,
      "distanceUnits": 1475.0
    }
  ],
  "chokepoints": [
    {
      "id": "cp_great_stone_bridge",
      "name": "Great Stone Bridge",
      "position": { "x": 5504.0, "y": 5280.0 },
      "widthUnits": 96.0,
      "description": "Primary eastern river crossing between Central Fortress and Desert Wastes."
    }
  ]
}
```

---

## 3. C++ Ingestion & Validation Architecture
The `WorldManager` class in `src/world/WorldManager.hpp` ingests this metadata via `nlohmann::json`.
During initialization, `WorldManager::ValidateIntegrity()` verifies:
1. Playable boundaries reside strictly within world bounds.
2. Every settlement belongs to a recognized biome ID.
3. Every POI references an existing biome and lies within valid world coordinates.
4. All spawn markers lie inside the playable boundary.
5. All traversal routes connect valid POI identifiers.
6. All chokepoints possess positive passage width.

