#!/usr/bin/env python3
"""
WILDBOUND — VILLAGE INTERIOR MAPS GENERATOR
Generates 6 Tiled JSON interior maps in maps/interiors/:
1. interior_general_store.json    (20x15 tiles)
2. interior_blacksmith.json       (20x15 tiles)
3. interior_town_hall.json        (24x18 tiles)
4. interior_cottage.json          (18x14 tiles)
5. interior_twostory_residence.json (22x16 tiles)
6. interior_warehouse.json        (20x16 tiles)
All use assets/tilesets/interior/interior_tileset.png and integrate with MapLoader.
"""

import json
import os

TILE_SZ = 32

def create_interior_map(name, w, h, floor_gid, wall_gid, floor_decor_fn, wall_decor_fn, obj_list):
    ground = [0] * (w * h)
    walls = [0] * (w * h)
    collision = [0] * (w * h)

    def set_t(grid, c, r, val):
        if 0 <= c < w and 0 <= r < h:
            grid[r * w + c] = val

    def get_t(grid, c, r):
        if 0 <= c < w and 0 <= r < h:
            return grid[r * w + c]
        return 0

    # 1. Fill ground floor
    for r in range(h):
        for c in range(w):
            ground[r * w + c] = floor_gid

    # Custom floor decorations (rugs, tiles)
    if floor_decor_fn:
        floor_decor_fn(ground, w, h)

    # 2. Build Perimeter Walls (2 tiles thick at top, 1 tile thick on sides/bottom)
    # Top wall: rows 0 and 1
    for c in range(w):
        set_t(walls, c, 0, 11) # Wall top cap
        set_t(walls, c, 1, wall_gid)
        set_t(collision, c, 0, 8)
        set_t(collision, c, 1, 8)

    # Left & Right walls
    for r in range(2, h):
        set_t(walls, 0, r, wall_gid)
        set_t(walls, w - 1, r, wall_gid)
        set_t(collision, 0, r, 8)
        set_t(collision, w - 1, r, 8)

    # Bottom wall (with 2-tile doorway gap at center)
    door_c1 = w // 2 - 1
    door_c2 = w // 2
    for c in range(w):
        if c == door_c1 or c == door_c2:
            # Door threshold mat
            set_t(ground, c, h - 1, 7)
            # Doorway has no wall and no collision!
        else:
            set_t(walls, c, h - 1, wall_gid)
            set_t(collision, c, h - 1, 8)

    # Custom wall & furniture placement
    if wall_decor_fn:
        wall_decor_fn(walls, collision, w, h)

    # 3. Object group
    map_json = {
        "width": w,
        "height": h,
        "tilewidth": TILE_SZ,
        "tileheight": TILE_SZ,
        "tilesets": [
            {
                "firstgid": 1,
                "name": "interior_tileset",
                "tilewidth": TILE_SZ,
                "tileheight": TILE_SZ,
                "columns": 8,
                "tilecount": 64,
                "image": "tilesets/interior/interior_tileset.png"
            }
        ],
        "layers": [
            {
                "id": 1,
                "name": "Ground",
                "type": "tilelayer",
                "width": w,
                "height": h,
                "visible": True,
                "opacity": 1.0,
                "data": ground
            },
            {
                "id": 2,
                "name": "Walls",
                "type": "tilelayer",
                "width": w,
                "height": h,
                "visible": True,
                "opacity": 1.0,
                "data": walls
            },
            {
                "id": 3,
                "name": "Collision",
                "type": "tilelayer",
                "width": w,
                "height": h,
                "visible": True,
                "opacity": 0.0,
                "data": collision
            },
            {
                "id": 4,
                "name": "Objects",
                "type": "objectgroup",
                "visible": True,
                "opacity": 1.0,
                "objects": obj_list
            }
        ]
    }
    return map_json

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

def build_general_store():
    W, H = 20, 15
    def floor_decor(ground, w, h):
        # Runner rug from door to counter
        for r in range(6, h - 1):
            for c in [w // 2 - 1, w // 2]:
                ground[r * w + c] = 17 # Rug tile

    def wall_decor(walls, collision, w, h):
        # Top wall shelves
        for c in range(2, 8):
            walls[1 * w + c] = 28 # Bookshelves
            collision[1 * w + c] = 8
        for c in range(12, 18):
            walls[1 * w + c] = 29 # Potion shelves
            collision[1 * w + c] = 8

        # L-shaped shop counter across row 5
        for c in range(4, 16):
            walls[5 * w + c] = 25 # Shop counter front
            collision[5 * w + c] = 8

        # Display tables on sides
        for r in [8, 10]:
            walls[r * w + 2] = 27 # Display table left
            collision[r * w + 2] = 8
            walls[r * w + 17] = 27 # Display table right
            collision[r * w + 17] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Store_Crate_1", "crate_small_001", 3 * 32, 3 * 32, 24, 24, 20, 16),
        make_obj(4, "Store_Barrel_1", "barrel_standard_001", 16 * 32, 3 * 32, 24, 24, 20, 16),
        make_obj(5, "Store_Counter_Lamp", "lamp_hanging_001", 10 * 32, 4 * 32, 24, 32),
    ]
    return create_interior_map("interior_general_store", W, H, 1, 9, floor_decor, wall_decor, objs)

