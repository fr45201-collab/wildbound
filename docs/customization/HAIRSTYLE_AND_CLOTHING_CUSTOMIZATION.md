# Hairstyle and Clothing Customization

## Overview
Hairstyle and clothing items provide players with expressive visual identity options while retaining visual clarity in combat. The system supports full tinting, multi-frame directional animations, and runtime swatch changes.

---

## 1. Hairstyle Customization

### Registered Hair Variants
Wildbound registers 5 default hairstyles in `CosmeticRegistry`:
- `cosmetic_hair_short` ("Short Hair", asset `sheet_hair_short`)
- `cosmetic_hair_long` ("Long Hair", asset `sheet_hair_long`)
- `cosmetic_hair_buzz` ("Buzz Cut", asset `sheet_hair_buzz`)
- `cosmetic_hair_braids` ("Warrior Braids", asset `sheet_hair_braids`)
- `cosmetic_hair_ponytail` ("High Ponytail", asset `sheet_hair_ponytail`)

### Hair Tinting / Color Palette
Hair color is defined as an RGBA 32-bit color tint (`Color` in raylib):
```cpp
appearance.hairColor = Color{ 140, 70, 30, 255 }; // Chestnut brown
```
The modular renderer multiplies each pixel's source luminance by `hairColor`, permitting dynamic real-time color picking from natural shades (blonde, brunette, raven, auburn) to fantasy tones (azure, crimson, emerald).

---

## 2. Clothing Slots

### Chest (Tunics, Shirts, Cuirasses)
- Supported archetypes: Adventurer's Tunic, Leather Vest, Cloth Shirt, Heavy Plate Cuirass.
- Tunic assets contain 4-directional 16x16 / 32x32 frames mapped to `Walk` and `Idle` cycles.
- Multi-colored tunics utilize the `tintColor` property of `CosmeticDefinition`.

### Legs (Pants, Robes, Leggings)
- Supported archetypes: Linen Trousers, Leather Pants, Plate Greaves, Iron-Reinforced Leggings.
- Synchronized to the character locomotion state machine to guarantee stepping and sprint alignment.

### Feet (Boots, Sandals, Sabatons)
- Distinct left/right foot movement profiles during directional walk cycles.
- Integrated audio footstep cues determined by surface and footwear category.

---

## 3. Data Integration Example

```json
{
  "id": "cosmetic_tunic_adventurer",
  "name": "Adventurer's Tunic",
  "slot": "chest",
  "category": "clothing",
  "assetId": "sheet_chest_adventurer_001",
  "tintColor": [50, 120, 200, 255],
  "rarity": "Common",
  "unlockedByDefault": true
}
```
In the Wardrobe UI, players select from available clothing items, inspect the real-time render preview, and swap clothing items with zero gameplay latency.

