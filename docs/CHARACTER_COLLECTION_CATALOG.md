# Wildbound — Character Collection Catalog: First Production Collection (32 Assets)

## 1. Overview & Technical Conventions

This catalog documents the **First Production Collection** of modular player customization assets for **Wildbound**.

### Core Technical Specifications:
- **Canvas Resolution**: $128 \times 384$ pixels per spritesheet.
- **Frame Dimensions**: $32 \times 48$ pixels per frame cell.
- **Grid Layout**: 4 columns $\times$ 8 rows (32 total frames per spritesheet).
  - Rows 0..3: Walk animation (Down, Up, Left, Right) @ 8 fps.
  - Rows 4..7: Idle animation (Down, Up, Left, Right) @ 4 fps.
- **Logical Anchor Point**: $(16, 48)$ bottom-center (aligned with character feet and ground contact).
- **Collision Footprint**: $16 \times 12$ pixels.
- **Directory Pattern**: `assets/characters/player/<category>/<item_id>/`
  - `sprite.png`: Master pixel art spritesheet.
  - `spritesheet.json`: Raylib / AMS spritesheet descriptor.
  - `metadata.json`: Section 16 cosmetic metadata schema.

---

## 2. Category Breakdown & Inventory

The collection consists of **exactly 32 production cosmetic assets**:

| Category | Count | Base Z-Index | Layer Name | Target Anatomical Anchor |
| :--- | :---: | :---: | :--- | :--- |
| **Hairstyles** | 5 | 60 | `head_hair` | Head / Crown $(16, 14)$ |
| **Shirts** | 5 | 50 | `upper_body` | Torso / Arms $(16, 26)$ |
| **Pants** | 5 | 30 | `lower_body` | Pelvis / Legs $(16, 36)$ |
| **Shoes** | 3 | 40 | `footwear` | Feet / Ground $(16, 44)$ |
| **Hats** | 5 | 80 | `headwear` | Head / Crown $(16, 14)$ |
| **Goggles** | 3 | 72 | `eyewear` | Face / Eyes $(16, 13)$ |
| **Masks** | 3 | 68 | `face_mask` | Lower Face $(16, 17)$ |
| **Backpacks** | 3 | 10 / 85 | `back_accessory`| Upper Torso $(16, 26)$ |
| **TOTAL** | **32** | — | — | — |

---

## 3. Detailed Asset Specifications

### 3.1 Hairstyles (5 Designs)

#### 1. `hair_short_001` — Short Adventurer
- **Description**: Classic neat short adventurer cut with cropped sides and fringe across the forehead.
- **Palette**: Chestnut Brown `#73411E` with Golden Highlight `#9B5F2D`.
- **Directional Occlusion**: Renders on crown in Down/Left/Right; full back of skull in Up.
- **Compatibility**: Compatible with all hats except full helmets (`hat_helmet_001`).

#### 2. `hair_long_001` — Long Wanderer
- **Description**: Flowing raven-black shoulder-length locks parting down the center.
- **Palette**: Raven Black `#232630` with Slate Navy Specular `#414B5F`.
- **Directional Occlusion**: Side tresses drape over shoulders when facing South; covers upper back in North view.
- **Compatibility**: Compatible with hoods, fedoras, and caps. Occluded by full helmets.

#### 3. `hair_messy_001` — Messy Explorer
- **Description**: Textured, unstructured wind-swept auburn hairstyle with spiky strands.
- **Palette**: Auburn Copper `#A54B23` with Warm Amber Highlight `#CD733C`.
- **Directional Occlusion**: Spikes bounce with walking bobbing cycle ($y \pm 1$ px).
- **Compatibility**: Excellent fit under loose headwear like `hat_adventurer_001`.

#### 4. `hair_ponytail_001` — High Ponytail
- **Description**: High tied-back golden blonde ponytail bound with a crimson ribbon.
- **Palette**: Golden Blonde `#EBC341` with Sunbeam Highlight `#FFE16E` and Crimson Tie `#C32D37`.
- **Directional Occlusion**: Ponytail sweeps dynamically to the side during walk cycles; fully visible extending down back in North view.
- **Compatibility**: Compatible with open-back headwear. Occluded by full helmets.

#### 5. `hair_curly_001` — Curly Hair
- **Description**: Voluminous, textured dark curly puffs framing the temple and crown.
- **Palette**: Dark Espresso `#462D19` with Warm Mocha Highlight `#694628`.
- **Directional Occlusion**: Sits above brow line to avoid obscuring eyes and goggles.
- **Compatibility**: Compatible with wide-brim hats and cowls.

---

### 3.2 Shirts (5 Designs)

