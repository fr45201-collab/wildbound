# Wildbound — Weapon Metadata Specification

## 1. Schema Overview

Every weapon in Wildbound is defined via a structured JSON configuration adhering to a strict schema. This metadata format unifies visual asset resolution, hand attachment socketing, directional depth sorting, and animation keyframe indexing.

---

## 2. Complete Weapon JSON Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "WildboundWeaponConfig",
  "type": "object",
  "required": [
    "id",
    "display_name",
    "category",
    "subcategory",
    "description",
    "icon_file",
    "ground_file",
    "held_file",
    "anim_file",
    "grip_anchor",
    "render_order",
    "hand",
    "directions",
    "animation_frames"
  ],
  "properties": {
    "id": {
      "type": "string",
      "description": "Unique stable identifier for the weapon (e.g. sword_iron_001)"
    },
    "display_name": {
      "type": "string",
      "description": "Localized, human-readable name of the weapon"
    },
    "category": {
      "type": "string",
      "enum": ["sword", "bow", "ammunition"]
    },
    "subcategory": {
      "type": "string",
      "description": "Specific weapon archetype (e.g. arming_sword, longsword, recurve_bow)"
    },
    "description": {
      "type": "string",
      "description": "Narrative visual description and lore"
    },
    "icon_file": {
      "type": "string",
      "description": "Relative file path to the 64x64 inventory icon PNG"
    },
    "ground_file": {
      "type": "string",
      "description": "Relative file path to the 32x32 ground representation PNG"
    },
    "held_file": {
      "type": "string",
      "description": "Relative file path to the 128x32 held 4-directional sprite strip PNG"
    },
    "anim_file": {
      "type": "string",
      "description": "Relative file path to the 128x32 4-frame action animation strip PNG"
    },
    "grip_anchor": {
      "type": "object",
      "required": ["x", "y"],
      "properties": {
        "x": { "type": "number", "minimum": 0, "maximum": 32 },
        "y": { "type": "number", "minimum": 0, "maximum": 32 }
      }
    },
    "render_order": {
      "type": "object",
      "required": ["down", "up", "left", "right"],
      "properties": {
        "down": { "type": "integer", "description": "Z-order when character faces South" },
        "up": { "type": "integer", "description": "Z-order when character faces North" },
        "left": { "type": "integer", "description": "Z-order when character faces West" },
        "right": { "type": "integer", "description": "Z-order when character faces East" }
      }
    },
    "hand": {
      "type": "string",
      "enum": ["right_hand", "left_hand", "both_hands"]
    },
    "directions": {
      "type": "array",
      "items": { "type": "string" },
      "minItems": 4
    },
    "animation_frames": {
      "type": "array",
      "items": { "type": "string" },
      "minItems": 4
    }
  }
}
```

---

## 3. Master Catalog Specification (`data/weapons/weapon_catalog.json`)

The master catalog provides a fast, centralized enumeration of all available weapons:

```json
{
  "version": "1.0",
  "weapon_count": 12,
  "categories": ["sword", "bow", "ammunition"],
  "weapons": [
    { ... weapon definitions ... }
  ]
}
```

---

## 4. Asset Management System (AMS) Integration Rules

Every weapon produces 6 distinct AMS-registered items:
1. `tex_<id>_icon`: 64×64 PNG texture in category `weapons/icons`.
2. `tex_<id>_ground`: 32×32 PNG texture in category `weapons/ground`.
3. `tex_<id>_held`: 128×32 PNG texture in category `weapons/held`.
4. `sheet_<id>_held`: 4-frame (32×32) spritesheet metadata in category `weapons/held`.
5. `tex_<id>_anim`: 128×32 PNG texture in category `weapons/animations`.
6. `sheet_<id>_anim`: 4-frame (32×32) spritesheet metadata in category `weapons/animations`.

All assets are validated via `WeaponManager::ValidateAssets(assetMgr, errors)`.

