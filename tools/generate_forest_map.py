import os
import json
import math

def generate_forest_01():
    WIDTH = 80
    HEIGHT = 60
    TILE_SIZE = 32

    # Tile GIDs based on our 8x8 tileset:
    # Row 0: Grass (1..8)
    G_GRASS = 1
    G_GRASS_FLOWER_YEL = 2
    G_GRASS_PEBBLE = 3
    G_GRASS_DARK = 4
    G_GRASS_FLOWER_BLU = 5
    G_GRASS_LEAF = 6
    G_GRASS_TUFT = 7
    G_GRASS_CLOVER = 8

    # Row 1: Dirt (9..16)
    G_DIRT = 9
    G_DIRT_PEBBLE = 10
    G_DIRT_N = 11
    G_DIRT_S = 12
    G_DIRT_W = 13
    G_DIRT_E = 14
    G_DIRT_NW = 15
    G_DIRT_NE = 16

    # Row 2: Cobblestone (17..24)
    G_COBBLE = 17
    G_COBBLE_MOSS = 18

    # Row 3: Water (25..32)
    G_WATER = 25
    G_WATER_RIPPLE = 26
    G_SHORE_N = 27
    G_SHORE_S = 28
    G_SHORE_W = 29
    G_SHORE_E = 30
    G_WATER_ROCKS = 31
    G_WATER_LILIES = 32

    # Row 4: Bridge, Fence, Cabin (33..40)
    G_BRIDGE_H = 33
    G_BRIDGE_V = 34
    G_BRIDGE_POST = 35
    G_FENCE_H = 36
    G_FENCE_V = 37

    # Row 5: Roof & Woodland props (41..48)
    G_MUSHROOM_RED = 43
    G_MUSHROOM_BRN = 44
    G_WOOD_BUSH = 45
    G_MOSSY_ROCK = 46
    G_STUMP = 47
    G_CAMPFIRE = 48

    # Row 6: Props (49..56)
    G_SIGNPOST = 49
    G_STONE_RUNE = 50
    G_HOLLOW_LOG = 51
    G_CRATE = 52
    G_BARREL = 53
    G_LAVENDER = 54

    # Row 7: Collision (57..59)
    G_COLLISION_SOLID = 57
    G_COLLISION_WATER = 58

    # Initialize layers (80x60 = 4800 cells)
    ground_layer = [G_GRASS] * (WIDTH * HEIGHT)
    terrain_layer = [0] * (WIDTH * HEIGHT)
    water_layer = [0] * (WIDTH * HEIGHT)
    decorations_layer = [0] * (WIDTH * HEIGHT)
    collision_layer = [0] * (WIDTH * HEIGHT)

    def set_tile(layer, x, y, gid):
        if 0 <= x < WIDTH and 0 <= y < HEIGHT:
            layer[y * WIDTH + x] = gid

    def get_tile(layer, x, y):
        if 0 <= x < WIDTH and 0 <= y < HEIGHT:
            return layer[y * WIDTH + x]
        return 0

    # 1. Procedural grass variation
    for y in range(HEIGHT):
        for x in range(WIDTH):
            h = (x * 37 + y * 53 + (x ^ y) * 17) % 100
            if h < 6:
                ground_layer[y * WIDTH + x] = G_GRASS_FLOWER_YEL
            elif h < 11:
                ground_layer[y * WIDTH + x] = G_GRASS_FLOWER_BLU
            elif h < 16:
                ground_layer[y * WIDTH + x] = G_GRASS_PEBBLE
            elif h < 22:
                ground_layer[y * WIDTH + x] = G_GRASS_DARK
            elif h < 26:
                ground_layer[y * WIDTH + x] = G_GRASS_CLOVER

    # 2. Main Dirt Path
    # West entrance (x=0..30, y=30)
    for x in range(0, 32):
        for dy in [-1, 0, 1]:
            gid = G_DIRT if dy == 0 else G_DIRT_PEBBLE
            set_tile(terrain_layer, x, 30 + dy, gid)

    # Path fork point at (x=30, y=30)
    # Fork 1: heading NE towards cabin at (x=60, y=16)
    cx, cy = 30.0, 30.0
    target1_x, target1_y = 60.0, 16.0
    steps = 45
    for s in range(steps):
        t = s / steps
        px = int(cx + (target1_x - cx) * t)
        py = int(cy + (target1_y - cy) * t)
        for dx in [-1, 0, 1]:
            for dy in [-1, 0]:
                set_tile(terrain_layer, px + dx, py + dy, G_DIRT)

    # Fork 2: heading towards bridge at (x=44, y=37) then SE to (x=62, y=59)
    cx, cy = 30.0, 30.0
    for s in range(25):
        t = s / 25
        px = int(cx + (44.0 - cx) * t)
        py = int(cy + (37.0 - cy) * t)
        for dx in [-1, 0]:
            for dy in [-1, 0]:
                set_tile(terrain_layer, px + dx, py + dy, G_DIRT)

    for s in range(35):
        t = s / 35
        px = int(44.0 + (62.0 - 44.0) * t)
        py = int(37.0 + (59.0 - 37.0) * t)
        for dx in [-1, 0]:
            for dy in [-1, 0]:
                set_tile(terrain_layer, px + dx, py + dy, G_DIRT)

    # Cobblestone courtyard around Cabin (x=56..64, y=14..20)
    for y in range(14, 21):
        for x in range(56, 65):
            set_tile(terrain_layer, x, y, G_COBBLE)

    # 3. Water: River from North (x=41) to South-East (x=60)
    river_x_center = []
    for y in range(HEIGHT):
        # S-curve river
        rx = 41.0 + math.sin(y * 0.1) * 3.5 + (y * 0.3)
        river_x_center.append(int(rx))

    for y in range(HEIGHT):
        center_x = river_x_center[y]
        for w in range(-3, 4):
            rx = center_x + w
            set_tile(water_layer, rx, y, G_WATER)
            set_tile(collision_layer, rx, y, G_COLLISION_WATER)

    # Water ripples in river
    for y in range(HEIGHT):
        center_x = river_x_center[y]
        if (y * 7) % 5 == 0:
            set_tile(water_layer, center_x, y, G_WATER_RIPPLE)
            if (y * 11) % 7 == 0:
                set_tile(water_layer, center_x - 1, y, G_WATER_ROCKS)

    # Scenic forest pond on West side (x=10..22, y=12..22)
    for y in range(12, 23):
        for x in range(10, 23):
            dx = (x - 16) / 5.5
            dy = (y - 17) / 4.5
            dist = dx * dx + dy * dy
            if dist < 1.0:
                set_tile(water_layer, x, y, G_WATER)
                set_tile(collision_layer, x, y, G_COLLISION_WATER)
            elif dist < 1.25:
                set_tile(water_layer, x, y, G_WATER_LILIES)
                set_tile(collision_layer, x, y, G_COLLISION_WATER)

    # Wooden Bridge over river at y=36..38
    # Clear collision on the bridge so player can cross!
    bridge_center_x = river_x_center[37]
    for by in range(36, 39):
        for bx in range(bridge_center_x - 3, bridge_center_x + 4):
            set_tile(terrain_layer, bx, by, G_BRIDGE_H)
            set_tile(collision_layer, bx, by, 0) # Clear water collision on bridge!

    # 4. Ground Decorations
    for y in range(2, HEIGHT - 2):
        for x in range(2, WIDTH - 2):
            if water_layer[y * WIDTH + x] == 0 and terrain_layer[y * WIDTH + x] == 0:
                h = (x * 79 + y * 43) % 150
                if h == 7:
                    set_tile(decorations_layer, x, y, G_MUSHROOM_RED)
                elif h == 14:
                    set_tile(decorations_layer, x, y, G_MUSHROOM_BRN)
                elif h == 21:
                    set_tile(decorations_layer, x, y, G_WOOD_BUSH)
                elif h == 28:
                    set_tile(decorations_layer, x, y, G_MOSSY_ROCK)
                elif h == 35:
                    set_tile(decorations_layer, x, y, G_LAVENDER)
                elif h == 42:
                    set_tile(decorations_layer, x, y, G_GRASS_TUFT)

    # Campfire, Crates, Barrels at Cabin Courtyard
    set_tile(decorations_layer, 58, 17, G_CAMPFIRE)
    set_tile(decorations_layer, 57, 19, G_CRATE)
    set_tile(decorations_layer, 56, 19, G_BARREL)
    set_tile(decorations_layer, 58, 20, G_STUMP)
    set_tile(decorations_layer, 31, 31, G_SIGNPOST)
    set_tile(decorations_layer, 24, 25, G_STONE_RUNE)

    # 5. Collision Boundaries (2-tile outer border)
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if x <= 1 or x >= WIDTH - 2 or y <= 1 or y >= HEIGHT - 2:
                # Keep West entrance open
                if x <= 1 and (28 <= y <= 32):
                    continue
                # Keep South exit open
                if y >= HEIGHT - 2 and (60 <= x <= 64):
                    continue
                set_tile(collision_layer, x, y, G_COLLISION_SOLID)

    # 6. Environmental Objects & Spawns
    objects = []
    obj_id = 1

    # Player Spawn Marker
    objects.append({
        "id": obj_id,
        "name": "player_spawn",
        "type": "spawn",
        "x": 256.0,
        "y": 960.0,
        "width": 32.0,
        "height": 32.0,
        "properties": [
            { "name": "facing", "type": "string", "value": "right" }
        ]
    })
    obj_id += 1

    # Abandoned Cabin (128x128)
    objects.append({
        "id": obj_id,
        "name": "Abandoned Cabin",
        "type": "cabin",
        "x": 60.0 * 32,
        "y": 14.0 * 32,
        "width": 128.0,
        "height": 128.0,
        "properties": [
            { "name": "collision", "type": "bool", "value": True },
            { "name": "collision_type", "type": "string", "value": "solid" },
            { "name": "collision_width", "type": "float", "value": 110.0 },
            { "name": "collision_height", "type": "float", "value": 55.0 }
        ]
    })
    obj_id += 1

    # Perimeter Trees (creating impassable forest edge)
    # North border trees
    for tx in range(2, WIDTH - 2, 3):
        objects.append({
            "id": obj_id,
            "name": f"Pine_Border_N_{tx}",
            "type": "pine_tree",
            "x": float(tx * 32 + 16),
            "y": float(2 * 32 + 16),
            "width": 48.0,
            "height": 80.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 24.0 },
                { "name": "collision_height", "type": "float", "value": 20.0 }
            ]
        })
        obj_id += 1

    # South border trees
    for tx in range(2, WIDTH - 6, 3):
        objects.append({
            "id": obj_id,
            "name": f"Pine_Border_S_{tx}",
            "type": "pine_tree",
            "x": float(tx * 32 + 16),
            "y": float((HEIGHT - 2) * 32),
            "width": 48.0,
            "height": 80.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 24.0 },
                { "name": "collision_height", "type": "float", "value": 20.0 }
            ]
        })
        obj_id += 1

    # West border trees
    for ty in range(3, HEIGHT - 3, 3):
        if 27 <= ty <= 33: continue # Path gap
        objects.append({
            "id": obj_id,
            "name": f"Oak_Border_W_{ty}",
            "type": "oak_tree",
            "x": float(2 * 32),
            "y": float(ty * 32),
            "width": 48.0,
            "height": 64.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 24.0 },
                { "name": "collision_height", "type": "float", "value": 16.0 }
            ]
        })
        obj_id += 1

    # East border trees
    for ty in range(3, HEIGHT - 3, 3):
        objects.append({
            "id": obj_id,
            "name": f"Oak_Border_E_{ty}",
            "type": "oak_tree",
            "x": float((WIDTH - 2) * 32),
            "y": float(ty * 32),
            "width": 48.0,
            "height": 64.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 24.0 },
                { "name": "collision_height", "type": "float", "value": 16.0 }
            ]
        })
        obj_id += 1

    # Woodland Groves inside the map
    groves = [
        # (x, y, type)
        (8, 6, "oak_tree"), (12, 5, "pine_tree"), (16, 7, "oak_tree"),
        (26, 8, "pine_tree"), (29, 12, "oak_tree"), (32, 7, "pine_tree"),
        (6, 40, "pine_tree"), (10, 44, "oak_tree"), (14, 42, "pine_tree"),
        (20, 48, "oak_tree"), (24, 46, "pine_tree"), (28, 50, "oak_tree"),
        (48, 8, "pine_tree"), (52, 6, "oak_tree"),
        (68, 26, "oak_tree"), (72, 30, "pine_tree"), (69, 36, "oak_tree"),
        (50, 48, "pine_tree"), (54, 46, "oak_tree"), (58, 52, "pine_tree")
    ]
    for gx, gy, gtype in groves:
        w = 48.0
        h = 80.0 if gtype == "pine_tree" else 64.0
        objects.append({
            "id": obj_id,
            "name": f"{gtype}_{gx}_{gy}",
            "type": gtype,
            "x": float(gx * 32 + 16),
            "y": float(gy * 32 + 16),
            "width": w,
            "height": h,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 24.0 },
                { "name": "collision_height", "type": "float", "value": 18.0 }
            ]
        })
        obj_id += 1

    # Granite Boulders
    boulders = [
        (24, 18), (8, 22), (37, 24), (48, 32), (39, 44), (52, 38)
    ]
    for bx, by in boulders:
        objects.append({
            "id": obj_id,
            "name": f"Rock_{bx}_{by}",
            "type": "rock_large",
            "x": float(bx * 32),
            "y": float(by * 32),
            "width": 64.0,
            "height": 48.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 52.0 },
                { "name": "collision_height", "type": "float", "value": 32.0 }
            ]
        })
        obj_id += 1

    # Berry Bushes
    bushes = [
        (18, 28), (28, 28), (34, 26), (36, 33), (48, 22), (64, 22), (58, 34)
    ]
    for bx, by in bushes:
        objects.append({
            "id": obj_id,
            "name": f"Bush_{bx}_{by}",
            "type": "bush_large",
            "x": float(bx * 32),
            "y": float(by * 32),
            "width": 48.0,
            "height": 36.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" },
                { "name": "collision_width", "type": "float", "value": 36.0 },
                { "name": "collision_height", "type": "float", "value": 20.0 }
            ]
        })
        obj_id += 1

    # Fences around cabin
    for fx in range(55, 66, 2):
        objects.append({
            "id": obj_id,
            "name": f"Fence_{fx}_13",
            "type": "fence",
            "x": float(fx * 32),
            "y": float(13 * 32),
            "width": 32.0,
            "height": 32.0,
            "properties": [
                { "name": "collision", "type": "bool", "value": True },
                { "name": "collision_type", "type": "string", "value": "solid" }
            ]
        })
        obj_id += 1

    # Save to JSON
    map_json = {
        "width": WIDTH,
        "height": HEIGHT,
        "tilewidth": TILE_SIZE,
        "tileheight": TILE_SIZE,
        "tilesets": [
            {
                "firstgid": 1,
                "name": "forest_tileset",
                "tilewidth": 32,
                "tileheight": 32,
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
                "width": WIDTH,
                "height": HEIGHT,
                "visible": True,
                "opacity": 1.0,
                "data": ground_layer
            },
            {
                "id": 2,
                "name": "Terrain",
                "type": "tilelayer",
                "width": WIDTH,
                "height": HEIGHT,
                "visible": True,
                "opacity": 1.0,
                "data": terrain_layer
            },
            {
                "id": 3,
                "name": "Water",
                "type": "tilelayer",
                "width": WIDTH,
                "height": HEIGHT,
                "visible": True,
                "opacity": 1.0,
                "data": water_layer
            },
            {
                "id": 4,
                "name": "Ground Decorations",
                "type": "tilelayer",
                "width": WIDTH,
                "height": HEIGHT,
                "visible": True,
                "opacity": 1.0,
                "data": decorations_layer
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
                "width": WIDTH,
                "height": HEIGHT,
                "visible": True,
                "opacity": 0.5,
                "data": collision_layer
            }
        ]
    }

    os.makedirs("maps/forest", exist_ok=True)
    with open("maps/forest/forest_01.json", "w", encoding="utf-8") as f:
        json.dump(map_json, f, indent=2)
    print(f"Generated maps/forest/forest_01.json ({len(objects)} objects, {WIDTH}x{HEIGHT} tiles)")

    # Save to TMX
    tmx_content = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<map version="1.10" tiledversion="1.10.2" orientation="orthogonal" renderorder="right-down" width="{WIDTH}" height="{HEIGHT}" tilewidth="{TILE_SIZE}" tileheight="{TILE_SIZE}">',
        f' <tileset firstgid="1" name="forest_tileset" tilewidth="32" tileheight="32" tilecount="64" columns="8">',
        '  <image source="tilesets/forest/forest_tileset.png" width="256" height="256"/>',
        ' </tileset>'
    ]

    # Layers
    layers = [
        (1, "Ground", ground_layer),
        (2, "Terrain", terrain_layer),
        (3, "Water", water_layer),
        (4, "Ground Decorations", decorations_layer)
    ]
    for lid, lname, ldata in layers:
        tmx_content.append(f' <layer id="{lid}" name="{lname}" width="{WIDTH}" height="{HEIGHT}">')
        tmx_content.append('  <data encoding="csv">')
        # Format CSV with newlines every 80 numbers
        lines = []
        for r in range(HEIGHT):
            row_slice = ldata[r * WIDTH:(r + 1) * WIDTH]
            lines.append(",".join(map(str, row_slice)))
        tmx_content.append(",\n".join(lines))
        tmx_content.append('  </data>')
        tmx_content.append(' </layer>')

    # Environmental Objects
    tmx_content.append(' <objectgroup id="5" name="Environmental Objects">')
    for obj in objects:
        tmx_content.append(f'  <object id="{obj["id"]}" name="{obj["name"]}" type="{obj["type"]}" x="{obj["x"]}" y="{obj["y"]}" width="{obj["width"]}" height="{obj["height"]}">')
        if "properties" in obj:
            tmx_content.append('   <properties>')
            for p in obj["properties"]:
                tmx_content.append(f'    <property name="{p["name"]}" type="{p["type"]}" value="{str(p["value"]).lower()}"/>')
            tmx_content.append('   </properties>')
        tmx_content.append('  </object>')
    tmx_content.append(' </objectgroup>')

    # Collision layer
    tmx_content.append(f' <layer id="6" name="Collision" width="{WIDTH}" height="{HEIGHT}">')
    tmx_content.append('  <data encoding="csv">')
    lines = []
    for r in range(HEIGHT):
        row_slice = collision_layer[r * WIDTH:(r + 1) * WIDTH]
        lines.append(",".join(map(str, row_slice)))
    tmx_content.append(",\n".join(lines))
    tmx_content.append('  </data>')
    tmx_content.append(' </layer>')

    tmx_content.append('</map>')

    with open("maps/forest/forest_01.tmx", "w", encoding="utf-8") as f:
        f.write("\n".join(tmx_content))
    print(f"Generated maps/forest/forest_01.tmx ({len(objects)} objects, {WIDTH}x{HEIGHT} tiles)")

if __name__ == "__main__":
    generate_forest_01()

