# Wildbound — Armor Art Specification

## 1. Design Principles & Material Aesthetics

The Wildbound armor collection provides visually distinct protective equipment layers adhering to the Phase 6 Art Bible. Each armor piece visually communicates its physical material, weight, craft origin, and biome affiliation without requiring artificial statistical labels.

### Material Appearance & Shading Rules
* **Boiled Leather**: Warm organic tones (`#87502D` to `#B97D4B`), soft matte specular highlight, riveted brass grommets, and visible shoulder harness strapping.
* **Forged Iron**: Heavy desaturated iron tones (`#9B9DAA` to `#696E78`), subtle hammer-marked matte texture, wide deflecting surfaces, and dark steel edging.
* **Polished Steel**: High-value metallic highlights (`#D2DAE6` to `#919BAA`), sharp specular central ridge line, and flared defensive pauldrons.
* **Enchanted Heartwood**: Emerald and moss tones (`#378250` to `#C8E6A0`), natural grain striations, living bark ridges, and silver filigree.
* **Gilded Bronze**: Warm metallic golds and burnt umbers (`#D7AF4B` to `#916E28`), layered scale lamellar plates, and solar crest emblems.
* **Glacial Frost Steel**: Pale cyan glacial reflections (`#7DB9EB` to `#DCF5FF`), sharp icicle flanges, and fur-padded interior linings.
* **Arcane Meteoric Plate**: Dark obsidian stone (`#2D303E`), deep bevels, and glowing neon-cyan arcane rune inlays (`#50DCF5`).

---

## 2. Armor Collection — 8 Designs

| ID | Name | Material Family | Silhouette & Defensive Features |
| :--- | :--- | :--- | :--- |
| `arm_leather_armor_001` | Leather Armor | Boiled Cowhide | Padded cuirass with double-thick chest panel, shoulder buckle straps, and brass rivets. |
| `arm_reinforced_leather_001` | Reinforced Leather Armor | Studded Leather & Steel | Dark leather breastplate reinforced with hammered steel splints and articulated shoulder cops. |
| `arm_iron_armor_001` | Iron Armor | Cast Iron | Solid iron breastplate with flared gorget neck guard and broad protective shoulder pauldrons. |
| `arm_steel_armor_001` | Steel Armor | Polished Folded Steel | High-polish knight's cuirass with pronounced central deflecting ridge and flared couters. |
| `arm_forest_guardian_001` | Forest Guardian Armor | Heartwood & Leaf-Scales | Interlocking hardened bark plates laced with enchanted ivy cords and silver leaf clasps. |
| `arm_desert_armor_001` | Desert Armor | Bronze Lamellar | Overlapping gilded bronze scales providing flexible coverage with an embossed sunburst motif. |
| `arm_frost_armor_001` | Frost Armor | Glacial Alloy Steel | Heavy ice-tempered steel breastplate edged with white wolf fur and jagged frost deflectors. |
| `arm_ancient_rune_001` | Ancient Rune Armor | Celestial Meteoric Plate | Star-metal dark plate etched with glowing elder runes that pulse with radiant celestial energy. |

---

## 3. Modular Layering & Undergarment Compatibility

In Wildbound's modular character pipeline, armor is rendered on its own dedicated visual layer:
* **Shirt / Clothing Layer**: `Z-Index = 50` (Rendered directly above the skin and pants).
* **Armor Layer**: `Z-Index = 52` (Rendered directly on top of the shirt).

This architecture allows players to freely mix and match any tunic, robe, or shirt under their armor:
* A player wearing `clo_basic_traveler_001` under `arm_iron_armor_001` will see the cloth sleeves and collar extending naturally around the metal cuirass.
* A player can unequip their shirt to wear bare armor, or unequip their armor to display purely civilian clothing.

---

## 4. Four-Directional Armor Rendering

Armor pieces feature bespoke pixel-art compositions across all 4 cardinal facing directions:

1. **Facing Down (South)**:
   * Full view of the front breastplate, central deflecting ridge, neck gorget, and bilateral shoulder pauldrons.
2. **Facing Up (North)**:
   * Backplate coverage protecting the spine and shoulder blades, pauldron rear flanges, and harness strapping.
3. **Facing Left (West)**:
   * Lateral side profile of the breastplate, forward chest curvature, left shoulder pauldron, and side harness laces.
4. **Facing Right (East)**:
   * Mirrored lateral side profile with right shoulder pauldron, proper lighting key (top-left), and side strapping.

---

## 5. Scope Boundary Compliance

> [!NOTE]
> All armor assets in Phase 17 are visual cosmetic layers only.
> * NO defense values or armor ratings are defined.
> * NO damage reduction, posture points, or weight penalties are calculated.
> * NO gameplay statistics, durability mechanics, or combat modifications are implemented.

