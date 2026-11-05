# Wildbound — Cosmetic Compatibility & Occlusion Specification

## 1. Architectural Overview

Wildbound's modular character system allows complete player appearance customization without requiring pre-baked spritesheets for every combination. Instead, each character is composited in real time from independent visual layers managed by `ModularCharacter` and `CharacterEquipment`.

To guarantee visual polish and prevent graphical glitches (such as hair clipping through solid metal helmets, goggles rendering over full porcelain masks, or backpacks floating in front of characters when viewed from the front), the engine utilizes a **data-driven compatibility and occlusion system**.

---

## 2. Equipment Slots & Categories

The engine defines 12 standard cosmetic equipment slots:

| Slot / Category | Enum Value | Standard Z-Index (South) | Standard Z-Index (North) | Description |
| :--- | :--- | :---: | :---: | :--- |
| **Shadow** | *Internal* | 5 | 5 | Contact ground ellipse. |
| **Back** | `CosmeticCategory::Back` | **10** *(Behind)* | **85** *(Foreground)* | Backpacks, quivers, bedrolls. |
| **Skin** | `CosmeticCategory::Skin` | 20 | 20 | Base humanoid anatomy & skin tones. |
| **Pants** | `CosmeticCategory::Pants` | 30 | 30 | Trousers, breeches, leg garments. |
| **Shoes** | `CosmeticCategory::Shoes` | 40 | 40 | Boots, sabatons, shoes, sandals. |
| **Shirt** | `CosmeticCategory::Shirt` | 50 | 50 | Tunics, shirts, robes, civilian jackets. |
| **Armor** | `CosmeticCategory::Armor` | **52** | **52** | Cuirasses, breastplates, scale vests. |
| **Waist** | `CosmeticCategory::Waist` | 55 | 55 | Belts, sashes, utility pouches. |
| **Hair** | `CosmeticCategory::Hair` | 60 | 60 | Hairstyles and beards. |
| **Face Details** | `CosmeticCategory::Face` | 62 *(Visible)* | 13 *(Hidden)* | Warpaint, scars, eyepatches. |
| **Mask** | `CosmeticCategory::Mask` | 68 *(Visible)* | 14 *(Hidden)* | Cloth bandanas, leather masks, porcelain masks. |
| **Goggles** | `CosmeticCategory::Goggles` | 72 *(Visible)* | 15 *(Hidden)* | Eyewear, adventurer goggles, monocles. |
| **Headwear** | `CosmeticCategory::Headwear` | 80 | 80 | Hats, hoods, helmets, crowns. |
| **Held Weapon** | *Weapon System* | 85 | 18 | Directional held swords and bows. |

---

## 3. Data-Driven Occlusion Flags

Occlusion flags are defined in each cosmetic's `metadata.json` and evaluated dynamically during `ModularCharacter::BuildLayerStack()`:

### 1. `hair_hide_all`
* **Triggered By**: Solid metal closed helmets (e.g., `hat_iron_helmet_001`, `hat_steel_helmet_001`, `hat_helmet_001`).
* **Behavior**: Completely suppresses the `Hair` visual layer from the render stack (`hideHair = true`).
* **Result**: Eliminates hair poking through steel helmets without requiring custom "under-helmet" hair meshes.

### 2. `mask_hide_goggles`
* **Triggered By**: Rigid full-face masks (e.g., `fac_mystic_mask_001`, `mask_mystic_001`).
* **Behavior**: Suppresses the `Goggles` and `Face` eyewear layers (`hideGoggles = true`).
* **Result**: Prevents goggles from floating awkwardly or clipping into porcelain and solid masks.

### 3. `robe_hide_pants`
* **Triggered By**: Floor-length ceremonial robes and heavy cloaks.
* **Behavior**: Suppresses the `Pants` layer (`hidePants = true`) to prevent leg geometry from poking through robe skirts.

---

## 4. Directional Depth Sorting & Occlusion Matrix

Because Wildbound is rendered in top-down 2D, the visible ordering of layers changes depending on the character's facing direction:

```mermaid
flowchart TD
    subgraph SouthFacing ["Facing Down (South) / Side (West/East)"]
        A1["Backpack (Z=10)"] --> B1["Skin/Body (Z=20)"]
        B1 --> C1["Pants (Z=30)"]
        C1 --> D1["Shoes (Z=40)"]
        D1 --> E1["Shirt (Z=50)"]
        E1 --> F1["Armor (Z=52)"]
        F1 --> G1["Hair & Face (Z=60-72)"]
        G1 --> H1["Headwear (Z=80)"]
        H1 --> I1["Weapon (Z=85)"]
    end

    subgraph NorthFacing ["Facing Up (North)"]
        A2["Weapon (Z=18)"] --> B2["Skin/Body (Z=20)"]
        B2 --> C2["Pants (Z=30)"]
        C2 --> D2["Shoes (Z=40)"]
        D2 --> E2["Shirt (Z=50)"]
        E2 --> F2["Armor (Z=52)"]
        F2 --> G2["Headwear (Z=80)"]
        G2 --> H2["Backpack (Z=85)"]
    end
```

### Key Directional Behaviors:
1. **Backpack Flipping**:
   * Facing South, East, West: Backpack rests on the character's back behind the torso (`Z = 10`).
   * Facing North: The player views the character from behind; the backpack renders *over* the back of the shirt and armor (`Z = 85`).
2. **Face Accessory Culling**:
   * Facing South, East, West: Warpaint, masks, and goggles are rendered in front of the face (`visible = true`).
   * Facing North: The character is facing away; facial accessories are culled entirely (`visible = false`).

---

## 5. Animation Synchronization

To prevent animation drift and visual tearing:
* All cosmetic layers share the identical spritesheet frame dimensions ($32 \times 48$ px) and layout (32 frames, 4 columns × 8 rows).
* The master `AnimationController` on the entity computes the active frame index and cell coordinates once per frame.
* Every equipped layer samples from its respective spritesheet using the exact same calculated frame rectangle.
* Hot-swapping equipment in DevUI or through code occurs immediately without interrupting the running animation cycle.

