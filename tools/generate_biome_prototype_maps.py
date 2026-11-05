#!/usr/bin/env python3
"""
WILDBOUND — PROTOTYPE BIOME MAPS GENERATOR (PHASE 14)
Generates 4 fully-featured prototype maps in Tiled JSON format:
1. maps/desert/desert_01.json   (Sunscorched Wastes)
2. maps/snow/snow_01.json       (Frostfall Taiga)
3. maps/swamp/swamp_01.json     (Murkwood Mire)
4. maps/mountain/mountain_01.json (Iron Crag Highlands)

Dimensions: 60x45 tiles (1920x1440 px).
"""

import json
import math
import random
import os

MAP_W = 60
MAP_H = 45
TILE_SZ = 32
WORLD_W = MAP_W * TILE_SZ # 1920 px
WORLD_H = MAP_H * TILE_SZ # 1440 px

def create_grid(val=0):
    return [val] * (MAP_W * MAP_H)

def set_t(grid, c, r, val):
    if 0 <= c < MAP_W and 0 <= r < MAP_H:
        grid[r * MAP_W + c] = val

def get_t(grid, c, r):
    if 0 <= c < MAP_W and 0 <= r < MAP_H:
        return grid[r * MAP_W + c]
    return 0

def fill_rect(grid, c1, r1, c2, r2, val):
    for r in range(max(0, r1), min(MAP_H, r2 + 1)):
        for c in range(max(0, c1), min(MAP_W, c2 + 1)):
            grid[r * MAP_W + c] = val

def fill_circle(grid, cx, cy, rad, val):
    r_sq = rad * rad
    for r in range(max(0, int(cy - rad)), min(MAP_H, int(cy + rad + 1))):
        for c in range(max(0, int(cx - rad)), min(MAP_W, int(cx + rad + 1))):
            if (c - cx)**2 + (r - cy)**2 <= r_sq:
                grid[r * MAP_W + c] = val

def draw_line_path(grid, c1, r1, c2, r2, width, val):
    dx = c2 - c1
    dy = r2 - r1
    dist = math.hypot(dx, dy)
    steps = max(1, int(dist * 2))
    for s in range(steps + 1):
        t = s / steps
        cx = c1 + dx * t
        cy = r1 + dy * t
        fill_circle(grid, cx, cy, width * 0.5, val)

def make_obj(oid, name, otype, x, y, w, h, col_w=0, col_h=0):
    props = []
    if col_w > 0 and col_h > 0:
        props = [
            { "name": "collision", "type": "bool", "value": True },
            { "name": "collision_type", "type": "string", "value": "solid" },
            { "name": "collision_width", "type": "float", "value": float(col_w) },
            { "name": "collision_height", "type": "float", "value": float(col_h) }
        ]
    obj = {
        "id": oid,
        "name": name,
        "type": otype,
        "x": float(x),
        "y": float(y),
        "width": float(w),
        "height": float(h),
        "rotation": 0.0,
        "visible": True
    }
    if props:
        obj["properties"] = props
    return obj

def export_tiled_json(out_path, map_name, tileset_name, tileset_img, ground, terrain, water, decor, collision, objects):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    layers = [
        { "id": 1, "name": "Ground", "type": "tilelayer", "width": MAP_W, "height": MAP_H, "visible": True, "opacity": 1.0, "data": ground },
        { "id": 2, "name": "Terrain", "type": "tilelayer", "width": MAP_W, "height": MAP_H, "visible": True, "opacity": 1.0, "data": terrain },
        { "id": 3, "name": "Water", "type": "tilelayer", "width": MAP_W, "height": MAP_H, "visible": True, "opacity": 1.0, "data": water },
        { "id": 4, "name": "Ground Decorations", "type": "tilelayer", "width": MAP_W, "height": MAP_H, "visible": True, "opacity": 1.0, "data": decor },
        { "id": 5, "name": "Environmental Objects", "type": "objectgroup", "visible": True, "opacity": 1.0, "objects": objects },
        { "id": 6, "name": "Collision", "type": "tilelayer", "width": MAP_W, "height": MAP_H, "visible": True, "opacity": 0.0, "data": collision }
    ]
    doc = {
        "width": MAP_W,
        "height": MAP_H,
        "tilewidth": TILE_SZ,
        "tileheight": TILE_SZ,
        "tilesets": [
            {
                "firstgid": 1,
                "name": tileset_name,
                "tilewidth": TILE_SZ,
                "tileheight": TILE_SZ,
                "columns": 8,
                "tilecount": 64,
                "image": tileset_img
            }
        ],
        "layers": layers
    }
    with open(out_path, "w") as f:
        json.dump(doc, f, indent=2)
    print(f"Exported {map_name} map: {out_path} ({MAP_W}x{MAP_H} tiles, {len(objects)} objects)")

