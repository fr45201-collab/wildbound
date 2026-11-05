# Wildbound — Environment Asset Catalog (Phase 10)

## 1. Overview & Technical Conventions

This catalog documents the **First Substantial Environment Asset Library** produced in Phase 10 of **Wildbound**.

The collection establishes the visual, spatial, and technical foundation for the **Forest Biome**, **Village Construction**, and future fantasy environments. Every asset complies with the [Phase 6 Art Bible](file:///c:/dev/Wildbound/docs/art/WILDBOUND_ART_BIBLE.md) and integrates directly with the [Asset Management System](file:///c:/dev/Wildbound/docs/ASSET_MANAGEMENT.md), [Tilemap Subsystem](file:///c:/dev/Wildbound/docs/TILEMAP_SYSTEM.md), and [WorldRenderer](file:///c:/dev/Wildbound/docs/RENDERING_ARCHITECTURE.md).

### Core Technical Conventions:
- **Perspective**: Top-down $2.5\text{D}$ orthographic with an apparent camera inclination of $45^\circ$ to $60^\circ$.
- **Virtual Resolution**: Native 1:1 pixel rendering at $640 \times 360$ virtual canvas resolution.
- **Independent Canvas Dimensions**: Non-uniform sprite dimensions suited to each object's real-world scale (e.g. $16 \times 16$ px for flowers, up to $120 \times 112$ px for two-story buildings).
- **Decoupled Visual and Collision Footprints**: Collision boxes represent only the physical ground base or trunk contact area rather than full visual bounds.
- **Ground Anchor Alignment**: Standardized ground contact anchors $(x_{\text{center}}, y_{\text{ground}})$, guaranteeing accurate depth sorting ($Y$-sorting) and player traversal around structures.
- **Licensing & Provenance**: All 77 assets are original creations released under the **CC0 1.0 Universal License**.

---

## 2. Quantitative Summary

| Category | Subcategory | Asset Count | Dimensions Range | Collision Enabled |
| :--- | :--- | :---: | :---: | :---: |
| **Nature** | Oak Trees | 4 | $48 \times 64$ to $112 \times 128$ px | Yes (Trunk footprint) |
| | Pine Trees | 3 | $32 \times 64$ to $64 \times 96$ px | Yes (Trunk footprint) |
| | Bushes | 5 | $24 \times 24$ to $48 \times 36$ px | 4 Solid, 1 Walk-through |
| | Grass | 6 | $16 \times 16$ to $32 \times 24$ px | No (Decorative ground) |
| | Flowers | 5 | $16 \times 16$ to $24 \times 20$ px | No (Decorative ground) |
| | Rocks | 5 | $16 \times 16$ to $48 \times 40$ px | 3 Solid, 2 Walk-over |
| | Mushrooms | 4 | $16 \times 16$ to $24 \times 20$ px | No (Decorative ground) |
| | Fallen Logs | 3 | $24 \times 16$ to $64 \times 32$ px | 2 Solid, 1 Walk-over |
| **Buildings** | Small Houses | 3 | $80 \times 80$ to $96 \times 88$ px | Yes (Ground footprint) |
| | Large Houses | 2 | $112 \times 96$ to $120 \times 112$ px | Yes (Ground footprint) |
| | Cabins | 3 | $72 \times 72$ to $80 \times 76$ px | Yes (Ground footprint) |
| | Shops | 2 | $88 \times 80$ to $96 \times 88$ px | Yes (Ground footprint) |
| | Blacksmith | 1 | $112 \times 96$ px | Yes (Ground footprint) |
| | Storage | 2 | $64 \times 64$ to $96 \times 88$ px | Yes (Ground footprint) |
| | Watchtowers | 2 | $64 \times 112$ to $64 \times 120$ px | Yes (Tower base) |
| | Ruins | 3 | $64 \times 40$ to $96 \times 80$ px | Yes (Crumbled walls) |
| **Props** | Fences | 4 | $32 \times 24$ to $32 \times 32$ px | Yes (Fence rail) |
| | Barrels | 3 | $16 \times 18$ to $22 \times 26$ px | Yes (Barrel base) |
| | Crates | 3 | $20 \times 20$ to $32 \times 32$ px | Yes (Crate base) |
| | Wells | 2 | $36 \times 40$ to $36 \times 44$ px | Yes (Well basin) |
| | Lamps | 3 | $16 \times 24$ to $20 \times 48$ px | 1 Solid, 2 Wall/Hanging |
| | Signs | 4 | $20 \times 32$ to $36 \times 36$ px | 3 Solid, 1 Hanging |
| | Bridges | 3 | $64 \times 32$ to $96 \times 48$ px | No (Traversable deck) |
| | Benches | 2 | $32 \times 20$ px | Yes (Bench seat) |
| **TOTAL** | **24 Subcategories** | **77 Assets** | **$16 \times 16$ to $120 \times 112$ px** | **52 Solid, 25 Decorative** |

---

## 3. Nature Asset Collection (35 Assets)

### 3.1 Oak Trees (4 Assets)
- `tree_oak_001` (**Small Oak Tree**):
  - Canvas: $48 \times 64$ px | Ground Anchor: $(24, 60)$
  - Collision Footprint: Rectangle $16 \times 10$ px at base | Offset: $[16, 52]$
  - Description: Young woodland oak with compact rounded foliage canopy and slender textured trunk.
  - Palettes: Moss Green `#2D6934`, Leaf Mid `#3C8246`, Bark Brown `#5A3C23`.
- `tree_oak_002` (**Medium Oak Tree**):
  - Canvas: $64 \times 80$ px | Ground Anchor: $(32, 74)$
  - Collision Footprint: Rectangle $20 \times 12$ px at base | Offset: $[22, 64]$
  - Description: Standard forest canopy tree with multi-tiered leaf clumps and visible roots.
- `tree_oak_003` (**Large Oak Tree**):
  - Canvas: $80 \times 96$ px | Ground Anchor: $(40, 90)$
  - Collision Footprint: Rectangle $24 \times 14$ px at base | Offset: $[28, 78]$
  - Description: Mature forest oak with expansive canopy casting broad ground shade.
- `tree_oak_004` (**Ancient Oak Tree**):
  - Canvas: $112 \times 128$ px | Ground Anchor: $(56, 120)$
  - Collision Footprint: Rectangle $36 \times 18$ px at base | Offset: $[38, 104]$
  - Description: Colossal elder tree with gnarled trunk, sprawling surface roots, and massive crown.

### 3.2 Pine Trees (3 Assets)
- `tree_pine_001` (**Small Pine Tree**):
  - Canvas: $32 \times 64$ px | Ground Anchor: $(16, 60)$
  - Collision Footprint: Rectangle $12 \times 10$ px | Offset: $[10, 52]$
  - Description: Young mountain conifer with conical tiered needles and straight slender trunk.
  - Palettes: Deep Pine `#1E4B32`, Forest Needle `#28643C`, Trunk `#4B321E`.
- `tree_pine_002` (**Medium Pine Tree**):
  - Canvas: $48 \times 80$ px | Ground Anchor: $(24, 76)$
  - Collision Footprint: Rectangle $16 \times 12$ px | Offset: $[16, 66]$
  - Description: Standard boreal pine with three distinct foliage tiers and ground needle scatter.
- `tree_pine_003` (**Large Pine Tree**):
  - Canvas: $64 \times 96$ px | Ground Anchor: $(32, 90)$
  - Collision Footprint: Rectangle $20 \times 14$ px | Offset: $[22, 78]$
  - Description: Tall apex pine tree suited for mountain slopes, borders, and forest clearings.

### 3.3 Bushes (5 Assets)
- `bush_small_001` (**Small Bush**): $24 \times 24$ px, anchor $(12, 22)$. Collision disabled (walk-through).
- `bush_round_001` (**Round Bush**): $32 \times 32$ px, anchor $(16, 28)$. Collision $16 \times 10$ px at base.
- `bush_dense_001` (**Dense Bush**): $40 \times 32$ px, anchor $(20, 28)$. Collision $24 \times 12$ px at base.
- `bush_flowering_001` (**Flowering Bush**): $32 \times 32$ px, anchor $(16, 28)$. Scattered pink blossoms `#F0B4D2` with yellow centers. Collision $16 \times 10$ px.
- `bush_large_001` (**Large Bush**): $48 \times 36$ px, anchor $(24, 32)$. Broad shrub with multiple rounded clumps. Collision $30 \times 14$ px.

### 3.4 Grass & Undergrowth (6 Assets)
- `grass_small_001` (**Small Grass Patch**): $16 \times 16$ px, anchor $(8, 14)$. Natural blade cluster.
- `grass_dense_001` (**Dense Grass Patch**): $24 \times 20$ px, anchor $(12, 18)$. Lush multi-stem turf.
- `grass_tall_001` (**Tall Grass Tuft**): $24 \times 32$ px, anchor $(12, 30)$. High waving savanna/forest reeds.
- `grass_dry_001` (**Dry Grass Patch**): $20 \times 20$ px, anchor $(10, 18)$. Sun-baked golden-amber tufts `#CDAF55`.
- `grass_flowering_001` (**Flowering Grass**): $24 \times 24$ px, anchor $(12, 22)$. Grass tuft with white micro-blossoms.
- `grass_forest_001` (**Lush Forest Grass**): $32 \times 24$ px, anchor $(16, 22)$. Wide ground carpet cluster.

### 3.5 Flowers (5 Assets)
- `flower_white_001` (**White Flowers**): $16 \times 16$ px, anchor $(8, 14)$. Delicate white petals with yellow core.
- `flower_yellow_001` (**Yellow Buttercups**): $16 \times 16$ px, anchor $(8, 14)$. Bright sunny meadow blossoms `#F5D228`.
- `flower_red_001` (**Red Poppies**): $16 \times 16$ px, anchor $(8, 14)$. Crimson petals with dark seed center.
- `flower_blue_001` (**Bluebells**): $16 \times 16$ px, anchor $(8, 14)$. Azure forest bellflowers.
- `flower_wildflower_001` (**Wildflower Cluster**): $24 \times 20$ px, anchor $(12, 18)$. Mixed multicolor meadow bouquet.

### 3.6 Rocks & Boulders (5 Assets)
- `rock_small_001` (**Small Stone**): $16 \times 16$ px, anchor $(8, 14)$. Flat ground pebble (walk-over).
- `rock_medium_001` (**Medium Rock**): $24 \times 20$ px, anchor $(12, 18)$. Solid granite boulder, collision $16 \times 9$ px.
- `rock_boulder_001` (**Large Boulder**): $48 \times 40$ px, anchor $(24, 36)$. Heavy fractured crag stone, collision $36 \times 18$ px.
- `rock_flat_001` (**Flat Stepping Stone**): $32 \times 20$ px, anchor $(16, 18)$. River stepping slab (walk-over).
- `rock_mossy_001` (**Mossy Rock**): $32 \times 28$ px, anchor $(16, 26)$. Weathered forest rock with green lichen cover, collision $22 \times 12$ px.

### 3.7 Mushrooms & Fungi (4 Assets)
- `mushroom_small_001` (**Small Mushroom**): $16 \times 16$ px, anchor $(8, 14)$. Pale cream field mushroom.
- `mushroom_red_001` (**Red Toadstool**): $16 \times 20$ px, anchor $(8, 18)$. Classic crimson cap with white warts.
- `mushroom_brown_001` (**Brown Mushroom**): $20 \times 20$ px, anchor $(10, 18)$. Earthy brown woodland cap.
- `mushroom_cluster_001` (**Mushroom Cluster**): $24 \times 20$ px, anchor $(12, 18)$. Group of three varied fungi.

### 3.8 Fallen Logs (3 Assets)
- `log_branch_001` (**Small Fallen Branch**): $24 \times 16$ px, anchor $(12, 14)$. Walk-over woodland debris.
- `log_medium_001` (**Medium Fallen Log**): $48 \times 24$ px, anchor $(24, 20)$. Solid fallen trunk with cut ring, collision $40 \times 10$ px.
- `log_trunk_001` (**Large Fallen Tree Trunk**): $64 \times 32$ px, anchor $(32, 28)$. Mossy hollow barrier log, collision $54 \times 14$ px.

---

## 4. Building Asset Collection (18 Assets)

### 4.1 Small Houses (3 Assets)
- `house_cottage_001` (**Wooden Cottage**):
  - Canvas: $80 \times 80$ px | Ground Anchor: $(40, 76)$
  - Collision Footprint: $64 \times 40$ px | Door: $(40, 74)$ | Windows: $(24, 60), (56, 60)$
  - Description: Timber-frame cottage with terracotta pitched tile roof and brass-handled wooden door.
- `house_stone_small_001` (**Small Stone House**):
  - Canvas: $80 \times 80$ px | Ground Anchor: $(40, 76)$
  - Collision Footprint: $64 \times 40$ px | Slate Shingle Roof: $76 \times 34$ px
  - Description: Sturdy mortared granite cottage with dark slate roof and leaded glass windows.
- `house_village_001` (**Forest Village House**):
  - Canvas: $96 \times 88$ px | Ground Anchor: $(48, 84)$
  - Collision Footprint: $76 \times 44$ px | Door: $(48, 82)$
  - Description: Extended village residence with reinforced timber corners and gabled entrance.

### 4.2 Large Houses (2 Assets)
- `house_residence_large_001` (**Large Wooden Residence**):
  - Canvas: $112 \times 96$ px | Ground Anchor: $(56, 92)$
  - Collision Footprint: $92 \times 48$ px | Multi-pane windows on upper & lower levels
  - Description: Manor-style wooden estate with dual gables and expansive living footprint.
- `house_twostory_001` (**Two-Story Medieval House**):
  - Canvas: $120 \times 112$ px | Ground Anchor: $(60, 108)$
  - Collision Footprint: $96 \times 54$ px | Roof: $110 \times 42$ px
  - Description: Imposing two-story medieval town residence with cantilevered upper floor.

### 4.3 Cabins (3 Assets)
- `cabin_forest_001` (**Forest Cabin**): $72 \times 72$ px, anchor $(36, 68)$, collision $56 \times 36$ px. Rough-hewn log walls with fieldstone chimney.
- `cabin_hunting_001` (**Hunting Cabin**): $72 \times 72$ px, anchor $(36, 68)$, collision $56 \times 36$ px. Features carved antler trophy rack over entrance.
- `cabin_woodland_001` (**Remote Woodland Cabin**): $80 \times 76$ px, anchor $(40, 72)$, collision $64 \times 38$ px. Extended porch and side woodpile.

### 4.4 Shops & Commercial Buildings (2 Assets)
- `shop_general_001` (**General Goods Store**): $96 \times 88$ px, anchor $(48, 84)$, collision $76 \times 44$ px. Striped storefront awning and display window.
- `shop_market_001` (**Market Shop Stall**): $88 \times 80$ px, anchor $(44, 76)$, collision $70 \times 40$ px. Colorful azure cloth canopy with wooden counter.

### 4.5 Blacksmith (1 Asset)
- `blacksmith_forge_001` (**Village Blacksmith Forge**):
  - Canvas: $112 \times 96$ px | Ground Anchor: $(56, 92)$ | Collision: $90 \times 48$ px
  - Exterior Features: Massive stone forge chimney, outdoor coal storage bin, heavy anvil horn block, and reinforced workshop entrance.

### 4.6 Storage Buildings (2 Assets)
- `storage_shed_001` (**Wooden Storage Shed**): $64 \times 64$ px, anchor $(32, 60)$, collision $50 \times 32$ px. Simple timber utility shed with padlock latch.
- `storage_warehouse_001` (**Supply Warehouse**): $96 \times 88$ px, anchor $(48, 84)$, collision $78 \times 44$ px. Large cargo facility with double wide loading doors.

### 4.7 Watchtowers (2 Assets)
- `watchtower_wood_001` (**Wooden Watchtower**): $64 \times 112$ px, anchor $(32, 108)$, ground footprint $40 \times 36$ px. Tall timber pillar frame with elevated lookout platform.
- `watchtower_stone_001` (**Stone Watchtower**): $64 \times 120$ px, anchor $(32, 116)$, ground footprint $44 \times 40$ px. Fortified stone turret with parapet arrow slits and pyramid slate cap.

### 4.8 Ruins (3 Assets)
- `ruin_wall_broken_001` (**Broken Stone Wall**): $64 \times 40$ px, anchor $(32, 36)$, collision $52 \times 16$ px. Ancient crumbled barrier overgrown with moss.
- `ruin_building_abandoned_001` (**Abandoned Stone Ruin**): $96 \times 80$ px, anchor $(48, 76)$, collision $74 \times 40$ px. Collapsed timber rafters inside crumbling stone shell.
- `ruin_tower_small_001` (**Small Ruined Tower**): $64 \times 88$ px, anchor $(32, 84)$, collision $44 \times 36$ px. Half-collapsed turret with rubble base.

---

## 5. Environmental Details & Props (24 Assets)

### 5.1 Fences (4 Assets)
- `fence_straight_001` (**Wooden Straight Fence**): $32 \times 24$ px, anchor $(16, 22)$, collision $30 \times 8$ px. Modular horizontal two-rail fence.
- `fence_corner_001` (**Wooden Corner Fence**): $32 \times 32$ px, anchor $(16, 28)$, collision $24 \times 24$ px. Orthogonal $90^\circ$ post junction.
- `fence_gate_001` (**Wooden Fence Gate**): $32 \times 24$ px, anchor $(16, 22)$, collision $30 \times 8$ px. Cross-braced swinging gate with brass latch.
- `fence_broken_001` (**Broken Wooden Fence**): $32 \times 24$ px, anchor $(16, 22)$, collision $18 \times 8$ px. Splintered fence section with gap.

### 5.2 Barrels & Containers (3 Assets)
- `barrel_standard_001` (**Standard Barrel**): $20 \times 24$ px, anchor $(10, 22)$, collision $14 \times 12$ px. Oak staves with iron hoops.
- `barrel_small_001` (**Small Keg**): $16 \times 18$ px, anchor $(8, 16)$, collision $12 \times 10$ px. Compact ale cask.
- `barrel_banded_001` (**Metal-Banded Barrel**): $22 \times 26$ px, anchor $(11, 24)$, collision $16 \times 12$ px. Heavy reinforced storage barrel.

### 5.3 Crates & Cargo (3 Assets)
- `crate_small_001` (**Small Wooden Crate**): $20 \times 20$ px, anchor $(10, 18)$, collision $16 \times 14$ px. Cross-slatted packing box.
- `crate_large_001` (**Large Cargo Crate**): $32 \times 32$ px, anchor $(16, 28)$, collision $26 \times 20$ px. Bulk cargo timber container.
- `crate_reinforced_001` (**Reinforced Storage Crate**): $28 \times 28$ px, anchor $(14, 25)$, collision $22 \times 18$ px. Iron corner brackets.

### 5.4 Wells (2 Assets)
- `well_stone_001` (**Stone Village Well**): $36 \times 40$ px, anchor $(18, 36)$, collision $26 \times 22$ px. Circular ashlar stonework with water basin.
- `well_wooden_001` (**Wooden Roofed Well**): $36 \times 44$ px, anchor $(18, 40)$, collision $26 \times 22$ px. Overhead shingled canopy and winch rope.

### 5.5 Lamps & Lanterns (3 Assets)
- `lamp_post_001` (**Wooden Post Street Lamp**): $20 \times 48$ px, anchor $(10, 46)$, collision $10 \times 10$ px. Timber pole with hanging lantern.
- `lamp_wall_001` (**Wall-Mounted Lantern**): $16 \times 24$ px, anchor $(8, 22)$. Wall sconce fixture (collision disabled).
- `lamp_hanging_001` (**Hanging Chain Lantern**): $16 \times 32$ px, anchor $(8, 30)$. Ceiling/beam link-chain lantern (collision disabled).

### 5.6 Signs & Notice Boards (4 Assets)
- `sign_direction_001` (**Village Direction Signpost**): $24 \times 36$ px, anchor $(12, 34)$, collision $8 \times 8$ px. Dual pointing arrow planks.
- `sign_shop_001` (**Hanging Shop Signboard**): $24 \times 28$ px, anchor $(12, 26)$. Wrought-iron bracket with wooden crest (collision disabled).
- `sign_noticeboard_001` (**Wooden Notice Board**): $36 \times 36$ px, anchor $(18, 34)$, collision $28 \times 10$ px. Dual posts with pinned parchment notes.
- `sign_warning_001` (**Warning Signpost**): $20 \times 32$ px, anchor $(10, 30)$, collision $8 \times 8$ px. Diamond marker board.

### 5.7 Bridges (3 Assets)
- `bridge_wood_small_001` (**Small Wooden Footbridge**): $64 \times 32$ px, anchor $(32, 28)$. Traversable horizontal plank deck.
- `bridge_wood_large_001` (**Large Timber Bridge**): $96 \times 48$ px, anchor $(48, 44)$. Heavy river bridge with side safety railings.
- `bridge_stone_001` (**Arched Stone Bridge**): $96 \times 48$ px, anchor $(48, 44)$. Ashlar arched stone viaduct.

### 5.8 Benches (2 Assets)
- `bench_wooden_001` (**Village Wooden Bench**): $32 \times 20$ px, anchor $(16, 18)$, collision $26 \times 10$ px. Slatted backrest and armrests.
- `bench_stone_001` (**Carved Stone Bench**): $32 \times 20$ px, anchor $(16, 18)$, collision $26 \times 10$ px. Solid carved granite plaza seat.

