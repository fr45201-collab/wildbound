#!/usr/bin/env python3
"""
WILDBOUND — PHASE 12: EXPANDED FOREST BIOME GENERATOR
Generates:
1. maps/forest/forest_01.json (Tiled JSON format, 160x120 tiles, 5120x3840 px)
2. maps/forest/forest_01.tmx  (Tiled TMX XML format, 160x120 tiles)
"""

import json
import math
import random
import os

MAP_W = 160
MAP_H = 120
TILE_SZ = 32
WORLD_W = MAP_W * TILE_SZ # 5120 px
WORLD_H = MAP_H * TILE_SZ # 3840 px

def create_layer_grid(default_val=0):
    return [default_val] * (MAP_W * MAP_H)

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

def draw_path_line(grid, c1, r1, c2, r2, width, val):
    dx = c2 - c1
    dy = r2 - r1
    dist = math.hypot(dx, dy)
    steps = max(1, int(dist * 2))
    for s in range(steps + 1):
        t = s / steps
        cx = c1 + dx * t
        cy = r1 + dy * t
        fill_circle(grid, cx, cy, width * 0.5, val)

def main():
    print(f"Generating Wildbound Phase 12 Forest Biome Map: {MAP_W}x{MAP_H} tiles ({WORLD_W}x{WORLD_H} px)...")
    random.seed(42) # Deterministic layout for reproducible game testing

    # 1. GROUND LAYER: Organic grass variations
    ground = create_layer_grid(1) # Base grass GID 1
    for r in range(MAP_H):
        for c in range(MAP_W):
            n1 = math.sin(c * 0.15) + math.cos(r * 0.18)
            n2 = math.sin(c * 0.35 + r * 0.25)
            roll = random.random()
            if n1 > 1.1:
                ground[r * MAP_W + c] = 4 if roll < 0.6 else 8 # Dark/lush forest grass
            elif n1 < -1.1:
                ground[r * MAP_W + c] = 5 if roll < 0.6 else 2 # Sunlit meadow grass
            elif n2 > 0.8:
                ground[r * MAP_W + c] = 2 if roll < 0.5 else 3 # Grass tufts
            else:
                ground[r * MAP_W + c] = 1 if roll < 0.8 else 3

    # 2. WATER LAYER & BODIES
    water = create_layer_grid(0)

    # 2.1 Silver Lake (Subregion 4, West): Big organic lake centered at C26, R56
    # Blob combination
    for (cx, cy, rad) in [
        (26, 56, 14), (20, 52, 10), (32, 60, 11),
        (22, 64, 9), (30, 50, 10), (16, 58, 8), (35, 55, 7)
    ]:
        fill_circle(water, cx, cy, rad, 25)

    # Lake island (Sunken Shrine Islet): cut out island at C26, R56
    for (cx, cy, rad) in [(26, 56, 3.5)]:
        fill_circle(water, cx, cy, rad, 0)

    # 2.2 Misty Pond (Subregion 2, North cliff basin): C76, R20
    for (cx, cy, rad) in [(76, 20, 7), (72, 19, 5), (80, 22, 5)]:
        fill_circle(water, cx, cy, rad, 25)

    # 2.3 Lily Pond (Subregion 3, NE near Greenwood): C124, R14
    for (cx, cy, rad) in [(124, 14, 5.5), (128, 15, 4)]:
        fill_circle(water, cx, cy, rad, 25)

    # 2.4 Serpentine River System
    # Originates at Misty Pond (C76, R24), flows south-east, then loops south-west
    river_nodes = [
        (76, 24), (79, 32), (84, 40), (92, 46), (102, 50),
        (110, 55), # Riverwood crossing bridge here!
        (116, 62), (122, 72), (120, 84), (112, 92),
        (100, 96), (92, 94), # South basin bridge here!
        (80, 92), (68, 88), (56, 82), (48, 76),
        (38, 70)  # Empties into Silver Lake!
    ]
    for i in range(len(river_nodes) - 1):
        c1, r1 = river_nodes[i]
        c2, r2 = river_nodes[i+1]
        w = 3.5 if i < 5 else (4.5 if i < 11 else 4.0)
        draw_path_line(water, c1, r1, c2, r2, w, 25)

    # Add water surface variation (GID 26 deep water and GID 31 shimmers)
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] == 25:
                # If surrounded by water, deep water GID 26
                neighbors = [get_tile(water, c+dc, r+dr) > 0 for dc, dr in [(-1,0),(1,0),(0,-1),(0,1)]]
                if all(neighbors):
                    water[r * MAP_W + c] = 26 if random.random() < 0.75 else 31
                else:
                    water[r * MAP_W + c] = 32 if random.random() < 0.4 else 25

    # 3. TERRAIN & ROADS LAYER
    terrain = create_layer_grid(0)

    # 3.1 Northern Ridge / Hill Cliff (R8 to R12, C55 to C105)
    for c in range(55, 105):
        # Stepped rock cliff face
        set_tile(terrain, c, 9, 17)
        set_tile(terrain, c, 10, 17)
        if random.random() < 0.6:
            set_tile(terrain, c, 8, 17)
            set_tile(terrain, c, 11, 17)

    # 3.2 South-West Rolling Hill Outcrop (R85 to R105, C10 to C30)
    for r in range(88, 102):
        for c in range(12, 28):
            if math.sin(c * 0.4) + math.cos(r * 0.35) > 0.8:
                set_tile(terrain, c, r, 17)

    # 3.3 Main Connected Road Network
    # Highway 1: Greenwood Hamlet (C130, R0) -> South-West to Crossroads (C80, R56)
    draw_path_line(terrain, 130, 0, 130, 20, 2.5, 9)
    draw_path_line(terrain, 130, 20, 105, 42, 2.5, 9)
    draw_path_line(terrain, 105, 42, 80, 56, 2.5, 9)

    # Greenwood Village Square Plaza & Internal Streets (C115..150, R18..38)
    # Central Square Cobblestone Paving
    fill_rect(terrain, 128, 23, 138, 29, 9)
    fill_rect(terrain, 130, 24, 136, 28, 17) # Cobblestone plaza center

    # West Street (Commercial District -> Blacksmith, General Store, Warehouse)
    draw_path_line(terrain, 128, 26, 116, 26, 2.2, 9)
    draw_path_line(terrain, 120, 26, 120, 31, 2.0, 9) # Branch to Blacksmith & Tool Shed
    draw_path_line(terrain, 126, 26, 126, 21, 2.0, 9) # Branch to Warehouse

    # North Avenue to Town Hall
    draw_path_line(terrain, 133, 23, 133, 19, 2.4, 9)

    # East Street (Residential District -> Cottages & Weaver)
    draw_path_line(terrain, 138, 26, 149, 26, 2.2, 9)
    draw_path_line(terrain, 142, 26, 142, 32, 2.0, 9) # Branch to Two-Story & Trapper

    # South Road to Watchtower & River
    draw_path_line(terrain, 133, 29, 136, 38, 2.2, 9)

    # Highway 2: Crossroads (C80, R56) -> South to South Border (C80, R119)
    draw_path_line(terrain, 80, 56, 80, 85, 2.5, 9)
    draw_path_line(terrain, 80, 85, 80, 119, 2.5, 9)

    # Highway 3 (East-West): Crossroads (C80, R56) -> East to Riverwood Crossing (C110, R55) -> East Border (C159, R55)
    draw_path_line(terrain, 80, 56, 110, 55, 2.5, 9)
    draw_path_line(terrain, 110, 55, 159, 55, 2.5, 9)

    # Highway 4 (West Branch): Crossroads (C80, R56) -> West past Cabin to Silver Lake Shore (C45, R56)
    draw_path_line(terrain, 80, 56, 60, 56, 2.5, 9)
    draw_path_line(terrain, 60, 56, 43, 56, 2.5, 9)

    # Secondary Path 1: To Highland Homestead (NW): from C80, R56 -> NW to C25, R24
    draw_path_line(terrain, 80, 56, 50, 40, 1.8, 10)
    draw_path_line(terrain, 50, 40, 25, 24, 1.8, 10)
    # Trail connecting West Spawn point (C8, R30) to Highland trail
    draw_path_line(terrain, 8, 30, 25, 24, 1.8, 10)

    # Secondary Path 2: To Ruined Watchtower (SE): from C110, R55 -> SE to C138, R100
    draw_path_line(terrain, 110, 55, 125, 75, 1.8, 10)
    draw_path_line(terrain, 125, 75, 138, 100, 1.8, 10)

    # Secondary Path 3: Trail to Waterfall Cove (North): from C80, R56 -> North to C76, R26
    draw_path_line(terrain, 80, 56, 76, 35, 1.6, 10)
    draw_path_line(terrain, 76, 35, 76, 25, 1.6, 10)

    # Secondary Path 4: Sunlit Meadow Trail (SW): from C80, R85 -> SW through meadows (C30, R100)
    draw_path_line(terrain, 80, 85, 55, 95, 1.8, 10)
    draw_path_line(terrain, 55, 95, 30, 100, 1.8, 10)

    # 3.4 Wooden Bridge Crossings over River!
    # Bridge 1: Riverwood Crossing at C110, R55 (tiles c: 108..112, r: 54..56)
    for c in range(108, 113):
        for r in range(54, 57):
            set_tile(terrain, c, r, 33) # Wooden bridge plank tile

    # Bridge 2: South Basin Crossing at C92, R94 (tiles c: 90..94, r: 93..95)
    for c in range(90, 95):
        for r in range(93, 96):
            set_tile(terrain, c, r, 33)

    # 4. GROUND DECORATIONS LAYER
    ground_decor = create_layer_grid(0)

    # Wildflowers in Sunlit Meadows (SW, C10-C50, R80-R115)
    for r in range(80, 115):
        for c in range(10, 50):
            if water[r * MAP_W + c] == 0 and terrain[r * MAP_W + c] == 0:
                if random.random() < 0.14:
                    ground_decor[r * MAP_W + c] = random.choice([43, 44, 45, 46]) # Flowers

    # Wildflowers near Lily Pond and Greenwood pastures
    for r in range(5, 35):
        for c in range(115, 155):
            if water[r * MAP_W + c] == 0 and terrain[r * MAP_W + c] == 0:
                if random.random() < 0.10:
                    ground_decor[r * MAP_W + c] = random.choice([43, 44, 45, 46])

    # Pebbles along roads
    for r in range(MAP_H):
        for c in range(MAP_W):
            if terrain[r * MAP_W + c] in [9, 10]:
                if random.random() < 0.08:
                    ground_decor[r * MAP_W + c] = random.choice([47, 48, 52]) # Pebbles / clods

    # Mushrooms in deep forest shade
    for r in range(5, 45):
        for c in range(5, 50): # NW woods
            if water[r * MAP_W + c] == 0 and terrain[r * MAP_W + c] == 0:
                if random.random() < 0.06:
                    ground_decor[r * MAP_W + c] = random.choice([49, 50, 54]) # Mushrooms

    # 5. COLLISION LAYER (Tile-based solid blocking)
    collision = create_layer_grid(0)

    # 5.1 Water collision: all water tiles are IMPASSABLE except wooden bridges (GID 33)
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] > 0:
                if terrain[r * MAP_W + c] == 33:
                    # Bridge: fully traversable! Zero collision
                    pass
                else:
                    collision[r * MAP_W + c] = 58 # Water collision (solid)

    # 5.2 Northern Cliff Wall: Impassable rocky slope (except path at C76)
    for c in range(55, 105):
        if c not in [75, 76, 77]: # Path through waterfall basin
            set_tile(collision, c, 9, 57)
            set_tile(collision, c, 10, 57)

    # 6. ENVIRONMENTAL OBJECTS (Object Group)
    objects = []
    obj_id_counter = 1

    def add_obj(name, obj_type, x, y, w, h, col_w=0, col_h=0):
        nonlocal obj_id_counter
        props = []
        if col_w > 0 and col_h > 0:
            props = [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": float(col_w) },
                { "name": "collision_height", "type": "float", "value": float(col_h) }
            ]
        obj = {
            "id": obj_id_counter,
            "name": name,
            "type": obj_type,
            "x": float(x),
            "y": float(y),
            "width": float(w),
            "height": float(h),
            "rotation": 0.0,
            "visible": True
        }
        if props:
            obj["properties"] = props
        objects.append(obj)
        obj_id_counter += 1
        return obj

    # 6.1 Player Spawn at Western Forest Trailhead (X: 256, Y: 960)
    add_obj("player_spawn", "spawn", 256.0, 960.0, 32, 32)

    # 6.2 Settlement 1: Greenwood Village & Commons (NE, X: 3650..4750, Y: 520..1300)
    # Total 13 Village Buildings (6 enterable interiors + 7 exterior-only)
    # A. Commercial & Crafting District (West side)
    add_obj("Greenwood_General_Store", "shop_general_001", 3820, 720, 96, 80, 80, 36)
    add_obj("Greenwood_Blacksmith", "blacksmith_forge_001", 3800, 920, 112, 96, 96, 40)
    add_obj("Greenwood_Warehouse", "storage_warehouse_001", 3980, 600, 128, 96, 110, 44)
    add_obj("Greenwood_Tool_Shed", "storage_shed_001", 4020, 960, 64, 64, 52, 28)
    add_obj("Greenwood_North_Watchtower", "watchtower_wood_001", 3660, 820, 64, 112, 52, 36)

    # B. Central Market Square
    add_obj("Greenwood_Market_Pavilion", "shop_market_001", 4180, 840, 96, 80, 80, 36)
    add_obj("Greenwood_Central_Well", "well_stone_001", 4280, 780, 48, 48, 40, 24)
    add_obj("Greenwood_Noticeboard", "sign_noticeboard_001", 4210, 810, 32, 32, 16, 10)
    add_obj("Greenwood_Road_Sign", "sign_direction_001", 4100, 800, 24, 32, 12, 10)
    add_obj("Greenwood_Shop_Sign", "sign_shop_001", 3870, 800, 24, 32, 12, 10)
    add_obj("Greenwood_Bench_Square", "bench_wooden_001", 4240, 800, 32, 24, 24, 12)
    add_obj("Greenwood_Street_Lamp_1", "lamp_post_001", 4140, 760, 24, 48, 12, 12)
    add_obj("Greenwood_Street_Lamp_2", "lamp_post_001", 4380, 760, 24, 48, 12, 12)
    add_obj("Greenwood_Street_Lamp_3", "lamp_post_001", 4240, 900, 24, 48, 12, 12)
    add_obj("Greenwood_Street_Lamp_4", "lamp_post_001", 3750, 820, 24, 48, 12, 12)
    add_obj("Greenwood_Market_Barrels", "barrel_standard_001", 4260, 860, 24, 24, 20, 14)
    add_obj("Greenwood_Market_Crates", "crate_large_001", 4276, 865, 32, 32, 26, 20)

    # C. Civic & Manor District (North)
    add_obj("Greenwood_Town_Hall", "house_residence_large_001", 4240, 560, 160, 112, 140, 48)
    add_obj("Greenwood_Bench_Hall", "bench_stone_001", 4360, 680, 32, 24, 24, 12)
    add_obj("Greenwood_Town_Hall_Smoke", "smoke_chimney_anim", 4350, 520, 16, 32)
    for fx in range(4180, 4440, 32):
        add_obj(f"Greenwood_Hall_Fence_{fx}", "fence_broken_001", fx, 690, 32, 32, 32, 14)

    # D. Residential District & Outskirts (East and South)
    add_obj("Greenwood_Cottage", "house_cottage_001", 4460, 720, 96, 80, 80, 36)
    add_obj("Greenwood_Twostory_Residence", "house_twostory_001", 4420, 940, 128, 96, 110, 44)
    add_obj("Greenwood_Mason_Cottage", "house_stone_small_001", 4620, 740, 80, 80, 68, 36)
    add_obj("Greenwood_Weaver_House", "house_village_001", 4640, 940, 96, 80, 80, 36)
    add_obj("Greenwood_Trapper_Cabin", "cabin_woodland_001", 4600, 1120, 80, 80, 68, 36)
    add_obj("Greenwood_South_Guard_Post", "watchtower_stone_001", 4320, 1140, 64, 128, 52, 40)

    # Crafting details
    add_obj("Greenwood_Blacksmith_Smoke", "smoke_chimney_anim", 3870, 870, 16, 32)
    add_obj("Greenwood_Blacksmith_Fire", "fire_medium_anim", 3760, 940, 24, 32, 16, 12)
    for fx in range(4400, 4720, 32):
        add_obj(f"Greenwood_Res_Fence_{fx}", "fence_straight_001", fx, 1060, 32, 32, 32, 14)

    # 6.3 Settlement 2: Riverwood Outpost (CE, centered around C112, R56 -> X: 3584, Y: 1792)
    add_obj("Riverwood_Shop", "shop_general_001", 3450, 1720, 96, 80, 80, 36)
    add_obj("Riverwood_Hunter_Cabin", "cabin_hunter_001", 3620, 1700, 96, 80, 80, 36)
    add_obj("Riverwood_Shed", "storage_shed_001", 3420, 1840, 64, 64, 52, 28)
    add_obj("Riverwood_Sign", "sign_direction_001", 3530, 1750, 24, 32, 12, 10)
    add_obj("Riverwood_Campfire", "fire_medium_anim", 3550, 1820, 24, 32, 16, 12)
    add_obj("Riverwood_Bridge_East", "bridge_wood_001", 3520, 1760, 64, 48, 0, 0)
    add_obj("Riverwood_Lamp", "lamp_post_iron_001", 3500, 1730, 24, 48, 12, 12)
    add_obj("Riverwood_Barrels_1", "barrel_wood_001", 3400, 1730, 24, 24, 20, 14)
    add_obj("Riverwood_Barrels_2", "barrel_wood_001", 3418, 1735, 24, 24, 20, 14)
    add_obj("Riverwood_Chimney_Smoke", "smoke_chimney_anim", 3490, 1670, 16, 32)

    # 6.4 Settlement 3: Highland Homestead (NW, X: 800, Y: 768)
    add_obj("Highland_Cabin", "cabin_wood_001", 760, 720, 80, 80, 68, 36)
    add_obj("Highland_Granary", "storage_granary_001", 900, 740, 80, 96, 68, 40)
    add_obj("Highland_Water_Trough", "water_trough_001", 860, 780, 32, 24, 24, 12)
    add_obj("Highland_Torch", "fire_small_anim", 750, 730, 16, 24, 10, 10)
    for fx in range(700, 940, 32):
        add_obj(f"Highland_Stone_Fence_{fx}", "fence_stone_001", fx, 840, 32, 32, 32, 16)

    # 6.5 Abandoned Structures (5 locations)
    # 1. Overgrown Cabin in Emerald Grove (Original prototype cabin preserved at 1920, 480!)
    add_obj("Abandoned_Overgrown_Cabin", "cabin_overgrown_001", 1920, 480, 80, 80, 70, 36)
    add_obj("Abandoned_Cabin_Stump", "stump_old_001", 1880, 520, 24, 24, 18, 14)
    add_obj("Abandoned_Cabin_Log", "log_fallen_mossy_001", 2010, 510, 48, 20, 40, 14)

    # 2. Ruined Stone Watchtower (SE, X: 4400, Y: 3200)
    add_obj("Ruined_Watchtower", "ruin_tower_stone_001", 4380, 3180, 80, 112, 70, 48)
    add_obj("Ruin_Wall_A", "ruin_wall_stone_001", 4300, 3240, 64, 48, 56, 24)
    add_obj("Ruin_Wall_B", "ruin_wall_stone_001", 4480, 3240, 64, 48, 56, 24)
    add_obj("Ruin_Smoke_Plume", "smoke_plume_anim", 4410, 3120, 32, 48)

    # 3. Sunken Shrine on Silver Lake Islet (C26, R56 -> X: 832, Y: 1792)
    add_obj("Sunken_Shrine_Arch", "ruin_arch_stone_001", 820, 1770, 64, 80, 48, 28)
    add_obj("Sunken_Shrine_Pillar_L", "ruin_pillar_stone_001", 790, 1780, 32, 64, 24, 20)
    add_obj("Sunken_Shrine_Pillar_R", "ruin_pillar_stone_001", 880, 1780, 32, 64, 24, 20)
    add_obj("Sunken_Shrine_Butterfly", "ambient_butterfly", 830, 1790, 16, 16)

    # 4. Destroyed Bridge & Smoldering Camp (South River Bend, X: 2950, Y: 3000)
    add_obj("Destroyed_Campfire", "fire_campfire_medium_001", 2950, 2980, 24, 32, 16, 12)
    add_obj("Destroyed_Camp_Crate_1", "crate_wood_001", 2920, 2960, 24, 24, 20, 14)
    add_obj("Destroyed_Camp_Crate_2", "crate_wood_001", 2936, 2965, 24, 24, 20, 14)
    add_obj("Destroyed_Camp_Log", "log_fallen_small_001", 2980, 3010, 32, 16, 28, 10)

    # 5. Overgrown Hermit's Shed (Deep Eastern Woods, X: 4700, Y: 2100)
    add_obj("Hermit_Shed", "storage_shed_001", 4680, 2080, 64, 64, 52, 28)
    add_obj("Hermit_Barrels", "barrel_wood_001", 4650, 2110, 24, 24, 20, 14)
    add_obj("Hermit_Log", "log_fallen_mossy_001", 4760, 2120, 48, 20, 40, 14)

    # 6.6 Hidden Locations (5 Secret Spots)
    # 1. Secluded Waterfall Cove (North Cliff, X: 2432, Y: 480)
    add_obj("Waterfall_Large_Main", "waterfall_large_anim", 2400, 440, 64, 80)
    add_obj("Waterfall_Medium_Side", "waterfall_medium_anim", 2480, 460, 48, 64)
    add_obj("Waterfall_Cove_Boulder", "rock_boulder_001", 2360, 520, 48, 40, 40, 24)
    add_obj("Waterfall_Cove_Mossy_Rock", "rock_mossy_001", 2520, 530, 32, 28, 26, 18)

    # 2. Concealed Forest Clearing (NW Pine Woods, X: 1350, Y: 850)
    add_obj("Secret_Clearing_Butterfly", "ambient_butterfly", 1350, 850, 16, 16)
    add_obj("Secret_Clearing_Bird", "ambient_bird", 1370, 830, 16, 16)

    # 3. Smuggler's Hidden Cache (SW Meadow Ridge, X: 480, Y: 3300)
    add_obj("Smuggler_Boulder_1", "rock_boulder_001", 460, 3280, 48, 40, 42, 24)
    add_obj("Smuggler_Boulder_2", "rock_mossy_001", 510, 3290, 32, 28, 28, 18)
    add_obj("Smuggler_Crate", "crate_wood_001", 490, 3310, 24, 24, 20, 14)
    add_obj("Smuggler_Bush", "bush_dense_001", 470, 3320, 40, 32, 34, 18)

    # 4. Ancient Woodland Altar (SE Woods, X: 3900, Y: 2700)
    add_obj("Ancient_Pillar_1", "ruin_pillar_stone_001", 3880, 2680, 32, 64, 24, 20)
    add_obj("Ancient_Pillar_2", "ruin_pillar_stone_001", 3940, 2680, 32, 64, 24, 20)
    add_obj("Ancient_Flat_Rock", "rock_flat_001", 3910, 2710, 32, 20, 0, 0)

    # 5. Whispering Pine Lookout (North-West peak, X: 800, Y: 300)
    add_obj("Lookout_Boulder", "rock_boulder_001", 780, 290, 48, 40, 42, 24)
    add_obj("Lookout_Pine", "tree_pine_003", 840, 270, 64, 96, 30, 16)

    # 6.7 Natural Tree Populations, Bushes & Rocks Distribution
    # Place natural clusters across dense forest subregions without overlapping roads or water
    def is_blocked_for_prop(px, py, radius=32):
        if 3600 <= px <= 4800 and 500 <= py <= 1300:
            return True # Protect Greenwood Village center from random clutter!
        tc = int(px // TILE_SZ)
        tr = int(py // TILE_SZ)
        for dr in range(-1, 2):
            for dc in range(-1, 2):
                c = tc + dc
                r = tr + dr
                if get_tile(water, c, r) > 0: return True
                if get_tile(terrain, c, r) in [9, 10, 33]: return True # Do not block roads/bridges!
        return False

    # A. Pine Trees in Whispering Pines (NW: X: 150..1800, Y: 150..1200)
    for _ in range(75):
        px = random.randint(150, 1800)
        py = random.randint(150, 1200)
        if not is_blocked_for_prop(px, py):
            ptype = random.choice(["tree_pine_001", "tree_pine_002", "tree_pine_003"])
            pw, ph = (32, 64) if "001" in ptype else ((48, 80) if "002" in ptype else (64, 96))
            add_obj(f"Pine_NW_{px}_{py}", ptype, px, py, pw, ph, pw * 0.45, 16)

    # B. Oak Trees in Emerald Grove & Central Woods (X: 1800..3500, Y: 200..1800)
    for _ in range(85):
        px = random.randint(1800, 3500)
        py = random.randint(200, 1800)
        if not is_blocked_for_prop(px, py):
            otype = random.choice(["tree_oak_001", "tree_oak_002", "tree_oak_003"])
            ow, oh = (48, 64) if "001" in otype else ((64, 80) if "002" in otype else (80, 96))
            add_obj(f"Oak_Center_{px}_{py}", otype, px, py, ow, oh, ow * 0.40, 18)

    # C. Mixed Woods in Southern & Eastern Woods (X: 1500..4900, Y: 2000..3600)
    for _ in range(95):
        px = random.randint(1500, 4900)
        py = random.randint(2000, 3600)
        if not is_blocked_for_prop(px, py):
            ttype = random.choice(["tree_oak_001", "tree_oak_002", "tree_pine_001", "tree_pine_002"])
            tw, th = (48, 64) if "oak_001" in ttype else (64, 80)
            add_obj(f"Tree_South_{px}_{py}", ttype, px, py, tw, th, tw * 0.40, 16)

    # D. Scattered trees in Sunlit Meadows (SW: X: 200..1500, Y: 2000..3600) - Sparse!
    for _ in range(25):
        px = random.randint(200, 1500)
        py = random.randint(2000, 3600)
        if not is_blocked_for_prop(px, py):
            add_obj(f"Meadow_Oak_{px}_{py}", "tree_oak_002", px, py, 64, 80, 28, 16)

    # E. Bushes throughout all regions
    for _ in range(60):
        px = random.randint(150, 4900)
        py = random.randint(150, 3600)
        if not is_blocked_for_prop(px, py):
            btype = random.choice(["bush_small_001", "bush_round_001", "bush_dense_001", "bush_flowering_001"])
            bw, bh = (24, 24) if "small" in btype else ((40, 32) if "dense" in btype else (32, 32))
            has_col = "small" not in btype
            add_obj(f"Bush_{px}_{py}", btype, px, py, bw, bh, bw * 0.7 if has_col else 0, bh * 0.4 if has_col else 0)

    # F. Boulders & Rocks
    for _ in range(45):
        px = random.randint(150, 4900)
        py = random.randint(150, 3600)
        if not is_blocked_for_prop(px, py):
            rtype = random.choice(["rock_small_001", "rock_medium_001", "rock_boulder_001", "rock_mossy_001"])
            rw, rh = (16, 16) if "small" in rtype else ((24, 20) if "medium" in rtype else (48, 40))
            has_col = "small" not in rtype
            add_obj(f"Rock_{px}_{py}", rtype, px, py, rw, rh, rw * 0.8 if has_col else 0, rh * 0.5 if has_col else 0)

    # G. Ambient Birds & Butterflies
    for i in range(12):
        bx = random.randint(300, 4800)
        by = random.randint(300, 3500)
        if i % 2 == 0:
            add_obj(f"Ambient_Bird_{i}", "ambient_bird", bx, by, 16, 16)
        else:
            add_obj(f"Ambient_Butterfly_{i}", "ambient_butterfly", bx, by, 16, 16)

    print(f"Total Environmental Objects placed: {len(objects)}")

    # 7. BUILD TILED JSON MAP
    json_map = {
        "width": MAP_W,
        "height": MAP_H,
        "tilewidth": TILE_SZ,
        "tileheight": TILE_SZ,
        "tilesets": [
            {
                "firstgid": 1,
                "name": "forest_tileset",
                "tilewidth": TILE_SZ,
                "tileheight": TILE_SZ,
                "columns": 8,
                "tilecount": 64,
                "image": "tilesets/forest/forest_tileset.png"
            }
        ],
        "layers": [
            {
                "id": 1,
                "name": "Ground",
                "type": "tilelayer",
                "width": MAP_W,
                "height": MAP_H,
                "visible": True,
                "opacity": 1.0,
                "data": ground
            },
            {
                "id": 2,
                "name": "Terrain",
                "type": "tilelayer",
                "width": MAP_W,
                "height": MAP_H,
                "visible": True,
                "opacity": 1.0,
                "data": terrain
            },
            {
                "id": 3,
                "name": "Water",
                "type": "tilelayer",
                "width": MAP_W,
                "height": MAP_H,
                "visible": True,
                "opacity": 1.0,
                "data": water
            },
            {
                "id": 4,
                "name": "Ground Decorations",
                "type": "tilelayer",
                "width": MAP_W,
                "height": MAP_H,
                "visible": True,
                "opacity": 1.0,
                "data": ground_decor
            },
            {
                "id": 5,
                "name": "Environmental Objects",
                "type": "objectgroup",
                "visible": True,
                "opacity": 1.0,
                "objects": objects
            },
            {
                "id": 6,
                "name": "Collision",
                "type": "tilelayer",
                "width": MAP_W,
                "height": MAP_H,
                "visible": True,
                "opacity": 0.0,
                "data": collision
            }
        ]
    }

    out_json = "maps/forest/forest_01.json"
    with open(out_json, "w") as f:
        json.dump(json_map, f, indent=2)
    print(f"Successfully saved {out_json} ({os.path.getsize(out_json):,} bytes)")

    # 8. BUILD TILED TMX (XML) MAP
    # For layers, CSV data
    def grid_to_csv(grid):
        lines = []
        for r in range(MAP_H):
            row_vals = [str(grid[r * MAP_W + c]) for c in range(MAP_W)]
            lines.append(",".join(row_vals))
        return ",\n".join(lines)

    tmx_lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{MAP_W}" height="{MAP_H}" tilewidth="{TILE_SZ}" tileheight="{TILE_SZ}" infinite="0" nextlayerid="7" nextobjectid="{obj_id_counter + 1}">',
        f' <tileset firstgid="1" name="forest_tileset" tilewidth="{TILE_SZ}" tileheight="{TILE_SZ}" tilecount="64" columns="8">',
        '  <image source="tilesets/forest/forest_tileset.png" width="256" height="256"/>',
        ' </tileset>',
        f' <layer id="1" name="Ground" width="{MAP_W}" height="{MAP_H}">',
        '  <data encoding="csv">',
        grid_to_csv(ground),
        '  </data>',
        ' </layer>',
        f' <layer id="2" name="Terrain" width="{MAP_W}" height="{MAP_H}">',
        '  <data encoding="csv">',
        grid_to_csv(terrain),
        '  </data>',
        ' </layer>',
        f' <layer id="3" name="Water" width="{MAP_W}" height="{MAP_H}">',
        '  <data encoding="csv">',
        grid_to_csv(water),
        '  </data>',
        ' </layer>',
        f' <layer id="4" name="Ground Decorations" width="{MAP_W}" height="{MAP_H}">',
        '  <data encoding="csv">',
        grid_to_csv(ground_decor),
        '  </data>',
        ' </layer>',
        f' <objectgroup id="5" name="Environmental Objects">'
    ]

    for obj in objects:
        tmx_lines.append(f'  <object id="{obj["id"]}" name="{obj["name"]}" type="{obj["type"]}" x="{obj["x"]}" y="{obj["y"]}" width="{obj["width"]}" height="{obj["height"]}"/>')

    tmx_lines.append(' </objectgroup>')
    tmx_lines.append(f' <layer id="6" name="Collision" width="{MAP_W}" height="{MAP_H}">')
    tmx_lines.append('  <data encoding="csv">')
    tmx_lines.append(grid_to_csv(collision))
    tmx_lines.append('  </data>')
    tmx_lines.append(' </layer>')
    tmx_lines.append('</map>')

    out_tmx = "maps/forest/forest_01.tmx"
    with open(out_tmx, "w") as f:
        f.write("\n".join(tmx_lines))
    print(f"Successfully saved {out_tmx} ({os.path.getsize(out_tmx):,} bytes)")

if __name__ == "__main__":
    main()

