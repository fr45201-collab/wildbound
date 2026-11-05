# Wildbound — Greenwood Village Design Specification

**Phase:** Phase 13 — Village Construction  
**Location:** Subregion 3 (North-East), Greenwood Hamlet & Commons  
**World Coordinates:** X: 3600–4800, Y: 500–1300 (Tile Columns 114–150, Rows 16–41)  
**Footprint:** $1200 \times 800$ pixels ($37.5 \times 25$ tiles)

---

## 1. Overview & World Context

**Greenwood Village** is the primary civilian settlement nestled within the verdant North-East temperate woodlands of Wildbound's expanded Forest Biome. Founded as a timber outpost and trading hamlet along the Northern Highway, Greenwood has evolved into a thriving, self-sufficient community serving farmers, foresters, craftsmen, and travelers moving between the eastern riverlands and the high western trails.

The settlement is planned around organic medieval village principles:
1. **Central Gathering Core:** A paved cobblestone square with a public stone well, community noticeboard, and bustling market stalls.
2. **Radial Functional Districts:** Clear zoning separating commercial crafts, civic administration, residential hearths, and perimeter fortifications.
3. **Paved & Traversed Street Grid:** Connected walkways linking Highway 1 directly to shopfronts, residential gates, and outbuildings without collision obstruction.
4. **Architectural Cohesion:** Consistent rustic timber frames, stone foundation masonry, warm cedar shingle roofs, glowing lanterns, and smoking chimneys.

```
+-----------------------------------------------------------------------------------+
|                            GREENWOOD VILLAGE OVERVIEW                             |
+-----------------------------------------------------------------------------------+
|                                                                                   |
|                      [Civic & Manor District]                                     |
|                      * Greenwood Manor & Town Hall (Enterable)                    |
|                      * Formal Stone Gardens & Benches                             |
|                                     |                                             |
|   [Commercial District]             |              [Residential District]         |
|   * General Store (Enterable)       |              * Forester's Cottage (Enterable|
|   * Blacksmith & Forge (Enterable) -+-[CENTRAL SQ]-* Two-Story Residence (Enter) |
|   * Village Warehouse (Enterable)   |  * Well      * Mason's Cottage              |
|   * Community Tool Shed             |  * Market    * Weaver's House               |
|   * North Gate Watchtower           |  * Lanterns                                 |
|                                     |                                             |
|                                     |              [Residential Fringe]           |
|                                     +--------------* Trapper's Cabin              |
|                                     |              * South Guard Post             |
|                              (To River Crossings)                                 |
+-----------------------------------------------------------------------------------+
```

---

## 2. Settlement Districts & Zoning

### 2.1 Central Market Square (The Village Commons)
* **Coordinates:** X: 4150–4380, Y: 740–920
* **Visual Anchor:** **Central Stone Well** (`well_stone_001` at X: 4280, Y: 780), a deep freshwater well capped with ancient fieldstone and timber rope winding.
* **Surface Paving:** Paved with flagstone and cobblestone (Tile GID 17 and 9), transitioning seamlessly to packed earth roads.
* **Market Activity:**
  - **Market Pavilion** (`shop_market_001` at X: 4180, Y: 840): Open-sided timber canopy with woven fabric awning where local farm produce is traded.
  - **Produce Stacks:** Timber crates (`crate_large_001`) and standard wooden barrels (`barrel_standard_001`) loaded with apples, grains, and salted goods.
  - **Civic Infrastructure:** Village Noticeboard (`sign_noticeboard_001` at X: 4210, Y: 810) displaying bounties and notices; Crossroads Direction Sign (`sign_direction_001` at X: 4100, Y: 800).
  - **Street Furniture:** Woven timber benches (`bench_wooden_001`), decorative iron lamp posts (`lamp_post_001`).

