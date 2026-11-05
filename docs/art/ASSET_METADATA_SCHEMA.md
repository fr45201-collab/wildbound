# Wildbound Asset Metadata Schema Specification

## 1. Overview & JSON Schema Principles

Wildbound uses human-readable, data-driven JSON schemas parsed natively by `nlohmann::json`. To support future backend synchronization, multiplayer character loadouts, and automatic client asset validation, all metadata documents follow standardized schemas.

---

## 2. Schema Definitions & Concrete Examples

### 2.1 Modular Cosmetic Metadata Schema
Saved in `assets/characters/cosmetics/<category>/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/cosmetic_v1.json",
  "id": "cos_torso_forest_tunic_01",
  "version": "1.0.0",
  "name": "Forest Ranger Tunic",
  "category": "torso",
  "layer": "layer_torso",
  "rarity": "common",
  "spritesheet": "characters/cosmetics/torso/forest_tunic_sheet.png",
  "frame_width": 32,
  "frame_height": 48,
  "columns": 4,
  "rows": 8,
  "default_origin": "bottom_center",
  "color_palette_swaps": {
    "default": "palette_ranger_green",
    "variants": ["palette_autumn_brown", "palette_midnight_black"]
  },
  "conflicts": {
    "suppress_categories": [],
    "occlusion_flags": [],
    "required_body_type": "humanoid"
  },
  "compatible_directions": ["down", "up", "left", "right"],
  "compatible_animations": ["idle", "walk", "run", "attack", "hurt", "death"],
  "owned_by_default": true,
  "license": {
    "type": "CC0 1.0 Universal",
    "source": "Wildbound Development Studio",
    "author": "Wildbound Art Team",
    "commercial_use": true
  }
}
```

---

### 2.2 Character Base Metadata Schema
Saved in `data/characters/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/character_v1.json",
  "id": "player_base_male",
  "version": "1.0.0",
  "name": "Explorer",
  "body_type": "humanoid",
  "spritesheet": "player_spritesheet",
  "base_stats": {
    "walk_speed": 100.0,
    "run_speed": 165.0,
    "max_health": 100.0,
    "diagonal_normalization": true
  },
  "collision": {
    "enabled": true,
    "width": 16.0,
    "height": 12.0,
    "offset_x": 0.0,
    "offset_y": 0.0
  },
  "default_equipment": {
    "hair": "cos_hair_short_brown_01",
    "torso": "cos_torso_linen_shirt_01",
    "legs": "cos_legs_traveler_trousers_01",
    "feet": "cos_feet_leather_boots_01"
  }
}
```

---

### 2.3 Animation Clip Metadata Schema
Saved in `assets/animations/<id>.json` or bundled in character manifests:

```json
{
  "$schema": "https://wildbound.gg/schemas/animation_v1.json",
  "version": "1.0.0",
  "animations": [
    {
      "animation": "player_slash_down",
      "spritesheet": "player_spritesheet",
      "frames": [32, 33, 34, 35],
      "frame_duration": 0.09,
      "loop": false,
      "events": [
        { "frame": 1, "event_type": "spawn_hitbox", "param": "slash" },
        { "frame": 1, "event_type": "play_sfx", "param": "sfx_sword_slash_01" }
      ]
    }
  ]
}
```

---

### 2.4 Environmental Prop Metadata Schema
Saved in `assets/environment/props/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/prop_v1.json",
  "id": "prop_chest_iron_01",
  "name": "Ironbound Treasure Chest",
  "type": "container",
  "texture": "environment/props/chest_iron.png",
  "width": 32,
  "height": 24,
  "anchor": { "x": 16.0, "y": 24.0 },
  "collision": {
    "solid": true,
    "type": "solid",
    "width": 30.0,
    "height": 16.0
  },
  "interaction": {
    "enabled": true,
    "prompt": "Open Chest [E]",
    "interaction_radius": 28.0,
    "animates": true,
    "open_animation": "chest_iron_open"
  }
}
```

---

### 2.5 Building Structure Metadata Schema
Saved in `assets/environment/buildings/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/building_v1.json",
  "id": "bld_forest_cabin_01",
  "name": "Woodcutter Cabin",
  "biome": "biome_verdant_forest",
  "dimensions": { "tiles_w": 4, "tiles_h": 4, "pixels_w": 128, "pixels_h": 128 },
  "anchor": { "x": 64.0, "y": 128.0 },
  "textures": {
    "exterior": "environment/buildings/cabin_exterior.png",
    "roof": "environment/buildings/cabin_roof.png",
    "interior_floor": "environment/buildings/cabin_interior.png"
  },
  "collision_perimeter": [
    { "x": -60.0, "y": -48.0, "w": 120.0, "h": 12.0 },
    { "x": -60.0, "y": -48.0, "w": 12.0, "h": 48.0 },
    { "x": 48.0, "y": -48.0, "w": 12.0, "h": 48.0 },
    { "x": -60.0, "y": -4.0, "w": 44.0, "h": 8.0 },
    { "x": 16.0, "y": -4.0, "w": 44.0, "h": 8.0 }
  ],
  "doorway": {
    "x": -4.0,
    "y": -4.0,
    "width": 20.0,
    "height": 8.0,
    "interior_trigger": { "x": -48.0, "y": -44.0, "w": 96.0, "h": 40.0 }
  }
}
```

---

### 2.6 Biome Definition Schema
Saved in `data/biomes/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/biome_v1.json",
  "id": "biome_verdant_forest",
  "name": "Verdant Forest",
  "tilesets": ["forest_tileset"],
  "ambient_palette": {
    "day_tint": [255, 255, 255, 255],
    "dusk_tint": [245, 190, 150, 255],
    "night_tint": [60, 70, 110, 255]
  },
  "atmospheric_weather": {
    "primary": "clear",
    "secondary": "light_rain",
    "particle_density": 0.4
  },
  "music_track": "bgm_forest_ambient"
}
```

---

### 2.7 Ambient Wildlife Metadata Schema
Saved in `data/wildlife/<id>.json`:

```json
{
  "$schema": "https://wildbound.gg/schemas/wildlife_v1.json",
  "id": "wildlife_forest_bird_01",
  "name": "Forest Thrush",
  "spritesheet": "characters/wildlife/birds/thrush.png",
  "frame_width": 16,
  "frame_height": 16,
  "anchor": { "x": 8.0, "y": 16.0 },
  "animations": {
    "perch": [0, 1, 2],
    "fly": [3, 4, 5, 6]
  },
  "ai": {
    "fear_radius": 75.0,
    "flight_speed": 140.0,
    "despawn_time": 8.0
  }
}
```

---

## 3. Data Integrity & Validation Rules

1. **Mandatory Type Verification**: Strings, floats, and integer arrays are validated during load time by `AssetValidator.cpp`.
2. **Missing Reference Graceful Degradation**:
   - If a referenced cosmetic spritesheet is missing, the engine logs a warning and falls back to rendering the default undergarment body mesh, preventing crash-to-desktop.
3. **Multiplayer Serialization Readiness**:
   - A player's entire cosmetic loadout serializes into a compact 16-byte network payload (eight 16-bit cosmetic integer IDs).

