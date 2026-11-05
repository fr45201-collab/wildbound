#!/usr/bin/env python3
"""
WILDBOUND — PHASE 30: BATTLE ROYALE ISLAND MAP & METADATA GENERATOR
Generates:
1. maps/world/battle_royale_island.json (320x320 tiles, 10240x10240 world px, multi-tileset Tiled JSON)
2. data/world/battle_royale_island.json (World metadata specification)
3. data/world/biome_regions.json (Geographical biome zones)
4. data/world/settlements.json (5 major villages with buildings and districts)
5. data/world/points_of_interest.json (14 strategic POIs)
6. data/world/spawn_regions.json (12 player spawn regions with coordinates)
7. data/world/loot_locations.json (45 loot containers and placement markers)
8. data/world/traversal_routes.json (Primary, secondary, and chokepoint routes)
9. data/world/world_landmarks.json (Major natural and constructed landmarks)
"""

import os
import json
import math
import random

MAP_W = 320
MAP_H = 320
TILE_SZ = 32
WORLD_W = MAP_W * TILE_SZ # 10240 px
WORLD_H = MAP_H * TILE_SZ # 10240 px

# Tileset firstgids (64 tiles per tileset)
GID_FOREST = 1     # 1..64
GID_DESERT = 65    # 65..128
GID_SNOW = 129     # 129..192
GID_SWAMP = 193    # 193..256
GID_MOUNTAIN = 257 # 257..320

def create_grid(val=0):
    return [val] * (MAP_W * MAP_H)

def set_tile(grid, c, r, val):
    if 0 <= c < MAP_W and 0 <= r < MAP_H:
        grid[r * MAP_W + c] = val

def get_tile(grid, c, r):
    if 0 <= c < MAP_W and 0 <= r < MAP_H:
        return grid[r * MAP_W + c]
    return 0

def fill_rect(grid, c1, r1, c2, r2, val):
    for r in range(max(0, r1), min(MAP_H, r2 + 1)):
        for c in range(max(0, c1), min(MAP_W, c2 + 1)):
            grid[r * MAP_W + c] = val

def fill_circle(grid, cx, cy, radius, val):
    r_sq = radius * radius
    for r in range(max(0, int(cy - radius)), min(MAP_H, int(cy + radius + 1))):
        for c in range(max(0, int(cx - radius)), min(MAP_W, int(cx + radius + 1))):
            dx = c - cx
            dy = r - cy
            if dx * dx + dy * dy <= r_sq:
                grid[r * MAP_W + c] = val

def draw_path(grid, c1, r1, c2, r2, width, val):
    dx = c2 - c1
    dy = r2 - r1
    dist = math.hypot(dx, dy)
    steps = max(1, int(dist * 2))
    for s in range(steps + 1):
        t = s / steps
        cx = c1 + dx * t
        cy = r1 + dy * t
        fill_circle(grid, cx, cy, width * 0.5, val)

