#!/usr/bin/env python3
"""
WILDBOUND — INTERIOR TILESET GENERATOR
Generates assets/tilesets/interior/interior_tileset.png
Dimensions: 256x256 px (8 columns x 8 rows of 32x32 tiles, 64 tiles total)
Conforms to Wildbound Art Bible (16-color top-down fantasy palette).
"""

from PIL import Image, ImageDraw
import os

TILE_SZ = 32
COLS = 8
ROWS = 8
WIDTH = COLS * TILE_SZ   # 256 px
HEIGHT = ROWS * TILE_SZ  # 256 px

def hex_to_rgb(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

# Art Bible Interior Palette
PALETTE = {
    # Wood Floor & Trim
    "wood_dark": hex_to_rgb("#3E2723"),
    "wood_mid": hex_to_rgb("#5D4037"),
    "wood_light": hex_to_rgb("#8D6E63"),
    "wood_highlight": hex_to_rgb("#A1887F"),
    "wood_pale": hex_to_rgb("#D7CCC8"),
    "wood_grain": hex_to_rgb("#4E342E"),

    # Stone Floor & Masonry
    "stone_dark": hex_to_rgb("#263238"),
    "stone_mid": hex_to_rgb("#37474F"),
    "stone_light": hex_to_rgb("#546E7A"),
    "stone_highlight": hex_to_rgb("#78909C"),
    "stone_pale": hex_to_rgb("#B0BEC5"),
    "mortar": hex_to_rgb("#1E272C"),

    # Rugs & Carpets
    "rug_red_dark": hex_to_rgb("#880E4F"),
    "rug_red_mid": hex_to_rgb("#C2185B"),
    "rug_red_light": hex_to_rgb("#E91E63"),
    "rug_gold_trim": hex_to_rgb("#FFD54F"),
    "rug_blue_dark": hex_to_rgb("#0D47A1"),
    "rug_blue_mid": hex_to_rgb("#1976D2"),
    "rug_blue_light": hex_to_rgb("#42A5F5"),

    # Furniture, Glass & Metal
    "iron_dark": hex_to_rgb("#212121"),
    "iron_mid": hex_to_rgb("#424242"),
    "iron_light": hex_to_rgb("#757575"),
    "brass": hex_to_rgb("#FFA000"),
    "potion_red": hex_to_rgb("#D32F2F"),
    "potion_blue": hex_to_rgb("#1976D2"),
    "potion_green": hex_to_rgb("#388E3C"),
    "sheet_white": hex_to_rgb("#ECEFF1"),
    "quilt_blue": hex_to_rgb("#303F9F"),
    "quilt_gold": hex_to_rgb("#FFB300"),
    "fireplace_ember": hex_to_rgb("#FF5722"),
    "fireplace_flame": hex_to_rgb("#FFC107"),
    "collision_red": (230, 40, 40, 140)
}

def draw_wood_floor(draw, ox, oy, variant=0):
    # Base wood fill
    base = PALETTE["wood_mid"] if variant != 1 else PALETTE["wood_dark"]
    draw.rectangle([ox, oy, ox + 31, oy + 31], fill=base)
    # Horizontal plank seams every 8 pixels
    for row in range(0, 32, 8):
        draw.line([ox, oy + row, ox + 31, oy + row], fill=PALETTE["wood_dark"])
        draw.line([ox, oy + row + 1, ox + 31, oy + row + 1], fill=PALETTE["wood_highlight"])
        # Vertical nail/joint offsets
        stagger = (row * 3) % 24 + 4
        draw.line([ox + stagger, oy + row, ox + stagger, oy + row + 7], fill=PALETTE["wood_dark"])
        # Nail dots
        draw.point([ox + stagger - 2, oy + row + 2], fill=PALETTE["iron_dark"])
        draw.point([ox + stagger + 2, oy + row + 2], fill=PALETTE["iron_dark"])
        # Subtle grain
        if variant == 2:
            draw.line([ox + 2, oy + row + 4, ox + 14, oy + row + 4], fill=PALETTE["wood_grain"])

def draw_stone_floor(draw, ox, oy, variant=0):
    # Flagstone or tile
    base = PALETTE["stone_mid"] if variant != 1 else PALETTE["stone_dark"]
    draw.rectangle([ox, oy, ox + 31, oy + 31], fill=base)
    # Slate flagstone grid
    step = 16
    for y in range(0, 32, step):
        draw.line([ox, oy + y, ox + 31, oy + y], fill=PALETTE["mortar"])
        draw.line([ox, oy + y + 1, ox + 31, oy + y + 1], fill=PALETTE["stone_highlight"])
    for x in range(0, 32, step):
        draw.line([ox + x, oy, ox + x, oy + 31], fill=PALETTE["mortar"])
        draw.line([ox + x + 1, oy, ox + x + 1, oy + 31], fill=PALETTE["stone_highlight"])
    # Texture noise
    if variant == 2:
        draw.rectangle([ox + 4, oy + 4, ox + 12, oy + 12], fill=PALETTE["stone_light"])
        draw.rectangle([ox + 20, oy + 20, ox + 28, oy + 28], fill=PALETTE["stone_light"])

def draw_wall_tile(draw, ox, oy, wall_type="wood"):
    if wall_type == "wood":
        # Heavy timber beam wall
        draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["wood_dark"])
        draw.rectangle([ox + 2, oy + 4, ox + 29, oy + 27], fill=PALETTE["wood_mid"])
        # Timber posts
        draw.line([ox, oy, ox + 31, oy], fill=PALETTE["wood_highlight"])
        draw.line([ox, oy + 31, ox + 31, oy + 31], fill=PALETTE["wood_grain"])
        draw.line([ox + 8, oy + 4, ox + 8, oy + 27], fill=PALETTE["wood_dark"])
        draw.line([ox + 24, oy + 4, ox + 24, oy + 27], fill=PALETTE["wood_dark"])
    elif wall_type == "stone":
        # Sturdy stone masonry wall
        draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["stone_dark"])
        # Bricks
        for r, y in enumerate([0, 10, 20]):
            offset = 8 if r % 2 == 1 else 0
            draw.line([ox, oy + y, ox + 31, oy + y], fill=PALETTE["mortar"])
            draw.line([ox, oy + y + 1, ox + 31, oy + y + 1], fill=PALETTE["stone_highlight"])
            for bx in range(offset, 32, 16):
                draw.line([ox + bx, oy + y, ox + bx, oy + y + 9], fill=PALETTE["mortar"])
    elif wall_type == "wall_top":
        # Wall cap / top roof edge
        draw.rectangle([ox, oy, ox + 31, oy + 15], fill=PALETTE["wood_grain"])
        draw.rectangle([ox, oy + 16, ox + 31, oy + 31], fill=PALETTE["wood_dark"])
        draw.line([ox, oy, ox + 31, oy], fill=PALETTE["wood_highlight"])
        draw.line([ox, oy + 16, ox + 31, oy + 16], fill=PALETTE["wood_mid"])

