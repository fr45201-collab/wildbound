# Accessory and Armor Integration

## Overview
Wildbound features a dual-layer equipment philosophy:
1. **Mechanical Equipment (`EquipmentSystem`)**: Dictates defense stats, damage resistance, durability, movement penalties, and combat bonuses.
2. **Cosmetic Customization (`CharacterCustomizationManager`)**: Dictates visual appearance, vanity overlays, and thematic styling.

This document describes how accessories, hats, helmets, masks, backpacks, and armor pieces interface with both systems.

---

## 1. Accessory Categories

### Headwear & Hats
- Includes soft headwear: Wizard Hats, Rogue Cowls, Straw Hats, Bandanas, and Circlets.
- Renders on layer 7 (`ModularLayer::Headwear`).
- Does not suppress hair rendering, allowing hair bangs and side-locks to peek through.

### Helmets & Armored Visors
- Heavy battle helms: Iron Bascinets, Knight Visors, Horned Barbarian Helms, Bone Masks.
- Renders on layer 8 (`ModularLayer::Helmet`).
- Triggers `occludesHair = true`, hiding the player's underlying hairstyle geometry to prevent clipping.

### Facial Accessories
- Beards, Spectacles, Goggles, Bandits' Half-Masks, Eye Patches, and Warpaint.
- Renders on layer 5 (`ModularLayer::Face`).
- Can coexist with both hairstyles and soft headwear.

### Back Accessories
- Capes, Backpacks, Cloaks, and Quivers.
- Dynamic directional anchor layering:
  - When facing South/East/West: Rendered behind torso (`layer 0`).
  - When facing North (away from camera): Rendered in front of torso (`layer 8`).

---

## 2. Decoupling Armor Stats from Cosmetic Appearances

Players can equip combat-heavy armor for high defense while projecting whatever aesthetic appearance they choose (vanity / transmogrification):

- If a player equips an *Iron Cuirass* in `EquipmentSlot::Chest`, the `EquipmentSystem` applies `+15 Physical Defense` and `-5% Move Speed`.
- The player may select "Mage Robes" in their Wardrobe loadout.
- The visual rendering shows the Mage Robes; combat calculations use the Iron Cuirass defense statistics.
- If the cosmetic slot is set to `"default"` or empty, the renderer falls back to displaying the visual sprite associated with the currently equipped gameplay item.

---

## 3. Data Integration

Cosmetic definitions specify asset metadata loaded directly from Phase 17/18 costume catalogs (`data/cosmetics/costume_catalog.json`):

```json
{
  "id": "cosmetic_cape_ranger",
  "name": "Ranger's Cloak",
  "slot": "back",
  "category": "accessory",
  "assetId": "sheet_back_ranger_cloak",
  "directionalRendering": true,
  "occludesHair": false
}
```
All assets are resolved dynamically through `AssetManager` by asset ID.

