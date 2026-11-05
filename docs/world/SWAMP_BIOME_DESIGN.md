# Wildbound — Swamp Biome Design (Murkwood Mire)

## 1. Geographical Identity & Aesthetic Direction

* **Biome Name**: Murkwood Mire (`swamp`)
* **Color Palette**: Peat moss mud (`#222119`), toxic algal scum green (`#597334`), stagnant deep bog black (`#171c14`), and decaying wood brown (`#4a3b32`).
* **Visual Theme**: Stagnant marshlands choked with cypress roots, weeping moss, duckboard walkways, decaying sunken ruins, and murky green water channels emitting eerie atmospheric vapors.

## 2. Tileset Specifications

* **Asset Key**: `swamp_tileset`
* **File Path**: `assets/tilesets/swamp/swamp_tileset.png`
* **Resolution**: 256×256 px (8×8 grid of 32×32 px tiles, 64 total tiles).
* **Key Tile Classes**:
  * *Peat Mud & Reeds*: Waterlogged soil, soggy hummocks, and tall swamp grass.
  * *Murky Bog Water*: Algae-coated stagnant channels and impassable deep mire.
  * *Duckboard Planks*: Raised wooden boardwalks keeping travellers above the bog.
  * *Mossy Stone Ruins*: Weathered swamp stones enveloped in damp vines and lichens.
  * *Twisted Cypress Roots*: Gnarled mangrove-like roots protruding from waterlogged terrain.

## 3. Prototype Map Specifications

* **Map File**: `maps/swamp/swamp_01.json`
* **Tile Dimensions**: 60×45 tiles (1920×1440 world pixels).
* **Key Landmarks & Geographic Zones**:
  1. *Central Murk Canal*: Deep impassable mire dividing the east and west sectors.
  2. *Elevated Boardwalk Circuit*: Wooden plank trails connecting dry hummocks.
  3. *Sunken Hermit's Hovels*: Decayed wooden shacks standing on stilts.
  4. *Altar of the Mire*: Overgrown stone dais hidden deep in weeping willows.
* **Objects & Spawners**: 23 map objects including gnarled trees, water ripple animations, and damp crates.

## 4. Atmospheric Configuration

* **Weather**: Murky Mist / Humid Drizzle (`murky_fog`)
* **Fog Density**: 0.65 (heavy ground fog and persistent marsh vapors).
* **Cloud Layer**: Dense, low-hanging overcast clouds moving sluggishly at 6.0 units/sec.
* **Ambient Lighting Tint**: Eerie yellow-green swamp light (`#dce6cca0`).

