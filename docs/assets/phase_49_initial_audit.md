# Wildbound — Phase 49: Initial Repository Asset Audit

**Project:** Wildbound  
**Phase:** 49 — Advanced Asset Acquisition, Processing & Rendering Integration  
**Stage:** GATE 1 — Feasibility & Repository Audit  
**Date:** November 2026  
**Status:** Complete (Zero Assets Modified / Zero Assets Deleted)

---

## 1. Executive Summary

This initial audit establishes an empirical baseline of Wildbound's visual, audio, and metadata assets, the Asset Management System (AMS) architecture, rendering pipeline, animation system, and validation toolchain.

The core rule of Phase 49 is strictly respected: **Never destroy, overwrite, replace, or rename existing assets.** All new acquisitions will coexist in separate namespaces (`assets/external/`, `assets/processed/`), preserving 100% backward compatibility with existing maps, characters, combat systems, and tests.

---

## 2. Existing Asset Inventory & Directory Structure

An automated scan of the `assets/` directory revealed **1,114 total asset files**:

```
assets/
├── audio/                   (133 .wav audio files: SFX, ambience, UI, combat, footsteps)
├── branding/                (Splash, logos, icons)
├── characters/              (Player, modular cosmetics, enemies, wildlife, NPCs)
│   ├── cosmetics/           (Modular layers: back, face, hair, hats, pants, shirts, shoes, skin)
│   ├── enemies/             (Bat, goblin, skeleton, wild_boar, wolf)
│   ├── npcs/                (Villagers, merchants)
│   ├── player/              (Base humanoid body, player spritesheet)
│   └── wildlife/            (Birds, butterflies)
├── config/                  (Engine, load test, network configurations)
├── cosmetics/               (Costumes, color variants, loadout presets)
├── costumes/                (Costume definition files)
├── data/                    (Item catalogs, cosmetics catalogs, match definitions)
├── environment/             (Trees, rocks, bushes, buildings, water, fire, smoke, props)
├── fonts/                   (Pixel fonts, UI typography)
├── items/                   (40 item definitions: consumables, materials, collectibles)
├── manifests/               (asset_manifest.json — 661 registered entries)
├── tilesets/                (Forest, desert, interior, mountain, snow, swamp)
├── ui/                      (Icons, panels, buttons, backgrounds)
└── weapons/                 (Swords, bows, tools, held/ground sprites, attack animations)
```

### 2.1 File Extension Breakdown
| File Extension | File Count | Primary Function |
|---|---|---|
| `.png` | 564 | Sprites, textures, tilesets, UI elements, icons |
| `.json` | 417 | Spritesheet slicing, animation clips, manifests, item definitions |
| `.wav` | 133 | Sound effects, ambient loops, footsteps, UI sounds |
| `.md` | 1 | Documentation |
| **Total Files** | **1,115** | |

### 2.2 Master Manifest Asset Registry (`assets/manifests/asset_manifest.json`)
The master manifest indexes **661 registered assets**:
- **Textures (`texture`)**: 373 registered entries
- **Spritesheets (`spritesheet`)**: 155 registered entries
- **Audio / SFX (`audio` / `sound`)**: 133 registered entries
- **Licensing Compliance**: All 661 existing entries carry `license: "CC0 1.0 Universal"`, `commercial_use: true`, and `status: "approved"`.

---

## 3. Existing Asset Categories vs. Missing / Underrepresented Categories

