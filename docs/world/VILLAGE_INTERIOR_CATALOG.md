# Wildbound — Village Interior Catalog

**Phase:** Phase 13 — Village Construction  
**Total Enterable Interiors:** 6 functional buildings  
**Tileset:** `assets/tilesets/interior/interior_tileset.png` ($256 \times 256$ px, 64 tiles, 32x32 px/tile)  
**Map Format:** Tiled JSON (compatible with `MapLoader` and `World`)

---

## Catalog Index

| ID | Building Name | Exterior Asset | Interior Dimensions | Interior Map File | Spawn (X, Y) | Return (X, Y) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `village_general_store` | Greenwood Supplies & Sundries | `shop_general_001` | $20 \times 15$ tiles ($640 \times 480$ px) | `maps/interiors/interior_general_store.json` | $(320, 416)$ | $(3868, 816)$ |
| `village_blacksmith` | Vulcan's Hearth & Forge | `blacksmith_forge_001` | $20 \times 15$ tiles ($640 \times 480$ px) | `maps/interiors/interior_blacksmith.json` | $(320, 416)$ | $(3856, 1032)$ |
| `village_town_hall` | Greenwood Manor & Council | `house_residence_large_001` | $24 \times 18$ tiles ($768 \times 576$ px) | `maps/interiors/interior_town_hall.json` | $(384, 512)$ | $(4320, 688)$ |
| `village_cottage` | Forester's Cottage | `house_cottage_001` | $18 \times 14$ tiles ($576 \times 448$ px) | `maps/interiors/interior_cottage.json` | $(288, 384)$ | $(4508, 816)$ |
| `village_twostory_residence` | Miller's Homestead | `house_twostory_001` | $22 \times 16$ tiles ($704 \times 512$ px) | `maps/interiors/interior_twostory_residence.json` | $(352, 448)$ | $(4484, 1052)$ |
| `village_warehouse` | Village Granary & Warehouse | `storage_warehouse_001` | $20 \times 16$ tiles ($640 \times 512$ px) | `maps/interiors/interior_warehouse.json` | $(320, 448)$ | $(4044, 712)$ |

---

## 1. Greenwood Supplies & Sundries (`village_general_store`)

### Architectural Layout & Features
* **Dimensions:** $20 \times 15$ tiles ($640 \times 480$ pixels).
* **Flooring:** Warm polished oak plank flooring with an ornate woven burgundy runner rug leading from the entrance threshold to the merchant counter.
* **North Wall:** Lined with solid bookshelves (Tile GID 28) filled with trade ledgers, maps, and tomes, alongside apothecary potion racks (Tile GID 29) bearing red, blue, and emerald tincture bottles.
* **Service Counter:** L-shaped timber shop counter (Tile GID 25) across row 5 separating the customer area from merchant storage.
* **Display Aisles:** Freestanding merchandise display tables (Tile GID 27) loaded with sundries, field provisions, and rope coils.
* **Objects & Props:**
  - `Store_Crate_1` (`crate_small_001` at $96, 96$): Solid storage crate.
  - `Store_Barrel_1` (`barrel_standard_001` at $512, 96$): Pickled fish barrel.
  - `Store_Counter_Lamp` (`lamp_hanging_001` at $320, 128$): Overhead hanging lantern.
  - `exit_door`: $64 \times 32$ px door trigger mat at $(320, 448)$.
* **Collision Zones:** 100 collision boxes protecting perimeter walls, shelving units, display tables, and shop counters.

---

## 2. Vulcan's Hearth & Forge (`village_blacksmith`)

### Architectural Layout & Features
* **Dimensions:** $20 \times 15$ tiles ($640 \times 480$ pixels).
* **Flooring:** Heavy fireproof dark slate flagstones with textured mortar joints and ash residue.
* **Forge & Chimney:** Central stone hearth (Tile GID 30 and 31) built from volcanic riverstone with glowing charcoal embers and an animated forge flame (`fire_medium_anim` at $X: 304, Y: 80$).
* **Tool Benches:** Heavy iron-banded workbenches (Tile GID 27) holding tongs, hammers, and files along the northwest wall.
* **Weapon Racks:** Timber tool and blade racks (Tile GID 28) along the northeast wall holding tempered broadswords and axes.
* **Objects & Props:**
  - `Cooling_Trough` (`barrel_banded_001` at $384, 96$): Heavy quenching barrel with cold water.
  - `Blacksmith_Anvil_Rock` (`rock_flat_001` at $192, 160$): Massive stone block serving as the anvil mounting pedestal.
  - `Coal_Crate` (`crate_reinforced_001` at $96, 224$): Reinforced timber crate filled with metallurgical coal.
  - `exit_door`: Threshold exit mat at $(320, 448)$.
* **Collision Zones:** 95 collision boxes ensuring the player cannot walk into the roaring hearth, tool benches, or anvil pedestal.

---