def build_blacksmith():
    W, H = 20, 15
    def floor_decor(ground, w, h):
        # Flagstone around forge
        for r in range(2, 6):
            for c in range(7, 13):
                ground[r * w + c] = 6

    def wall_decor(walls, collision, w, h):
        # Stone forge at center of top wall
        walls[1 * w + 9] = 30 # Chimney
        walls[1 * w + 10] = 30
        walls[2 * w + 9] = 31 # Hearth fire
        walls[2 * w + 10] = 31
        collision[1 * w + 9] = 8
        collision[1 * w + 10] = 8
        collision[2 * w + 9] = 8
        collision[2 * w + 10] = 8

        # Left anvil & tool benches
        for c in range(2, 6):
            walls[2 * w + c] = 27 # Tool bench
            collision[2 * w + c] = 8

        # Right weapon racks
        for c in range(14, 18):
            walls[2 * w + c] = 28 # Tool rack
            collision[2 * w + c] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Forge_Fire", "fire_medium_anim", 9.5 * 32, 2.5 * 32, 24, 32),
        make_obj(4, "Cooling_Trough", "barrel_banded_001", 12 * 32, 3 * 32, 24, 24, 20, 16),
        make_obj(5, "Blacksmith_Anvil_Rock", "rock_flat_001", 6 * 32, 5 * 32, 32, 20, 26, 14),
        make_obj(6, "Coal_Crate", "crate_reinforced_001", 3 * 32, 7 * 32, 24, 24, 20, 16),
    ]
    return create_interior_map("interior_blacksmith", W, H, 4, 10, floor_decor, wall_decor, objs)

def build_town_hall():
    W, H = 24, 18
    def floor_decor(ground, w, h):
        # Grand center carpet
        for r in range(5, 13):
            for c in range(5, 19):
                ground[r * w + c] = 17

    def wall_decor(walls, collision, w, h):
        # Grand fireplace at center top
        walls[1 * w + 11] = 30
        walls[1 * w + 12] = 30
        walls[2 * w + 11] = 31
        walls[2 * w + 12] = 31
        collision[1 * w + 11] = 8
        collision[1 * w + 12] = 8
        collision[2 * w + 11] = 8
        collision[2 * w + 12] = 8

        # Bookcases on top wall
        for c in [2, 3, 4, 19, 20, 21]:
            walls[1 * w + c] = 28
            collision[1 * w + c] = 8

        # Council table at center (rows 8, 9, cols 8..15)
        for r in [8, 9]:
            for c in range(8, 16):
                walls[r * w + c] = 27
                collision[r * w + c] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Hearth_Flame", "fire_small_anim", 11.5 * 32, 2.5 * 32, 16, 24),
        make_obj(4, "Elder_Bench_L", "bench_wooden_001", 6 * 32, 8.5 * 32, 32, 24, 24, 12),
        make_obj(5, "Elder_Bench_R", "bench_wooden_001", 17 * 32, 8.5 * 32, 32, 24, 24, 12),
        make_obj(6, "Council_Lamp_1", "lamp_wall_001", 4 * 32, 2 * 32, 24, 32),
        make_obj(7, "Council_Lamp_2", "lamp_wall_001", 19 * 32, 2 * 32, 24, 32),
    ]
    return create_interior_map("interior_town_hall", W, H, 1, 9, floor_decor, wall_decor, objs)

def build_cottage():
    W, H = 18, 14
    def floor_decor(ground, w, h):
        # Bedside rug
        for r in range(4, 7):
            for c in range(3, 7):
                ground[r * w + c] = 17

    def wall_decor(walls, collision, w, h):
        # Cozy bed in top-left
        walls[1 * w + 2] = 32 # Bed top
        walls[2 * w + 2] = 33 # Bed bottom
        collision[1 * w + 2] = 8
        collision[2 * w + 2] = 8

        # Small hearth in top-right
        walls[1 * w + 14] = 30
        walls[2 * w + 14] = 31
        collision[1 * w + 14] = 8
        collision[2 * w + 14] = 8

        # Dining table at center-right
        walls[6 * w + 11] = 27
        walls[6 * w + 12] = 27
        collision[6 * w + 11] = 8
        collision[6 * w + 12] = 8

        # Bookshelf on top wall
        walls[1 * w + 7] = 28
        collision[1 * w + 7] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Cottage_Fire", "fire_small_anim", 14 * 32, 2.5 * 32, 16, 24),
        make_obj(4, "Cottage_Chest", "crate_small_001", 4 * 32, 2 * 32, 24, 24, 20, 16),
        make_obj(5, "Cottage_Barrel", "barrel_small_001", 15 * 32, 4 * 32, 24, 24, 20, 16),
    ]
    return create_interior_map("interior_cottage", W, H, 3, 9, floor_decor, wall_decor, objs)