def draw_rug_tile(draw, ox, oy, part="center"):
    # Wood floor background
    draw_wood_floor(draw, ox, oy, variant=0)
    # Rug fill
    if part == "center":
        draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["rug_red_mid"])
        draw.rectangle([ox + 4, oy + 4, ox + 27, oy + 27], fill=PALETTE["rug_red_light"])
        # Diamond motif
        draw.polygon([(ox+16, oy+6), (ox+26, oy+16), (ox+16, oy+26), (ox+6, oy+16)], fill=PALETTE["rug_gold_trim"])
    elif part == "top_edge":
        draw.rectangle([ox, oy + 6, ox + 31, oy + 31], fill=PALETTE["rug_red_mid"])
        draw.line([ox, oy + 6, ox + 31, oy + 6], fill=PALETTE["rug_gold_trim"])
        draw.line([ox, oy + 7, ox + 31, oy + 7], fill=PALETTE["rug_gold_trim"])
    elif part == "bottom_edge":
        draw.rectangle([ox, oy, ox + 31, oy + 25], fill=PALETTE["rug_red_mid"])
        draw.line([ox, oy + 25, ox + 31, oy + 25], fill=PALETTE["rug_gold_trim"])
        draw.line([ox, oy + 26, ox + 31, oy + 26], fill=PALETTE["rug_gold_trim"])
    elif part == "corner_tl":
        draw.rectangle([ox + 6, oy + 6, ox + 31, oy + 31], fill=PALETTE["rug_red_mid"])
        draw.line([ox + 6, oy + 6, ox + 31, oy + 6], fill=PALETTE["rug_gold_trim"])
        draw.line([ox + 6, oy + 6, ox + 6, oy + 31], fill=PALETTE["rug_gold_trim"])