def generate_world():
    print(f"Generating Battle Royale Island: {MAP_W}x{MAP_H} tiles ({WORLD_W}x{WORLD_H} world units)...")
    random.seed(1337) # Deterministic layout for repeatable testing

    os.makedirs("maps/world", exist_ok=True)
    os.makedirs("data/world", exist_ok=True)

    # Grids for tile layers
    deep_water = create_grid(0)
    shallow_water = create_grid(0)
    ground_base = create_grid(0)
    terrain_details = create_grid(0)
    paths_roads = create_grid(0)
    collision_layer = create_grid(0)

    # 1. COASTLINE & OCEAN BASE
    # Outside radius ~135 is deep ocean water. Inside is the island landmass.
    center_c = MAP_W // 2 # 160
    center_r = MAP_H // 2 # 160

    for r in range(MAP_H):
        for c in range(MAP_W):
            dx = (c - center_c) / 135.0
            dy = (r - center_r) / 135.0
            dist = math.sqrt(dx * dx + dy * dy)
            # Add organic coastal noise
            angle = math.atan2(dy, dx)
            noise = 0.08 * math.sin(5 * angle) + 0.04 * math.cos(9 * angle) + 0.02 * math.sin(17 * angle)
            effective_dist = dist + noise

            if effective_dist > 1.05:
                # Deep ocean abyss
                deep_water[r * MAP_W + c] = GID_FOREST + 24 # Deep water tile (GID 25)
                collision_layer[r * MAP_W + c] = 1 # Solid obstacle
            elif effective_dist > 0.95:
                # Coastal shallows & surf
                shallow_water[r * MAP_W + c] = GID_FOREST + 25 # Shallow shoreline water
            else:
                # Solid landmass: determine primary biome by coordinate zones
                # Mountain: North (c: 110..210, r: 25..120)
                # Snow: North-East (c: 200..290, r: 25..140)
                # Swamp: North-West (c: 30..120, r: 35..135)
                # Desert: South-East (c: 180..290, r: 160..280)
                # Forest: Center & South-West (remainder of landmass)

                is_mountain = (110 <= c <= 210 and 25 <= r <= 120 and (r - 25) * 1.5 + abs(c - 160) < 100)
                is_snow = (c >= 200 and r <= 145 and not is_mountain)
                is_swamp = (c <= 125 and r <= 140 and not is_mountain)
                is_desert = (c >= 170 and r >= 155)

                if is_mountain:
                    # Iron Crag Highlands
                    ground_base[r * MAP_W + c] = GID_MOUNTAIN + 0 # Solid Granite Terrace
                    if random.random() < 0.25:
                        terrain_details[r * MAP_W + c] = GID_MOUNTAIN + 8 # Heather Alpine Turf
                elif is_snow:
                    # Frostfall Taiga
                    ground_base[r * MAP_W + c] = GID_SNOW + 0 # Fresh Powder Snow
                    if random.random() < 0.20:
                        terrain_details[r * MAP_W + c] = GID_SNOW + 16 # Glacial Ice Sheet
                elif is_swamp:
                    # Murkwood Mire
                    ground_base[r * MAP_W + c] = GID_SWAMP + 0 # Mud Peat Bank
                    if random.random() < 0.22:
                        terrain_details[r * MAP_W + c] = GID_SWAMP + 8 # Shallow Algae Water
                elif is_desert:
                    # Sunscorched Wastes
                    ground_base[r * MAP_W + c] = GID_DESERT + 0 # Sand Dune
                    if random.random() < 0.25:
                        terrain_details[r * MAP_W + c] = GID_DESERT + 8 # Baked Cracked Clay
                else:
                    # Verdant Forest (Heartland)
                    ground_base[r * MAP_W + c] = GID_FOREST + 0 # Forest Grass
                    if random.random() < 0.18:
                        terrain_details[r * MAP_W + c] = GID_FOREST + 3 # Dark/Lush Grass

    # 2. INTERNAL WATERWAYS (RIVER & OASIS)
    # Great Forest River running from Mountain waterfalls down through the center into the South Ocean
    for t_step in range(40, 290):
        # S-curve river
        rc = int(160 + 35 * math.sin(t_step * 0.035))
        rr = t_step
        fill_circle(shallow_water, rc, rr, 4, GID_FOREST + 24) # River water
        fill_circle(collision_layer, rc, rr, 3, 1) # Blocked river water

    # Desert Oasis Pool at (c: 240, r: 210)
    fill_circle(shallow_water, 240, 210, 8, GID_DESERT + 24) # Oasis Deep Water
    fill_circle(collision_layer, 240, 210, 6, 1)

    # 3. HIGHWAY & ROAD NETWORK (PRIMARY & SECONDARY TRAVERSAL ROUTES)
    # Village Centers:
    # Greenwood Village: (130, 185) -> (4160, 5920 px)
    # Sunfall Outpost:   (240, 235) -> (7680, 7520 px)
    # Frostpeak Village: (245, 80)  -> (7840, 2560 px)
    # Highstone Village: (160, 65)  -> (5120, 2080 px)
    # Mirewood Settlement: (75, 85) -> (2400, 2720 px)
    # Ancient Fortress (Center): (160, 150) -> (5120, 4800 px)

    # Central Hub connections (Primary Highways)
    # Forest to Ancient Fortress
    draw_path(paths_roads, 130, 185, 160, 150, 4, GID_FOREST + 8) # Forest Road GID 9
    # Ancient Fortress to Highstone Village
    draw_path(paths_roads, 160, 150, 160, 65, 4, GID_MOUNTAIN + 16) # Scree Trail GID 273
    # Ancient Fortress to Sunfall Outpost (Crosses River via Great Stone Bridge)
    draw_path(paths_roads, 160, 150, 240, 235, 4, GID_DESERT + 16) # Caravan Trail GID 81
    # Ancient Fortress to Mirewood Settlement
    draw_path(paths_roads, 160, 150, 75, 85, 4, GID_SWAMP + 24) # Duckboard GID 217
    # Highstone Village to Frostpeak Village (Mountain Pass)
    draw_path(paths_roads, 160, 65, 245, 80, 4, GID_SNOW + 8) # Snow Path GID 137
    # Frostpeak Village to Sunfall Outpost (Eastern Coast Highway)
    draw_path(paths_roads, 245, 80, 240, 235, 3, GID_DESERT + 16)
    # Mirewood to Highstone Village (Western Scree Pass)
    draw_path(paths_roads, 75, 85, 160, 65, 3, GID_MOUNTAIN + 16)

    # Clear river collision at bridge locations!
    # Bridge 1: Great Stone Bridge between (160, 150) and (240, 235) where it intersects the river (approx c: 172, r: 165)
    for br in range(162, 170):
        for bc in range(168, 178):
            set_tile(collision_layer, bc, br, 0) # Clear collision
            set_tile(paths_roads, bc, br, GID_FOREST + 32) # Bridge Wood/Stone tile

    # Bridge 2: Mountain Chasm Bridge at (c: 160, r: 105)
    for br in range(103, 108):
        for bc in range(156, 165):
            set_tile(collision_layer, bc, br, 0)
            set_tile(paths_roads, bc, br, GID_MOUNTAIN + 16)

    # 4. ENVIRONMENTAL OBJECTS & BUILDINGS (OBJECT LAYER)
    objects = []
    obj_id = 1

    def add_obj(name, obj_type, x, y, w, h, col_w=None, col_h=None, col_off_x=0, col_off_y=0):
        nonlocal obj_id
        if col_w is None: col_w = w * 0.8
        if col_h is None: col_h = h * 0.4
        
        props = [
            {"name": "collision", "type": "bool", "value": True},
            {"name": "collision_width", "type": "float", "value": float(col_w)},
            {"name": "collision_height", "type": "float", "value": float(col_h)},
            {"name": "collision_offset_x", "type": "float", "value": float(col_off_x)},
            {"name": "collision_offset_y", "type": "float", "value": float(col_off_y)}
        ]
        
        o = {
            "id": obj_id,
            "name": name,
            "type": obj_type,
            "x": float(x),
            "y": float(y),
            "width": float(w),
            "height": float(h),
            "rotation": 0.0,
            "visible": True,
            "properties": props
        }
        objects.append(o)
        obj_id += 1
        return o

    # =========================================================================
    # SETTLEMENT 1: GREENWOOD VILLAGE (Forest, Center-West) (4160, 5920)
    # =========================================================================
    gv_x, gv_y = 4160.0, 5920.0
    add_obj("Greenwood_Town_Hall", "house_residence_large_001", gv_x, gv_y, 128, 112, 110, 48)
    add_obj("Greenwood_General_Store", "shop_general_001", gv_x - 160, gv_y + 40, 96, 80, 80, 36)
    add_obj("Greenwood_Blacksmith", "blacksmith_forge_001", gv_x - 180, gv_y + 160, 112, 96, 96, 40)
    add_obj("Greenwood_Warehouse", "storage_warehouse_001", gv_x + 160, gv_y + 40, 96, 80, 80, 36)
    add_obj("Greenwood_Cottage_1", "house_cottage_001", gv_x + 180, gv_y + 160, 96, 80, 80, 36)
    add_obj("Greenwood_TwoStory_1", "house_twostory_001", gv_x, gv_y + 260, 112, 112, 96, 44)
    add_obj("Greenwood_Watchtower", "watchtower_wood_001", gv_x - 260, gv_y - 40, 48, 112, 40, 32)
    add_obj("Greenwood_Central_Well", "well_stone_001", gv_x - 20, gv_y + 140, 32, 36, 28, 20)
    add_obj("Greenwood_Noticeboard", "sign_noticeboard_001", gv_x + 30, gv_y + 140, 32, 32, 16, 10)
    add_obj("Greenwood_Bench_1", "bench_wooden_001", gv_x - 60, gv_y + 140, 32, 24, 28, 14)
    add_obj("Greenwood_Bench_2", "bench_wooden_001", gv_x + 70, gv_y + 140, 32, 24, 28, 14)

    # =========================================================================
    # SETTLEMENT 2: SUNFALL OUTPOST (Desert, South-East) (7680, 7520)
    # =========================================================================
    sf_x, sf_y = 7680.0, 7520.0
    add_obj("Sunfall_Watchtower", "watchtower_stone_001", sf_x, sf_y, 48, 112, 40, 32)
    add_obj("Sunfall_Bazaar_Shop", "shop_market_001", sf_x + 100, gv_y + 40, 96, 80, 80, 36)
    add_obj("Sunfall_Residency_1", "house_stone_small_001", sf_x - 120, sf_y + 60, 80, 72, 68, 30)
    add_obj("Sunfall_Residency_2", "house_stone_small_001", sf_x + 120, sf_y + 80, 80, 72, 68, 30)
    add_obj("Sunfall_Storage", "storage_shed_001", sf_x - 140, sf_y - 40, 64, 56, 52, 24)
    add_obj("Sunfall_Water_Cistern", "well_stone_001", sf_x, sf_y + 120, 32, 36, 28, 20)
    add_obj("Sunfall_Noticeboard", "sign_noticeboard_001", sf_x + 40, sf_y + 120, 32, 32, 16, 10)

    # =========================================================================
    # SETTLEMENT 3: FROSTPEAK VILLAGE (Snow, North-East) (7840, 2560)
    # =========================================================================
    fp_x, fp_y = 7840.0, 2560.0
    add_obj("Frostpeak_Longhouse", "cabin_woodland_001", fp_x, fp_y, 80, 72, 68, 32)
    add_obj("Frostpeak_Lodge_1", "cabin_hunting_001", fp_x - 120, fp_y + 50, 80, 72, 68, 32)
    add_obj("Frostpeak_Lodge_2", "cabin_forest_001", fp_x + 120, fp_y + 50, 80, 72, 68, 32)
    add_obj("Frostpeak_Watchtower", "watchtower_wood_001", fp_x + 200, fp_y - 50, 48, 112, 40, 32)
    add_obj("Frostpeak_Storehouse", "storage_shed_001", fp_x - 120, fp_y - 60, 64, 56, 52, 24)
    add_obj("Frostpeak_Campfire", "smoke_campfire_anim", fp_x, fp_y + 100, 32, 32, 16, 16)
    add_obj("Frostpeak_Well", "well_wooden_001", fp_x + 60, fp_y + 100, 32, 36, 28, 20)

    # =========================================================================
    # SETTLEMENT 4: MIREWOOD SETTLEMENT (Swamp, North-West) (2400, 2720)
    # =========================================================================
    mw_x, mw_y = 2400.0, 2720.0
    add_obj("Mirewood_Elder_Hut", "cabin_forest_001", mw_x, mw_y, 80, 72, 68, 32)
    add_obj("Mirewood_Hut_1", "cabin_hunting_001", mw_x - 110, mw_y + 60, 80, 72, 68, 32)
    add_obj("Mirewood_Hut_2", "cabin_woodland_001", mw_x + 110, mw_y + 60, 80, 72, 68, 32)
    add_obj("Mirewood_Shed", "storage_shed_001", mw_x - 120, mw_y - 60, 64, 56, 52, 24)
    add_obj("Mirewood_Watchpost", "watchtower_wood_001", mw_x + 160, mw_y - 40, 48, 112, 40, 32)
    add_obj("Mirewood_Lantern_1", "lamp_post_001", mw_x - 40, mw_y + 100, 16, 48, 8, 8)
    add_obj("Mirewood_Lantern_2", "lamp_post_001", mw_x + 40, mw_y + 100, 16, 48, 8, 8)

    # =========================================================================
    # SETTLEMENT 5: HIGHSTONE VILLAGE (Mountain, North) (5120, 2080)
    # =========================================================================
    hs_x, hs_y = 5120.0, 2080.0
    add_obj("Highstone_Fortress_Keep", "house_residence_large_001", hs_x, hs_y, 128, 112, 110, 48)
    add_obj("Highstone_Cottage_1", "house_stone_small_001", hs_x - 140, hs_y + 40, 80, 72, 68, 30)
    add_obj("Highstone_Cottage_2", "house_stone_small_001", hs_x + 140, hs_y + 40, 80, 72, 68, 30)
    add_obj("Highstone_High_Watchtower", "watchtower_stone_001", hs_x + 220, hs_y - 60, 48, 112, 40, 32)
    add_obj("Highstone_Warehouse", "storage_warehouse_001", hs_x - 160, hs_y + 140, 96, 80, 80, 36)
    add_obj("Highstone_Bridge", "bridge_wood_large_001", hs_x, hs_y + 180, 64, 48, 48, 32)

    # =========================================================================
    # STRATEGIC POI STRUCTURES (Fortresses, Shrines, Camps, Ruins)
    # =========================================================================
    # 1. Ancient Fortress (Center of Island Crossroads) (5120, 4800)
    af_x, af_y = 5120.0, 4800.0
    add_obj("Ancient_Fortress_Tower_N", "watchtower_stone_001", af_x - 80, af_y - 80, 48, 112, 40, 32)
    add_obj("Ancient_Fortress_Tower_S", "watchtower_stone_001", af_x + 80, af_y - 80, 48, 112, 40, 32)
    add_obj("Ancient_Fortress_Hall", "ruin_building_abandoned_001", af_x, af_y + 20, 80, 64, 68, 32)
    add_obj("Ancient_Fortress_Wall_W", "ruin_wall_broken_001", af_x - 120, af_y + 20, 64, 40, 60, 20)
    add_obj("Ancient_Fortress_Wall_E", "ruin_wall_broken_001", af_x + 120, af_y + 20, 64, 40, 60, 20)

    # 2. Desert Buried Temple (7200, 6400)
    add_obj("Desert_Temple_Tower", "ruin_tower_small_001", 7200.0, 6400.0, 48, 80, 40, 32)
    add_obj("Desert_Temple_Wall", "ruin_wall_broken_001", 7260.0, 6420.0, 64, 40, 60, 20)

    # 3. Forgotten Forest Shrine (3200, 6800)
    add_obj("Forest_Shrine_Ruin", "ruin_building_abandoned_001", 3200.0, 6800.0, 80, 64, 68, 32)
    add_obj("Forest_Shrine_Well", "well_stone_001", 3240.0, 6880.0, 32, 36, 28, 20)

    # 4. Frostfall Expedition Camp (6800, 3400)
    add_obj("Frostfall_Camp_Cabin", "cabin_hunting_001", 6800.0, 3400.0, 80, 72, 68, 32)
    add_obj("Frostfall_Camp_Fire", "fire_medium_anim", 6860.0, 3440.0, 32, 32, 16, 16)

    # 5. Drowned Mire Ruins (3000, 3800)
    add_obj("Drowned_Ruin_Tower", "ruin_tower_small_001", 3000.0, 3800.0, 48, 80, 40, 32)
    add_obj("Drowned_Ruin_Wall", "ruin_wall_broken_001", 3060.0, 3820.0, 64, 40, 60, 20)

    # 6. Great River Stone Bridge (5500, 5280)
    add_obj("Great_River_Bridge_Stone", "bridge_stone_001", 5500.0, 5280.0, 64, 48, 48, 32)

    # =========================================================================
    # DENSE VEGETATION & PROPS SCATTERING (NATURAL BIOMES)
    # =========================================================================
    # Distribute trees, rocks, and bushes organically across biomes
    for _ in range(350):
        c = random.randint(35, 285)
        r = random.randint(35, 285)
        # Avoid roads, water, and village cores
        if get_tile(collision_layer, c, r) == 0 and get_tile(paths_roads, c, r) == 0:
            px = c * TILE_SZ + 16.0
            py = r * TILE_SZ + 24.0

            # Determine biome for nature placement
            is_mountain = (110 <= c <= 210 and 25 <= r <= 120)
            is_snow = (c >= 200 and r <= 145)
            is_swamp = (c <= 125 and r <= 140)
            is_desert = (c >= 170 and r >= 155)

            if is_desert:
                if random.random() < 0.6:
                    add_obj(f"Desert_Boulder_{obj_id}", "rock_boulder_001", px, py, 48, 40, 36, 18)
                else:
                    add_obj(f"Desert_Crate_{obj_id}", "crate_reinforced_001", px, py, 32, 32, 24, 16)
            elif is_snow:
                roll = random.random()
                if roll < 0.65:
                    add_obj(f"Snow_Pine_{obj_id}", "tree_pine_001", px, py, 48, 80, 20, 12, 0, 30)
                else:
                    add_obj(f"Snow_Rock_{obj_id}", "rock_medium_001", px, py, 32, 28, 24, 14)
            elif is_swamp:
                roll = random.random()
                if roll < 0.5:
                    add_obj(f"Swamp_Oak_{obj_id}", "tree_oak_001", px, py, 48, 64, 16, 10, 0, 24)
                elif roll < 0.8:
                    add_obj(f"Swamp_Log_{obj_id}", "log_trunk_001", px, py, 48, 24, 40, 12)
                else:
                    add_obj(f"Swamp_Bush_{obj_id}", "bush_dense_001", px, py, 32, 28, 20, 12)
            elif is_mountain:
                roll = random.random()
                if roll < 0.5:
                    add_obj(f"Mountain_Boulder_{obj_id}", "rock_boulder_001", px, py, 48, 40, 36, 18)
                elif roll < 0.8:
                    add_obj(f"Mountain_Pine_{obj_id}", "tree_pine_002", px, py, 48, 96, 22, 14, 0, 36)
                else:
                    add_obj(f"Mountain_FlatRock_{obj_id}", "rock_flat_001", px, py, 32, 24, 24, 12)
            else:
                # Forest
                roll = random.random()
                if roll < 0.45:
                    add_obj(f"Forest_Oak_{obj_id}", "tree_oak_002", px, py, 64, 80, 20, 12, 0, 28)
                elif roll < 0.75:
                    add_obj(f"Forest_Pine_{obj_id}", "tree_pine_001", px, py, 48, 80, 20, 12, 0, 30)
                elif roll < 0.90:
                    add_obj(f"Forest_Bush_{obj_id}", "bush_large_001", px, py, 48, 36, 32, 16)
                else:
                    add_obj(f"Forest_Log_{obj_id}", "log_medium_001", px, py, 32, 20, 24, 10)

    # 5. PLAYER SPAWN REGIONS (OBJECT LAYER)
    # 12 Geographically distributed spawn regions around the island perimeter & POI outskirts
    spawn_zones = [
        ("Spawn_Forest_North", 4160.0, 5200.0, "forest"),
        ("Spawn_Forest_South", 4160.0, 6800.0, "forest"),
        ("Spawn_Forest_West", 3200.0, 6000.0, "forest"),
        ("Spawn_Desert_North", 6800.0, 6800.0, "desert"),
        ("Spawn_Desert_East", 8400.0, 7520.0, "desert"),
        ("Spawn_Desert_Oasis", 7680.0, 6720.0, "desert"),
        ("Spawn_Snow_South", 7840.0, 3600.0, "snow"),
        ("Spawn_Snow_East", 8600.0, 2560.0, "snow"),
        ("Spawn_Snow_Glacier", 7200.0, 2200.0, "snow"),
        ("Spawn_Mountain_Pass", 5120.0, 3200.0, "mountain"),
        ("Spawn_Mountain_Peak", 4400.0, 2080.0, "mountain"),
        ("Spawn_Swamp_Perimeter", 2400.0, 3600.0, "swamp"),
    ]

    for s_name, sx, sy, s_biome in spawn_zones:
        # Create 4 specific spawn points around each zone center
        for i, (ox, oy) in enumerate([(-48, -48), (48, -48), (-48, 48), (48, 48)]):
            sp_x = sx + ox
            sp_y = sy + oy
            s_obj = {
                "id": obj_id,
                "name": f"{s_name}_{i+1}",
                "type": "spawn",
                "x": sp_x,
                "y": sp_y,
                "width": 32.0,
                "height": 32.0,
                "rotation": 0.0,
                "visible": True,
                "properties": [
                    {"name": "biome", "type": "string", "value": s_biome},
                    {"name": "region_id", "type": "string", "value": s_name}
                ]
            }
            objects.append(s_obj)
            obj_id += 1

    # 6. LOOT PLACEMENT MARKERS (OBJECT LAYER)
    # 45 Dedicated Loot Containers placed across villages, ruins, watchtowers, and camps
    loot_points = [
        # Greenwood Village (8 containers)
        ("loot_greenwood_hall", 4160.0, 5950.0, "chest_rare", "greenwood"),
        ("loot_greenwood_store_1", 4000.0, 5960.0, "crate_supplies", "greenwood"),
        ("loot_greenwood_store_2", 4010.0, 5980.0, "barrel_ammo", "greenwood"),
        ("loot_greenwood_smith_1", 3980.0, 6080.0, "chest_weapons", "greenwood"),
        ("loot_greenwood_smith_2", 3990.0, 6100.0, "crate_armor", "greenwood"),
        ("loot_greenwood_warehouse", 4320.0, 5960.0, "crate_supplies", "greenwood"),
        ("loot_greenwood_tower", 3900.0, 5880.0, "chest_weapons", "greenwood"),
        ("loot_greenwood_cottage", 4340.0, 6080.0, "crate_consumables", "greenwood"),

        # Sunfall Outpost & Oasis (8 containers)
        ("loot_sunfall_tower", 7680.0, 7550.0, "chest_weapons", "sunfall"),
        ("loot_sunfall_bazaar_1", 7780.0, 7560.0, "crate_supplies", "sunfall"),
        ("loot_sunfall_bazaar_2", 7790.0, 7580.0, "barrel_ammo", "sunfall"),
        ("loot_sunfall_residence", 7560.0, 7580.0, "crate_consumables", "sunfall"),
        ("loot_sunfall_shed", 7540.0, 7480.0, "crate_supplies", "sunfall"),
        ("loot_sunfall_oasis_1", 7680.0, 6720.0, "chest_rare", "oasis"),
        ("loot_sunfall_oasis_2", 7700.0, 6740.0, "barrel_ammo", "oasis"),
        ("loot_sunfall_temple", 7200.0, 6420.0, "chest_ancient", "temple"),

        # Frostpeak Village & Camp (8 containers)
        ("loot_frostpeak_hall", 7840.0, 2590.0, "chest_rare", "frostpeak"),
        ("loot_frostpeak_lodge_1", 7720.0, 2610.0, "crate_supplies", "frostpeak"),
        ("loot_frostpeak_lodge_2", 7960.0, 2610.0, "crate_armor", "frostpeak"),
        ("loot_frostpeak_tower", 8040.0, 2510.0, "chest_weapons", "frostpeak"),
        ("loot_frostpeak_shed", 7720.0, 2500.0, "barrel_ammo", "frostpeak"),
        ("loot_frostfall_camp_1", 6800.0, 3420.0, "crate_supplies", "frostfall_camp"),
        ("loot_frostfall_camp_2", 6820.0, 3440.0, "crate_consumables", "frostfall_camp"),
        ("loot_frostpeak_glacier", 7200.0, 2220.0, "chest_ancient", "glacier"),

        # Mirewood Settlement & Ruins (8 containers)
        ("loot_mirewood_elder", 2400.0, 2750.0, "chest_rare", "mirewood"),
        ("loot_mirewood_hut_1", 2290.0, 2780.0, "crate_supplies", "mirewood"),
        ("loot_mirewood_hut_2", 2510.0, 2780.0, "crate_consumables", "mirewood"),
        ("loot_mirewood_shed", 2280.0, 2660.0, "barrel_ammo", "mirewood"),
        ("loot_mirewood_watchpost", 2560.0, 2680.0, "chest_weapons", "mirewood"),
        ("loot_drowned_ruin_1", 3000.0, 3820.0, "chest_ancient", "drowned_ruins"),
        ("loot_drowned_ruin_2", 3020.0, 3840.0, "crate_supplies", "drowned_ruins"),
        ("loot_swamp_crossing", 2400.0, 3620.0, "barrel_ammo", "swamp_crossing"),

        # Highstone Village & Fortress (8 containers)
        ("loot_highstone_keep", 5120.0, 2110.0, "chest_rare", "highstone"),
        ("loot_highstone_cottage", 4980.0, 2120.0, "crate_supplies", "highstone"),
        ("loot_highstone_tower", 5340.0, 2020.0, "chest_weapons", "highstone"),
        ("loot_highstone_warehouse", 4960.0, 2220.0, "crate_armor", "highstone"),
        ("loot_highstone_cave", 4400.0, 2100.0, "chest_ancient", "highstone_cave"),
        ("loot_mountain_pass", 5120.0, 3220.0, "crate_consumables", "mountain_pass"),
        ("loot_ancient_fortress_1", 5120.0, 4840.0, "chest_ancient", "ancient_fortress"),
        ("loot_ancient_fortress_2", 5040.0, 4760.0, "chest_weapons", "ancient_fortress"),

        # Forest Deep Woods (5 containers)
        ("loot_forest_shrine", 3200.0, 6820.0, "chest_ancient", "forest_shrine"),
        ("loot_hunter_camp_1", 4160.0, 6820.0, "crate_supplies", "hunter_camp"),
        ("loot_hunter_camp_2", 4180.0, 6840.0, "barrel_ammo", "hunter_camp"),
        ("loot_river_crossing", 5500.0, 5300.0, "crate_supplies", "river_crossing"),
        ("loot_south_cove", 5120.0, 8400.0, "crate_supplies", "south_cove"),
    ]

    for l_id, lx, ly, l_type, l_parent in loot_points:
        l_obj = {
            "id": obj_id,
            "name": l_id,
            "type": "loot_container",
            "x": lx,
            "y": ly,
            "width": 32.0,
            "height": 32.0,
            "rotation": 0.0,
            "visible": True,
            "properties": [
                {"name": "container_type", "type": "string", "value": l_type},
                {"name": "parent_poi", "type": "string", "value": l_parent},
                {"name": "collision", "type": "bool", "value": True},
                {"name": "collision_width", "type": "float", "value": 24.0},
                {"name": "collision_height", "type": "float", "value": 20.0}
            ]
        }
        objects.append(l_obj)
        obj_id += 1

    # 7. STRATEGIC POI MARKERS (OBJECT LAYER)
    pois = [
        ("poi_greenwood_village", "Greenwood Village", 4160.0, 5920.0, "settlement", "forest"),
        ("poi_sunfall_outpost", "Sunfall Outpost", 7680.0, 7520.0, "settlement", "desert"),
        ("poi_frostpeak_village", "Frostpeak Village", 7840.0, 2560.0, "settlement", "snow"),
        ("poi_mirewood_settlement", "Mirewood Settlement", 2400.0, 2720.0, "settlement", "swamp"),
        ("poi_highstone_village", "Highstone Village", 5120.0, 2080.0, "settlement", "mountain"),
        ("poi_ancient_fortress", "Ancient Fortress", 5120.0, 4800.0, "fortress", "forest"),
        ("poi_desert_temple", "Buried Temple of Sunfall", 7200.0, 6400.0, "temple", "desert"),
        ("poi_desert_oasis", "Sunfall Oasis & Bazaar", 7680.0, 6720.0, "oasis", "desert"),
        ("poi_forest_shrine", "Forgotten Forest Shrine", 3200.0, 6800.0, "shrine", "forest"),
        ("poi_frostfall_camp", "Frostfall Expedition Camp", 6800.0, 3400.0, "camp", "snow"),
        ("poi_frostpeak_glacier", "Frostpeak Glacial Cavern", 7200.0, 2200.0, "landmark", "snow"),
        ("poi_drowned_ruins", "Drowned Mire Ruins", 3000.0, 3800.0, "ruins", "swamp"),
        ("poi_highstone_mine", "Highstone Mine & Cave Portal", 4400.0, 2080.0, "mine", "mountain"),
        ("poi_river_crossing", "Great River Stone Bridge", 5500.0, 5280.0, "bridge", "forest"),
    ]

    for p_id, p_name, px, py, p_type, p_biome in pois:
        p_obj = {
            "id": obj_id,
            "name": p_name,
            "type": "poi",
            "x": px,
            "y": py,
            "width": 64.0,
            "height": 64.0,
            "rotation": 0.0,
            "visible": True,
            "properties": [
                {"name": "poi_id", "type": "string", "value": p_id},
                {"name": "poi_type", "type": "string", "value": p_type},
                {"name": "biome", "type": "string", "value": p_biome}
            ]
        }
        objects.append(p_obj)
        obj_id += 1

    # 8. ASSEMBLE TILED JSON MAP
    tilesets = [
        {
            "firstgid": GID_FOREST,
            "name": "forest_tileset",
            "tilewidth": 32,
            "tileheight": 32,
            "columns": 8,
            "tilecount": 64,
            "image": "tilesets/forest/forest_tileset.png",
            "imagewidth": 256,
            "imageheight": 256,
            "margin": 0,
            "spacing": 0
        },
        {
            "firstgid": GID_DESERT,
            "name": "desert_tileset",
            "tilewidth": 32,
            "tileheight": 32,
            "columns": 8,
            "tilecount": 64,
            "image": "tilesets/desert/desert_tileset.png",
            "imagewidth": 256,
            "imageheight": 256,
            "margin": 0,
            "spacing": 0
        },
        {
            "firstgid": GID_SNOW,
            "name": "snow_tileset",
            "tilewidth": 32,
            "tileheight": 32,
            "columns": 8,
            "tilecount": 64,
            "image": "tilesets/snow/snow_tileset.png",
            "imagewidth": 256,
            "imageheight": 256,
            "margin": 0,
            "spacing": 0
        },
        {
            "firstgid": GID_SWAMP,
            "name": "swamp_tileset",
            "tilewidth": 32,
            "tileheight": 32,
            "columns": 8,
            "tilecount": 64,
            "image": "tilesets/swamp/swamp_tileset.png",
            "imagewidth": 256,
            "imageheight": 256,
            "margin": 0,
            "spacing": 0
        },
        {
            "firstgid": GID_MOUNTAIN,
            "name": "mountain_tileset",
            "tilewidth": 32,
            "tileheight": 32,
            "columns": 8,
            "tilecount": 64,
            "image": "tilesets/mountain/mountain_tileset.png",
            "imagewidth": 256,
            "imageheight": 256,
            "margin": 0,
            "spacing": 0
        }
    ]

    layers = [
        {
            "id": 1,
            "name": "Deep Water",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "data": deep_water
        },
        {
            "id": 2,
            "name": "Shallow Water",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "data": shallow_water
        },
        {
            "id": 3,
            "name": "Ground Base",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "data": ground_base
        },
        {
            "id": 4,
            "name": "Terrain Details",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "data": terrain_details
        },
        {
            "id": 5,
            "name": "Paths and Roads",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "data": paths_roads
        },
        {
            "id": 6,
            "name": "Environmental Objects",
            "type": "objectgroup",
            "visible": True,
            "opacity": 1.0,
            "x": 0,
            "y": 0,
            "objects": objects
        },
        {
            "id": 7,
            "name": "Collision Layer",
            "type": "tilelayer",
            "width": MAP_W,
            "height": MAP_H,
            "visible": False,
            "opacity": 0.5,
            "x": 0,
            "y": 0,
            "data": collision_layer
        }
    ]

    tiled_map = {
        "version": "1.10",
        "tiledversion": "1.10.2",
        "orientation": "orthogonal",
        "renderorder": "right-down",
        "width": MAP_W,
        "height": MAP_H,
        "tilewidth": TILE_SZ,
        "tileheight": TILE_SZ,
        "infinite": False,
        "tilesets": tilesets,
        "layers": layers,
        "properties": [
            {"name": "world_id", "type": "string", "value": "battle_royale_island"},
            {"name": "world_name", "type": "string", "value": "Isle of Wildbound"},
            {"name": "author", "type": "string", "value": "Wildbound World Architecture Team"}
        ]
    }

    map_out_path = "maps/world/battle_royale_island.json"
    with open(map_out_path, "w", encoding="utf-8") as f:
        json.dump(tiled_map, f)
    print(f"Wrote Tiled map: {map_out_path} ({len(objects)} objects, 5 tilesets, 7 layers).")

    # =========================================================================
    # GENERATE COMPANION WORLD METADATA FILES
    # =========================================================================
    # 1. World Definition
    world_def = {
        "world_id": "battle_royale_island",
        "name": "Isle of Wildbound",
        "description": "The primary Battle Royale island of Wildbound featuring 5 diverse biomes, 5 settlements, ancient ruins, and hazardous waterways.",
        "map_path": map_out_path,
        "dimensions_tiles": {"width": MAP_W, "height": MAP_H},
        "dimensions_world": {"width": WORLD_W, "height": WORLD_H},
        "tile_size": TILE_SZ,
        "playable_bounds": {
            "x": 30.0 * TILE_SZ,
            "y": 30.0 * TILE_SZ,
            "width": 260.0 * TILE_SZ,
            "height": 260.0 * TILE_SZ
        },
        "default_spawn": {"x": 4160.0, "y": 5920.0},
        "biomes": ["forest", "desert", "snow", "swamp", "mountain"],
        "settlements_count": len(settlements_data := [
            {"id": "greenwood_village", "name": "Greenwood Village", "biome": "forest", "center": [4160.0, 5920.0], "radius": 450.0},
            {"id": "sunfall_outpost", "name": "Sunfall Outpost", "biome": "desert", "center": [7680.0, 7520.0], "radius": 400.0},
            {"id": "frostpeak_village", "name": "Frostpeak Village", "biome": "snow", "center": [7840.0, 2560.0], "radius": 400.0},
            {"id": "mirewood_settlement", "name": "Mirewood Settlement", "biome": "swamp", "center": [2400.0, 2720.0], "radius": 380.0},
            {"id": "highstone_village", "name": "Highstone Village", "biome": "mountain", "center": [5120.0, 2080.0], "radius": 420.0}
        ]),
        "poi_count": len(pois),
        "spawn_regions_count": len(spawn_zones),
        "loot_markers_count": len(loot_points)
    }
    with open("data/world/battle_royale_island.json", "w", encoding="utf-8") as f:
        json.dump(world_def, f, indent=2)

    # 2. Biome Regions
    biome_regions = {
        "regions": [
            {
                "biome_id": "forest",
                "name": "Verdant Forest",
                "bounds": {"x": 1920.0, "y": 3840.0, "width": 4480.0, "height": 4480.0},
                "tileset": "forest_tileset",
                "audio_profile": "amb_forest_day",
                "settlement_id": "greenwood_village"
            },
            {
                "biome_id": "desert",
                "name": "Sunscorched Wastes",
                "bounds": {"x": 5440.0, "y": 4960.0, "width": 3840.0, "height": 3840.0},
                "tileset": "desert_tileset",
                "audio_profile": "amb_desert_wind",
                "settlement_id": "sunfall_outpost"
            },
            {
                "biome_id": "snow",
                "name": "Frostfall Taiga",
                "bounds": {"x": 6400.0, "y": 800.0, "width": 3200.0, "height": 3840.0},
                "tileset": "snow_tileset",
                "audio_profile": "amb_snow_blizzard",
                "settlement_id": "frostpeak_village"
            },
            {
                "biome_id": "swamp",
                "name": "Murkwood Mire",
                "bounds": {"x": 960.0, "y": 1120.0, "width": 3040.0, "height": 3360.0},
                "tileset": "swamp_tileset",
                "audio_profile": "amb_swamp_murk",
                "settlement_id": "mirewood_settlement"
            },
            {
                "biome_id": "mountain",
                "name": "Iron Crag Highlands",
                "bounds": {"x": 3520.0, "y": 800.0, "width": 3200.0, "height": 3040.0},
                "tileset": "mountain_tileset",
                "audio_profile": "amb_mountain_peaks",
                "settlement_id": "highstone_village"
            }
        ]
    }
    with open("data/world/biome_regions.json", "w", encoding="utf-8") as f:
        json.dump(biome_regions, f, indent=2)

    # 3. Settlements
    with open("data/world/settlements.json", "w", encoding="utf-8") as f:
        json.dump({"settlements": settlements_data}, f, indent=2)

    # 4. Points of Interest
    poi_list = [
        {"id": p[0], "name": p[1], "position": [p[2], p[3]], "type": p[4], "biome": p[5]}
        for p in pois
    ]
    with open("data/world/points_of_interest.json", "w", encoding="utf-8") as f:
        json.dump({"points_of_interest": poi_list}, f, indent=2)

    # 5. Spawn Regions
    spawn_data = []
    for s_name, sx, sy, s_biome in spawn_zones:
        spawn_data.append({
            "region_id": s_name,
            "center": [sx, sy],
            "biome": s_biome,
            "spawn_points": [
                [sx - 48.0, sy - 48.0],
                [sx + 48.0, sy - 48.0],
                [sx - 48.0, sy + 48.0],
                [sx + 48.0, sy + 48.0]
            ]
        })
    with open("data/world/spawn_regions.json", "w", encoding="utf-8") as f:
        json.dump({"spawn_regions": spawn_data}, f, indent=2)

    # 6. Loot Locations
    loot_data = [
        {"id": l[0], "position": [l[1], l[2]], "container_type": l[3], "parent_poi": l[4]}
        for l in loot_points
    ]
    with open("data/world/loot_locations.json", "w", encoding="utf-8") as f:
        json.dump({"loot_locations": loot_data}, f, indent=2)

    # 7. Traversal Routes
    routes = {
        "primary_routes": [
            {"id": "route_greenwood_fortress", "name": "Forest King's Way", "start_poi": "poi_greenwood_village", "end_poi": "poi_ancient_fortress", "distance_units": 1470.0, "type": "highway"},
            {"id": "route_fortress_highstone", "name": "Highland Crest Road", "start_poi": "poi_ancient_fortress", "end_poi": "poi_highstone_village", "distance_units": 2720.0, "type": "highway"},
            {"id": "route_fortress_sunfall", "name": "Great Caravan Highway", "start_poi": "poi_ancient_fortress", "end_poi": "poi_sunfall_outpost", "distance_units": 3720.0, "type": "highway_bridge"},
            {"id": "route_fortress_mirewood", "name": "Mire Boardwalk Trail", "start_poi": "poi_ancient_fortress", "end_poi": "poi_mirewood_settlement", "distance_units": 3430.0, "type": "duckboard"},
            {"id": "route_highstone_frostpeak", "name": "Alpine Frost Pass", "start_poi": "poi_highstone_village", "end_poi": "poi_frostpeak_village", "distance_units": 2760.0, "type": "snow_pass"}
        ],
        "chokepoints": [
            {"id": "choke_river_bridge", "name": "Great River Stone Bridge", "position": [5500.0, 5280.0], "width_units": 64.0, "type": "river_crossing"},
            {"id": "choke_mountain_pass", "name": "Iron Crag Chasm Bridge", "position": [5120.0, 3360.0], "width_units": 64.0, "type": "chasm_crossing"},
            {"id": "choke_swamp_boardwalk", "name": "Murkwood Duckboard Pass", "position": [2400.0, 3600.0], "width_units": 48.0, "type": "bog_crossing"}
        ]
    }
    with open("data/world/traversal_routes.json", "w", encoding="utf-8") as f:
        json.dump(routes, f, indent=2)

    # 8. World Landmarks
    landmarks = {
        "landmarks": [
            {"id": "lm_ancient_fortress", "name": "The Citadel of Old Wildbound", "type": "monument", "biome": "forest", "position": [5120.0, 4800.0]},
            {"id": "lm_desert_temple", "name": "Buried Temple of the Sun God", "type": "ruins", "biome": "desert", "position": [7200.0, 6400.0]},
            {"id": "lm_frostpeak_glacier", "name": "Frostpeak Glacial Rift", "type": "natural", "biome": "snow", "position": [7200.0, 2200.0]},
            {"id": "lm_highstone_crag", "name": "Iron Crag Waterfall Falls", "type": "natural", "biome": "mountain", "position": [5120.0, 1600.0]},
            {"id": "lm_mire_drowned", "name": "The Sunken Reliquary", "type": "ruins", "biome": "swamp", "position": [3000.0, 3800.0]}
        ]
    }
    with open("data/world/world_landmarks.json", "w", encoding="utf-8") as f:
        json.dump(landmarks, f, indent=2)

    print("All world metadata JSON files successfully generated in data/world/!")

if __name__ == "__main__":
    generate_world()

