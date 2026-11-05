# Wildbound Asset Naming & Directory Standards

## 1. Directory Structure Standard

To support the massive expansion into modular cosmetics, multiple biomes, settlement buildings, and wildlife without breaking any working Phase 0–5 asset paths, the `assets/` directory is standardized as follows:

```text
assets/
├── audio/
│   ├── ambient/                     # Long looping ambient background beds
│   ├── music/                       # Biome exploration & combat soundtracks
│   └── sfx/
│       ├── combat/                  # Weapon swings, impacts, blocks, parries
│       ├── environment/             # Waterfalls, doors, footsteps, thunder
│       ├── monsters/                # Goblin growls, skeleton clatters
│       ├── player/                  # Grunts, rolls, footfalls
│       └── ui/                      # Button clicks, loot pick chimes
├── characters/
│   ├── cosmetics/                   # Modular Player Customization
│   │   ├── back/                    # Capes, backpacks, quivers
│   │   ├── face/                    # Goggles, masks, eyepatches
│   │   ├── feet/                    # Boots, shoes, sandals
│   │   ├── hair/                    # Hair styles (split front/back)
│   │   ├── head/                    # Hats, helmets, hoods, circlets
│   │   ├── legs/                    # Pants, skirts, chausses
│   │   ├── torso/                   # Tunics, plate armor, robes
│   │   └── waist/                   # Belts, pouches, scabbards
│   ├── enemies/                     # Hostile Monsters & Creatures
│   │   ├── goblin/                  # Goblin archetypes (warrior, archer, shaman)
│   │   ├── skeleton/                # Skeleton archetypes (blade, guard, mage)
│   │   ├── wolf/                    # Dire wolves, timber wolves
│   │   └── treant/                  # Boss treants, corrupted saplings
│   ├── npcs/                        # Friendly Townspeople & Vendors
│   │   ├── blacksmith/
│   │   ├── merchant/
│   │   └── villager/
│   ├── player/                      # Base Humanoid Bodies
│   │   ├── base_female/
│   │   └── base_male/
│   └── wildlife/                    # Ambient Non-Combat Creatures
│       ├── birds/
│       ├── critters/
│       └── insects/
├── environment/
│   ├── biomes/                      # Biome-specific props and nature
│   │   ├── desert/
│   │   ├── forest/
│   │   ├── mountain/
│   │   ├── snow/
│   │   └── swamp/
│   ├── buildings/                   # Houses, towers, ruins, cabins
│   │   ├── exteriors/
│   │   ├── interiors/
│   │   └── roofs/
│   ├── props/                       # Reusable barrels, crates, chests, fences
│   └── vegetation/                  # Shared trees, bushes, flowers
├── items/                           # Ground Drops & Inventory
│   ├── consumables/                 # Potions, food, bandages
│   ├── materials/                   # Wood, ore, leather, herbs
│   └── quest/                       # Keys, relics, scrolls
├── manifests/
│   └── asset_manifest.json          # Authoritative asset registry
├── tilesets/                        # Terrain & Structure Tilesets
│   ├── desert/
│   ├── forest/
│   ├── mountain/
│   ├── snow/
│   └── swamp/
├── ui/                              # In-Game HUD, Icons, & Menus
│   ├── fonts/                       # Bitmap & TTF fonts
│   ├── hud/                         # Health orbs, hotbars, minimap frames
│   └── icons/                       # Item and skill icons
└── weapons/                         # In-Hand Sprites & Projectiles
    ├── arrows/
    ├── bows/
    ├── staves/
    └── swords/
```

---

## 2. File Naming Conventions

All assets in Wildbound adhere to **`snake_case` with explicit prefix and suffix identifiers**. No spaces, capital letters, or special characters are permitted.

### 2.1 File Prefix Rules

| Asset Category | Mandatory Prefix | Pattern Example | Description |
|---|---|---|---|
| **Raw Texture** | `tex_` | `tex_oak_tree.png` | Standalone static image texture |
| **Spritesheet Image** | `sheet_` | `sheet_player_base.png` | Multi-frame character or prop grid |
| **Spritesheet Meta** | `sheet_` | `sheet_player_base.json` | Grid dimensions, margins, and origins |
| **Animation Meta** | `anim_` | `anim_goblin_warrior.json` | Sequence of frames and durations |
| **Tileset Image** | `ts_` | `ts_forest_ground.png` | Orthographic grid tileset |
| **Map File** | `map_` | `map_forest_01.json` | Tiled map container |
| **Sound Effect** | `sfx_` | `sfx_sword_slash_01.wav` | Short audio cue (PCM 16-bit) |
| **Music Track** | `bgm_` | `bgm_forest_ambient.ogg` | Compressed looping audio stream |
| **UI Element** | `ui_` | `ui_health_bar_frame.png` | Canvas HUD asset |
| **Icon** | `icon_` | `icon_potion_health_01.png`| $16 \times 16$ or $32 \times 32$ item icon |

---

## 3. Standardized Identifier (ID) Conventions

Every asset and gameplay object is indexed by a globally unique string ID stored in `asset_manifest.json` and referenced in C++ and Lua.

### 3.1 ID Formatting Rules
- **Cosmetic ID**: `cos_<category>_<name>_<variant>`  
  *Examples*: `cos_hair_messy_brown_01`, `cos_torso_iron_cuirass_01`, `cos_head_wizard_hat_blue_01`.
- **Enemy Archetype ID**: `enemy_<species>_<role>_<variant>`  
  *Examples*: `enemy_goblin_club_01`, `enemy_skeleton_blade_01`, `enemy_wolf_alpha_01`.
- **Weapon Definition ID**: `wpn_<type>_<name>_<tier>`  
  *Examples*: `wpn_sword_iron_broadsword_t1`, `wpn_bow_oak_recurve_t2`.
- **Biome ID**: `biome_<name>`  
  *Examples*: `biome_verdant_forest`, `biome_frostfall_peaks`, `biome_murkwood_mire`.
- **Building ID**: `bld_<biome>_<type>_<size>`  
  *Examples*: `bld_forest_cabin_4x4`, `bld_snow_longhouse_8x6`.
- **Animation Clip ID**: `<character_or_item>_<action>_<direction>`  
  *Examples*: `player_walk_down`, `goblin_attack_left`, `chest_iron_open`.

---

## 4. Versioning & Deprecation Standards

- **Asset Checksums**: Every entry in `asset_manifest.json` carries a short hexadecimal hash (`checksum: "a1b2c3d4"`). When an asset is re-authored, its checksum updates to invalidate client-side texture caches.
- **Deprecation**: Deprecated assets are never deleted immediately. Their manifest status changes from `"approved"` to `"deprecated"`, and the engine logs a non-fatal warning when accessed, allowing artists to migrate references cleanly.

