# WILDBOUND — FOREST BIOME ASSET PLACEMENT SPECIFICATION

## 1. Overview

This document catalogs the spatial distribution and roles of environmental assets placed throughout the expanded **Forest Biome** ($160 \times 120$ tiles, $5120 \times 3840$ pixels).

All assets reference valid entries from the Asset Management System (`assets/manifests/asset_manifest.json`) created across Phases 2, 10, and 11.

---

## 2. Settlements Asset Placement

### 2.1 Greenwood Hamlet (Subregion 3: NE, Center $(4280, 750)$)
| Object Name | Asset Type | Dimensions | Collision AABB | Role |
|---|---|---|---|---|
| `Greenwood_Town_Hall` | `house_large_001` | $160 \times 112$ | $140 \times 48$ | Central civic building and northern settlement anchor. |
| `Greenwood_Cottage_A` | `house_small_001` | $96 \times 80$ | $80 \times 36$ | Western residential cottage facing south village road. |
| `Greenwood_Cottage_B` | `house_small_001` | $96 \times 80$ | $80 \times 36$ | Eastern residential cottage with garden fence. |
| `Greenwood_Barn` | `storage_barn_001` | $112 \times 96$ | $96 \times 40$ | Community grain and agricultural storage building. |
| `Greenwood_Well` | `well_stone_001` | $48 \times 48$ | $40 \times 24$ | Village center fresh water source and gathering landmark. |
| `Greenwood_Lamp_1/2` | `lamp_post_iron_001`| $24 \times 48$ | $12 \times 12$ | Street illumination along main paved square. |
| `Greenwood_Bench` | `bench_wood_001` | $32 \times 24$ | $24 \times 12$ | Outdoor resting spot beside town hall. |
| `Greenwood_Picnic` | `table_picnic_001` | $40 \times 32$ | $32 \times 16$ | Community gathering table. |
| `Greenwood_Chimney` | `smoke_chimney_anim`| $16 \times 32$ | None | Active chimney smoke puff emitter on Town Hall roof. |

### 2.2 Riverwood Outpost (Subregion 6: CE, Center $(3520, 1750)$)
| Object Name | Asset Type | Dimensions | Collision AABB | Role |
|---|---|---|---|---|
| `Riverwood_Shop` | `shop_general_001` | $96 \times 80$ | $80 \times 36$ | Riverside trading outpost and tavern structure. |
| `Riverwood_Hunter_Cabin` | `cabin_hunter_001` | $96 \times 80$ | $80 \times 36$ | Angler and hunter lodge facing river crossing. |
| `Riverwood_Shed` | `storage_shed_001` | $64 \times 64$ | $52 \times 28$ | Riverside storage shed for fishing tackle and boat gear. |
| `Riverwood_Sign` | `sign_direction_001` | $24 \times 32$ | $12 \times 10$ | Road navigation sign pointing toward Hamlet & Crossroads. |
| `Riverwood_Campfire` | `fire_medium_anim` | $24 \times 32$ | $16 \times 12$ | Warm gathering campfire on riverside terrace. |
| `Riverwood_Bridge_East` | `bridge_wood_001` | $64 \times 48$ | Traversable (None) | Wooden highway bridge crossing the Serpentine River. |

### 2.3 Highland Homestead (Subregion 1: NW, Center $(800, 750)$)
| Object Name | Asset Type | Dimensions | Collision AABB | Role |
|---|---|---|---|---|
| `Highland_Cabin` | `cabin_wood_001` | $80 \times 80$ | $68 \times 36$ | Sturdy timber homestead cabin built on western slope. |
| `Highland_Granary` | `storage_granary_001` | $80 \times 96$ | $68 \times 40$ | Elevated wood storage barn for harvested corn. |
| `Highland_Water_Trough` | `water_trough_001` | $32 \times 24$ | $24 \times 12$ | Animal watering trough in courtyard. |
| `Highland_Torch` | `fire_small_anim` | $16 \times 24$ | $10 \times 10$ | Wall torch illumination outside cabin doorway. |
| `Highland_Stone_Fence` | `fence_stone_001` | $32 \times 32$ | $32 \times 16$ | Dry stone perimeter wall defending homestead terrace. |

---

## 3. Abandoned Structures & Narrative Storytelling

