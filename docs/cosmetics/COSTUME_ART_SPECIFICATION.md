# Wildbound — Costume & Apparel Art Specification

## 1. Visual Direction & Design Principles

Wildbound's costume and apparel collection adheres strictly to the established Phase 6 Art Bible standards: a cohesive, readable 2D top-down fantasy aesthetic calibrated for a virtual resolution of 640×360 pixels and a base tile size of 32×32 pixels.

### Core Visual Principles
* **Consistent Humanoid Proportions**: Player characters are authored over a standardized $32 \times 48$ px frame grid with an anatomical anchor at $(16, 48)$ (bottom-center of the feet). All cosmetic layers fit this exact spatial frame without visible gaps, seam misalignments, or silhouette clipping.
* **Master Lockstep Animation**: Every garment and cosmetic layer mirrors the master `player_spritesheet` locomotion and idle cycles identically. Sub-pixel bobbing ($1$ px vertical shift during frames 1 & 3 of walking, frames 1 & 2 of breathing idle) is baked identically across all layers.
* **Palette Cohesion**: All fabrics, leathers, and dyes adhere strictly to the master fantasy palette: warm earthy tans (`#D2C8B4`), woodland greens (`#2D7337`), desert golds (`#E1D2A0`), arctic winter parkas (`#41699B`), boiled cowhide browns (`#693C23`), arcane indigos (`#552D87`), imperial velvet crimsons (`#961E2D`), and stealth charcoals (`#262830`).

---

## 2. Technical Asset Standards & Grid Alignment

| Property | Standard Specification | Description |
| :--- | :--- | :--- |
| **Spritesheet Dimensions** | 128 × 384 px | 4 columns × 8 rows (32 frames total per cosmetic item). |
| **Frame Size** | 32 × 48 px | Universal cell size matching player humanoid proportions. |
| **Row Layout** | Rows 0–3: Walk Cycle<br>Rows 4–7: Idle Cycle | Row 0: `walk_down`, Row 1: `walk_up`, Row 2: `walk_left`, Row 3: `walk_right`<br>Row 4: `idle_down`, Row 5: `idle_up`, Row 6: `idle_left`, Row 7: `idle_right` |
| **Base Origin Anchor** | `bottom_center` `(16, 48)` | Standard foot contact anchor used across all character compositing layers. |
| **Color Depth** | 32-bit RGBA PNG | Fully transparent backing (`#00000000`) with crisp, non-feathered alpha edges. |

---

## 3. Clothing Collection — 8 Designs

| ID | Name | Dominant Colors | Silhouette & Design Details |
| :--- | :--- | :--- | :--- |
| `clo_basic_traveler_001` | Basic Traveler Shirt | Linen Beige, Saddle Leather | Rustic open-collar linen shirt with neck lacing cord, rolled sleeves, and leather wrist bands. |
| `clo_forest_tunic_001` | Forest Tunic | Woodland Green, Pine Shadow | Durable ranger tunic with reinforced hem, double-stitched leather edging, and short sleeves. |
| `clo_desert_robe_001` | Desert Robe | Sand Khaki, Desert Dune | Loose linen nomad robe offering sun deflection, broad cowl collar, and relaxed sleeve cuffs. |
| `clo_snow_jacket_001` | Snow Jacket | Glacier Blue, Polar Fur White | Heavy winter expedition parka with thick fleece/fur collar, toggle fasteners, and insulated cuffs. |
| `clo_leather_jacket_001` | Leather Jacket | Boiled Cowhide Brown, Brass | Weathered leather jacket with lapels, reinforced shoulder seams, and polished brass buttons. |
| `clo_mystic_robe_001` | Mystic Robe | Midnight Violet, Solar Gold | Scholarly arcane vestment embroidered with glowing gold runic script along the collar and hem. |
| `clo_royal_outfit_001` | Royal Outfit | Imperial Crimson, Ermine White | Ceremonial velvet court doublet with ermine fur trimming, gold piping, and flared shoulders. |
| `clo_dark_adventurer_001` | Dark Adventurer Outfit | Midnight Charcoal, Slate Gray | Lightweight stealth tunic with dampened cloth seams and matte dark slate belt trim. |

---

## 4. Pants & Legwear Collection — 6 Designs

