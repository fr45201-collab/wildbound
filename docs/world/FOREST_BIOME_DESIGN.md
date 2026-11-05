# WILDBOUND — FOREST BIOME DESIGN SPECIFICATION

## 1. Executive Summary & Design Philosophy

The **Forest Biome** represents Wildbound's premier, fully realized natural battle-royale and adventure landscape.

Building on the Phase 0–11 architecture, Phase 12 expands the initial prototype into a vast, cohesive, and feature-rich biome. The design philosophy emphasizes:
- **Natural Organic Cohesion**: Smooth transitions between dense woodlands, open meadows, rolling hills, and riverbanks rather than disconnected, artificial tiles.
- **Strategic Exploration & Traversal**: An intuitive network of main roads, secondary dirt tracks, and secluded woodland trails that encourage exploration while ensuring clear routes toward points of interest.
- **Environmental Storytelling**: Hand-placed settlements, ruined watchtowers, sunken shrines, and hidden coves that convey history without requiring linear exposition.
- **High-Performance Spatial Partitioning**: Decoupling visual rendering, tile layers, and physical collision bounding boxes to maintain $>1000$ FPS at internal resolution ($640 \times 360$).

---

## 2. Technical Map Dimensions & Geometry

| Property | Value | Notes |
|---|---|---|
| **Map Width** | **160 tiles** | Expanded from prototype 80 tiles ($2\times$ width). |
| **Map Height** | **120 tiles** | Expanded from prototype 60 tiles ($2\times$ height). |
| **Tile Dimensions** | **$32 \times 32$ pixels** | Pixel-perfect grid adhering to Phase 6 Art Bible. |
| **World Dimensions** | **$5120 \times 3840$ pixels** | Total surface area: $19.66$ megapixels ($4\times$ prototype area). |
| **Active Map Layers** | **6 Layers** | Ground, Terrain, Water, Ground Decorations, Environmental Objects, Collision. |
| **Total Placed Objects** | **392 Objects** | Houses, cabins, ruins, trees, rocks, fences, props, and animation anchors. |
| **Static Collision Bounds** | **2,266 AABBs** | Built into spatial index during map initialization. |

---

## 3. Geographical Subregion Layout

The $160 \times 120$ tile world is divided into nine interconnected macro-subregions:

```
+---------------------------------------------------------------------------------------------------------+
| [R0-R35, C0-C50]                | [R0-R35, C51-C105]              | [R0-R35, C106-C159]                 |
| Subregion 1: Whispering Pines   | Subregion 2: Northern Ridge &   | Subregion 3: Greenwood Hamlet &     |
| & Foothills (Dense Pine Forest, | Waterfall Cove (Rocky slopes,   | Farmlands (Settlement 1: Cottages,  |
| Mountain crags, pioneer cabin)  | secluded waterfall, lake feed)  | Barn, Stone Well, Fenced pastures)  |
+---------------------------------+---------------------------------+-------------------------------------+
| [R36-R75, C0-C50]               | [R36-R75, C51-C105]             | [R36-R75, C106-C159]                |
| Subregion 4: Silver Lake &      | Subregion 5: Emerald Grove &    | Subregion 6: Riverwood Outpost &    |
| Sunken Islet (Major lake, sandy | Crossroads (Central heartland,  | Crossing (Settlement 2: Wooden      |
| shores, shrine islet, ripples)  | old cabin, highway crossroads)  | bridges, river shop, angler camp)   |
+---------------------------------+---------------------------------+-------------------------------------+
| [R76-R119, C0-C50]              | [R76-R119, C51-C105]            | [R76-R119, C106-C159]               |
| Subregion 7: Sunlit Meadows &   | Subregion 8: Serpentine River   | Subregion 9: Forgotten Ruins &      |
| Rolling Hills (Open fields,     | Basin & Southern Woods (Deep    | Ancient Watchtower (Abandoned       |
| wildflowers, rocky knolls)      | winding river, valley, rapids)  | stone tower, ruined arches, shrine) |
+---------------------------------------------------------------------------------------------------------+
```

### Detailed Subregion Descriptions

#### Subregion 1: Whispering Pines & Foothills (NW)
- **Coordinates**: $X \in [0, 1600]$, $Y \in [0, 1120]$ (Tiles $C \in [0, 50]$, $R \in [0, 35]$).
- **Geography**: Dense, elevated boreal pine forest with steep rocky ridges. High canopy coverage creates deep shade punctuated by wild mushrooms and mossy logs.
- **Key Features**:
  - **Highland Homestead** $(X: 800, Y: 750)$: Isolated log cabin (`cabin_wood_001`), grain storage (`storage_granary_001`), stone fences, and torchlight.
  - **Western Forest Trailhead** $(X: 256, Y: 960)$: Historic player spawn location opening into the western woods.
  - **Pine Ridge Lookout** $(X: 800, Y: 300)$: Natural granite vantage point overlooking the northern foothills.

