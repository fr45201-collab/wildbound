# Cosmetic Catalogue Specification (Phase 26)

## 1. Scope & Catalog Organization

The Wildbound Cosmetic Catalogue unifies **98 unique cosmetic items** populated from Phase 8 (production cosmetics), Phase 17 (costumes & armor assets), and Phase 7 (modular character base assets).

Cosmetics are classified by:
1. **Target Slot** (`CustomizationSlot`): Hair, Headwear, Helmet, Face, Chest, Legs, Feet, Back, Skin.
2. **Category**:
   - `hair` (Hairstyles)
   - `clothing` (Civilian, adventurer, mystic, royal tunics and shirts)
   - `armor` (Platemail, cuirasses, iron/steel armor, shields)
   - `headwear` (Hats, hoods, caps)
   - `helmet` (Combat helmets, visors, circlets)
   - `face` (Masks, goggles, spectacles, warpaint)
   - `backpacks` (Traveler packs, quivers, satchels)
   - `footwear` (Leather boots, sandals, greaves)
   - `skin` (Base skin tone variations)
3. **Rarity Tier**: Common, Uncommon, Rare, Epic, Legendary, Mythic.
4. **Gameplay Equipment Mapping**: Distinguishes purely cosmetic costumes from visual representations of combat equipment.

---

## 2. Canonical Catalogue Breakdown

| Category | Item Count | Example Asset IDs | Default Status |
|---|---|---|---|
| **Skin Tones** | 4 | `cosmetic_skin_default`, `cosmetic_skin_pale`, `cosmetic_skin_tan`, `cosmetic_skin_dark` | **Owned** (Starter) |
| **Hairstyles** | 12 | `cosmetic_hair_short`, `cosmetic_hair_medium`, `cosmetic_hair_long`, `hair_messy_001`, `hair_braided_red_001` | Short/Med/Long: **Owned**; Others: **Locked** |
| **Clothing** | 22 | `cosmetic_tunic_adventurer`, `clo_basic_traveler_001`, `shirt_leather_001`, `shirt_royal_001` | Adventurer Tunic: **Owned**; Others: **Locked** |
| **Armor** | 18 | `arm_iron_armor_001`, `arm_steel_armor_001`, `sheet_armor_iron_cuirass`, `shirt_armor_001` | **Locked** |
| **Headwear** | 14 | `hat_adventurer_001`, `hat_explorer_hood_001`, `hat_straw_001` | **Locked** |
| **Helmets** | 8 | `hat_helmet_001`, `hat_iron_helmet_001`, `sheet_helm_iron_001` | **Locked** |
| **Face Wear** | 8 | `goggles_adventure_001`, `mask_leather_001`, `fac_cloth_mask_001` | **Locked** |
| **Backpacks** | 6 | `backpack_leather_001`, `backpack_adventure_001`, `bag_traveler_001` | **Locked** |
| **Footwear** | 6 | `cosmetic_boots_leather`, `sho_leather_boots_001`, `sho_iron_boots_001` | Leather Boots: **Owned**; Others: **Locked** |

---

## 3. Schema & Validation

Each cosmetic item in the catalogue conforms to the following schema definition:

```json
{
  "id": "clo_royal_outfit_001",
  "displayName": "Royal Regalia",
  "description": "Ceremonial garments woven with gold embroidery.",
  "slot": "Chest",
  "category": "clothing",
  "spritesheetId": "sheet_clo_royal_outfit_001",
  "textureId": "tex_clo_royal_outfit_001",
  "iconId": "icon_clothing",
  "compatibleDirections": ["up", "down", "left", "right"],
  "compatibleAnimations": ["idle", "walk", "run"],
  "occlusionFlags": [],
  "anchorOffset": [0.0, 0.0],
  "tint": [255, 255, 255, 255],
  "rarity": "legendary",
  "isGameplayEquipment": false
}
```

### Validation Rules:
1. `id` must be non-empty and globally unique within the registry.
2. `spritesheetId` must resolve to an asset registered in `AssetRegistry`.
3. `rarity` must map to one of the 6 canonical tiers.
4. `compatibleDirections` must contain at least the forward (`down`) orientation.