def build_twostory_residence():
    W, H = 22, 16
    def floor_decor(ground, w, h):
        for r in range(5, 10):
            for c in range(12, 18):
                ground[r * w + c] = 17

    def wall_decor(walls, collision, w, h):
        # Partition wall splitting pantry from living room (cols 8, rows 1..9)
        for r in range(1, 9):
            walls[r * w + 8] = 9
            collision[r * w + 8] = 8

        # Kitchen pantry counter
        for r in range(3, 7):
            walls[r * w + 3] = 25
            collision[r * w + 3] = 8

        # Fireplace in living room
        walls[1 * w + 14] = 30
        walls[2 * w + 14] = 31
        collision[1 * w + 14] = 8
        collision[2 * w + 14] = 8

        # Large dining table
        for c in range(13, 17):
            walls[7 * w + c] = 27
            collision[7 * w + c] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Residence_Hearth", "fire_medium_anim", 14 * 32, 2.5 * 32, 24, 32),
        make_obj(4, "Pantry_Barrel_1", "barrel_standard_001", 2 * 32, 2 * 32, 24, 24, 20, 16),
        make_obj(5, "Pantry_Crate_1", "crate_small_001", 2 * 32, 4 * 32, 24, 24, 20, 16),
        make_obj(6, "Residence_Lamp", "lamp_wall_001", 18 * 32, 2 * 32, 24, 32),
    ]
    return create_interior_map("interior_twostory_residence", W, H, 1, 9, floor_decor, wall_decor, objs)

def build_warehouse():
    W, H = 20, 16
    def floor_decor(ground, w, h):
        pass # Rustic wooden warehouse floor

    def wall_decor(walls, collision, w, h):
        # Heavy storage shelving on north wall
        for c in range(2, 9):
            walls[1 * w + c] = 28
            collision[1 * w + c] = 8
        for c in range(11, 18):
            walls[1 * w + c] = 28
            collision[1 * w + c] = 8

        # Cargo aisles in middle
        for r in [5, 6]:
            for c in [4, 5, 14, 15]:
                walls[r * w + c] = 27 # Cargo stacking platform
                collision[r * w + c] = 8

    objs = [
        make_obj(1, "player_spawn", "spawn", (W // 2) * TILE_SZ, (H - 2) * TILE_SZ, 32, 32),
        make_obj(2, "exit_door", "door_trigger", (W // 2) * TILE_SZ, (H - 1) * TILE_SZ, 64, 32),
        make_obj(3, "Wh_Crate_A", "crate_large_001", 4 * 32, 5 * 32, 32, 32, 28, 24),
        make_obj(4, "Wh_Crate_B", "crate_reinforced_001", 14 * 32, 5 * 32, 24, 24, 20, 16),
        make_obj(5, "Wh_Barrel_A", "barrel_banded_001", 4 * 32, 8 * 32, 24, 24, 20, 16),
        make_obj(6, "Wh_Barrel_B", "barrel_standard_001", 15 * 32, 8 * 32, 24, 24, 20, 16),
        make_obj(7, "Wh_Lamp", "lamp_hanging_001", 10 * 32, 3 * 32, 24, 32),
    ]
    return create_interior_map("interior_warehouse", W, H, 2, 9, floor_decor, wall_decor, objs)

def main():
    out_dir = "maps/interiors"
    os.makedirs(out_dir, exist_ok=True)

    interiors = [
        ("interior_general_store.json", build_general_store()),
        ("interior_blacksmith.json", build_blacksmith()),
        ("interior_town_hall.json", build_town_hall()),
        ("interior_cottage.json", build_cottage()),
        ("interior_twostory_residence.json", build_twostory_residence()),
        ("interior_warehouse.json", build_warehouse())
    ]

    for filename, data in interiors:
        path = os.path.join(out_dir, filename)
        with open(path, "w") as f:
            json.dump(data, f, indent=2)
        print(f"Generated {filename}: {data['width']}x{data['height']} tiles ({data['width']*32}x{data['height']*32} px), {len(data['layers'][-1]['objects'])} objects")

if __name__ == "__main__":
    main()

