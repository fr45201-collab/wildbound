# Cosmetic Slot and Compatibility Specification

## Overview
Wildbound implements a strict 9-slot character appearance hierarchy. Each cosmetic asset is classified under an authoritative slot enum `CustomizationSlot` and layered according to deterministic 2D top-down rendering rules.

```
       [Back]             (Behind torso, e.g. capes, quivers, backpacks)
         │
       [Skin]             (Base body sprite: head, torso, limbs)
         │
   [Chest] / [Legs]       (Inner garments: tunics, shirts, pants, robes)
         │
       [Feet]             (Footwear: boots, sandals, greaves)
         │
       [Face]             (Facial cosmetics: beards, eye patches, masks)
         │
       [Hair]             (Hairstyles: buzz, long, braids, ponytail)
         │
     [Headwear]           (Hats, bandanas, circlets, hoods)
         │
      [Helmet]            (Heavy full helmets, visors, armored helms)
```

---

## 1. Authoritative Customization Slots

| CustomizationSlot Enum | String ID | Target Layer | Rendering Order | Compatible Categories |
| :--- | :--- | :--- | :--- | :--- |
| `Skin` | `"skin"` | `ModularLayer::Skin` | 1 | Base skin tones, tints |
| `Back` | `"back"` | `ModularLayer::Back` | 0 (Behind) / 8 | Backpacks, capes, quivers |
| `Legs` | `"legs"` | `ModularLayer::Legs` | 2 | Pants, leggings, skirts |
| `Feet` | `"feet"` | `ModularLayer::Feet` | 3 | Boots, greaves, shoes |
| `Chest` | `"chest"` | `ModularLayer::Chest` | 4 | Tunics, plate cuirasses, shirts |
| `Face` | `"face"` | `ModularLayer::Face` | 5 | Facial hair, glasses, warpaint |
| `Hair` | `"hair"` | `ModularLayer::Hair` | 6 | Hairstyles, pony-tails, curls |
| `Headwear` | `"headwear"` | `ModularLayer::Headwear` | 7 | Hats, hoods, crowns |
| `Helmet` | `"helmet"` | `ModularLayer::Helmet` | 8 | Plate helmets, full helms |

---

## 2. Occlusion and Compatibility Rules

To prevent sprite clipping and visual dissonance, Wildbound implements data-driven occlusion rules:

1. **Helmet vs. Hair Occlusion**:
   - Equipping a full helmet (`CustomizationSlot::Helmet` with `CosmeticDefinition::occludesHair = true`) suppresses the rendering of `CustomizationSlot::Hair`.
   - The equipped hairstyle ID remains saved in the loadout/appearance, but `ModularCharacterRenderer` conditionally skips drawing the hair layer while the full helmet is active.
2. **Helmet vs. Headwear Mutual Exclusivity**:
   - `CustomizationSlot::Headwear` and `CustomizationSlot::Helmet` represent distinct visual tiers.
   - If a heavy closed helmet is equipped, `ModularCharacterRenderer` yields precedence to the helmet layer over soft headwear.
3. **Chest and Back Compatibility**:
   - Bulkier back items (such as the Heavy Backpack) render anchor-adjusted sprites depending on character facing direction (`South`, `North`, `East`, `West`).
   - When facing `North`, the `Back` slot renders in front of the `Chest` slot; when facing `South`, it renders behind the torso.
4. **Legs vs. Feet Layering**:
   - High boots (`CustomizationSlot::Feet`) render on layer 3, neatly overlaying trousers (`CustomizationSlot::Legs`, layer 2).

---

## 3. Data-Driven Slot Validation

Cosmetic definitions specify their target slot and compatibility constraints via JSON configuration:

```json
{
  "id": "cosmetic_iron_helmet",
  "name": "Iron Bascinet",
  "slot": "helmet",
  "category": "headwear",
  "assetId": "sheet_helm_iron_001",
  "occludesHair": true,
  "occludesFace": false,
  "color": [255, 255, 255, 255]
}
```

When validating appearance payloads, `CharacterCustomizationManager::ValidateAppearance()` verifies:
- All slot assignments match valid registered cosmetics in `CosmeticRegistry`.
- Cosmetic slot enum matches the target slot in `CharacterAppearance`.
- Unknown cosmetic IDs cleanly fallback to `""` (empty/unequipped) without crashing.