def draw_counter_tile(draw, ox, oy, part="front"):
    # Wooden counter / shop bar
    draw_wood_floor(draw, ox, oy)
    if part == "front":
        draw.rectangle([ox, oy + 4, ox + 31, oy + 28], fill=PALETTE["wood_dark"])
        draw.rectangle([ox + 2, oy + 4, ox + 29, oy + 10], fill=PALETTE["wood_highlight"]) # Countertop
        draw.rectangle([ox + 4, oy + 12, ox + 27, oy + 26], fill=PALETTE["wood_mid"])      # Front panel
        draw.line([ox, oy + 4, ox + 31, oy + 4], fill=PALETTE["wood_pale"])
    elif part == "corner":
        draw.rectangle([ox, oy + 4, ox + 26, oy + 31], fill=PALETTE["wood_dark"])
        draw.rectangle([ox + 2, oy + 4, ox + 26, oy + 10], fill=PALETTE["wood_highlight"])
        draw.rectangle([ox + 4, oy + 12, ox + 20, oy + 28], fill=PALETTE["wood_mid"])

def draw_bookshelf_tile(draw, ox, oy):
    # Sturdy oak bookcase against wall
    draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["wood_dark"])
    # 3 Shelves
    for sy in [6, 16, 26]:
        draw.line([ox + 2, oy + sy, ox + 29, oy + sy], fill=PALETTE["wood_highlight"])
    # Colorful book spines
    books = [
        (4, 7, 3, PALETTE["potion_red"]),
        (8, 7, 4, PALETTE["potion_blue"]),
        (13, 7, 3, PALETTE["potion_green"]),
        (17, 7, 5, PALETTE["rug_gold_trim"]),
        (23, 7, 4, PALETTE["wood_pale"]),
        # Lower shelf
        (4, 17, 4, PALETTE["rug_red_mid"]),
        (9, 17, 3, PALETTE["stone_highlight"]),
        (13, 17, 5, PALETTE["quilt_blue"]),
        (19, 17, 4, PALETTE["wood_mid"]),
        (24, 17, 3, PALETTE["potion_green"]),
    ]
    for bx, by, bw, col in books:
        draw.rectangle([ox + bx, oy + by, ox + bx + bw - 1, oy + by + 7], fill=col)

def draw_potion_shelf_tile(draw, ox, oy):
    # Apothecary shelf with glass jars & bottles
    draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["wood_dark"])
    for sy in [12, 26]:
        draw.line([ox + 2, oy + sy, ox + 29, oy + sy], fill=PALETTE["wood_highlight"])
    # Potions
    potions = [
        (5, 4, PALETTE["potion_red"]),
        (12, 4, PALETTE["potion_blue"]),
        (19, 4, PALETTE["potion_green"]),
        (26, 4, PALETTE["rug_gold_trim"]),
        (6, 18, PALETTE["potion_blue"]),
        (14, 18, PALETTE["potion_red"]),
        (22, 18, PALETTE["potion_green"]),
    ]
    for px, py, col in potions:
        # Bottle neck
        draw.rectangle([ox + px + 1, oy + py, ox + px + 2, oy + py + 2], fill=PALETTE["iron_light"])
        # Glass body
        draw.rectangle([ox + px, oy + py + 3, ox + px + 3, oy + py + 7], fill=col)
        draw.point([ox + px + 1, oy + py + 4], fill=PALETTE["sheet_white"]) # Specular shine

