# Wildbound — Snow Biome Design (Frostfall Taiga)

## 1. Geographical Identity & Aesthetic Direction

* **Biome Name**: Frostfall Taiga (`snow`)
* **Color Palette**: Fresh powder white (`#eef3fa`), icy cobalt-blue (`#93c5d6`), deep frozen water navy (`#1e3d59`), and cold granite gray (`#4a5463`).
* **Visual Theme**: Cold subarctic coniferous forest, deep snowdrifts, frozen glacial rivers, frost-rimed granite outcrops, pine trees heavy with snow, and fortified log cabins built to survive blizzards.

## 2. Tileset Specifications

* **Asset Key**: `snow_tileset`
* **File Path**: `assets/tilesets/snow/snow_tileset.png`
* **Resolution**: 256×256 px (8×8 grid of 32×32 px tiles, 64 total tiles).
* **Key Tile Classes**:
  * *Fresh Powder Snow*: Soft white snow with subtle blue shading in depressions.
  * *Compacted Snow & Ice*: Hard-packed trail snow, slippery glacial ice sheets.
  * *Frozen Water*: Deep sub-zero blue ice with frozen stress fracture details.
  * *Granite Crag Outcrops*: Snow-capped cliff edges and rocky boulders.
  * *Log Cabin Foundations*: Heavy timber walls, frost-covered wood planking.

## 3. Prototype Map Specifications

* **Map File**: `maps/snow/snow_01.json`
* **Tile Dimensions**: 60×45 tiles (1920×1440 world pixels).
* **Key Landmarks & Geographic Zones**:
  1. *Frostfall Glacial Stream*: A central frozen watercourse cutting through the taiga.
  2. *Northern Timber Outpost*: An alpine refuge with stone fireplace, wood piles, and smoke.
  3. *Southern Granite Crags*: Elevated rocky plateau with dense clusters of pine trees.
  4. *Snowbound Trail*: Beaten pathway guiding players across the subarctic terrain.
* **Objects & Spawners**: 29 map objects including snow pine trees, log cabins, campfires, and ambient taiga birds.

## 4. Atmospheric Configuration

* **Weather**: Subarctic Blizzard / Snow Flurries (`subarctic_chill`)
* **Fog Density**: 0.45 (dense cold winter haze rolling across valleys).
* **Cloud Layer**: Heavy snow clouds moving at 10.0 units/sec.
* **Ambient Lighting Tint**: Cool crisp blue-white daylight (`#e6f0ffa0`).