#### Subregion 2: Northern Ridge & Waterfall Cove (North Center)
- **Coordinates**: $X \in [1632, 3360]$, $Y \in [0, 1120]$ (Tiles $C \in [51, 105]$, $R \in [0, 35]$).
- **Geography**: Dramatic granite cliff terrace rising 2 tiles high, forming a natural geological barrier.
- **Key Features**:
  - **Misty Basin & Waterfall Cove** $(X: 2400, Y: 460)$: A massive torrential waterfall (`waterfall_large_anim`) flanked by a medium cascade (`waterfall_medium_anim`) feeding the origin of the Serpentine River.
  - **Hidden Cove Alcove**: Secluded mist-shrouded hollow behind overhanging rock faces.

#### Subregion 3: Greenwood Hamlet & Farmlands (NE)
- **Coordinates**: $X \in [3392, 5120]$, $Y \in [0, 1120]$ (Tiles $C \in [106, 159]$, $R \in [0, 35]$).
- **Geography**: Gently rolling, cleared woodland pastureland and agricultural meadows surrounding a rustic village settlement.
- **Key Features**:
  - **Greenwood Hamlet** $(X: 4280, Y: 750)$: The largest settlement in the biome, featuring a Town Hall (`house_large_001`), residential cottages (`house_small_001`), a community barn (`storage_barn_001`), a public stone well (`well_stone_001`), iron lamp posts, picnic benches, and wood-fenced sheep paddocks.
  - **Lily Pond** $(X: 4000, Y: 480)$: Serene northern pond with water ripples and wildflower banks.

#### Subregion 4: Silver Lake & Sunken Islet (CW)
- **Coordinates**: $X \in [0, 1600]$, $Y \in [1152, 2400]$ (Tiles $C \in [0, 50]$, $R \in [36, 75]$).
- **Geography**: A sprawling, irregularly shaped natural lake covering over $1000 \times 960$ pixels of water, bordered by sandy banks and bulrushes.
- **Key Features**:
  - **Sunken Shrine Islet** $(X: 832, Y: 1792)$: A small island situated in the center of the lake, reachable via submerged stone shallows. Features ancient carved stone arches (`ruin_arch_stone_001`), stone pillars, and rare azure butterflies.
  - **Lake Shimmer Expanses**: Animated specular wave crests (`water_lake_anim`) and expanding shoreline ripples (`water_ripple_anim`).

#### Subregion 5: Emerald Grove & Crossroads (Center)
- **Coordinates**: $X \in [1632, 3360]$, $Y \in [1152, 2400]$ (Tiles $C \in [51, 105]$, $R \in [36, 75]$).
- **Geography**: The lush heart of the forest, filled with colossal ancient oak trees (`tree_oak_003`, `tree_oak_004`) and wide, inviting clearings.
- **Key Features**:
  - **Grand Crossroads** $(X: 2560, Y: 1800)$: The central junction where the King's Highway connects the northern hamlet, western lake, eastern river crossing, and southern plains.
  - **Abandoned Overgrown Cabin** $(X: 1920, Y: 480)$: The original prototype forest cabin preserved and enhanced with overgrown vegetation, mossy fallen logs, and an aged stump.

#### Subregion 6: Riverwood Outpost & Crossing (CE)
- **Coordinates**: $X \in [3392, 5120]$, $Y \in [1152, 2400]$ (Tiles $C \in [106, 159]$, $R \in [36, 75]$).
- **Geography**: A busy riverside trading post established where the main highway crosses the Serpentine River via a sturdy wooden bridge.
- **Key Features**:
  - **Riverwood Outpost** $(X: 3520, Y: 1750)$: General store/trading tavern (`shop_general_001`), hunter's lodge (`cabin_hunter_001`), storage shed (`storage_shed_001`), commercial crates, barrels, and a central outdoor gathering campfire (`fire_medium_anim`).
  - **Great Riverwood Bridge** $(X: 3520, Y: 1760)$: $64 \times 48$ wooden deck bridge over rushing river currents (`water_flow_anim`).