def draw_fireplace_tile(draw, ox, oy, is_top=False):
    if is_top:
        # Stone chimney hood
        draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["stone_dark"])
        draw.polygon([(ox+4, oy+31), (ox+10, oy+4), (ox+21, oy+4), (ox+27, oy+31)], fill=PALETTE["stone_mid"])
        draw.line([ox+10, oy+4, ox+21, oy+4], fill=PALETTE["stone_highlight"])
    else:
        # Fireplace hearth with glowing embers
        draw.rectangle([ox, oy, ox + 31, oy + 31], fill=PALETTE["stone_dark"])
        draw.rectangle([ox + 4, oy + 8, ox + 27, oy + 28], fill=PALETTE["iron_dark"])
        # Stone arch
        draw.line([ox + 2, oy + 8, ox + 29, oy + 8], fill=PALETTE["stone_highlight"])
        draw.line([ox + 2, oy + 8, ox + 2, oy + 28], fill=PALETTE["stone_highlight"])
        draw.line([ox + 29, oy + 8, ox + 29, oy + 28], fill=PALETTE["stone_highlight"])
        # Fire embers & logs
        draw.line([ox + 6, oy + 24, ox + 25, oy + 24], fill=PALETTE["wood_dark"])
        draw.rectangle([ox + 8, oy + 18, ox + 23, oy + 23], fill=PALETTE["fireplace_ember"])
        draw.polygon([(ox+12, oy+21), (ox+16, oy+12), (ox+20, oy+21)], fill=PALETTE["fireplace_flame"])

def draw_bed_tile(draw, ox, oy, is_top=True):
    draw_wood_floor(draw, ox, oy)
    if is_top:
        # Headboard & Pillow
        draw.rectangle([ox + 4, oy + 4, ox + 27, oy + 10], fill=PALETTE["wood_dark"]) # Headboard
        draw.line([ox + 4, oy + 4, ox + 27, oy + 4], fill=PALETTE["wood_highlight"])
        draw.rectangle([ox + 6, oy + 12, ox + 25, oy + 22], fill=PALETTE["sheet_white"]) # Pillow
        draw.rectangle([ox + 4, oy + 23, ox + 27, oy + 31], fill=PALETTE["quilt_blue"])   # Blanket start
    else:
        # Quilt & Footboard
        draw.rectangle([ox + 4, oy, ox + 27, oy + 24], fill=PALETTE["quilt_blue"])
        draw.line([ox + 4, oy + 12, ox + 27, oy + 12], fill=PALETTE["quilt_gold"])
        draw.rectangle([ox + 4, oy + 25, ox + 27, oy + 29], fill=PALETTE["wood_dark"]) # Footboard
        draw.line([ox + 4, oy + 25, ox + 27, oy + 25], fill=PALETTE["wood_highlight"])

def draw_table_tile(draw, ox, oy):
    draw_wood_floor(draw, ox, oy)
    # Oak tabletop
    draw.rectangle([ox + 2, oy + 4, ox + 29, oy + 27], fill=PALETTE["wood_highlight"])
    draw.rectangle([ox + 4, oy + 6, ox + 27, oy + 25], fill=PALETTE["wood_mid"])
    # Bevel edge
    draw.line([ox + 2, oy + 4, ox + 29, oy + 4], fill=PALETTE["wood_pale"])
    draw.line([ox + 2, oy + 27, ox + 29, oy + 27], fill=PALETTE["wood_dark"])

def draw_door_mat_tile(draw, ox, oy):
    draw_wood_floor(draw, ox, oy)
    # Woven coir welcome mat at doorway threshold
    draw.rectangle([ox + 4, oy + 6, ox + 27, oy + 25], fill=PALETTE["rug_gold_trim"])
    draw.rectangle([ox + 6, oy + 8, ox + 25, oy + 23], fill=PALETTE["wood_highlight"])
    # Texture stripes
    for my in range(10, 23, 3):
        draw.line([ox + 7, oy + my, ox + 24, oy + my], fill=PALETTE["wood_dark"])

def draw_collision_tile(draw, ox, oy):
    # Collision tile (transparent with red hazard border for collision layer)
    draw.rectangle([ox, oy, ox + 31, oy + 31], fill=(220, 20, 20, 100), outline=(255, 60, 60, 200))
    draw.line([ox, oy, ox + 31, oy + 31], fill=(255, 60, 60, 180))
    draw.line([ox, oy + 31, ox + 31, oy], fill=(255, 60, 60, 180))

