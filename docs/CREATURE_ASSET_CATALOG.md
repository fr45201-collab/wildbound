# Wildbound — Creature & Wildlife Asset Catalog (Phase 9)

## 1. Overview & Technical Standards

This catalog provides an exhaustive technical specification for the **Enemy & Wildlife Asset Collection** produced in Phase 9 of **Wildbound**.

Wildbound's creature artwork follows the visual and technical guidelines defined in the [Phase 6 Art Bible](file:///c:/dev/Wildbound/docs/art/WILDBOUND_ART_BIBLE.md) and integrates directly with the [Asset Management System](file:///c:/dev/Wildbound/docs/ASSET_MANAGEMENT.md), [Animation Controller](file:///c:/dev/Wildbound/docs/ANIMATION_SYSTEM.md), and [Combat System](file:///c:/dev/Wildbound/docs/COMBAT_SYSTEM.md).

### Core Conventions:
- **Perspective**: Top-down $2.5\text{D}$ orthographic with an apparent camera inclination of $45^\circ$ to $60^\circ$.
- **Pixel Density**: Crisp pixel-art scaled integer times onto the virtual canvas ($640 \times 360$).
- **Color Grading**: Restricted fantasy palettes using rich shadows, warm specular highlights, and high silhouette contrast.
- **Anchoring**: Logical ground anchor points align with the bottom-center of ground contacts $(x_{\text{center}}, y_{\text{feet}})$, ensuring accurate depth sorting and footstep placement.
- **Collision Volumes**: Defined relative to ground anchors rather than visual sprite bounding boxes, allowing flying entities and elongated quadrupeds to navigate naturally.

---

## 2. Creature Inventory Summary

| ID | Name | Classification | Frame Size | Sheet Size | Frames | Anchor | Collision $(W \times H)$ | Movement |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| `goblin` | Forest Goblin | Hostile (Combat) | $32 \times 48$ px | $128 \times 384$ px | 32 | $(16, 48)$ | $16 \times 12$ px | Bipedal Ground |
| `skeleton` | Undead Skeleton | Hostile (Combat) | $32 \times 48$ px | $128 \times 384$ px | 32 | $(16, 48)$ | $16 \times 12$ px | Bipedal Ground |
| `wolf` | Forest Wolf | Hostile (Combat) | $48 \times 40$ px | $192 \times 320$ px | 32 | $(24, 40)$ | $24 \times 14$ px | Quadruped Ground |
| `wild_boar` | Wild Boar | Hostile (Combat) | $48 \times 40$ px | $192 \times 320$ px | 32 | $(24, 40)$ | $26 \times 16$ px | Quadruped Ground |
| `bat` | Dusk Bat | Hostile (Combat) | $32 \times 32$ px | $128 \times 256$ px | 32 | $(16, 28)$ | $14 \times 14$ px | Aerial Flying |
| `small_bird` | Small Songbird | Ambient Wildlife | $24 \times 24$ px | $96 \times 96$ px | 16 | $(12, 22)$ | $8 \times 8$ px | Aerial / Perch |
| `forest_butterfly` | Forest Butterfly | Ambient Wildlife | $16 \times 16$ px | $64 \times 32$ px | 8 | $(8, 14)$ | $6 \times 6$ px | Flutter Flying |
| `blue_butterfly` | Blue Butterfly | Ambient Wildlife | $16 \times 16$ px | $64 \times 32$ px | 8 | $(8, 14)$ | $6 \times 6$ px | Flutter Flying |
| `golden_butterfly` | Golden Butterfly | Ambient Wildlife | $16 \times 16$ px | $64 \times 32$ px | 8 | $(8, 14)$ | $6 \times 6$ px | Flutter Flying |

---

## 3. Category A — Hostile & Combat-Capable Creatures