| ID | Name | Dominant Colors | Design & Leg Alignment |
| :--- | :--- | :--- | :--- |
| `clo_p_traveler_pants_001` | Traveler Pants | Earth Tan, Bark Brown | Tough canvas trekking trousers with reinforced knee patches and tailored leg taper. |
| `clo_p_leather_pants_001` | Leather Pants | Dark Espresso Leather | Fitted supple leather riding breeches with lateral seam stitching. |
| `clo_p_armored_pants_001` | Armored Pants | Steel Gray, Charcoal Padding | Padded arming hose fitted with reinforced steel knee poleyns and greave straps. |
| `clo_p_desert_trousers_001` | Desert Trousers | Pale Sandstone, Desert Ochre | Billowy lightweight pleated sirwal trousers designed for ventilation in extreme heat. |
| `clo_p_snow_trousers_001` | Snow Trousers | Slate Blue, Fleece White | Quilted heavy trousers packed with goose-down insulation and reinforced cuffs. |
| `clo_p_mystic_pants_001` | Mystic Pants | Twilight Indigo, Silk Gold | Pleated scholar breeches tailored with gold thread along the outer seam. |

---

## 5. Footwear & Shoes Collection — 5 Designs

| ID | Name | Dominant Colors | Sole & Ankle Detailing |
| :--- | :--- | :--- | :--- |
| `sho_leather_boots_001` | Leather Boots | Chestnut Brown, Dark Sole | Mid-calf traveler boots with folded cuff collars and thick stitched leather soles. |
| `sho_iron_boots_001` | Iron Boots | Forged Iron, Dark Gray Tread | Heavy articulated iron sabatons with reinforced steel toe caps and ankle guards. |
| `sho_traveler_shoes_001` | Traveler Shoes | Soft Tan, Rawhide Sole | Low-profile flexible leather walking shoes designed for silent, swift locomotion. |
| `sho_snow_boots_001` | Snow Boots | Winter Navy, Fleece White | Thick arctic boots lined with warm fleece, cross-laced fronts, and deep tread lugs. |
| `sho_desert_sandals_001` | Desert Sandals | Natural Rawhide, Brass Studs | Sturdy open-toe leather sandals with braided ankle cords and breathable sole straps. |

---

## 6. Headwear Collection — 6 Designs

| ID | Name | Occlusion Flags | Design Characteristics |
| :--- | :--- | :--- | :--- |
| `hat_adventurer_hat_001` | Adventurer Hat | *None* (Hair visible) | Broad-brimmed felt traveler hat with a dark band and traveling feather. |
| `hat_explorer_hood_001` | Explorer Hood | *None* (Hair visible) | Soft cowled hood framing the face and draping naturally onto the shoulders. |
| `hat_iron_helmet_001` | Iron Helmet | `hair_hide_all` | Forged conical iron pot helm with rigid nose guard; completely encases hair. |
| `hat_steel_helmet_001` | Steel Helmet | `hair_hide_all` | High-polish close helmet with central skull ridge and narrow eye slits. |
| `hat_mystic_hat_001` | Mystic Hat | *None* (Hair visible) | Conical pointed sorcerer hat with wide brim, celestial embroidery, and curled tip. |
| `hat_forest_ranger_hood_001` | Forest Ranger Hood | *None* (Hair visible) | Deep forest green hunting hood secured with an embossed golden leaf clasp. |

---

## 7. Backpack Collection — 5 Designs

| ID | Name | Dominant Colors | Directional Presentation |
| :--- | :--- | :--- | :--- |
| `bag_small_traveler_001` | Small Traveler Backpack | Canvas Khaki, Brass Buckle | Compact day rucksack; single flap and brass buckle; tucked behind shoulders. |
| `bag_leather_backpack_001` | Leather Backpack | Saddle Brown, Dark Seams | Heavy-duty stitched leather pack with twin utility pouches on left and right. |
| `bag_adventurer_backpack_001` | Adventurer Backpack | Camp Tan, Woolen Blanket | Expedition pack featuring a tightly rolled woolen sleeping bedroll strapped on top. |
| `bag_explorer_backpack_001` | Explorer Backpack | Field Olive, Brass Canteen | Surveyor pack fitted with leather map tube and polished brass water canteen. |
| `bag_military_backpack_001` | Military-Style Backpack | Tactical Green, Black Webbing | Heavy field rucksack featuring modular webbing loops and dual bedroll straps. |

---

## 8. Shading, Transparency & Occlusion Rules

1. **Strict Alpha Cleanliness**: No anti-aliasing against solid backgrounds; all sprites are rendered on clean transparent pixels (`RGBA 0,0,0,0`).
2. **Contact Shadow Independence**: Ground contact shadows are managed authoritatively by the character renderer, never embedded into individual shoe or garment sprites.
3. **Four-Directional Consistency**: Down (facing player), Up (facing away), Left (facing West), Right (facing East) maintain consistent light source direction (top-left key light).