#### 1. `shirt_tunic_001` — Forest Tunic
- **Description**: Classic fantasy forest-green linen tunic with leather belt, bronze buckle, and reinforced wrist cuffs.
- **Palette**: Forest Green `#2D6934`, Olive Border `#204B26`, Leather Belt `#503018`, Bronze Buckle `#DCB432`.
- **Layering**: Fits snugly over base mannequin torso; allows pants to tuck naturally underneath.

#### 2. `shirt_leather_001` — Leather Jacket
- **Description**: Rugged explorer-style jacket with reinforced double-breasted leather and brass studs.
- **Palette**: Warm Saddle Leather `#694123`, Deep Shadow `#4B2D19`, Brass Studs `#DCB432`.
- **Layering**: Slightly heavier silhouette; pairs cleanly with traveler backpacks and bandoliers.

#### 3. `shirt_armor_001` — Light Armor
- **Description**: Polished steel breastplate over quilted cloth gambeson with articulated pauldrons.
- **Palette**: Steel Plate `#A0AAB9`, Gambeson Cloth `#736955`, Trim `#554B3C`.
- **Layering**: Rigid metallic chest silhouette with shoulder pauldrons moving in lockstep with arm swings.

#### 4. `shirt_traveler_001` — Traveler Shirt
- **Description**: Relaxed unbleached linen shirt with loose rolled sleeves and leather cord V-neck.
- **Palette**: Cream Linen `#E6DCC8`, Soft Shadow `#BEB4A0`, Lacing `#503018`.
- **Layering**: Open collar reveals neck skin; flexible lightweight sleeves.

#### 5. `shirt_mage_001` — Mystic Robe
- **Description**: Deep royal violet arcane robe with gold runic embroidery and draped sleeves.
- **Palette**: Arcane Violet `#552878`, Shadow Violet `#411E5F`, Gold Runic Trim `#D4AF32`.
- **Layering**: Features extended hem reaching down to mid-calf ($y + 36$ px).

---

### 3.3 Pants (5 Designs)

#### 1. `pants_traveler_001` — Traveler Pants
- **Description**: Standard durable dark brown adventure trousers.
- **Palette**: Dark Brown `#4B3223`, Shadow `#372319`.
- **Integration**: Full coverage down to lower shins; boots layer directly over the cuffs.

#### 2. `pants_leather_001` — Leather Pants
- **Description**: Fitted charcoal-black leather trousers with reinforced knee patches.
- **Palette**: Charcoal Leather `#282A30`, Seam Stitch `#1C1E23`.
- **Integration**: Form-fitting cut providing sharp contrast against bright tunics.

#### 3. `pants_armored_001` — Armored Pants
- **Description**: Steel-reinforced plate greaves and armored thigh culets over dark padded trousers.
- **Palette**: Dark Cloth `#414650`, Steel Plates `#9BA5B4`.
- **Integration**: Pairs seamlessly with `shoes_armored_001` for full leg plate protection.

#### 4. `pants_short_001` — Adventurer Shorts
- **Description**: Slate grey explorer shorts ending above the knees.
- **Palette**: Slate Grey `#41505F`, Trim `#2D3C4B`.
- **Integration**: Leaves lower thighs and knees exposed; ideal for hot biomes and scout builds.

#### 5. `pants_mage_001` — Mystic Trousers
- **Description**: Flowing arcane violet culottes with golden embroidered cuffs.
- **Palette**: Arcane Violet `#46235F`, Gold Embroidery `#BEA02D`.
- **Integration**: Designed to complement `shirt_mage_001` or contrast with light shirts.

---

### 3.4 Shoes (3 Designs)

#### 1. `shoes_boots_001` — Leather Boots
- **Description**: Traditional dark brown riding boots with folded cuffs.
- **Palette**: Dark Leather `#3C2616`, Highlight `#553720`.
- **Foot Contact**: Aligned flush with ground contact line ($y = 44 \dots 47$).

#### 2. `shoes_armored_001` — Armored Boots
- **Description**: Articulated steel sabatons with polished toe caps and ankle guards.
- **Palette**: Steel `#A0AAB9`, Trim `#737D8C`.
- **Foot Contact**: Rigid metallic foot shape with specular ground contact shine.

#### 3. `shoes_light_001` — Light Shoes
- **Description**: Soft moccasin-style travel shoes crafted from lightweight tan calfskin.
- **Palette**: Tan Calfskin `#735537`, Highlight `#916E4B`.
- **Foot Contact**: Low-profile silhouette emphasizing agility and silent movement.

---

### 3.5 Hats (5 Designs)

#### 1. `hat_adventurer_001` — Adventurer Hat
- **Description**: Classic wide-brim felt explorer fedora with dark leather band and brass buckle.
- **Palette**: Felt Brown `#5A3C23`, Band `#322014`, Buckle `#DCB432`.
- **Occlusion**: Sits above forehead without obscuring eyes or facial accessories.