### 2.2 Commercial & Crafting District (West District)
* **Coordinates:** X: 3750–4120, Y: 600–1040
* **Function:** Heavy production, goods storage, metallurgy, and merchant trading.
* **Key Structures:**
  1. **General Store** (`shop_general_001` at X: 3820, Y: 720, Enterable): Two-story storefront marked by hanging shop sign (`sign_shop_001`), offering provisions and tools.
  2. **Blacksmith's Forge** (`blacksmith_forge_001` at X: 3800, Y: 920, Enterable): Heavy stone-and-beam workshop featuring outdoor charcoal bins, anvil stand, animated chimney smoke plume (`smoke_chimney_anim` at X: 3870, Y: 870), and open outdoor hearth fire (`fire_medium_anim` at X: 3760, Y: 940).
  3. **Village Warehouse & Granary** (`storage_warehouse_001` at X: 3980, Y: 600, Enterable): High-capacity cargo repository with reinforced doors and loading dock.
  4. **Community Tool Shed** (`storage_shed_001` at X: 4020, Y: 960): Secured outbuilding storing plows, timber axes, and repair planks.
  5. **North Gate Watchtower** (`watchtower_wood_001` at X: 3660, Y: 820): Timber observation platform guarding the western entrance road from the crossroads.

### 2.3 Civic & Manor District (North District)
* **Coordinates:** X: 4180–4450, Y: 520–700
* **Function:** Governance, judicial council, historical archives, and elder residence.
* **Key Structures:**
  1. **Greenwood Manor & Town Hall** (`house_residence_large_001` at X: 4240, Y: 560, Enterable): Stately two-story timber-and-slate mansion with twin dormers, stone hearth chimney (`smoke_chimney_anim` at X: 4350, Y: 520), and wide oak front doors.
  2. **Formal Gardens & Promenade:** Enclosed by weathered stone fences (`fence_broken_001`), manicured flowerbeds, stone resting benches (`bench_stone_001`), and ornamental iron lampposts.

### 2.4 Residential District & Outskirts (East & South District)
* **Coordinates:** X: 4400–4750, Y: 700–1300
* **Function:** Domestic housing, craft workshops, family gardens, and boundary scouting.
* **Key Structures:**
  1. **Forester's Cottage** (`house_cottage_001` at X: 4460, Y: 720, Enterable): Timber residence with front garden, firewood stack, and cedar shingles.
  2. **Miller's Two-Story Residence** (`house_twostory_001` at X: 4420, Y: 940, Enterable): Multi-room domestic homestead with second-story living loft.
  3. **Mason's Stone Cottage** (`house_stone_small_001` at X: 4620, Y: 740): Compact residence crafted from dry-stacked river stones.
  4. **Weaver's Village House** (`house_village_001` at X: 4640, Y: 940): Extended woodhouse with outdoor drying racks and textile frames.
  5. **Trapper's Cabin** (`cabin_woodland_001` at X: 4600, Y: 1120): Frontier log cabin positioned at the deep forest perimeter.
  6. **South Hill Guard Post** (`watchtower_stone_001` at X: 4320, Y: 1140): Fortified stone tower looking down toward the river crossings.

---

## 3. Street Network & Pathway Connectivity

Greenwood Village features a dedicated hierarchic street network constructed with collision-free paved pathways:
* **Highway 1 (Main Road):** 2.5-tile wide arterial dirt highway entering from Crossroads (C105, R42) into the village at C130, R20.
* **Plaza Cobblestones:** Paved rectangular square spanning C128..138, R23..29, connecting all four district arteries.
* **Commercial West Lane:** 2.2-tile paved street branching to the General Store and Blacksmith with 2.0-tile sub-lanes directly to doorways.
* **Residential East Avenue:** 2.2-tile paved lane linking residential porches, cottages, and gardens.
* **South River Road:** 2.2-tile thoroughfare exiting south past the guard post toward the Riverwood bridges.

Every building's door entrance coordinates have an unobstructed 32px clearance zone free of trees, boulders, or water collision, ensuring fluid movement and intuitive approach angles.

---

## 4. Atmospheric Details & Environmental Integration

1. **Active Illumination:**
   - 4 cast-iron street lamps (`lamp_post_001`) illuminate the square and commercial lane at night.
   - Forge fires and fireplace hearths provide warm localized radiance.
2. **Procedural Atmospheric Particles:**
   - Dual chimney smoke emitters (`smoke_chimney_anim`) over Town Hall and Blacksmith forge.
   - Ground wind-blown leaves and drifting grass rustling on outskirts.
3. **Audio-Visual Atmosphere:**
   - Ambient birds nesting in garden trees (`ambient_bird`).
   - Butterflies drifting over square flowerbeds (`ambient_butterfly`).