#### Subregion 7: Sunlit Meadows & Rolling Hills (SW)
- **Coordinates**: $X \in [0, 1600]$, $Y \in [2432, 3840]$ (Tiles $C \in [0, 50]$, $R \in [76, 119]$).
- **Geography**: An open expanse of tall swaying grass, rolling knolls, and vibrant wildflower carpets. Designed to provide optimal space for player movement and large-scale combat encounters.
- **Key Features**:
  - **Wildflower Fields**: Dense clusters of white, yellow, and red wildflowers populated by fluttering butterflies.
  - **Smuggler's Rock Cache** $(X: 480, Y: 3300)$: Hidden alcove concealed behind mossy boulders and thick bushes containing concealed crates.

#### Subregion 8: Serpentine River Basin & Southern Woods (South Center)
- **Coordinates**: $X \in [1632, 3360]$, $Y \in [2432, 3840]$ (Tiles $C \in [51, 105]$, $R \in [76, 119]$).
- **Geography**: A deep, winding river valley where the Serpentine River loops in broad meanders before emptying westward into Silver Lake.
- **Key Features**:
  - **South Basin Wooden Bridge** $(X: 2950, Y: 3000)$: Traversal bridge connecting the central crossroads highway to the southern wilderness.
  - **Destroyed Campsite & Broken Bridge**: Environmental storytelling landmark featuring scorched campfire embers, broken crates, and overturned logs.

#### Subregion 9: Forgotten Ruins & Ancient Watchtower (SE)
- **Coordinates**: $X \in [3392, 5120]$, $Y \in [2432, 3840]$ (Tiles $C \in [106, 159]$, $R \in [76, 119]$).
- **Geography**: Weathered, overgrown stone ruins of an ancient civilization swallowed by dense oak woodlands.
- **Key Features**:
  - **Ruined Stone Watchtower** $(X: 4380, Y: 3180)$: A multi-tier stone tower (`ruin_tower_stone_001`) with collapsed wall segments (`ruin_wall_stone_001`) and an ascending dark smoke plume (`smoke_plume_anim`).
  - **Overgrown Hermit's Shed** $(X: 4680, Y: 2080)$: Isolated moss-covered storage shelter in deep eastern woods.

---

## 4. Roads, Paths & Bridges Network

1. **The King's Highway (Main Road)**: 2.5-tile wide packed dirt highway connecting the North-East border through Greenwood Hamlet, curving down to the Central Crossroads $(2560, 1800)$, and continuing south to the southern map boundary.
2. **The East-West Arterial**: Extends from Silver Lake's eastern shore across the Central Crossroads, over the Riverwood Bridge, and to the eastern map boundary.
3. **The Highland Trail**: A winding 1.8-tile secondary dirt path linking the Western Spawn Trailhead and the Crossroads to the Highland Homestead.
4. **The Ruins Path**: A woodland path branching south-east from Riverwood down to the Ancient Watchtower.
5. **The Waterfall Way**: A dedicated pathway following the northern river fork up into the Waterfall Cove.
6. **Bridges**: All water crossings feature full wooden plank decking (GID 33) and physical bridge objects (`bridge_wood_001`). Crucially, bridge tiles are explicitly omitted from the collision layer, ensuring $100\%$ seamless player traversal across rivers.

---

## 5. Map Layer Hierarchy

```
Layer 6: Collision           [Tilelayer]   - Solid collision tiles (GID 57: cliff/barrier, GID 58: deep water).
Layer 5: Environmental Objects[Objectgroup] - 392 physical props, buildings, trees, and animation anchors.
Layer 4: Ground Decorations   [Tilelayer]   - Wildflowers, pebbles, clovers, mushrooms, undergrowth tufts.
Layer 3: Water               [Tilelayer]   - Silver Lake, Serpentine River, Misty Basin, Lily Pond.
Layer 2: Terrain             [Tilelayer]   - Connected roads, secondary trails, bridges, rocky cliffs.
Layer 1: Ground              [Tilelayer]   - Base organic grass procedural variations.
```

---

## 6. Collision & Traversal Architecture

- **Separated Visual vs. Physical Footprints**: In accordance with the Phase 6 Art Bible, broad tree canopies, building roofs, and eaves do not block movement. Physical collision boxes are mapped strictly to tree trunks (e.g. $16 \times 16$ px at trunk base) and building foundation footprints.
- **Water Impassability with Bridge Traversal**: Deep water tiles carry GID 58 in the `Collision` layer, blocking player and enemy movement. Wooden bridge spans carry GID 0 in `Collision`, allowing uninterrupted traversal across water bodies.
- **Cliff Walls**: Northern ridge walls are blocked by GID 57, except where carved switchback paths and waterfall cove openings allow ascent.