#### 2. `hat_hood_001` — Explorer Hood
- **Description**: Forest green cowl hood draping smoothly over the shoulders.
- **Palette**: Forest Green `#28552D`, Trim Stitch `#DCB432`.
- **Occlusion**: Drapes down sides of the face; rear peak visible when facing North.

#### 3. `hat_helmet_001` — Iron Helmet
- **Description**: Medieval knight steel bascinet with brass crest and narrow horizontal visor slit.
- **Palette**: Steel `#9BA5B4`, Trim `#6E7887`, Crest `#DCB432`, Visor Slit `#19191E`.
- **Occlusion**: Sets `hair_hide_all` flag. Completely encloses the head and suppresses all hair layers.

#### 4. `hat_wizard_001` — Mystic Hat
- **Description**: Tall pointed wizard hat in midnight violet with curved brim and gold star brooch.
- **Palette**: Midnight Violet `#4B236E`, Gold Brooch `#DCB432`.
- **Occlusion**: Generous clearance allowing hair to flow beneath the brim.

#### 5. `hat_cap_001` — Traveler Cap
- **Description**: Simple flat adventure travel cap with short forward brim.
- **Palette**: Travel Cloth `#6E4B2D`, Brim Leather `#55371E`.
- **Occlusion**: Compact low-profile crown.

---

### 3.6 Goggles (3 Designs)

#### 1. `goggles_adventure_001` — Adventurer Goggles
- **Description**: Classic explorer brass-rimmed goggles with cyan-tinted glass lenses and leather head strap.
- **Palette**: Brass Rims `#C3912D`, Cyan Lenses `#50D2E6`, Leather Strap `#322319`.
- **Depth**: Renders at $Z = 72$; occluded when facing North or when wearing full-face masks.

#### 2. `goggles_steampunk_001` — Mechanical Goggles
- **Description**: Dual-gearwork mechanical goggles with amber magnifying lenses and bridge cogwheel.
- **Palette**: Polished Brass `#D7A028`, Amber Lenses `#F59B23`, Leather Strap `#412D1E`.
- **Depth**: Prominent side profile when facing West/East.

#### 3. `goggles_snow_001` — Snow Goggles
- **Description**: Heavy-duty polar snow goggles with slate white frame and dark polarized slit lenses.
- **Palette**: Polar White `#D7DCE1`, Polarized Lenses `#28374B`.
- **Depth**: Streamlined wrap-around shape designed for harsh tundra environments.

---

### 3.7 Masks (3 Designs)

#### 1. `mask_cloth_001` — Cloth Mask
- **Description**: Dark scout cloth bandanna covering the lower face, nose, and mouth.
- **Palette**: Dark Scout Cloth `#2D3237`.
- **Compatibility**: Leaves upper face clear, allowing full coexistence with all goggles.

#### 2. `mask_leather_001` — Leather Mask
- **Description**: Fantasy studded leather half-mask with twin brass ventilation grommets.
- **Palette**: Studded Leather `#55341E`, Brass Grommets `#C3AF78`.
- **Compatibility**: Fits beneath goggles without clipping.

#### 3. `mask_mystic_001` — Mystic Mask
- **Description**: Enigmatic porcelain full-face mask painted with glowing cobalt arcane runes.
- **Palette**: Ivory Porcelain `#EBEBF0`, Cobalt Runes `#326ED2`.
- **Occlusion**: Sets `mask_hide_goggles` flag. Full-face porcelain shell automatically occludes all eyewear.

---

### 3.8 Backpacks (3 Designs)

#### 1. `backpack_traveler_001` — Traveler Backpack
- **Description**: Sturdy adventurer rucksack with rolled forest-green bedroll and brass harness buckles.
- **Palette**: Saddle Leather `#734626`, Bedroll `#375F3C`, Harness Straps `#4B2D19`.
- **Dynamic Z-Order**: $Z = 10$ (behind body) when facing South/West/East; $Z = 85$ (in front) when facing North.

#### 2. `backpack_leather_001` — Leather Backpack
- **Description**: Compact rich mahogany leather rucksack with double side-pocket pouches.
- **Palette**: Mahogany Leather `#5F3019`, Straps `#412012`, Buckles `#DCB432`.
- **Dynamic Z-Order**: Low-profile silhouette designed for high-mobility scout builds.

#### 3. `backpack_adventure_001` — Adventure Pack
- **Description**: Heavy expedition pack with canvas tent roll, bedroll, and metal side canteen.
- **Palette**: Olive Canvas `#4B503C`, Tent Roll `#8C7350`, Steel Canteen `#A0AAB9`.
- **Dynamic Z-Order**: Large silhouette demonstrating pronounced rear/front depth inversion.

