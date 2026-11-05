# Wildbound — Desert Biome Design (Sunscorched Wastes)

## 1. Geographical Identity & Aesthetic Direction

* **Biome Name**: Sunscorched Wastes (`desert`)
* **Color Palette**: Warm ochres (`#e2bb7b`), baked red clay (`#8c4d29`), sun-bleached sandstone (`#996335`), and oasis turquoise (`#1e6d78` / `#38a3a5`).
* **Visual Theme**: Shifting windblown dunes, sun-cracked arroyo basins, dried caravan trails, ancient crumbling sandstone columns, and rare lifegiving desert oases surrounded by desert palms and scrub vegetation.

## 2. Tileset Specifications

* **Asset Key**: `desert_tileset`
* **File Path**: `assets/tilesets/desert/desert_tileset.png`
* **Resolution**: 256×256 px (8×8 grid of 32×32 px tiles, 64 total tiles).
* **Key Tile Classes**:
  * *Sand Dunes*: Soft undulating ridges, ripple patterns, wind erosion.
  * *Cracked Clay*: Parched mudflats with distinctive mosaic fissuring.
  * *Caravan Trails*: Trampled dusty paths guiding traversal across dunes.
  * *Oasis Water & Shore*: Clear shallow banks transitioning to deep turquoise water.
  * *Sandstone Ruins*: Weathered masonry blocks, pillar shafts, decorative friezes.
  * *Desert Scrub*: Hardy thorny bushes resilient to intense arid conditions.

## 3. Prototype Map Specifications

* **Map File**: `maps/desert/desert_01.json`
* **Tile Dimensions**: 60×45 tiles (1920×1440 world pixels).
* **Key Landmarks & Geographic Zones**:
  1. *Western Oasis Basin*: Centered freshwater reservoir bordered by sandy banks and palms.
  2. *Caravan Crossroads*: Traversal hub connecting west and east through parched dirt trails.
  3. *Eastern Sandstone Ruins*: Remnants of a fallen desert citadel featuring stone pillars and arches.
  4. *Northern Dune Ridges*: Elevated dunes channeling movement along valley corridors.
* **Objects & Spawners**: 22 map objects including crates, stone arches, ruined columns, and animated fire pits.

## 4. Atmospheric Configuration

* **Weather**: Arid Heat (`arid_heat`)
* **Fog Density**: 0.05 (near-zero ground fog; high visibility with heat haze).
* **Cloud Layer**: Sparse, high-velocity cirrus drifting at 12.0 units/sec.
* **Ambient Lighting Tint**: Warm sunlight wash (`#fff5dca0`).

