import os
import math
from PIL import Image, ImageDraw

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def hex_to_rgba(hex_str, alpha=255):
    rgb = hex_to_rgb(hex_str)
    return (rgb[0], rgb[1], rgb[2], alpha)

def create_forest_tileset():
    # 8 columns x 8 rows of 32x32 tiles -> 256x256
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Color Palette
    # Grass
    C_GRASS_BASE = hex_to_rgb("#3d7332")
    C_GRASS_LIGHT = hex_to_rgb("#4d8f3e")
    C_GRASS_DARK = hex_to_rgb("#2d5425")
    C_GRASS_SHADY = hex_to_rgb("#23421d")
    
    # Flowers
    C_FLOWER_YEL = hex_to_rgb("#ffd13b")
    C_FLOWER_BLU = hex_to_rgb("#5ca8ff")
    C_FLOWER_RED = hex_to_rgb("#e64545")
    C_FLOWER_WHT = hex_to_rgb("#f0f0e6")
    
    # Dirt
    C_DIRT_BASE = hex_to_rgb("#855b32")
    C_DIRT_LIGHT = hex_to_rgb("#9e6f3f")
    C_DIRT_DARK = hex_to_rgb("#634221")
    C_PEBBLE = hex_to_rgb("#808080")
    
    # Cobblestone
    C_COBBLE_BASE = hex_to_rgb("#697078")
    C_COBBLE_LIGHT = hex_to_rgb("#828b94")
    C_COBBLE_DARK = hex_to_rgb("#4d5257")
    
    # Water
    C_WATER_BASE = hex_to_rgb("#26659c")
    C_WATER_LIGHT = hex_to_rgb("#3b88cc")
    C_WATER_DARK = hex_to_rgb("#1b4870")
    C_WATER_FOAM = hex_to_rgba("#d2edfc", 220)
    
    # Wood
    C_WOOD_BASE = hex_to_rgb("#80522c")
    C_WOOD_LIGHT = hex_to_rgb("#9c6639")
    C_WOOD_DARK = hex_to_rgb("#59371c")
    C_WOOD_ROOF = hex_to_rgb("#4a2e18")

    # Helper to draw a 32x32 tile at (col, row)
    def tile_rect(c, r):
        return (c * 32, r * 32, (c + 1) * 32 - 1, (r + 1) * 32 - 1)

    # --- ROW 0: GRASS VARIANTS ---
    # Tile 0,0 (GID 1): Plain Lush Grass
    for y in range(32):
        for x in range(32):
            c = C_GRASS_BASE
            if (x * 7 + y * 13) % 11 == 0:
                c = C_GRASS_LIGHT
            elif (x * 17 + y * 5) % 13 == 0:
                c = C_GRASS_DARK
            img.putpixel((x, y), (*c, 255))
            
    # Tile 1,0 (GID 2): Grass with yellow flowers
    for y in range(32):
        for x in range(32):
            px = 32 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    # Yellow flowers at (8, 12), (22, 20), (14, 25)
    for fx, fy in [(8, 12), (22, 20), (14, 25)]:
        px = 32 + fx
        img.putpixel((px, fy), (*C_FLOWER_YEL, 255))
        img.putpixel((px+1, fy), (*C_FLOWER_YEL, 255))
        img.putpixel((px, fy+1), (*C_FLOWER_YEL, 255))
        img.putpixel((px-1, fy), (*C_FLOWER_WHT, 255))
        img.putpixel((px, fy-1), (*C_FLOWER_WHT, 255))

    # Tile 2,0 (GID 3): Grass with pebbles
    for y in range(32):
        for x in range(32):
            px = 64 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    for px, py in [(10, 15), (11, 15), (24, 8), (25, 8), (18, 26)]:
        img.putpixel((64 + px, py), (*C_PEBBLE, 255))
        img.putpixel((64 + px, py + 1), (*C_DIRT_DARK, 255))

    # Tile 3,0 (GID 4): Shady deep forest grass
    for y in range(32):
        for x in range(32):
            c = C_GRASS_DARK
            if (x * 9 + y * 17) % 7 == 0:
                c = C_GRASS_SHADY
            elif (x * 13 + y * 3) % 11 == 0:
                c = C_GRASS_BASE
            img.putpixel((96 + x, y), (*c, 255))

    # Tile 4,0 (GID 5): Grass with blue wildflowers
    for y in range(32):
        for x in range(32):
            px = 128 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    for fx, fy in [(12, 10), (20, 18), (6, 24)]:
        px = 128 + fx
        img.putpixel((px, fy), (*C_FLOWER_BLU, 255))
        img.putpixel((px+1, fy), (*C_FLOWER_BLU, 255))
        img.putpixel((px-1, fy), (*C_FLOWER_WHT, 255))

    # Tile 5,0 (GID 6): Grass with autumn red leaf
    for y in range(32):
        for x in range(32):
            px = 160 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    for lx, ly in [(14, 14), (15, 14), (16, 15), (15, 16)]:
        img.putpixel((160 + lx, ly), (*C_FLOWER_RED, 255))

    # Tile 6,0 (GID 7): Wild grass tuft
    for y in range(32):
        for x in range(32):
            px = 192 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    for tx in [14, 15, 16, 17]:
        for ty in range(12, 22):
            img.putpixel((192 + tx, ty), (*C_GRASS_LIGHT, 255))

    # Tile 7,0 (GID 8): Forest clover
    for y in range(32):
        for x in range(32):
            px = 224 + x
            img.putpixel((px, y), img.getpixel((x, y)))
    for cx, cy in [(14, 14), (16, 14), (15, 16), (15, 15)]:
        img.putpixel((224 + cx, cy), (*C_GRASS_LIGHT, 255))

    # --- ROW 1: DIRT PATHWAYS & TRANSITIONS ---
    # Tile 0,1 (GID 9): Plain Dirt Path
    for y in range(32):
        for x in range(32):
            c = C_DIRT_BASE
            if (x * 11 + y * 7) % 9 == 0:
                c = C_DIRT_LIGHT
            elif (x * 5 + y * 19) % 11 == 0:
                c = C_DIRT_DARK
            img.putpixel((x, 32 + y), (*c, 255))

    # Tile 1,1 (GID 10): Dirt with pebbles
    for y in range(32):
        for x in range(32):
            px = 32 + x
            img.putpixel((px, 32 + y), img.getpixel((x, 32 + y)))
    for px, py in [(12, 14), (13, 14), (20, 22), (21, 22)]:
        img.putpixel((32 + px, 32 + py), (*C_PEBBLE, 255))

    # Tile 2,1 (GID 11): Dirt Border North (Grass top 10px, dirt bottom 22px)
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 32 + y
            if y < 8 + ((x * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # Tile 3,1 (GID 12): Dirt Border South (Dirt top, Grass bottom)
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 32 + y
            if y > 24 - ((x * 5) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # Tile 4,1 (GID 13): Dirt Border West (Grass left, dirt right)
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 32 + y
            if x < 8 + ((y * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # Tile 5,1 (GID 14): Dirt Border East (Dirt left, grass right)
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 32 + y
            if x > 24 - ((y * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # Tile 6,1 (GID 15): Dirt Corner NW
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 32 + y
            if x + y < 16 + ((x * 3) % 4):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # Tile 7,1 (GID 16): Dirt Corner NE
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 32 + y
            if (31 - x) + y < 16 + ((x * 3) % 4):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, 32 + y)))

    # --- ROW 2: COBBLESTONE & STONE PATHS ---
    # Tile 0,2 (GID 17): Cobblestone Paving
    for y in range(32):
        for x in range(32):
            # Hexagonal/brick stone pattern
            bx = (x // 8)
            by = (y // 8)
            ox = 4 if by % 2 == 1 else 0
            lx = (x + ox) % 8
            ly = y % 8
            if lx == 0 or ly == 0:
                c = C_COBBLE_DARK
            elif lx == 1 or ly == 1:
                c = C_COBBLE_LIGHT
            else:
                c = C_COBBLE_BASE
            img.putpixel((x, 64 + y), (*c, 255))

    # Tile 1,2 (GID 18): Cobblestone with moss
    for y in range(32):
        for x in range(32):
            px = 32 + x
            py = 64 + y
            base_col = img.getpixel((x, py))
            if (x * 11 + y * 13) % 7 == 0:
                img.putpixel((px, py), (*C_GRASS_BASE, 255))
            else:
                img.putpixel((px, py), base_col)

    # Tile 2,2 (GID 19): Cobblestone Border North
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 64 + y
            if y < 8 + ((x * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 3,2 (GID 20): Cobblestone Border South
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 64 + y
            if y > 24 - ((x * 5) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 4,2 (GID 21): Cobblestone Border West
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 64 + y
            if x < 8 + ((y * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 5,2 (GID 22): Cobblestone Border East
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 64 + y
            if x > 24 - ((y * 7) % 5):
                img.putpixel((px, py), img.getpixel((x, y)))
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 6,2 (GID 23): Stone Steps
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 64 + y
            step = y // 8
            if y % 8 == 0:
                c = C_COBBLE_LIGHT
            elif y % 8 == 7:
                c = C_COBBLE_DARK
            else:
                c = C_COBBLE_BASE
            img.putpixel((px, py), (*c, 255))

    # Tile 7,2 (GID 24): Stone Well / Altar
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 64 + y
            dist = math.hypot(x - 16, y - 16)
            if dist < 6:
                img.putpixel((px, py), (*C_WATER_DARK, 255))
            elif dist < 12:
                img.putpixel((px, py), (*C_COBBLE_BASE, 255))
            elif dist < 14:
                img.putpixel((px, py), (*C_COBBLE_DARK, 255))
            else:
                img.putpixel((px, py), img.getpixel((x, y)))

    # --- ROW 3: WATER / RIVER / SHORELINES ---
    # Tile 0,3 (GID 25): Deep River Water
    for y in range(32):
        for x in range(32):
            c = C_WATER_BASE
            if (x * 7 + y * 13) % 9 == 0:
                c = C_WATER_LIGHT
            elif (x * 5 + y * 17) % 11 == 0:
                c = C_WATER_DARK
            img.putpixel((x, 96 + y), (*c, 255))

    # Tile 1,3 (GID 26): Water with gentle wave ripples
    for y in range(32):
        for x in range(32):
            px = 32 + x
            py = 96 + y
            base_w = img.getpixel((x, py))
            # Sine wave ripple highlight
            if (y == 8 or y == 20) and (x % 12 in [3, 4, 5, 6, 7]):
                img.putpixel((px, py), (*C_WATER_LIGHT, 255))
            elif (y == 9 or y == 21) and (x % 12 in [4, 5, 6]):
                img.putpixel((px, py), C_WATER_FOAM)
            else:
                img.putpixel((px, py), base_w)

    # Tile 2,3 (GID 27): Shoreline North (Grass bank top, water bottom)
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 96 + y
            edge = 8 + int(math.sin(x * 0.4) * 3)
            if y < edge:
                img.putpixel((px, py), img.getpixel((x, y)))
            elif y == edge:
                img.putpixel((px, py), (*C_DIRT_DARK, 255))
            elif y == edge + 1:
                img.putpixel((px, py), C_WATER_FOAM)
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 3,3 (GID 28): Shoreline South (Water top, grass bank bottom)
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 96 + y
            edge = 24 + int(math.sin(x * 0.4) * 3)
            if y > edge:
                img.putpixel((px, py), img.getpixel((x, y)))
            elif y == edge:
                img.putpixel((px, py), (*C_DIRT_DARK, 255))
            elif y == edge - 1:
                img.putpixel((px, py), C_WATER_FOAM)
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 4,3 (GID 29): Shoreline West (Grass left, water right)
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 96 + y
            edge = 8 + int(math.sin(y * 0.4) * 3)
            if x < edge:
                img.putpixel((px, py), img.getpixel((x, y)))
            elif x == edge:
                img.putpixel((px, py), (*C_DIRT_DARK, 255))
            elif x == edge + 1:
                img.putpixel((px, py), C_WATER_FOAM)
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 5,3 (GID 30): Shoreline East (Water left, grass right)
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 96 + y
            edge = 24 + int(math.sin(y * 0.4) * 3)
            if x > edge:
                img.putpixel((px, py), img.getpixel((x, y)))
            elif x == edge:
                img.putpixel((px, py), (*C_DIRT_DARK, 255))
            elif x == edge - 1:
                img.putpixel((px, py), C_WATER_FOAM)
            else:
                img.putpixel((px, py), img.getpixel((x, py)))

    # Tile 6,3 (GID 31): Shallow water with submerged rocks
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 96 + y
            img.putpixel((px, py), img.getpixel((x, py)))
    for rx, ry in [(10, 12), (11, 12), (20, 24), (21, 24)]:
        img.putpixel((192 + rx, 96 + ry), (*C_COBBLE_LIGHT, 200))

    # Tile 7,3 (GID 32): Water Lilies / Lily pads
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 96 + y
            img.putpixel((px, py), img.getpixel((x, py)))
    for lx, ly in [(12, 14), (13, 14), (14, 14), (13, 13), (13, 15)]:
        img.putpixel((224 + lx, 96 + ly), (*C_GRASS_LIGHT, 255))
    img.putpixel((224 + 14, 96 + 13), (*C_FLOWER_WHT, 255))

    # --- ROW 4: WOODEN BRIDGE, FENCES & STRUCTURES ---
    # Tile 0,4 (GID 33): Wooden Bridge Horizontal Plank
    for y in range(32):
        for x in range(32):
            py = 128 + y
            if y < 4 or y > 27:
                img.putpixel((x, py), (*C_WOOD_DARK, 255)) # Railings/ropes
            elif y % 6 == 0:
                img.putpixel((x, py), (*C_WOOD_DARK, 255)) # Seam
            elif y % 6 == 1:
                img.putpixel((x, py), (*C_WOOD_LIGHT, 255)) # Highlight
            else:
                img.putpixel((x, py), (*C_WOOD_BASE, 255))
            if (x == 4 or x == 27) and (y > 4 and y < 27):
                img.putpixel((x, py), (*C_COBBLE_DARK, 255)) # Iron nails

    # Tile 1,4 (GID 34): Wooden Bridge Vertical Plank
    for y in range(32):
        for x in range(32):
            px = 32 + x
            py = 128 + y
            if x < 4 or x > 27:
                img.putpixel((px, py), (*C_WOOD_DARK, 255))
            elif x % 6 == 0:
                img.putpixel((px, py), (*C_WOOD_DARK, 255))
            elif x % 6 == 1:
                img.putpixel((px, py), (*C_WOOD_LIGHT, 255))
            else:
                img.putpixel((px, py), (*C_WOOD_BASE, 255))
            if (y == 4 or y == 27) and (x > 4 and x < 27):
                img.putpixel((px, py), (*C_COBBLE_DARK, 255))

    # Tile 2,4 (GID 35): Bridge Support Post
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 128 + y
            img.putpixel((px, py), img.getpixel((x, 96 + y))) # water bg
    draw.rectangle((64 + 10, 128 + 4, 64 + 21, 128 + 28), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))

    # Tile 3,4 (GID 36): Wooden Fence Horizontal
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 128 + y
            img.putpixel((px, py), img.getpixel((x, y))) # grass bg
    draw.rectangle((96 + 0, 128 + 10, 96 + 31, 128 + 14), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.rectangle((96 + 0, 128 + 20, 96 + 31, 128 + 24), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.rectangle((96 + 13, 128 + 4, 96 + 18, 128 + 28), fill=(*C_WOOD_LIGHT, 255), outline=(*C_WOOD_DARK, 255))

    # Tile 4,4 (GID 37): Wooden Fence Post Vertical
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 128 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((128 + 12, 128 + 2, 128 + 19, 128 + 29), fill=(*C_WOOD_LIGHT, 255), outline=(*C_WOOD_DARK, 255))

    # Tile 5,4 (GID 38): Wooden Cabin Wall Planks
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 128 + y
            if y % 8 == 0:
                c = C_WOOD_DARK
            elif y % 8 == 1:
                c = C_WOOD_LIGHT
            else:
                c = C_WOOD_BASE
            img.putpixel((px, py), (*c, 255))

    # Tile 6,4 (GID 39): Wooden Cabin Window
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 128 + y
            img.putpixel((px, py), img.getpixel((160 + x, py)))
    # Golden glow window
    draw.rectangle((192 + 6, 128 + 6, 192 + 25, 128 + 25), fill=(255, 220, 120, 255), outline=(*C_WOOD_DARK, 255))
    draw.line([(192 + 15, 128 + 6), (192 + 15, 128 + 25)], fill=(*C_WOOD_DARK, 255), width=2)
    draw.line([(192 + 6, 128 + 15), (192 + 25, 128 + 15)], fill=(*C_WOOD_DARK, 255), width=2)

    # Tile 7,4 (GID 40): Wooden Cabin Doorway
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 128 + y
            img.putpixel((px, py), img.getpixel((160 + x, py)))
    draw.rectangle((224 + 6, 128 + 4, 224 + 25, 128 + 31), fill=(30, 20, 15, 255), outline=(*C_WOOD_DARK, 255))
    draw.ellipse((224 + 21, 128 + 18, 224 + 23, 128 + 20), fill=(*C_FLOWER_YEL, 255)) # door handle

    # --- ROW 5: THATCH ROOF & WOODLAND NATURE PROPS ---
    # Tile 0,5 (GID 41): Thatch / Wood Roof Tile
    for y in range(32):
        for x in range(32):
            px = x
            py = 160 + y
            if y % 6 == 0:
                c = C_WOOD_DARK
            elif y % 6 == 1:
                c = C_WOOD_LIGHT
            else:
                c = C_WOOD_ROOF
            img.putpixel((px, py), (*c, 255))

    # Tile 1,5 (GID 42): Roof Edge Overhang
    for y in range(32):
        for x in range(32):
            px = 32 + x
            py = 160 + y
            if y < 24:
                img.putpixel((px, py), img.getpixel((x, py)))
            elif y < 28:
                img.putpixel((px, py), (*C_WOOD_ROOF, 255))
            else:
                img.putpixel((px, py), (10, 10, 10, 150)) # shadow

    # Tile 2,5 (GID 43): Red Woodland Mushrooms
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    # Mushroom 1
    draw.rectangle((64 + 11, 160 + 18, 64 + 13, 160 + 26), fill=(240, 240, 230, 255))
    draw.ellipse((64 + 7, 160 + 13, 64 + 17, 160 + 19), fill=(*C_FLOWER_RED, 255))
    img.putpixel((64 + 10, 160 + 15), (255, 255, 255, 255))
    img.putpixel((64 + 14, 160 + 16), (255, 255, 255, 255))
    # Mushroom 2
    draw.rectangle((64 + 21, 160 + 21, 64 + 23, 160 + 27), fill=(240, 240, 230, 255))
    draw.ellipse((64 + 18, 160 + 17, 64 + 26, 160 + 22), fill=(*C_FLOWER_RED, 255))
    img.putpixel((64 + 21, 160 + 18), (255, 255, 255, 255))

    # Tile 3,5 (GID 44): Brown Chanterelle Mushrooms
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((96 + 14, 160 + 18, 96 + 16, 160 + 25), fill=(220, 200, 160, 255))
    draw.ellipse((96 + 11, 160 + 14, 96 + 19, 160 + 19), fill=(*C_DIRT_LIGHT, 255))

    # Tile 4,5 (GID 45): Woodland Shrub / Fern
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.ellipse((128 + 6, 160 + 8, 128 + 25, 160 + 26), fill=(*C_GRASS_LIGHT, 255), outline=(*C_GRASS_DARK, 255))
    draw.ellipse((128 + 10, 160 + 11, 128 + 21, 160 + 22), fill=(*C_GRASS_BASE, 255))

    # Tile 5,5 (GID 46): Mossy River Rock
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.ellipse((160 + 6, 160 + 10, 160 + 25, 160 + 25), fill=(*C_COBBLE_BASE, 255), outline=(*C_COBBLE_DARK, 255))
    draw.ellipse((160 + 9, 160 + 12, 160 + 18, 160 + 18), fill=(*C_GRASS_LIGHT, 255))

    # Tile 6,5 (GID 47): Tree Stump with Axe
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.ellipse((192 + 8, 160 + 12, 192 + 23, 160 + 26), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.ellipse((192 + 10, 160 + 14, 192 + 21, 160 + 20), fill=(*C_WOOD_LIGHT, 255))
    draw.line([(192 + 16, 160 + 6), (192 + 16, 160 + 16)], fill=(*C_COBBLE_LIGHT, 255), width=2)

    # Tile 7,5 (GID 48): Campfire with Logs and Embers
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 160 + y
            img.putpixel((px, py), img.getpixel((x, 32 + y))) # dirt bg
    # Stone ring
    for ang in range(0, 360, 45):
        rad = math.radians(ang)
        sx = int(224 + 16 + math.cos(rad) * 9)
        sy = int(160 + 16 + math.sin(rad) * 9)
        draw.ellipse((sx - 2, sy - 2, sx + 2, sy + 2), fill=(*C_COBBLE_BASE, 255))
    # Glowing embers
    draw.ellipse((224 + 12, 160 + 12, 224 + 20, 160 + 20), fill=(255, 100, 30, 255))
    draw.ellipse((224 + 14, 160 + 14, 224 + 18, 160 + 18), fill=(255, 230, 70, 255))

    # --- ROW 6: PROPS & CONTAINER TILES ---
    # Tile 0,6 (GID 49): Wooden Signpost
    for y in range(32):
        for x in range(32):
            px = x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((14, 192 + 12, 17, 192 + 28), fill=(*C_WOOD_DARK, 255))
    draw.rectangle((6, 192 + 6, 25, 192 + 14), fill=(*C_WOOD_LIGHT, 255), outline=(*C_WOOD_DARK, 255))

    # Tile 1,6 (GID 50): Ancient Stone Rune
    for y in range(32):
        for x in range(32):
            px = 32 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((32 + 10, 192 + 6, 32 + 21, 192 + 27), fill=(*C_COBBLE_LIGHT, 255), outline=(*C_COBBLE_DARK, 255))
    # Glowing cyan rune
    draw.line([(32 + 13, 192 + 12), (32 + 18, 192 + 12)], fill=(100, 230, 255, 255), width=2)
    draw.line([(32 + 16, 192 + 12), (32 + 16, 192 + 22)], fill=(100, 230, 255, 255), width=2)

    # Tile 2,6 (GID 51): Hollow Log
    for y in range(32):
        for x in range(32):
            px = 64 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((64 + 4, 192 + 12, 64 + 27, 192 + 22), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.ellipse((64 + 4, 192 + 12, 64 + 10, 192 + 22), fill=(20, 15, 10, 255))

    # Tile 3,6 (GID 52): Wooden Storage Crate
    for y in range(32):
        for x in range(32):
            px = 96 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.rectangle((96 + 6, 192 + 6, 96 + 25, 192 + 25), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.line([(96 + 6, 192 + 6), (96 + 25, 192 + 25)], fill=(*C_WOOD_DARK, 255), width=2)
    draw.line([(96 + 25, 192 + 6), (96 + 6, 192 + 25)], fill=(*C_WOOD_DARK, 255), width=2)

    # Tile 4,6 (GID 53): Wooden Barrel
    for y in range(32):
        for x in range(32):
            px = 128 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.ellipse((128 + 7, 192 + 5, 128 + 24, 192 + 26), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))
    draw.line([(128 + 7, 192 + 11), (128 + 24, 192 + 11)], fill=(*C_COBBLE_DARK, 255), width=2)
    draw.line([(128 + 7, 192 + 20), (128 + 24, 192 + 20)], fill=(*C_COBBLE_DARK, 255), width=2)

    # Tile 5,6 (GID 54): Lavender Bush
    for y in range(32):
        for x in range(32):
            px = 160 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, y)))
    draw.ellipse((160 + 6, 192 + 8, 160 + 25, 192 + 25), fill=(*C_GRASS_BASE, 255))
    for fx, fy in [(10, 12), (18, 14), (14, 20), (22, 18)]:
        draw.ellipse((160 + fx, 192 + fy, 160 + fx + 3, 192 + fy + 4), fill=(180, 130, 240, 255))

    # Tile 6,6 (GID 55): Sunken Chest
    for y in range(32):
        for x in range(32):
            px = 192 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, 96 + y))) # water bg
    draw.rectangle((192 + 7, 192 + 10, 192 + 24, 192 + 23), fill=(*C_WOOD_DARK, 255), outline=(*C_FLOWER_YEL, 255))

    # Tile 7,6 (GID 56): Pier Dock End
    for y in range(32):
        for x in range(32):
            px = 224 + x
            py = 192 + y
            img.putpixel((px, py), img.getpixel((x, 96 + y))) # water bg
    draw.rectangle((224 + 4, 192 + 4, 224 + 27, 192 + 20), fill=(*C_WOOD_BASE, 255), outline=(*C_WOOD_DARK, 255))

    # --- ROW 7: COLLISION & SYSTEM TILES ---
    # Tile 0,7 (GID 57): Solid Collision Overlay (semi-transparent red with outline)
    draw.rectangle((0, 224, 31, 255), fill=(220, 50, 50, 140), outline=(255, 80, 80, 240))
    draw.line([(0, 224), (31, 255)], fill=(255, 100, 100, 180), width=1)

    # Tile 1,7 (GID 58): Water Collision Overlay (semi-transparent blue with outline)
    draw.rectangle((32, 224, 63, 255), fill=(30, 100, 220, 140), outline=(60, 150, 255, 240))
    draw.line([(32, 224), (63, 255)], fill=(80, 180, 255, 180), width=1)

    # Tile 2,7 (GID 59): Trigger Overlay (semi-transparent yellow/orange)
    draw.rectangle((64, 224, 95, 255), fill=(240, 180, 20, 140), outline=(255, 210, 50, 240))

    # Tiles 3..7 row 7: Extra accent tiles
    for c in range(3, 8):
        for y in range(32):
            for x in range(32):
                img.putpixel((c * 32 + x, 224 + y), img.getpixel((x, y)))

    os.makedirs("assets/tilesets/forest", exist_ok=True)
    img.save("assets/tilesets/forest/forest_tileset.png", "PNG")
    print("Generated: assets/tilesets/forest/forest_tileset.png (256x256)")

def create_pine_tree():
    # 48x80 coniferous pine tree
    img = Image.new("RGBA", (48, 80), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Trunk
    draw.rectangle((21, 56, 26, 78), fill=(70, 45, 25, 255), outline=(45, 28, 15, 255))
    
    # Pine foliage tiers (dark forest pine greens)
    tiers = [
        # (center_y, width, height)
        (56, 44, 22),
        (42, 38, 20),
        (28, 30, 18),
        (16, 20, 16),
        (7, 10, 12),
    ]

    C_PINE_BASE = (28, 64, 38)
    C_PINE_LIGHT = (42, 92, 55)
    C_PINE_DARK = (18, 44, 26)

    for cy, w, h in tiers:
        x0 = 24 - w // 2
        x1 = 24 + w // 2
        y0 = cy - h
        y1 = cy
        # Triangle shape
        draw.polygon([(x0, y1), (24, y0), (x1, y1)], fill=(*C_PINE_BASE, 255), outline=(*C_PINE_DARK, 255))
        # Needle highlights on left side
        draw.line([(24, y0), (x0 + 4, y1 - 2)], fill=(*C_PINE_LIGHT, 255), width=2)

    os.makedirs("assets/environment/trees", exist_ok=True)
    img.save("assets/environment/trees/pine_tree.png", "PNG")
    print("Generated: assets/environment/trees/pine_tree.png (48x80)")

def create_cabin():
    # 128x128 forest cabin
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Stone chimney on the left
    draw.rectangle((16, 18, 30, 90), fill=(100, 105, 115, 255), outline=(60, 65, 75, 255))
    for y in range(22, 90, 8):
        draw.line([(16, y), (30, y)], fill=(60, 65, 75, 255), width=1)

    # Wooden cabin body
    draw.rectangle((24, 48, 112, 118), fill=(120, 75, 40, 255), outline=(65, 38, 18, 255))
    # Horizontal log siding
    for y in range(54, 118, 8):
        draw.line([(24, y), (112, y)], fill=(65, 38, 18, 255), width=2)
        draw.line([(24, y+1), (112, y+1)], fill=(145, 95, 52, 255), width=1)

    # Wooden Door
    draw.rectangle((60, 80, 82, 118), fill=(50, 30, 16, 255), outline=(35, 20, 10, 255))
    draw.ellipse((76, 98, 79, 101), fill=(255, 210, 50, 255)) # brass knob

    # Windows with warm glow
    # Window left
    draw.rectangle((34, 72, 52, 92), fill=(255, 220, 110, 255), outline=(65, 38, 18, 255))
    draw.line([(43, 72), (43, 92)], fill=(65, 38, 18, 255), width=2)
    draw.line([(34, 82), (52, 82)], fill=(65, 38, 18, 255), width=2)
    # Window right
    draw.rectangle((90, 72, 106, 92), fill=(255, 220, 110, 255), outline=(65, 38, 18, 255))
    draw.line([(98, 72), (98, 92)], fill=(65, 38, 18, 255), width=2)
    draw.line([(90, 82), (106, 82)], fill=(65, 38, 18, 255), width=2)

    # Roof (pitched A-frame)
    draw.polygon([(18, 54), (68, 12), (118, 54)], fill=(80, 48, 28, 255), outline=(50, 28, 15, 255))
    # Roof shingles shading
    for y in range(18, 54, 7):
        prog = (y - 12) / 42.0
        lx = int(68 - prog * 50)
        rx = int(68 + prog * 50)
        draw.line([(lx, y), (rx, y)], fill=(110, 68, 38, 255), width=2)

    os.makedirs("assets/environment/buildings", exist_ok=True)
    img.save("assets/environment/buildings/cabin.png", "PNG")
    print("Generated: assets/environment/buildings/cabin.png (128x128)")

def create_rock_large():
    # 64x48 granite rock
    img = Image.new("RGBA", (64, 48), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    draw.ellipse((4, 8, 60, 44), fill=(105, 112, 120, 255), outline=(60, 65, 70, 255))
    # Highlight facet
    draw.ellipse((10, 10, 42, 28), fill=(135, 142, 150, 255))
    # Moss patch
    draw.ellipse((26, 12, 54, 24), fill=(65, 115, 50, 255))
    # Cracks
    draw.line([(28, 22), (36, 32), (32, 40)], fill=(50, 54, 58, 255), width=2)

    os.makedirs("assets/environment/rocks", exist_ok=True)
    img.save("assets/environment/rocks/rock_large.png", "PNG")
    print("Generated: assets/environment/rocks/rock_large.png (64x48)")

def create_bush():
    # 48x36 lush berry bush
    img = Image.new("RGBA", (48, 36), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Shading base
    draw.ellipse((2, 6, 46, 34), fill=(40, 80, 32, 255), outline=(25, 52, 20, 255))
    # Mid clusters
    draw.ellipse((6, 8, 26, 26), fill=(65, 125, 50, 255))
    draw.ellipse((22, 10, 42, 28), fill=(65, 125, 50, 255))
    draw.ellipse((14, 6, 34, 22), fill=(80, 150, 60, 255))
    # Red berries
    for bx, by in [(12, 15), (20, 22), (28, 14), (34, 20), (18, 10)]:
        draw.ellipse((bx, by, bx + 3, by + 3), fill=(225, 45, 45, 255))

    os.makedirs("assets/environment/bushes", exist_ok=True)
    img.save("assets/environment/bushes/bush_large.png", "PNG")
    print("Generated: assets/environment/bushes/bush_large.png (48x36)")

if __name__ == "__main__":
    create_forest_tileset()
    create_pine_tree()
    create_cabin()
    create_rock_large()
    create_bush()