### 3.1 Forest Goblin (`goblin`)
- **Asset Directory**: [`assets/characters/enemies/goblin/`](file:///c:/dev/Wildbound/assets/characters/enemies/goblin/)
- **Visual Description**: Hunched, wiry woodland humanoid with mottled green skin, long pointed ears, ragged brown leather loincloth, and crude stone-tipped dagger.
- **Palette**: Goblin Green `#4B8232`, Skin Shadow `#325A1E`, Leather Rag `#5A3C23`, Dagger Blade `#969BAA`.
- **Anatomy & Dimensions**: $32 \times 48$ px frame; 4 columns $\times$ 8 rows (32 total frames).
- **Ground Anchor**: $(16, 48)$ bottom-center.
- **Collision Footprint**: $16 \times 12$ px centered at base.
- **Supported Animations**:
  - `goblin_idle_down`, `goblin_idle_up`, `goblin_idle_left`, `goblin_idle_right` (4 frames @ 0.20s, loop)
  - `goblin_walk_down`, `goblin_walk_up`, `goblin_walk_left`, `goblin_walk_right` (4 frames @ 0.15s, loop)
  - `goblin_run_down`, `goblin_run_up`, `goblin_run_left`, `goblin_run_right` (4 frames @ 0.09s, loop)
  - `goblin_attack_down`, `goblin_attack_up`, `goblin_attack_left`, `goblin_attack_right` (4 frames @ 0.10s, non-loop)
  - `goblin_hurt_down`, `goblin_hurt_up`, `goblin_hurt_left`, `goblin_hurt_right` (2 frames @ 0.10s, non-loop)
  - `goblin_death_down`, `goblin_death_up`, `goblin_death_left`, `goblin_death_right` (4 frames @ 0.12s, non-loop)
- **Gameplay Configuration**: [`data/characters/enemies/goblin.json`](file:///c:/dev/Wildbound/data/characters/enemies/goblin.json) (HP: 60, Attack: 8, Speed: 65, Detection: 150 px).

---

### 3.2 Undead Skeleton (`skeleton`)
- **Asset Directory**: [`assets/characters/enemies/skeleton/`](file:///c:/dev/Wildbound/assets/characters/enemies/skeleton/)
- **Visual Description**: Bleached animated skeletal warrior wielding a rusted iron arming sword. Hollow black eye sockets emitting faint pale luminescence.
- **Palette**: Aged Bone `#E6DFC8`, Bone Shadow `#AFA08C`, Rusted Iron `#78503C`, Luminescent Glow `#C8E6FF`.
- **Anatomy & Dimensions**: $32 \times 48$ px frame; 4 columns $\times$ 8 rows (32 total frames).
- **Ground Anchor**: $(16, 48)$ bottom-center.
- **Collision Footprint**: $16 \times 12$ px centered at base.
- **Supported Animations**:
  - `skeleton_idle_down`, `skeleton_idle_up`, `skeleton_idle_left`, `skeleton_idle_right` (4 frames @ 0.20s, loop)
  - `skeleton_walk_down`, `skeleton_walk_up`, `skeleton_walk_left`, `skeleton_walk_right` (4 frames @ 0.15s, loop)
  - `skeleton_run_down`, `skeleton_run_up`, `skeleton_run_left`, `skeleton_run_right` (4 frames @ 0.09s, loop)
  - `skeleton_attack_down`, `skeleton_attack_up`, `skeleton_attack_left`, `skeleton_attack_right` (4 frames @ 0.10s, non-loop)
  - `skeleton_hurt_down`, `skeleton_hurt_up`, `skeleton_hurt_left`, `skeleton_hurt_right` (2 frames @ 0.10s, non-loop)
  - `skeleton_death_down`, `skeleton_death_up`, `skeleton_death_left`, `skeleton_death_right` (4 frames @ 0.12s, non-loop)
- **Gameplay Configuration**: [`data/characters/enemies/skeleton.json`](file:///c:/dev/Wildbound/data/characters/enemies/skeleton.json) (HP: 80, Attack: 14, Speed: 50, Detection: 140 px).

---

### 3.3 Forest Wolf (`wolf`)
- **Asset Directory**: [`assets/characters/enemies/wolf/`](file:///c:/dev/Wildbound/assets/characters/enemies/wolf/)
- **Visual Description**: Lean, predatory quadruped with coarse charcoal and silver-grey fur, white throat patch, piercing amber eyes, and fanged muzzle.
- **Palette**: Slate Charcoal `#464650`, Shadow Grey `#2D2D37`, Silver Fur `#9696A5`, Amber Eyes `#FFAA28`.
- **Anatomy & Dimensions**: $48 \times 40$ px frame; 4 columns $\times$ 8 rows (32 total frames, $192 \times 320$ px sheet).
- **Ground Anchor**: $(24, 40)$ bottom-center.
- **Collision Footprint**: $24 \times 14$ px centered at base.
- **Supported Animations**:
  - `wolf_idle_down`, `wolf_idle_up`, `wolf_idle_left`, `wolf_idle_right` (4 frames @ 0.18s, loop)
  - `wolf_walk_down`, `wolf_walk_up`, `wolf_walk_left`, `wolf_walk_right` (4 frames @ 0.14s, loop)
  - `wolf_run_down`, `wolf_run_up`, `wolf_run_left`, `wolf_run_right` (4 frames @ 0.08s, loop)
  - `wolf_attack_down`, `wolf_attack_up`, `wolf_attack_left`, `wolf_attack_right` (4 frames @ 0.09s, non-loop)
  - `wolf_hurt_down`, `wolf_death_down` (non-loop)
- **Gameplay Configuration**: [`data/characters/enemies/wolf.json`](file:///c:/dev/Wildbound/data/characters/enemies/wolf.json) (HP: 75, Attack: 15, Speed: 110, Detection: 220 px).

---

### 3.4 Wild Boar (`wild_boar`)
- **Asset Directory**: [`assets/characters/enemies/wild_boar/`](file:///c:/dev/Wildbound/assets/characters/enemies/wild_boar/)
- **Visual Description**: Heavy, muscular quadruped covered in bristly reddish-brown fur with an armored snout, dorsal ridge, and formidable ivory tusks.
- **Palette**: Bristly Russet `#503223`, Deep Underbelly `#371E14`, Ivory Tusks `#E6DEC8`, Snout Pink `#AA645A`.
- **Anatomy & Dimensions**: $48 \times 40$ px frame; 4 columns $\times$ 8 rows (32 total frames, $192 \times 320$ px sheet).
- **Ground Anchor**: $(24, 40)$ bottom-center.
- **Collision Footprint**: $26 \times 16$ px centered at base.
- **Supported Animations**:
  - `boar_idle_down`, `boar_idle_up`, `boar_idle_left`, `boar_idle_right` (4 frames @ 0.20s, loop)
  - `boar_walk_down`, `boar_walk_up`, `boar_walk_left`, `boar_walk_right` (4 frames @ 0.16s, loop)
  - `boar_run_down`, `boar_run_up`, `boar_run_left`, `boar_run_right` (4 frames @ 0.09s, loop)
  - `boar_attack_down`, `boar_attack_up`, `boar_attack_left`, `boar_attack_right` (4 frames @ 0.10s, non-loop)
  - `boar_hurt_down`, `boar_death_down` (non-loop)
- **Gameplay Configuration**: [`data/characters/enemies/wild_boar.json`](file:///c:/dev/Wildbound/data/characters/enemies/wild_boar.json) (HP: 120, Attack: 20, Speed: 85, Detection: 160 px).

---

### 3.5 Dusk Bat (`bat`)
- **Asset Directory**: [`assets/characters/enemies/bat/`](file:///c:/dev/Wildbound/assets/characters/enemies/bat/)
- **Visual Description**: Fast nocturnal flying creature with leathery membranous wings, dark purple fur, red eyes, and sharp fangs.
- **Palette**: Dusk Purple `#3C2846`, Wing Shadow `#281432`, Wing Membrane `#553764`, Crimson Eye `#DC2828`.
- **Anatomy & Dimensions**: $32 \times 32$ px frame; 4 columns $\times$ 8 rows (32 total frames, $128 \times 256$ px sheet).
- **Ground Anchor**: $(16, 28)$ (visual shadow base; body floats at $z = 12$ px elevation).
- **Collision Footprint**: $14 \times 14$ px centered at shadow base.
- **Supported Animations**:
  - `bat_fly_down`, `bat_fly_up`, `bat_fly_left`, `bat_fly_right` (4 frames @ 0.10s, loop)
  - `bat_move_down`, `bat_move_up`, `bat_move_left`, `bat_move_right` (4 frames @ 0.08s, loop)
  - `bat_attack_down`, `bat_attack_up`, `bat_attack_left`, `bat_attack_right` (4 frames @ 0.08s, non-loop)
  - `bat_hurt_down`, `bat_death_down` (non-loop)
- **Gameplay Configuration**: [`data/characters/enemies/bat.json`](file:///c:/dev/Wildbound/data/characters/enemies/bat.json) (HP: 40, Attack: 10, Speed: 95, Detection: 180 px).

---

## 4. Category B — Ambient Wildlife

### 4.1 Small Songbird (`small_bird`)
- **Asset Directory**: [`assets/characters/wildlife/birds/small_bird/`](file:///c:/dev/Wildbound/assets/characters/wildlife/birds/small_bird/)
- **Visual Description**: Delicate woodland songbird with azure crown and back, bright golden-buff chest, black beak, and white wing accents.
- **Palette**: Azure Blue `#3278C8`, Breast Buff `#F0AA3C`, Beak Dark `#282832`, Wing White `#F0F0FA`.
- **Anatomy & Dimensions**: $24 \times 24$ px frame; 4 columns $\times$ 4 rows (16 total frames, $96 \times 96$ px sheet).
- **Ground Anchor**: $(12, 22)$ perched ground contact.
- **Collision Footprint**: $8 \times 8$ px.
- **Supported Animations**:
  - `bird_idle` (4 frames @ 0.22s, loop: pecking and head turning)
  - `bird_fly` (4 frames @ 0.08s, loop: rapid wing flap)
  - `bird_takeoff` (4 frames @ 0.10s, non-loop: crouch to spring takeoff)
  - `bird_landing` (4 frames @ 0.10s, non-loop: wing brake and perch)

---

### 4.2 Butterflies (3 Chromatic Variants)

All butterfly variations share the identical lightweight $16 \times 16$ px format (4 columns $\times$ 2 rows = 8 frames, $64 \times 32$ px sheet) with distinct color identities:

#### 1. Forest Butterfly (`forest_butterfly`)
- **Directory**: [`assets/characters/wildlife/butterflies/forest_butterfly/`](file:///c:/dev/Wildbound/assets/characters/wildlife/butterflies/forest_butterfly/)
- **Primary Color**: Emerald Forest Green `#28C864` with Dark Forest Veins `#146432`.
- **Habitat**: Dense woodlands, fern thickets, canopy clearings.
- **Animations**: `forest_butterfly_resting` (4 frames @ 0.25s), `forest_butterfly_fly` (4 frames @ 0.12s).

#### 2. Blue Butterfly (`blue_butterfly`)
- **Directory**: [`assets/characters/wildlife/butterflies/blue_butterfly/`](file:///c:/dev/Wildbound/assets/characters/wildlife/butterflies/blue_butterfly/)
- **Primary Color**: Radiant Azure Blue `#2882E6` with Deep Navy Borders `#143C78`.
- **Habitat**: Water margins, riverbanks, misty grottos.
- **Animations**: `blue_butterfly_resting` (4 frames @ 0.25s), `blue_butterfly_fly` (4 frames @ 0.12s).

#### 3. Golden Butterfly (`golden_butterfly`)
- **Directory**: [`assets/characters/wildlife/butterflies/golden_butterfly/`](file:///c:/dev/Wildbound/assets/characters/wildlife/butterflies/golden_butterfly/)
- **Primary Color**: Sunlit Honey Gold `#F0B41E` with Warm Umber Trim `#78500A`.
- **Habitat**: Sunny wildflower glades, wheat fields, open shrines.
- **Animations**: `golden_butterfly_resting` (4 frames @ 0.25s), `golden_butterfly_fly` (4 frames @ 0.12s).

---

## 5. AMS Registration & Asset IDs

All Phase 9 textures and spritesheets are registered in [`assets/manifests/asset_manifest.json`](file:///c:/dev/Wildbound/assets/manifests/asset_manifest.json) under the following IDs:

| Asset ID | Type | Resource File |
| :--- | :--- | :--- |
| `tex_wolf` | Texture | `characters/enemies/wolf/wolf_spritesheet.png` |
| `sheet_wolf` | Spritesheet | `characters/enemies/wolf/wolf_spritesheet.json` |
| `tex_wild_boar` | Texture | `characters/enemies/wild_boar/wild_boar_spritesheet.png` |
| `sheet_wild_boar` | Spritesheet | `characters/enemies/wild_boar/wild_boar_spritesheet.json` |
| `tex_bat` | Texture | `characters/enemies/bat/bat_spritesheet.png` |
| `sheet_bat` | Spritesheet | `characters/enemies/bat/bat_spritesheet.json` |
| `tex_small_bird` | Texture | `characters/wildlife/birds/small_bird/small_bird_spritesheet.png` |
| `sheet_small_bird` | Spritesheet | `characters/wildlife/birds/small_bird/small_bird_spritesheet.json` |
| `tex_forest_butterfly` | Texture | `characters/wildlife/butterflies/forest_butterfly/forest_butterfly_spritesheet.png` |
| `sheet_forest_butterfly` | Spritesheet | `characters/wildlife/butterflies/forest_butterfly/forest_butterfly_spritesheet.json` |
| `tex_blue_butterfly` | Texture | `characters/wildlife/butterflies/blue_butterfly/blue_butterfly_spritesheet.png` |
| `sheet_blue_butterfly` | Spritesheet | `characters/wildlife/butterflies/blue_butterfly/blue_butterfly_spritesheet.json` |
| `tex_golden_butterfly` | Texture | `characters/wildlife/butterflies/golden_butterfly/golden_butterfly_spritesheet.png` |
| `sheet_golden_butterfly`| Spritesheet | `characters/wildlife/butterflies/golden_butterfly/golden_butterfly_spritesheet.json` |

