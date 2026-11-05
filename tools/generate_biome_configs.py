#!/usr/bin/env python3
"""
WILDBOUND — BIOME CONFIGURATION GENERATOR (PHASE 14)
Creates structured JSON configurations in data/biomes/:
1. forest.json    (Verdant Forest)
2. desert.json    (Sunscorched Wastes)
3. snow.json      (Frostfall Taiga)
4. swamp.json     (Murkwood Mire)
5. mountain.json  (Iron Crag Highlands)
"""

import json
import os

BIOMES = [
    {
        "id": "forest",
        "display_name": "Verdant Forest",
        "description": "Lush temperate woodland characterized by ancient oaks, towering pines, flowing streams, and rustic settlements.",
        "tileset": "forest_tileset",
        "tileset_image": "tilesets/forest/forest_tileset.png",
        "terrain": [
            { "name": "Grass", "gid": 1, "walkable": True },
            { "name": "Packed Road", "gid": 9, "walkable": True },
            { "name": "Cliff Rock", "gid": 17, "walkable": False },
            { "name": "River Water", "gid": 25, "walkable": False },
            { "name": "Wood Bridge", "gid": 33, "walkable": True }
        ],
        "vegetation": [
            "tree_oak_001", "tree_oak_002", "tree_pine_001", "tree_pine_002",
            "bush_dense_001", "bush_flowering_001", "flower_wildflower_001"
        ],
        "environment_objects": [
            "house_cottage_001", "shop_general_001", "blacksmith_forge_001",
            "well_stone_001", "barrel_standard_001", "crate_large_001", "lamp_post_001"
        ],
        "animations": [
            "water_river_anim", "waterfall_large_anim", "tree_oak_breeze_anim",
            "tree_pine_breeze_anim", "grass_tall_wind_anim", "smoke_chimney_anim", "fire_medium_anim"
        ],
        "atmosphere": {
            "weather": "temperate",
            "fog_density": 0.25,
            "cloud_speed": 18.0,
            "cloud_layer_enabled": True,
            "ambient_tint": { "r": 255, "g": 255, "b": 255, "a": 255 }
        },
        "ambient_entities": ["ambient_bird", "forest_butterfly", "blue_butterfly", "golden_butterfly"],
        "maps": ["maps/forest/forest_01.json"],
        "default_spawn": { "x": 256.0, "y": 960.0 }
    },
    {
        "id": "desert",
        "display_name": "Sunscorched Wastes",
        "description": "An arid expanse of shifting dunes, cracked clay arroyos, sandstone ruins, and vital freshwater oases.",
        "tileset": "desert_tileset",
        "tileset_image": "tilesets/desert/desert_tileset.png",
        "terrain": [
            { "name": "Sand Dune", "gid": 1, "walkable": True },
            { "name": "Cracked Clay", "gid": 9, "walkable": True },
            { "name": "Caravan Trail", "gid": 17, "walkable": True },
            { "name": "Oasis Deep Water", "gid": 25, "walkable": False },
            { "name": "Sandstone Masonry", "gid": 33, "walkable": False },
            { "name": "Pillar Shaft", "gid": 41, "walkable": False },
            { "name": "Thorny Scrub", "gid": 49, "walkable": True }
        ],
        "vegetation": [
            "bush_small_001", "rock_boulder_001", "rock_flat_001"
        ],
        "environment_objects": [
            "ruin_pillar_stone_001", "ruin_arch_stone_001", "ruin_tower_small_001",
            "crate_reinforced_001", "barrel_banded_001", "fire_small_anim"
        ],
        "animations": [
            "water_ripple_anim", "smoke_plume_anim", "fire_small_anim"
        ],
        "atmosphere": {
            "weather": "arid_heat",
            "fog_density": 0.05,
            "cloud_speed": 12.0,
            "cloud_layer_enabled": True,
            "ambient_tint": { "r": 255, "g": 245, "b": 220, "a": 255 }
        },
        "ambient_entities": ["ambient_bird"],
        "maps": ["maps/desert/desert_01.json"],
        "default_spawn": { "x": 320.0, "y": 720.0 }
    },
    {
        "id": "snow",
        "display_name": "Frostfall Taiga",
        "description": "A sub-zero northern wilderness of deep powder drifts, frozen glacial lakes, snow-laden spruce, and warm hearth cabins.",
        "tileset": "snow_tileset",
        "tileset_image": "tilesets/snow/snow_tileset.png",
        "terrain": [
            { "name": "Fresh Powder Snow", "gid": 1, "walkable": True },
            { "name": "Packed Snow Path", "gid": 9, "walkable": True },
            { "name": "Glacial Ice Sheet", "gid": 17, "walkable": True },
            { "name": "Freezing Deep Water", "gid": 25, "walkable": False },
            { "name": "Frozen Granite Cliff", "gid": 33, "walkable": False },
            { "name": "Snowy Timber Wall", "gid": 41, "walkable": False },
            { "name": "Snowy Spruce", "gid": 49, "walkable": False },
            { "name": "Frozen Icicle Ledge", "gid": 57, "walkable": False }
        ],
        "vegetation": [
            "tree_pine_001", "tree_pine_002", "tree_pine_003", "bush_dense_001"
        ],
        "environment_objects": [
            "cabin_woodland_001", "storage_shed_001", "barrel_banded_001",
            "crate_large_001", "fire_medium_anim", "smoke_chimney_anim"
        ],
        "animations": [
            "waterfall_small_anim", "tree_pine_breeze_anim", "smoke_chimney_anim", "fire_medium_anim"
        ],
        "atmosphere": {
            "weather": "blizzard_chill",
            "fog_density": 0.40,
            "cloud_speed": 28.0,
            "cloud_layer_enabled": True,
            "ambient_tint": { "r": 230, "g": 240, "b": 255, "a": 255 }
        },
        "ambient_entities": ["ambient_bird"],
        "maps": ["maps/snow/snow_01.json"],
        "default_spawn": { "x": 320.0, "y": 720.0 }
    },
    {
        "id": "swamp",
        "display_name": "Murkwood Mire",
        "description": "A dark, decaying marshland filled with stagnant bogs, algae slicks, rickety duckboard walkways, and glowing fungi.",
        "tileset": "swamp_tileset",
        "tileset_image": "tilesets/swamp/swamp_tileset.png",
        "terrain": [
            { "name": "Mud Peat Bank", "gid": 1, "walkable": True },
            { "name": "Shallow Algae Water", "gid": 9, "walkable": True },
            { "name": "Deep Bog Void", "gid": 17, "walkable": False },
            { "name": "Wooden Duckboard Walkway", "gid": 25, "walkable": True },
            { "name": "Hollow Rotten Stump", "gid": 33, "walkable": False },
            { "name": "Luminescent Mushroom", "gid": 41, "walkable": True },
            { "name": "Cattail Reeds", "gid": 49, "walkable": True }
        ],
        "vegetation": [
            "tree_oak_001", "bush_dense_001", "mushroom_cluster_001", "mushroom_brown_001"
        ],
        "environment_objects": [
            "cabin_overgrown_001", "storage_shed_001", "log_fallen_mossy_001",
            "barrel_standard_001", "crate_small_001"
        ],
        "animations": [
            "water_pond_anim", "water_ripple_anim", "leaves_falling_anim", "smoke_campfire_anim"
        ],
        "atmosphere": {
            "weather": "fetid_mist",
            "fog_density": 0.65,
            "cloud_speed": 8.0,
            "cloud_layer_enabled": True,
            "ambient_tint": { "r": 220, "g": 240, "b": 215, "a": 255 }
        },
        "ambient_entities": ["forest_butterfly", "blue_butterfly"],
        "maps": ["maps/swamp/swamp_01.json"],
        "default_spawn": { "x": 320.0, "y": 720.0 }
    },
    {
        "id": "mountain",
        "display_name": "Iron Crag Highlands",
        "description": "Steep stepped granite terraces, windswept alpine heather ridges, roaring waterfalls, chasm paths, and dark cave entrances.",
        "tileset": "mountain_tileset",
        "tileset_image": "tilesets/mountain/mountain_tileset.png",
        "terrain": [
            { "name": "Solid Granite Terrace", "gid": 1, "walkable": True },
            { "name": "Heather Alpine Turf", "gid": 9, "walkable": True },
            { "name": "Scree Trail", "gid": 17, "walkable": True },
            { "name": "Basalt Cliff Face", "gid": 25, "walkable": False },
            { "name": "Iron Ore Vein", "gid": 33, "walkable": True },
            { "name": "Waterfall Cascade", "gid": 41, "walkable": False },
            { "name": "Cave Archway Portal", "gid": 49, "walkable": True }
        ],
        "vegetation": [
            "tree_pine_002", "tree_pine_003", "bush_flowering_001", "rock_boulder_001"
        ],
        "environment_objects": [
            "watchtower_stone_001", "ruin_tower_small_001", "bridge_wood_large_001",
            "sign_warning_001", "crate_reinforced_001"
        ],
        "animations": [
            "waterfall_large_anim", "waterfall_medium_anim", "tree_pine_breeze_anim"
        ],
        "atmosphere": {
            "weather": "highland_winds",
            "fog_density": 0.30,
            "cloud_speed": 35.0,
            "cloud_layer_enabled": True,
            "ambient_tint": { "r": 240, "g": 245, "b": 255, "a": 255 }
        },
        "ambient_entities": ["ambient_bird"],
        "maps": ["maps/mountain/mountain_01.json"],
        "default_spawn": { "x": 320.0, "y": 720.0 }
    }
]

def main():
    out_dir = "data/biomes"
    os.makedirs(out_dir, exist_ok=True)
    for b in BIOMES:
        fname = f"{b['id']}.json"
        fpath = os.path.join(out_dir, fname)
        with open(fpath, "w") as f:
            json.dump(b, f, indent=2)
        print(f"Generated biome configuration: {fpath} ({b['display_name']})")

if __name__ == "__main__":
    main()