| Asset Category | Current Status | Current Inventory | Gap Analysis & Expansion Priority |
|---|---|---|---|
| **Humanoid Player Base** | Established | 1 base body (32×48 px), 4-directional | Good foundation; needs female/alternative body frames |
| **Modular Clothing & Armor** | Moderate | Shirts, pants, shoes, vests, tunics | Needs heavy plate armor, robes, stealth cloaks, leather armor |
| **Hairstyles & Headwear** | Moderate | Short brown, ponytail blonde, basic hats | Needs curly hair, braids, helmets (iron, knight, wizard hats) |
| **Enemies & Monsters** | Limited | 5 enemies (Goblin, Skeleton, Bat, Boar, Wolf) | **HIGH PRIORITY**: Missing spiders, slimes, orcs, necromancers, bosses |
| **Wildlife & Ambient Fauna** | Minimal | Small birds, 3 butterfly variants | Needs deer, rabbits, squirrels, fish, frogs for living world |
| **Biome Tilesets: Forest** | Established | Full 32×32 autotile forest tileset | Well detailed; needs ruin walls, overgrown paths |
| **Biome Tilesets: Desert** | Minimal | Basic sand/dune tile (2.5 KB) | **HIGH PRIORITY**: Needs sandstone temples, cacti, oasis, canyon cliffs |
| **Biome Tilesets: Snow** | Minimal | Basic snow tile (2.4 KB) | **HIGH PRIORITY**: Needs frozen ice, pine snow caps, igloos, ice caves |
| **Biome Tilesets: Swamp** | Minimal | Basic murky water/mud tile (2.4 KB) | **HIGH PRIORITY**: Needs deadwood trees, lily pads, moss, stilt huts |
| **Biome Tilesets: Mountain**| Minimal | Basic rock cliff tile (2.1 KB) | **HIGH PRIORITY**: Needs elevation stairways, mine entrances, bridges |
| **Buildings & Interiors** | Moderate | Cabin, cottage, blacksmith, store, warehouse | Needs castle ruins, wizard towers, underground dungeons |
| **Weapons** | Moderate | Basic iron sword, wooden bow, tool | Needs daggers, greatswords, crossbows, magical staves |
| **Items & Resources** | Moderate | 40 catalogued item icons (potions, herbs, ores) | Needs loot chests, crates, quest items, artifacts |
| **Environmental Animations**| Moderate | Water, waterfall, fire, chimney smoke | Needs windblown grass, drifting fog sheets, torch flames |
| **UI Elements & HUD** | Basic | Basic health/mana bars, dev UI, inventory grid | Needs polished fantasy frames, minimap chrome, weapon slots |

---

## 4. Asset Management System (AMS) Architecture

The AMS (`src/resources/`) is organized into dedicated, single-responsibility components:

```
                          +-------------------+
                          |   AssetManager    |
                          +-------------------+
                             |      |      |
           +-----------------+      |      +-----------------+
           v                        v                        v
  +-----------------+      +-----------------+      +-----------------+
  |  AssetRegistry  |      | TextureManager  |      |  AudioManager   |
  | - JSON Manifest |      | - Raylib Cache  |      | - Wave / Sound  |
  | - Metadata Map  |      | - 16x16 Checkered|     | - Music Stream  |
  | - Asset ID Map  |      |   Fallback Tex  |      +-----------------+
  +-----------------+      +-----------------+
           |                        |
           v                        v
  +-----------------+      +-----------------+
  | AssetValidator  |      |   FontManager   |
  | - Path Resolve  |      | - TTF Font Cache|
  | - Ext Validation|      +-----------------+
  | - License Check |
  +-----------------+
```

### Key Technical Mechanisms:
1. **Stable ID Resolution**: Assets are indexed by string ID (`tex_player`, `sheet_player`, `iron_sword`).
2. **Headless Safety**: `TextureManager` detects when an OpenGL context is absent (`!IsWindowReady()`) and provides mock texture handles (`id = 1`), enabling fully headless CI/CD testing and dedicated server execution.
3. **Missing Asset Resilience**: If a texture fails to resolve on disk, `TextureManager` assigns a built-in 16×16 magenta/black checkerboard fallback texture (`GenImageChecked(16, 16, 8, 8, MAGENTA, BLACK)`), preventing null-pointer dereferencing or crashes.
4. **Spritesheet Slicing**: `SpriteSheet` stores frame dimensions (e.g. 32×48), columns, rows, spacing, and anchor origins (`Vector2 origin`).
5. **Animation Clips**: `AnimationClip` maps logical clips (`idle_down`, `walk_right`, `attack_up`) to frame index lists with configurable durations and loop flags.