# ==============================================================================
# 1. DESERT PROTOTYPE MAP
# ==============================================================================
def generate_desert_map():
    random.seed(101)
    ground = create_grid(1) # Sand dune base
    terrain = create_grid(0)
    water = create_grid(0)
    decor = create_grid(0)
    collision = create_grid(0)
    objs = []
    oid = 1

    # Sand variations
    for r in range(MAP_H):
        for c in range(MAP_W):
            n = math.sin(c * 0.2) + math.cos(r * 0.25)
            if n > 0.8: ground[r * MAP_W + c] = 2
            elif n < -0.8: ground[r * MAP_W + c] = 3

    # Cracked clay mesa (North-West plateau: C4..22, R4..14)
    fill_rect(terrain, 4, 4, 22, 14, 9)

    # Ancient Sandstone Ruins platform (South-West: C8..20, R26..38)
    fill_rect(terrain, 8, 26, 20, 38, 33)

    # Main Caravan Trail (West border -> Ruins -> Oasis -> East border)
    draw_line_path(terrain, 0, 22, 14, 24, 2.5, 17)
    draw_line_path(terrain, 14, 24, 38, 24, 2.5, 17)
    draw_line_path(terrain, 38, 24, 59, 22, 2.5, 17)

    # Freshwater Oasis Pool (C38..48, R20..28)
    fill_circle(water, 43, 24, 6.0, 25) # Oasis water
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] == 25:
                collision[r * MAP_W + c] = 58 # Water collision

    # Desert foliage & scrub decor
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] == 0 and terrain[r * MAP_W + c] == 0:
                if random.random() < 0.05:
                    decor[r * MAP_W + c] = 49 # Thorny scrub

    # Objects
    objs.append(make_obj(oid, "player_spawn", "spawn", 192.0, 704.0, 32, 32)); oid += 1
    # Sandstone Ruins
    objs.append(make_obj(oid, "Desert_Arch", "ruin_arch_stone_001", 450, 850, 64, 80, 48, 28)); oid += 1
    objs.append(make_obj(oid, "Desert_Pillar_1", "ruin_pillar_stone_001", 400, 830, 32, 64, 24, 20)); oid += 1
    objs.append(make_obj(oid, "Desert_Pillar_2", "ruin_pillar_stone_001", 530, 830, 32, 64, 24, 20)); oid += 1
    objs.append(make_obj(oid, "Desert_Watchtower", "ruin_tower_small_001", 350, 950, 80, 112, 70, 48)); oid += 1
    # Oasis Camp
    objs.append(make_obj(oid, "Oasis_Water_Ripple", "water_ripple_anim", 1370, 760, 32, 32)); oid += 1
    objs.append(make_obj(oid, "Oasis_Campfire", "fire_small_anim", 1260, 740, 16, 24, 10, 10)); oid += 1
    objs.append(make_obj(oid, "Oasis_Crates", "crate_reinforced_001", 1240, 770, 24, 24, 20, 14)); oid += 1
    objs.append(make_obj(oid, "Oasis_Barrels", "barrel_banded_001", 1280, 780, 24, 24, 20, 14)); oid += 1
    # Boulders
    for i in range(12):
        bx = random.randint(150, 1750)
        by = random.randint(150, 1300)
        if math.hypot(bx - 1370, by - 760) > 250: # Away from oasis
            objs.append(make_obj(oid, f"Desert_Boulder_{i}", "rock_boulder_001", bx, by, 48, 40, 40, 24)); oid += 1
    # Ambient Wildlife
    objs.append(make_obj(oid, "Desert_Hawk_1", "ambient_bird", 500, 400, 16, 16)); oid += 1
    objs.append(make_obj(oid, "Desert_Hawk_2", "ambient_bird", 1300, 600, 16, 16)); oid += 1

    export_tiled_json("maps/desert/desert_01.json", "desert_01", "desert_tileset",
                      "tilesets/desert/desert_tileset.png", ground, terrain, water, decor, collision, objs)