def main():
    out_dir = "assets/tilesets/interior"
    os.makedirs(out_dir, exist_ok=True)
    img = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Grid Mapping (64 tiles):
    # Row 0: Wood floors & stone floors
    # Tile 1: Wood plank standard
    draw_wood_floor(draw, 0 * 32, 0 * 32, variant=0)
    # Tile 2: Wood plank dark
    draw_wood_floor(draw, 1 * 32, 0 * 32, variant=1)
    # Tile 3: Wood plank light grain
    draw_wood_floor(draw, 2 * 32, 0 * 32, variant=2)
    # Tile 4: Stone slate floor
    draw_stone_floor(draw, 3 * 32, 0 * 32, variant=0)
    # Tile 5: Stone slate dark
    draw_stone_floor(draw, 4 * 32, 0 * 32, variant=1)
    # Tile 6: Stone flagstone accent
    draw_stone_floor(draw, 5 * 32, 0 * 32, variant=2)
    # Tile 7: Door threshold / entrance mat
    draw_door_mat_tile(draw, 6 * 32, 0 * 32)
    # Tile 8: Collision tile
    draw_collision_tile(draw, 7 * 32, 0 * 32)

    # Row 1: Walls
    # Tile 9: Timber wall horizontal
    draw_wall_tile(draw, 0 * 32, 1 * 32, "wood")
    # Tile 10: Stone masonry wall
    draw_wall_tile(draw, 1 * 32, 1 * 32, "stone")
    # Tile 11: Wall top trim / cap
    draw_wall_tile(draw, 2 * 32, 1 * 32, "wall_top")
    # Tile 12..16: Variations / pillars
    draw_wall_tile(draw, 3 * 32, 1 * 32, "wood")
    draw_wall_tile(draw, 4 * 32, 1 * 32, "stone")
    draw_wall_tile(draw, 5 * 32, 1 * 32, "wood")
    draw_stone_floor(draw, 6 * 32, 1 * 32, variant=0)
    draw_stone_floor(draw, 7 * 32, 1 * 32, variant=1)

    # Row 2: Rugs & Carpets
    # Tile 17: Rug center red
    draw_rug_tile(draw, 0 * 32, 2 * 32, "center")
    # Tile 18: Rug top edge
    draw_rug_tile(draw, 1 * 32, 2 * 32, "top_edge")
    # Tile 19: Rug bottom edge
    draw_rug_tile(draw, 2 * 32, 2 * 32, "bottom_edge")
    # Tile 20: Rug corner TL
    draw_rug_tile(draw, 3 * 32, 2 * 32, "corner_tl")
    for c in range(4, 8):
        draw_rug_tile(draw, c * 32, 2 * 32, "center")

    # Row 3: Counters & Furniture
    # Tile 25: Shop Counter Front
    draw_counter_tile(draw, 0 * 32, 3 * 32, "front")
    # Tile 26: Shop Counter Corner
    draw_counter_tile(draw, 1 * 32, 3 * 32, "corner")
    # Tile 27: Table
    draw_table_tile(draw, 2 * 32, 3 * 32)
    # Tile 28: Bookshelf
    draw_bookshelf_tile(draw, 3 * 32, 3 * 32)
    # Tile 29: Potion / Apothecary Shelf
    draw_potion_shelf_tile(draw, 4 * 32, 3 * 32)
    # Tile 30: Fireplace Top Chimney
    draw_fireplace_tile(draw, 5 * 32, 3 * 32, is_top=True)
    # Tile 31: Fireplace Hearth Bottom
    draw_fireplace_tile(draw, 6 * 32, 3 * 32, is_top=False)
    # Tile 32: Bed Top
    draw_bed_tile(draw, 7 * 32, 3 * 32, is_top=True)

    # Row 4: Bed Bottom & Further Props
    # Tile 33: Bed Bottom
    draw_bed_tile(draw, 0 * 32, 4 * 32, is_top=False)
    for c in range(1, 8):
        draw_wood_floor(draw, c * 32, 4 * 32, variant=c % 3)

    # Fill remaining rows with consistent floor & wall tiles
    for r in range(5, ROWS):
        for c in range(COLS):
            if r == 7 and c == 7:
                draw_collision_tile(draw, c * 32, r * 32)
            elif (r + c) % 2 == 0:
                draw_wood_floor(draw, c * 32, r * 32, variant=(r + c) % 3)
            else:
                draw_stone_floor(draw, c * 32, r * 32, variant=(r + c) % 3)

    out_file = os.path.join(out_dir, "interior_tileset.png")
    img.save(out_file, "PNG")
    print(f"Successfully generated interior tileset: {out_file} ({WIDTH}x{HEIGHT} px, 64 tiles)")

if __name__ == "__main__":
    main()