## 3. Greenwood Manor & Council (`village_town_hall`)

### Architectural Layout & Features
* **Dimensions:** $24 \times 18$ tiles ($768 \times 576$ pixels).
* **Flooring:** Wide dark timber planks centered with a grand royal crimson carpet (Tile GID 17) spanning $14 \times 8$ tiles with golden leaf borders.
* **Great Fireplace:** Grand stone masonry fireplace (Tile GID 30 & 31) at center-north with an animated warming fire (`fire_small_anim` at $X: 368, Y: 80$).
* **Council Banquet Table:** Long double-wide oak council table (Tile GID 27) spanning columns 8..15 across rows 8 and 9 where the village elders convene.
* **Wall Archives:** High oak bookcases flanking the north hearth containing tax scrolls and deed documents.
* **Objects & Props:**
  - `Elder_Bench_L` (`bench_wooden_001` at $192, 272$): Flanking council seating.
  - `Elder_Bench_R` (`bench_wooden_001` at $544, 272$): Flanking council seating.
  - `Council_Lamp_1` & `Council_Lamp_2` (`lamp_wall_001`): Polished brass wall sconces.
  - `exit_door`: Wide double doorway exit trigger at $(384, 544)$.
* **Collision Zones:** 122 collision bounds enforcing solid perimeter, grand hearth, and banquet table boundaries.

---

## 4. Forester's Cottage (`village_cottage`)

### Architectural Layout & Features
* **Dimensions:** $18 \times 14$ tiles ($576 \times 448$ pixels).
* **Flooring:** Warm rustic pine wood flooring with light grain variations.
* **Bedroom Nook:** Cozy timber bed with headboard, white pillow, and deep sapphire wool blanket (Tile GID 32 & 33) in the northwest corner, fronted by a woven hearth rug.
* **Hearth & Kitchen:** Compact stone cooking fireplace (Tile GID 30 & 31) with crackling fire (`fire_small_anim` at $X: 448, Y: 80$).
* **Dining Table:** Two-tile timber dining table (Tile GID 27) in the southeast corner.
* **Objects & Props:**
  - `Cottage_Chest` (`crate_small_001` at $128, 64$): Cedar clothing chest at foot of bed.
  - `Cottage_Barrel` (`barrel_small_001` at $480, 128$): Water storage cask.
  - `exit_door`: Woven coir doorway mat at $(288, 416)$.
* **Collision Zones:** 82 collision boxes enclosing the cozy home.

---

## 5. Miller's Homestead (`village_twostory_residence`)

### Architectural Layout & Features
* **Dimensions:** $22 \times 16$ tiles ($704 \times 512$ pixels).
* **Flooring:** Fine hardwood planking with decorative hallway runner rugs.
* **Partitioned Rooms:** An internal timber partition wall (Tile GID 9) dividing the western food pantry from the eastern living room and dining hall.
* **Kitchen Pantry:** Countertops (Tile GID 25) with flour bins, grain jars, and storage shelving.
* **Living Hearth:** Large stone fireplace (Tile GID 30 & 31) with animated flame (`fire_medium_anim` at $X: 448, Y: 80$).
* **Dining Suite:** Four-tile family dining table (Tile GID 27) centered in the living wing.
* **Objects & Props:**
  - `Pantry_Barrel_1` (`barrel_standard_001` at $64, 64$): Salted grain barrel.
  - `Pantry_Crate_1` (`crate_small_001` at $64, 128$): Dried fruit crate.
  - `Residence_Lamp` (`lamp_wall_001` at $576, 64$): Wall lamp.
  - `exit_door`: Exit threshold at $(352, 480)$.
* **Collision Zones:** 104 collision boxes separating rooms and fixtures.

---

## 6. Village Granary & Warehouse (`village_warehouse`)

### Architectural Layout & Features
* **Dimensions:** $20 \times 16$ tiles ($640 \times 512$ pixels).
* **Flooring:** Heavy weathered industrial planking (Tile GID 2) designed to support heavy cargo sleds.
* **Storage Racks:** Full-length heavy timber storage shelves (Tile GID 28) covering both halves of the northern wall.
* **Cargo Stacks:** Central pallet platforms (Tile GID 27) holding bulk goods.
* **Objects & Props:**
  - `Wh_Crate_A` (`crate_large_001` at $128, 160$): Massive export crate.
  - `Wh_Crate_B` (`crate_reinforced_001` at $448, 160$): Iron-reinforced goods crate.
  - `Wh_Barrel_A` (`barrel_banded_001` at $128, 256$): Banded ale cask.
  - `Wh_Barrel_B` (`barrel_standard_001` at $480, 256$): Oil barrel.
  - `Wh_Lamp` (`lamp_hanging_001` at $320, 96$): Central warehouse work lantern.
  - `exit_door`: Wide loading threshold at $(320, 480)$.
* **Collision Zones:** 106 collision boxes securing all cargo aisles.