# ==============================================================================
# 2. SNOW PROTOTYPE MAP
# ==============================================================================
def generate_snow_map():
    random.seed(202)
    ground = create_grid(1) # Powder snow
    terrain = create_grid(0)
    water = create_grid(0)
    decor = create_grid(0)
    collision = create_grid(0)
    objs = []
    oid = 1

    # Snow ground variations
    for r in range(MAP_H):
        for c in range(MAP_W):
            if (c + r) % 7 == 0: ground[r * MAP_W + c] = 2

    # Northern Frozen Granite Ridge (R3..8, C10..55)
    for r in range(3, 9):
        for c in range(10, 56):
            set_t(terrain, c, r, 33)
            set_t(collision, c, r, 58)

    # Glacial Lake with Ice Sheet (C12..30, R18..32)
    fill_circle(water, 21, 25, 8.0, 17) # Glacial ice (walkable!)
    fill_circle(water, 21, 25, 4.0, 25) # Freezing open water lead (impassable)
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] == 25:
                collision[r * MAP_W + c] = 58

    # Winding Packed Snow Trail (SW to NE)
    draw_line_path(terrain, 0, 32, 22, 38, 2.5, 9)
    draw_line_path(terrain, 22, 38, 42, 28, 2.5, 9)
    draw_line_path(terrain, 42, 28, 59, 14, 2.5, 9)

    # Objects
    objs.append(make_obj(oid, "player_spawn", "spawn", 192.0, 960.0, 32, 32)); oid += 1
    # Winter Cabin Settlement
    objs.append(make_obj(oid, "Winter_Cabin", "cabin_woodland_001", 1250, 680, 80, 80, 68, 36)); oid += 1
    objs.append(make_obj(oid, "Cabin_Smoke", "smoke_chimney_anim", 1310, 630, 16, 32)); oid += 1
    objs.append(make_obj(oid, "Winter_Fire", "fire_medium_anim", 1180, 750, 24, 32, 16, 12)); oid += 1
    objs.append(make_obj(oid, "Winter_Shed", "storage_shed_001", 1380, 700, 64, 64, 52, 28)); oid += 1
    objs.append(make_obj(oid, "Winter_Crate", "crate_large_001", 1220, 780, 32, 32, 26, 20)); oid += 1
    objs.append(make_obj(oid, "Frozen_Waterfall", "waterfall_small_anim", 850, 220, 32, 48)); oid += 1
    # Snow Pines
    for i in range(25):
        px = random.randint(150, 1800)
        py = random.randint(150, 1300)
        if get_t(water, px//32, py//32) == 0 and get_t(terrain, px//32, py//32) != 9:
            ptype = random.choice(["tree_pine_001", "tree_pine_002", "tree_pine_003"])
            pw = 48 if "002" in ptype else (64 if "003" in ptype else 32)
            ph = 80 if "002" in ptype else (96 if "003" in ptype else 64)
            objs.append(make_obj(oid, f"Snow_Pine_{i}", ptype, px, py, pw, ph, pw*0.4, 16)); oid += 1
    # Ambient Birds
    objs.append(make_obj(oid, "Snow_Owl_1", "ambient_bird", 600, 500, 16, 16)); oid += 1
    objs.append(make_obj(oid, "Snow_Owl_2", "ambient_bird", 1400, 800, 16, 16)); oid += 1

    export_tiled_json("maps/snow/snow_01.json", "snow_01", "snow_tileset",
                      "tilesets/snow/snow_tileset.png", ground, terrain, water, decor, collision, objs)

# ==============================================================================
# 3. SWAMP PROTOTYPE MAP
# ==============================================================================
def generate_swamp_map():
    random.seed(303)
    ground = create_grid(1) # Mud / peat
    terrain = create_grid(0)
    water = create_grid(0)
    decor = create_grid(0)
    collision = create_grid(0)
    objs = []
    oid = 1

    # Deep Stagnant Bog Lakes (C10..24, R12..24 and C32..48, R20..36)
    fill_circle(water, 18, 18, 7.5, 17) # Deep bog void (impassable)
    fill_circle(water, 40, 28, 9.0, 17) # Deep bog void (impassable)
    # Shallow algae water surrounding bogs (walkable)
    for r in range(MAP_H):
        for c in range(MAP_W):
            if water[r * MAP_W + c] == 17:
                collision[r * MAP_W + c] = 58 # Solid collision on deep bog
                # Expand shallow algae water edge
                for dc, dr in [(-1,0),(1,0),(0,-1),(0,1)]:
                    nc, nr = c+dc, r+dr
                    if 0 <= nc < MAP_W and 0 <= nr < MAP_H and water[nr * MAP_W + nc] == 0:
                        water[nr * MAP_W + nc] = 9

    # Wooden Duckboard Walkways crossing over the swamps (C0..59)
    draw_line_path(terrain, 0, 22, 18, 22, 1.8, 25)
    draw_line_path(terrain, 18, 22, 28, 26, 1.8, 25)
    draw_line_path(terrain, 28, 26, 42, 20, 1.8, 25)
    draw_line_path(terrain, 42, 20, 59, 22, 1.8, 25)
    # Remove water collision under duckboards!
    for r in range(MAP_H):
        for c in range(MAP_W):
            if terrain[r * MAP_W + c] == 25:
                collision[r * MAP_W + c] = 0

    # Luminescent mushrooms & reeds decor
    for r in range(MAP_H):
        for c in range(MAP_W):
            if collision[r * MAP_W + c] == 0 and terrain[r * MAP_W + c] == 0:
                if random.random() < 0.04: decor[r * MAP_W + c] = 41 # Glowing mushroom
                elif random.random() < 0.03: decor[r * MAP_W + c] = 49 # Cattails

    # Objects
    objs.append(make_obj(oid, "player_spawn", "spawn", 192.0, 704.0, 32, 32)); oid += 1
    # Overgrown Bog Hut
    objs.append(make_obj(oid, "Swamp_Hut", "cabin_overgrown_001", 900, 780, 80, 80, 70, 36)); oid += 1
    objs.append(make_obj(oid, "Swamp_Log_1", "log_fallen_mossy_001", 750, 740, 48, 20, 40, 14)); oid += 1
    objs.append(make_obj(oid, "Swamp_Log_2", "log_fallen_mossy_001", 1020, 820, 48, 20, 40, 14)); oid += 1
    objs.append(make_obj(oid, "Swamp_Campfire", "fire_medium_anim", 860, 860, 24, 32, 16, 12)); oid += 1
    objs.append(make_obj(oid, "Swamp_Smoke", "smoke_campfire_anim", 864, 840, 16, 24)); oid += 1
    objs.append(make_obj(oid, "Bog_Pond_Ripple", "water_pond_anim", 600, 600, 32, 32)); oid += 1
    # Swamp trees & stumps
    for i in range(18):
        px = random.randint(150, 1800)
        py = random.randint(150, 1300)
        if collision[py//32 * MAP_W + px//32] == 0 and terrain[py//32 * MAP_W + px//32] == 0:
            objs.append(make_obj(oid, f"Swamp_Oak_{i}", "tree_oak_001", px, py, 48, 64, 20, 16)); oid += 1
    # Swamp butterflies
    objs.append(make_obj(oid, "Swamp_Butterfly_1", "blue_butterfly", 500, 700, 16, 16)); oid += 1
    objs.append(make_obj(oid, "Swamp_Butterfly_2", "forest_butterfly", 950, 720, 16, 16)); oid += 1

    export_tiled_json("maps/swamp/swamp_01.json", "swamp_01", "swamp_tileset",
                      "tilesets/swamp/swamp_tileset.png", ground, terrain, water, decor, collision, objs)

# ==============================================================================
# 4. MOUNTAIN PROTOTYPE MAP
# ==============================================================================
def generate_mountain_map():
    random.seed(404)
    ground = create_grid(1) # Granite terrace
    terrain = create_grid(0)
    water = create_grid(0)
    decor = create_grid(0)
    collision = create_grid(0)
    objs = []
    oid = 1

    # Heather turf meadows (C4..20, R28..42 and C36..56, R12..24)
    fill_rect(terrain, 4, 28, 20, 42, 9)
    fill_rect(terrain, 36, 12, 56, 24, 9)

    # Basalt Stepped Cliff Face along North (R4..10, C10..55)
    for r in range(4, 11):
        for c in range(10, 56):
            set_t(terrain, c, r, 25) # Cliff face
            set_t(collision, c, r, 58) # Impassable cliff

    # Mountain Scree Trail (SW to NE, crossing chasm)
    draw_line_path(terrain, 0, 36, 24, 30, 2.2, 17)
    draw_line_path(terrain, 24, 30, 32, 22, 2.2, 17) # Bridge across chasm
    draw_line_path(terrain, 32, 22, 59, 16, 2.2, 17)

    # Waterfall cascade pouring down Northern cliff (C28..31, R4..10)
    for r in range(4, 11):
        for c in [28, 29, 30]:
            set_t(water, c, r, 41)
            set_t(collision, c, r, 58)

    # Cave Archway carved in North cliff at C42, R10
    set_t(decor, 42, 10, 49)
    set_t(collision, 42, 10, 0) # Entrance is passable!

    # Objects
    objs.append(make_obj(oid, "player_spawn", "spawn", 192.0, 1152.0, 32, 32)); oid += 1
    # Mountain Watchtower
    objs.append(make_obj(oid, "Mountain_Watchtower", "watchtower_stone_001", 750, 680, 64, 128, 52, 40)); oid += 1
    objs.append(make_obj(oid, "Chasm_Bridge", "bridge_wood_large_001", 860, 700, 64, 48, 0, 0)); oid += 1
    objs.append(make_obj(oid, "Bridge_Warning_Sign", "sign_warning_001", 800, 720, 24, 32, 12, 10)); oid += 1
    # Waterfall animations
    objs.append(make_obj(oid, "Highland_Waterfall_Main", "waterfall_large_anim", 900, 240, 64, 80)); oid += 1
    objs.append(make_obj(oid, "Highland_Waterfall_Side", "waterfall_medium_anim", 964, 270, 48, 64)); oid += 1
    # Highland Pines & Boulders
    for i in range(16):
        px = random.randint(150, 1800)
        py = random.randint(150, 1300)
        if collision[py//32 * MAP_W + px//32] == 0:
            if i % 2 == 0:
                objs.append(make_obj(oid, f"Highland_Pine_{i}", "tree_pine_003", px, py, 64, 96, 28, 16)); oid += 1
            else:
                objs.append(make_obj(oid, f"Highland_Rock_{i}", "rock_boulder_001", px, py, 48, 40, 40, 24)); oid += 1
    # Mountain Birds
    objs.append(make_obj(oid, "Mountain_Eagle_1", "ambient_bird", 700, 400, 16, 16)); oid += 1
    objs.append(make_obj(oid, "Mountain_Eagle_2", "ambient_bird", 1300, 500, 16, 16)); oid += 1

    export_tiled_json("maps/mountain/mountain_01.json", "mountain_01", "mountain_tileset",
                      "tilesets/mountain/mountain_tileset.png", ground, terrain, water, decor, collision, objs)

def main():
    print("Generating Wildbound Phase 14 Biome Prototype Maps (60x45 tiles)...")
    generate_desert_map()
    generate_snow_map()
    generate_swamp_map()
    generate_mountain_map()
    print("All 4 prototype maps generated successfully.")

if __name__ == "__main__":
    main()

