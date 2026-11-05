# Wildbound — Weapon Art Specification

## 1. Visual Direction & Design Principles

Wildbound's weapon art collection embodies the established Phase 6 Art Bible standards: a cohesive, readable 2D top-down fantasy aesthetic designed for a virtual resolution of 640×360 pixels and 32×32 base tile sizing.

### Core Visual Principles
* **Readability Over Noise**: Strong, bold silhouettes with high-contrast edge value differences so weapons remain instantly recognizable both in the user interface (icons) and in the active gameplay world.
* **Palette Cohesion**: All materials (seasoned wood, pitted iron, polished folded steel, gilded brass, ash wood, yew, horn, celestial runes) conform strictly to the unified game palette.
* **Modular Compatibility**: Weapons are authored as independent asset layers. No full character spritesheets are duplicated for weapon equipping.

---

## 2. Technical Asset Standards & Dimensions

Every weapon in Wildbound supports four dedicated visual representations:

| Representation | Dimensions | Canvas / Grid | Framing & Presentation |
| :--- | :--- | :--- | :--- |
| **Inventory Icon** | 64 × 64 px | Single 64×64 cell | Angled diagonal display (45°) inside circular translucent display badge with distinct silhouette and 8px perimeter padding. |
| **Ground Item** | 32 × 32 px | Single 32×32 cell | Flat top-down world perspective with semi-transparent elliptical drop shadow for natural placement on grass, dirt, stone, or snow. |
| **Player-Held Sprite** | 128 × 32 px | 4 horizontal 32×32 frames | 4 cardinal directions (Down, Up, Left, Right). Aligned to the player hand socket with defined grip anchor point. |
| **Animation Pose Strip** | 128 × 32 px | 4 horizontal 32×32 frames | 4 action keyframes representing attack sequence or draw cycle. |

---

## 3. Directional Compatibility & Depth Sorting

The Wildbound character system supports four cardinal facing directions: Down (South), Up (North), Left (West), and Right (East).

### Directional Z-Order Matrix
To ensure realistic weapon alignment and prevent visual bleeding or incorrect occlusion:

1. **Facing Down (South)**:
   * **Z-Index**: `85` (Foreground).
   * **Behavior**: Weapon is held in the character's right hand and rendered in front of the torso, belt, and leg garments.
2. **Facing Up (North)**:
   * **Z-Index**: `18` (Background).
   * **Behavior**: Character is viewed from behind. The held weapon is rendered behind the torso and shoulder armor, projecting upward or over the shoulder.
3. **Facing Left (West)**:
   * **Z-Index**: `85` (Foreground).
   * **Behavior**: Weapon is held pointing forward-left, rendered in front of the lateral torso.
4. **Facing Right (East)**:
   * **Z-Index**: `85` (Foreground).
   * **Behavior**: Weapon is held pointing forward-right, rendered in front of the lateral torso.

### Grip Anchor Points
* Swords: Standardized grip point at `(12.0, 18.0)` relative to the 32×32 frame origin.
* Bows: Standardized grip point at `(16.0, 16.0)` relative to the 32×32 frame origin.

---

## 4. Animation Readiness Specifications

Weapons are authored with 4-frame action sequences matching character locomotion and combat states:

### Swords (Melee Cycle)
* **Frame 0 (Idle / Held)**: Weapon rests naturally at the character's side at a downward angle.
* **Frame 1 (Attack Prep / Windup)**: Blade raised high behind the shoulder, coiling tension.
* **Frame 2 (Full Slash / Impact)**: Rapid horizontal forward cleave accompanied by a dynamic motion streak.
* **Frame 3 (Recovery / Follow-through)**: Follow-through deceleration returning to neutral ready stance.

### Bows (Ranged Cycle)
* **Frame 0 (Idle / Held)**: Bow un-drawn, resting at side with string taut.
* **Frame 1 (Drawing String)**: String drawn backward halfway with archer's hand engaging the nock.
* **Frame 2 (Full Draw)**: Full draw weight with broadhead arrow nocked and ready to fire.
* **Frame 3 (Release / Recoil)**: Bow limbs snap forward, string vibrates, arrow launched with air puff.

