# Wildbound — Cosmetic Equipment System

## 1. System Overview

The **Cosmetic Equipment System** manages the character's active visual items, enforces single-item-per-category exclusivity, handles dynamic appearance persistence, and provides developer wardrobe tools for runtime testing and inspection.

---

## 2. Data Structures

### `CosmeticItem`
Defined in [`src/entities/ModularCharacter.hpp`](file:///c:/dev/Wildbound/src/entities/ModularCharacter.hpp):
```cpp
struct CosmeticItem {
    std::string id;
    std::string name;
    CosmeticCategory category = CosmeticCategory::Shirt;
    std::string spritesheetId;
    std::string textureId;
    std::vector<std::string> occlusionFlags; // e.g. "hair_hide_all", "robe_hide_pants"
    Color tint = WHITE;
    Vector2 anchorOffset{ 0.0f, 0.0f };
    bool visible = true;

    bool IsValid() const;
    nlohmann::json ToJson() const;
    static CosmeticItem FromJson(const nlohmann::json& j);
};
```

### `CharacterEquipment`
Encapsulates an `std::unordered_map<CosmeticCategory, CosmeticItem>` ensuring strict exclusivity per slot:
- `Equip(item)`: Equipping an item in a slot automatically replaces any existing item in that slot.
- `Unequip(category)`: Removes the equipped item from the slot.
- `Clear()`: Strips all equipped cosmetic items, leaving a bare mannequin base.
- `HasOcclusionFlag(flag)`: Scans all currently equipped items across all categories to determine if a specific occlusion condition is active.

---

## 3. JSON Appearance Configuration Schema

Player and NPC character appearances are serialized to JSON. The active player default loadout is located at [`data/characters/player_appearance.json`](file:///c:/dev/Wildbound/data/characters/player_appearance.json):

```json
{
  "base_body": "sheet_skin_default",
  "render_shadow": true,
  "equipment": {
    "skin": {
      "id": "skin_default",
      "name": "Default Skin",
      "category": "skin",
      "spritesheet": "sheet_skin_default",
      "texture": "tex_skin_default",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    },
    "hair": {
      "id": "hair_short_brown",
      "name": "Short Brown Hair",
      "category": "hair",
      "spritesheet": "sheet_hair_short_brown",
      "texture": "tex_hair_short_brown",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    },
    "shirt": {
      "id": "shirt_forest_tunic",
      "name": "Forest Tunic",
      "category": "shirt",
      "spritesheet": "sheet_shirt_forest_tunic",
      "texture": "tex_shirt_forest_tunic",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    },
    "pants": {
      "id": "pants_traveler_trousers",
      "name": "Traveler Trousers",
      "category": "pants",
      "spritesheet": "sheet_pants_traveler_trousers",
      "texture": "tex_pants_traveler_trousers",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    },
    "shoes": {
      "id": "shoes_leather_boots",
      "name": "Leather Boots",
      "category": "shoes",
      "spritesheet": "sheet_shoes_leather_boots",
      "texture": "tex_shoes_leather_boots",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    },
    "back": {
      "id": "back_traveler_backpack",
      "name": "Traveler Backpack",
      "category": "back",
      "spritesheet": "sheet_back_traveler_backpack",
      "texture": "tex_back_traveler_backpack",
      "occlusion_flags": [],
      "tint": [255, 255, 255, 255],
      "anchor_offset": [0.0, 0.0],
      "visible": true
    }
  }
}
```

---

## 4. Representative Cosmetic Catalog

The Phase 7 asset suite provides 15 production-ready test cosmetics spanning all 8 visual categories:

| Category | Asset ID | Spritesheet ID | Key Attributes |
| :--- | :--- | :--- | :--- |
| **Skin** | `skin_default` | `sheet_skin_default` | Fair skin adventurer base |
| **Skin** | `skin_tan` | `sheet_skin_tan` | Warm golden-tan adventurer base |
| **Hair** | `hair_short_brown` | `sheet_hair_short_brown` | Cropped brown adventurer cut |
| **Hair** | `hair_ponytail_blonde` | `sheet_hair_ponytail_blonde` | High blonde ponytail with red ribbon |
| **Shirt** | `shirt_forest_tunic` | `sheet_shirt_forest_tunic` | Forest green linen tunic with leather belt |
| **Shirt** | `shirt_adventurer_vest` | `sheet_shirt_adventurer_vest` | Royal blue vest with cream undershirt |
| **Pants** | `pants_traveler_trousers` | `sheet_pants_traveler_trousers` | Full-length warm brown trousers |
| **Pants** | `pants_scout_shorts` | `sheet_pants_scout_shorts` | Slate gray scout knee shorts |
| **Shoes** | `shoes_leather_boots` | `sheet_shoes_leather_boots` | Dark brown riding leather boots |
| **Shoes** | `shoes_iron_greaves` | `sheet_shoes_iron_greaves` | Polished steel greaves with metallic shine |
| **Headwear**| `hat_ranger_hood` | `sheet_hat_ranger_hood` | Green hooded cowl |
| **Headwear**| `hat_iron_helm` | `sheet_hat_iron_helm` | Knight steel helmet with `hair_hide_all` flag |
| **Face** | `face_eyepatch` | `sheet_face_eyepatch` | Black leather rogue eyepatch |
| **Face** | `face_explorer_goggles`| `sheet_face_explorer_goggles` | Brass goggles with tinted cyan lenses |
| **Back** | `back_traveler_backpack`| `sheet_back_traveler_backpack`| Leather rucksack with rolled green bedroll |

---

## 5. Developer Wardrobe Suite (DevUI)

Integrated into the developer overlay ([`src/ui/DevUI.cpp`](file:///c:/dev/Wildbound/src/ui/DevUI.cpp)) under the **Wardrobe** tab:
1. **Quick Presets**: Instant loadouts ("Adventurer Default", "Armored Knight", "Forest Ranger", "Strip All").
2. **Direction Controls**: Down (South), Up (North), Left (West), Right (East) to preview layer depth changes and occlusion live.
3. **Dropdown Slot Selectors**: Interactive combos to equip or unequip any item per category.
4. **Occlusion Flags Inspector**: Real-time monitor displaying active status of `hair_hide_all` and `robe_hide_pants`.
5. **Runtime Render Layer Stack**: Live table displaying the active layers, their directional Z-index, and visibility.
6. **Persistence Buttons**: Save current wardrobe to `player_appearance.json` or reload from disk.