| Structure | Location | Asset Types Placed | Narrative & Visual Context |
|---|---|---|---|
| **The Ruined Watchtower** | SE Ridge $(4380, 3180)$ | `ruin_tower_stone_001`, `ruin_wall_stone_001`, `smoke_plume_anim` | Ancient stone defense outpost destroyed by fire; smoldering dark smoke still drifts into sky. |
| **Overgrown Cabin** | Emerald Grove $(1920, 480)$ | `cabin_overgrown_001`, `log_fallen_mossy_001`, `stump_old_001` | Historic Wildbound prototype cabin claimed by forest vines and mossy tree trunks. |
| **The Sunken Shrine** | Silver Lake Islet $(832, 1792)$ | `ruin_arch_stone_001`, `ruin_pillar_stone_001`, `ambient_butterfly` | Submerged pre-war altar surrounded by azure butterflies and mist. |
| **Destroyed Camp & Bridge**| South River $(2950, 2980)$ | `fire_campfire_medium_001`, `crate_wood_001`, `log_fallen_small_001` | Hastily evacuated campsite beside broken wooden bridge planks. |
| **The Overgrown Hermitage**| Eastern Woods $(4680, 2080)$| `storage_shed_001`, `barrel_wood_001`, `log_fallen_mossy_001` | Secluded retreat of an eccentric alchemist deep in the old trees. |

---

## 4. Hidden Locations & Secrets

| Location | Coordinates | Concealment & Visual Clues | Reusability / Gameplay Role |
|---|---|---|---|
| **Secluded Waterfall Cove** | $(2400, 440)$ | Hidden behind granite cliff promontory and overhanging boughs; heavy roaring waterfall sound. | Secluded natural sanctuary; future high-tier loot cache. |
| **Concealed Meadow Clearing**| $(1350, 850)$ | Ringed by dense towering pine trees; accessible via winding game trail. | Peaceful grove populated by rare golden butterflies. |
| **Smuggler's Rock Cache** | $(480, 3300)$ | Hidden behind massive granite boulders and dense flowering bushes. | Covert cache with abandoned wooden smuggling crates. |
| **Ancient Woodland Altar** | $(3900, 2700)$ | Hidden in deep oak canopy shadow; encircled by moss-covered pillars. | Mystical woodland shrine for future buffs or rituals. |
| **Whispering Pine Lookout** | $(800, 300)$ | Natural cliff outcrop overlooking the northern forest canopy. | Tactical sniper/scout vantage point over northern approach. |

---

## 5. Environmental Animation Placement Strategy

| Environment Feature | World Coordinate | Animation Asset Key | Clip / Duration | Role |
|---|---|---|---|---|
| **Northern Cliff Falls** | $(2400, 440)$ | `sheet_waterfall_large_anim` | `waterfall_large_anim_loop` | Dramatic cliff torrent feeding Serpentine River. |
| **Cascade Fall** | $(2480, 460)$ | `sheet_waterfall_medium_anim` | `waterfall_medium_anim_loop` | Secondary cliff cascade. |
| **Silver Lake Surface** | $(850, 1800)$ | `sheet_water_lake_anim` | `water_lake_anim_loop` | Specular wave reflections across central lake waters. |
| **Silver Lake Ripples** | $(750, 1720)$ | `sheet_water_ripple_anim` | `water_ripple_anim_loop` | Water droplet and breeze shoreline rings. |
| **Riverwood Rapids** | $(3520, 1760)$ | `sheet_water_flow_anim` | `water_flow_anim_loop` | Churning white-water rapids rushing beneath bridge. |
| **South Basin Flow** | $(2800, 3000)$ | `sheet_water_river_anim` | `water_river_anim_loop` | Winding river currents flowing around southern bend. |
| **Lily Pond Surface** | $(4000, 480)$ | `sheet_water_ripple_anim` | `water_ripple_anim_loop` | Gentle water rings between pond lilies. |
| **Greenwood Hearth** | $(4350, 600)$ | `sheet_smoke_chimney_anim` | `smoke_chimney_anim_loop` | Hearth smoke rising above village hall. |
| **Riverwood Campfire** | $(3550, 1820)$ | `sheet_fire_medium_anim` | `fire_medium_anim_loop` | Active campfire welcoming travelers at outpost. |
| **Meadow Wind Grass** | $(800, 3000)$ | `sheet_grass_tall_wind_anim` | `grass_tall_wind_anim_loop` | Wind-responsive foliage sway across open fields. |
| **Emerald Grove Canopy** | $(2560, 1750)$ | `sheet_tree_oak_breeze_anim` | `tree_oak_breeze_anim_loop` | Breathing oak tree foliage swaying over crossroads. |