---

## 5. Animation & Rendering Capabilities

### 5.1 AnimationController (`src/graphics/animation/AnimationController.cpp`)
- Fixed and variable delta-time animation stepping.
- Directional state machines (`Direction::Down`, `Left`, `Right`, `Up`).
- Frame rect querying via `GetCurrentFrameRect(assetMgr)` and origin offset retrieval.
- Seamless animation blending and loop modulo mechanics.

### 5.2 Modular Character Renderer (`src/entities/ModularCharacter.cpp`)
- Composites up to 10 visual layers in strict Z-index order:
  - 10: Back / Backpack (behind character if facing Down)
  - 20: Base Skin / Body
  - 30: Feet / Shoes
  - 40: Legs / Pants
  - 50: Torso / Shirt
  - 52: Armor / Cuirass
  - 55: Waist / Belts
  - 60: Hair
  - 62: Face details (warpaint, eyepatch)
  - 68: Mask (lower face)
  - 70: Eyewear / Goggles
  - 80: Hat / Helmet
  - 90: Weapon / Tool (drawn in front or behind depending on facing)
- Occlusion flag evaluation (e.g. `mask_hide_goggles`, `helmet_hide_hair`) automatically suppresses conflicting layers.

### 5.3 WorldRenderer (`src/world/WorldRenderer.cpp`)
- Strictly enforces 32×32 pixel grid alignment.
- Frustum culling prevents off-screen tile rendering.
- Depth Y-sorting merges static world objects and dynamic entities in a single sorted draw list.
- Tiled JSON map integration parses multi-layer tilemaps, object layers, and collision bounding boxes.

---

## 6. Existing Asset Validation Tools

1. **`AssetValidator.cpp`**: C++ runtime validator testing file presence, format extension compatibility, and license review flags.
2. **`test_assets.cpp`**: CTest suite verifying manifest loading, missing asset detection, duplicate detection, and category queries.
3. **`tools/storm_asset_generation/validate_storm_assets.py`**: Python script verifying PNG dimensions, alpha channel presence, and JSON schema conformity.
4. **`tools/audio_generation/validate_audio.py`**: Python script verifying WAV header conformity, sample rate (44.1 kHz), and channel count.

---

## 7. Limitations & Integration Risks

1. **Pixel Density Consistency ("Zero-Mixels Rule")**:
   - *Risk*: External assets often use varying tile grids (16×16, 24×24, 48×48) or differing pixel scales.
   - *Mitigation*: Assets must be normalized to Wildbound's 32×32 tile baseline or 32×48 humanoid baseline without blurring or bilinear scaling. Nearest-neighbor integer scaling only.
2. **Perspective Consistency**:
   - *Risk*: Mixing top-down orthographic (straight down) with 3/4 oblique (isometric/tilted) creates visual dissonance.
   - *Mitigation*: Only select assets matching the 3/4 top-down oblique view (front and top surfaces visible).
3. **Licensing Contamination**:
   - *Risk*: Viral copyleft licenses (GPL, CC BY-SA) or non-commercial restrictions (CC BY-NC) can compromise the project.
   - *Mitigation*: Restrict pilot and initial expansion to verified **CC0 1.0 Universal** and **CC BY 4.0/3.0** (with attribution). Reject CC BY-NC and unknown licenses.
4. **Tiled Map Compatibility**:
   - *Risk*: Altering existing tileset images breaks tile GIDs across existing maps (`battle_royale_island.json`, `forest_01.json`).
   - *Mitigation*: Never modify existing tileset files. New biomes must be separate tileset files registered with new firstgid ranges in separate test maps.

---

## 8. Conclusion & Gate 1 Status

The repository audit is complete. Existing assets are fully inventoried, licensed, and protected. The AMS architecture is robust, modular, and ready for non-destructive expansion.

