#!/usr/bin/env python3
"""
WILDBOUND — ADDITIONAL BIOME TILESETS GENERATOR (PHASE 14)
Generates 4 authentic pixel-art tilesets conforming to the Wildbound Art Bible:
1. assets/tilesets/desert/desert_tileset.png   (Sunscorched Wastes)
2. assets/tilesets/snow/snow_tileset.png       (Frostfall Taiga)
3. assets/tilesets/swamp/swamp_tileset.png     (Murkwood Mire)
4. assets/tilesets/mountain/mountain_tileset.png (Iron Crag Highlands)

Dimensions: 256x256 px each (8 cols x 8 rows = 64 tiles at 32x32 px/tile).
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

# ==============================================================================
# 1. DESERT TILESET (Sunscorched Wastes)
# ==============================================================================
DESERT_PALETTE = {
    "sand_base": hex_to_rgb("#e2bb7b"),
    "sand_light": hex_to_rgb("#edd398"),
    "sand_shadow": hex_to_rgb("#cfa061"),
    "sand_ridge": hex_to_rgb("#b57b42"),
    "clay_dark": hex_to_rgb("#8c4d29"),
    "clay_crack": hex_to_rgb("#592d15"),
    "oasis_water_deep": hex_to_rgb("#1e6d78"),
    "oasis_water_surf": hex_to_rgb("#38a3a5"),
    "oasis_water_shimmer": hex_to_rgb("#80ed99"),
    "sandstone_rock": hex_to_rgb("#996335"),
    "sandstone_light": hex_to_rgb("#ba834e"),
    "ruin_brick": hex_to_rgb("#b07d53"),
    "ruin_shadow": hex_to_rgb("#664124"),
    "dry_brush": hex_to_rgb("#a88c59"),
    "cactus_green": hex_to_rgb("#7d8c52"),
    "cactus_dark": hex_to_rgb("#526135")
}

def draw_desert_tiles(draw):
    # Row 0: Sand variations (Dunes & wind ripples)
    for c in range(COLS):
        ox, oy = c * 32, 0 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sand_base"])
        # Wind ripple curves
        offset = (c * 7) % 11
        for ry in range(4 + offset, 28, 8):
            draw.arc([ox - 8, oy + ry, ox + 40, oy + ry + 8], 0, 180, fill=DESERT_PALETTE["sand_shadow"])
            draw.arc([ox - 8, oy + ry + 1, ox + 40, oy + ry + 9], 0, 180, fill=DESERT_PALETTE["sand_light"])
        if c == 1 or c == 5:
            # Dune crest line
            draw.line([ox, oy+16, ox+31, oy+12], fill=DESERT_PALETTE["sand_ridge"])
            draw.line([ox, oy+17, ox+31, oy+13], fill=DESERT_PALETTE["sand_light"])

    # Row 1: Baked cracked clay & Arid dirt
    for c in range(COLS):
        ox, oy = c * 32, 1 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["clay_dark"])
        # Cracks
        draw.line([ox+4, oy+6, ox+14, oy+16], fill=DESERT_PALETTE["clay_crack"])
        draw.line([ox+14, oy+16, ox+26, oy+12], fill=DESERT_PALETTE["clay_crack"])
        draw.line([ox+14, oy+16, ox+18, oy+26], fill=DESERT_PALETTE["clay_crack"])
        draw.line([ox+26, oy+12, ox+30, oy+20], fill=DESERT_PALETTE["clay_crack"])
        # Highlights on crack edges
        draw.point([ox+13, oy+15], fill=DESERT_PALETTE["sand_light"])
        draw.point([ox+25, oy+11], fill=DESERT_PALETTE["sand_light"])

    # Row 2: Desert Road / Caravan trail & Sand transitions
    for c in range(COLS):
        ox, oy = c * 32, 2 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sand_shadow"])
        # Packed trail with small pebbles
        for py in range(0, 32, 6):
            px = (py * 3 + c * 5) % 24 + 4
            draw.rectangle([ox+px, oy+py, ox+px+2, oy+py+1], fill=DESERT_PALETTE["sand_ridge"])

    # Row 3: Oasis Water (Deep, Shallow & Shimmers)
    for c in range(COLS):
        ox, oy = c * 32, 3 * 32
        base = DESERT_PALETTE["oasis_water_deep"] if c < 4 else DESERT_PALETTE["oasis_water_surf"]
        draw.rectangle([ox, oy, ox+31, oy+31], fill=base)
        for wy in [6, 14, 22]:
            wx = (wy * 4 + c * 8) % 20 + 4
            draw.line([ox+wx, oy+wy, ox+wx+6, oy+wy], fill=DESERT_PALETTE["oasis_water_shimmer"])

    # Row 4: Ancient Sandstone Ruins & Wall masonry
    for c in range(COLS):
        ox, oy = c * 32, 4 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sandstone_rock"])
        # Carved sandstone block seams
        for by in [0, 11, 22]:
            draw.line([ox, oy+by, ox+31, oy+by], fill=DESERT_PALETTE["ruin_shadow"])
            draw.line([ox, oy+by+1, ox+31, oy+by+1], fill=DESERT_PALETTE["sandstone_light"])
            x_off = 8 if by == 11 else 0
            for bx in range(x_off, 32, 16):
                draw.line([ox+bx, oy+by, ox+bx, oy+by+10], fill=DESERT_PALETTE["ruin_shadow"])

    # Row 5: Sandstone Pillar & Column details
    for c in range(COLS):
        ox, oy = c * 32, 5 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sand_base"])
        # Pillar shaft
        draw.rectangle([ox+8, oy, ox+23, oy+31], fill=DESERT_PALETTE["ruin_brick"])
        draw.line([ox+8, oy, ox+8, oy+31], fill=DESERT_PALETTE["sandstone_light"])
        draw.line([ox+23, oy, ox+23, oy+31], fill=DESERT_PALETTE["ruin_shadow"])
        draw.line([ox+12, oy, ox+12, oy+31], fill=DESERT_PALETTE["sandstone_light"])
        draw.line([ox+19, oy, ox+19, oy+31], fill=DESERT_PALETTE["ruin_shadow"])

    # Row 6: Desert Foliage (Dry Brush & Saguaro Cactus)
    for c in range(COLS):
        ox, oy = c * 32, 6 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sand_base"])
        if c % 2 == 0:
            # Tumbleweed / Dry thorny scrub
            draw.polygon([(ox+16, oy+6), (ox+26, oy+24), (ox+6, oy+24)], fill=DESERT_PALETTE["dry_brush"])
            draw.line([ox+16, oy+8, ox+8, oy+22], fill=DESERT_PALETTE["sand_ridge"])
            draw.line([ox+16, oy+8, ox+24, oy+22], fill=DESERT_PALETTE["sand_ridge"])
        else:
            # Saguaro Cactus trunk and arm
            draw.rectangle([ox+13, oy+4, ox+18, oy+28], fill=DESERT_PALETTE["cactus_green"])
            draw.rectangle([ox+6, oy+12, ox+13, oy+15], fill=DESERT_PALETTE["cactus_green"])
            draw.rectangle([ox+6, oy+8, ox+9, oy+14], fill=DESERT_PALETTE["cactus_green"])
            draw.line([ox+13, oy+4, ox+13, oy+28], fill=DESERT_PALETTE["cactus_dark"])

    # Row 7: Collision & Hazard tiles
    for c in range(COLS):
        ox, oy = c * 32, 7 * 32
        if c == 7:
            # Collision marker tile
            draw.rectangle([ox, oy, ox+31, oy+31], fill=(220, 40, 20, 110), outline=(255, 60, 40, 220))
            draw.line([ox, oy, ox+31, oy+31], fill=(255, 60, 40, 200))
            draw.line([ox, oy+31, ox+31, oy], fill=(255, 60, 40, 200))
        else:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=DESERT_PALETTE["sand_base"])
            # Small sand stone pebbles
            draw.ellipse([ox+10, oy+12, ox+18, oy+18], fill=DESERT_PALETTE["sandstone_rock"])

# ==============================================================================
# 2. SNOW TILESET (Frostfall Taiga)
# ==============================================================================
SNOW_PALETTE = {
    "snow_pure": hex_to_rgb("#eef3fa"),
    "snow_light": hex_to_rgb("#f5f8ff"),
    "snow_shadow": hex_to_rgb("#c5d6e8"),
    "snow_drift": hex_to_rgb("#b0c8de"),
    "ice_cyan": hex_to_rgb("#93c5d6"),
    "ice_deep": hex_to_rgb("#335f7a"),
    "ice_crack": hex_to_rgb("#e0f7fa"),
    "water_freezing": hex_to_rgb("#1e3d59"),
    "granite_frost": hex_to_rgb("#4a5463"),
    "slate_cold": hex_to_rgb("#222a36"),
    "spruce_needle": hex_to_rgb("#182924"),
    "spruce_snow": hex_to_rgb("#d4e4ec"),
    "timber_frost": hex_to_rgb("#3e352f"),
    "path_packed": hex_to_rgb("#d6e0ea")
}

def draw_snow_tiles(draw):
    # Row 0: Snow ground & powder drifts
    for c in range(COLS):
        ox, oy = c * 32, 0 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["snow_pure"])
        # Windblown powder drifts
        offset = (c * 6) % 13
        draw.arc([ox - 4, oy + 8 + offset, ox + 36, oy + 20 + offset], 180, 360, fill=SNOW_PALETTE["snow_shadow"])
        draw.arc([ox - 4, oy + 9 + offset, ox + 36, oy + 21 + offset], 180, 360, fill=SNOW_PALETTE["snow_light"])
        if c % 3 == 0:
            draw.ellipse([ox+6, oy+20, ox+14, oy+24], fill=SNOW_PALETTE["snow_drift"])

    # Row 1: Packed Snow Trails & Footpaths
    for c in range(COLS):
        ox, oy = c * 32, 1 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["path_packed"])
        draw.line([ox, oy+6, ox+31, oy+6], fill=SNOW_PALETTE["snow_shadow"])
        draw.line([ox, oy+25, ox+31, oy+25], fill=SNOW_PALETTE["snow_shadow"])
        # Boot prints
        draw.point([ox+8, oy+14], fill=SNOW_PALETTE["snow_drift"])
        draw.point([ox+16, oy+18], fill=SNOW_PALETTE["snow_drift"])
        draw.point([ox+24, oy+14], fill=SNOW_PALETTE["snow_drift"])

    # Row 2: Glacial Ice Sheets (Slippery, reflective)
    for c in range(COLS):
        ox, oy = c * 32, 2 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["ice_cyan"])
        # Fissures & crystalline reflection lines
        draw.line([ox+3, oy+5, ox+16, oy+18], fill=SNOW_PALETTE["ice_crack"])
        draw.line([ox+16, oy+18, ox+28, oy+15], fill=SNOW_PALETTE["ice_crack"])
        draw.line([ox+16, oy+18, ox+20, oy+28], fill=SNOW_PALETTE["ice_deep"])
        draw.point([ox+8, oy+8], fill=(255, 255, 255))
        draw.point([ox+22, oy+22], fill=(255, 255, 255))

    # Row 3: Freezing Water & Ice floe borders
    for c in range(COLS):
        ox, oy = c * 32, 3 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["water_freezing"])
        # Floating ice shards
        draw.polygon([(ox+6, oy+8), (ox+16, oy+6), (ox+14, oy+18), (ox+4, oy+14)], fill=SNOW_PALETTE["ice_cyan"])
        draw.line([ox+6, oy+8, ox+16, oy+6], fill=SNOW_PALETTE["snow_light"])

    # Row 4: Frozen Granite Rock Face / Cliff
    for c in range(COLS):
        ox, oy = c * 32, 4 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["granite_frost"])
        # Snow cap on rock top
        draw.rectangle([ox, oy, ox+31, oy+6], fill=SNOW_PALETTE["snow_pure"])
        draw.line([ox, oy+7, ox+31, oy+7], fill=SNOW_PALETTE["snow_shadow"])
        # Rock fracture
        draw.line([ox+10, oy+12, ox+22, oy+28], fill=SNOW_PALETTE["slate_cold"])

    # Row 5: Snow-covered Timber Wall / Cabin
    for c in range(COLS):
        ox, oy = c * 32, 5 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["timber_frost"])
        # Horizontal frost logs
        for ly in [0, 10, 20]:
            draw.line([ox, oy+ly, ox+31, oy+ly], fill=SNOW_PALETTE["slate_cold"])
            draw.line([ox, oy+ly+1, ox+31, oy+ly+1], fill=SNOW_PALETTE["snow_light"])
        # Snow drift leaning on wall bottom
        draw.polygon([(ox, oy+31), (ox+31, oy+31), (ox+31, oy+26), (ox, oy+29)], fill=SNOW_PALETTE["snow_pure"])

    # Row 6: Snow-Laden Spruce Branches
    for c in range(COLS):
        ox, oy = c * 32, 6 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["snow_pure"])
        # Dark green pine needles with heavy white snow crust
        draw.polygon([(ox+16, oy+4), (ox+28, oy+24), (ox+4, oy+24)], fill=SNOW_PALETTE["spruce_needle"])
        draw.polygon([(ox+16, oy+4), (ox+26, oy+16), (ox+6, oy+16)], fill=SNOW_PALETTE["spruce_snow"])

    # Row 7: Collision & Frozen Waterfall
    for c in range(COLS):
        ox, oy = c * 32, 7 * 32
        if c == 7:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=(220, 40, 20, 110), outline=(255, 60, 40, 220))
            draw.line([ox, oy, ox+31, oy+31], fill=(255, 60, 40, 200))
            draw.line([ox, oy+31, ox+31, oy], fill=(255, 60, 40, 200))
        else:
            # Frozen icicles hanging
            draw.rectangle([ox, oy, ox+31, oy+31], fill=SNOW_PALETTE["ice_cyan"])
            for ix in [4, 12, 20, 28]:
                draw.polygon([(ox+ix-2, oy), (ox+ix+2, oy), (ox+ix, oy+16)], fill=SNOW_PALETTE["snow_light"])

# ==============================================================================
# 3. SWAMP TILESET (Murkwood Mire)
# ==============================================================================
SWAMP_PALETTE = {
    "mud_dark": hex_to_rgb("#222119"),
    "mud_mid": hex_to_rgb("#2a221b"),
    "silt_green": hex_to_rgb("#3a4d29"),
    "algae_ooze": hex_to_rgb("#597334"),
    "bog_water_deep": hex_to_rgb("#171c14"),
    "bog_water_murk": hex_to_rgb("#2b3b20"),
    "water_slick": hex_to_rgb("#4d613e"),
    "peat_moss": hex_to_rgb("#3b3823"),
    "duckboard_wood": hex_to_rgb("#4a3b32"),
    "duckboard_rot": hex_to_rgb("#2b241e"),
    "fungus_purple": hex_to_rgb("#573459"),
    "fungus_light": hex_to_rgb("#8a5a8c"),
    "cattail_brown": hex_to_rgb("#4a2e1b"),
    "reed_green": hex_to_rgb("#6b8e23")
}

def draw_swamp_tiles(draw):
    # Row 0: Mud banks & Peat earth
    for c in range(COLS):
        ox, oy = c * 32, 0 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["mud_dark"])
        # Wet silt patches
        offset = (c * 5) % 9
        draw.ellipse([ox+4+offset, oy+6, ox+20+offset, oy+16], fill=SWAMP_PALETTE["silt_green"])
        draw.ellipse([ox+8, oy+18, ox+24, oy+26], fill=SWAMP_PALETTE["peat_moss"])

    # Row 1: Shallow Murky Water & Algae scum
    for c in range(COLS):
        ox, oy = c * 32, 1 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["bog_water_murk"])
        # Green algae swirls
        draw.arc([ox+2, oy+4, ox+28, oy+20], 45, 225, fill=SWAMP_PALETTE["algae_ooze"])
        draw.arc([ox+4, oy+14, ox+26, oy+28], 180, 360, fill=SWAMP_PALETTE["water_slick"])

    # Row 2: Deep Stagnant Bog (Impassable water)
    for c in range(COLS):
        ox, oy = c * 32, 2 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["bog_water_deep"])
        for sy in [8, 20]:
            sx = (sy * 3 + c * 7) % 20 + 6
            draw.ellipse([ox+sx, oy+sy, ox+sx+3, oy+sy+2], fill=SWAMP_PALETTE["algae_ooze"])

    # Row 3: Wooden Duckboards & Walkways (Safe traversal)
    for c in range(COLS):
        ox, oy = c * 32, 3 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["bog_water_murk"])
        # Rickety timber planks across walkway
        draw.rectangle([ox+2, oy+4, ox+29, oy+27], fill=SWAMP_PALETTE["duckboard_wood"])
        for py in range(4, 28, 6):
            draw.line([ox+2, oy+py, ox+29, oy+py], fill=SWAMP_PALETTE["duckboard_rot"])
            draw.point([ox+4, oy+py+2], fill=(20, 20, 20))
            draw.point([ox+27, oy+py+2], fill=(20, 20, 20))

    # Row 4: Rotten Willow Stumps & Cypress Roots
    for c in range(COLS):
        ox, oy = c * 32, 4 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["mud_dark"])
        # Gnarled hollow stump
        draw.ellipse([ox+6, oy+8, ox+25, oy+24], fill=SWAMP_PALETTE["duckboard_rot"])
        draw.ellipse([ox+10, oy+11, ox+21, oy+21], fill=(15, 12, 10)) # Hollow hole
        # Moss ring
        draw.arc([ox+6, oy+8, ox+25, oy+24], 0, 180, fill=SWAMP_PALETTE["algae_ooze"])

    # Row 5: Luminescent Swamp Mushrooms
    for c in range(COLS):
        ox, oy = c * 32, 5 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["mud_dark"])
        # Glowing purple fungus cluster
        for mx, my in [(8, 12), (18, 16), (12, 22)]:
            draw.ellipse([ox+mx-4, oy+my-3, ox+mx+4, oy+my+3], fill=SWAMP_PALETTE["fungus_purple"])
            draw.ellipse([ox+mx-2, oy+my-2, ox+mx+2, oy+my], fill=SWAMP_PALETTE["fungus_light"])
            draw.line([ox+mx, oy+my, ox+mx, oy+my+4], fill=(200, 200, 200))

    # Row 6: Swamp Cattails & Reeds
    for c in range(COLS):
        ox, oy = c * 32, 6 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["silt_green"])
        # Tall green stalks
        for rx in [8, 16, 24]:
            draw.line([ox+rx, oy+28, ox+rx, oy+8], fill=SWAMP_PALETTE["reed_green"])
            # Brown cattail head
            draw.rectangle([ox+rx-1, oy+8, ox+rx+1, oy+16], fill=SWAMP_PALETTE["cattail_brown"])

    # Row 7: Collision Tile
    for c in range(COLS):
        ox, oy = c * 32, 7 * 32
        if c == 7:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=(220, 40, 20, 110), outline=(255, 60, 40, 220))
            draw.line([ox, oy, ox+31, oy+31], fill=(255, 60, 40, 200))
            draw.line([ox, oy+31, ox+31, oy], fill=(255, 60, 40, 200))
        else:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=SWAMP_PALETTE["mud_dark"])
            draw.point([ox+16, oy+16], fill=SWAMP_PALETTE["algae_ooze"])

# ==============================================================================
# 4. MOUNTAIN TILESET (Iron Crag Highlands)
# ==============================================================================
MOUNTAIN_PALETTE = {
    "granite_mid": hex_to_rgb("#5a636e"),
    "granite_light": hex_to_rgb("#7a8594"),
    "granite_dark": hex_to_rgb("#3d434d"),
    "basalt_dark": hex_to_rgb("#2b2f36"),
    "ore_rust": hex_to_rgb("#914b30"),
    "ore_copper": hex_to_rgb("#b86b40"),
    "scree_gravel": hex_to_rgb("#6b7380"),
    "heather_turf": hex_to_rgb("#8c8a56"),
    "heather_bloom": hex_to_rgb("#784c68"),
    "chasm_shadow": hex_to_rgb("#141724"),
    "water_cascade": hex_to_rgb("#64b5f6"),
    "water_foam": hex_to_rgb("#ffffff")
}

def draw_mountain_tiles(draw):
    # Row 0: Solid Granite Terrace
    for c in range(COLS):
        ox, oy = c * 32, 0 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["granite_mid"])
        # Rock fracture seams
        offset = (c * 7) % 11
        draw.line([ox, oy+10+offset, ox+31, oy+10+offset], fill=MOUNTAIN_PALETTE["granite_dark"])
        draw.line([ox, oy+11+offset, ox+31, oy+11+offset], fill=MOUNTAIN_PALETTE["granite_light"])
        if c % 2 == 1:
            draw.line([ox+16, oy, ox+16, oy+31], fill=MOUNTAIN_PALETTE["granite_dark"])

    # Row 1: Alpine Heather Turf & Mountain Moss
    for c in range(COLS):
        ox, oy = c * 32, 1 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["heather_turf"])
        # Purple heather blossom tufts
        for hx, hy in [(6, 8), (20, 10), (12, 22), (24, 24)]:
            draw.rectangle([ox+hx, oy+hy, ox+hx+2, oy+hy+2], fill=MOUNTAIN_PALETTE["heather_bloom"])

    # Row 2: Scree & Mountain Gravel Trail
    for c in range(COLS):
        ox, oy = c * 32, 2 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["scree_gravel"])
        # Scattered broken shale & pebbles
        for py in range(2, 30, 7):
            px = (py * 3 + c * 9) % 24 + 4
            draw.polygon([(ox+px, oy+py), (ox+px+3, oy+py+1), (ox+px+1, oy+py+3)], fill=MOUNTAIN_PALETTE["granite_dark"])

    # Row 3: Vertical Cliff Wall Face
    for c in range(COLS):
        ox, oy = c * 32, 3 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["basalt_dark"])
        # Stepped rock ledges with highlights
        for ly in [0, 10, 20]:
            draw.line([ox, oy+ly, ox+31, oy+ly], fill=MOUNTAIN_PALETTE["granite_light"])
            draw.line([ox, oy+ly+1, ox+31, oy+ly+1], fill=MOUNTAIN_PALETTE["granite_mid"])
            draw.line([ox, oy+ly+9, ox+31, oy+ly+9], fill=MOUNTAIN_PALETTE["chasm_shadow"])

    # Row 4: Iron Ore Veins in Granite
    for c in range(COLS):
        ox, oy = c * 32, 4 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["granite_mid"])
        # Rust and copper ore seam
        draw.line([ox+4, oy+8, ox+18, oy+18], fill=MOUNTAIN_PALETTE["ore_rust"])
        draw.line([ox+18, oy+18, ox+28, oy+26], fill=MOUNTAIN_PALETTE["ore_rust"])
        draw.point([ox+10, oy+12], fill=MOUNTAIN_PALETTE["ore_copper"])
        draw.point([ox+22, oy+21], fill=MOUNTAIN_PALETTE["ore_copper"])

    # Row 5: Mountain Torrent / Waterfall Cascade
    for c in range(COLS):
        ox, oy = c * 32, 5 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["water_cascade"])
        # Vertical white water foam streaks
        for wx in [4, 12, 20, 28]:
            draw.line([ox+wx, oy, ox+wx, oy+31], fill=MOUNTAIN_PALETTE["water_foam"])
            draw.point([ox+wx+1, oy+14], fill=MOUNTAIN_PALETTE["water_foam"])

    # Row 6: Cave Entrance Portal / Archway
    for c in range(COLS):
        ox, oy = c * 32, 6 * 32
        draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["granite_mid"])
        # Dark yawning cave void
        draw.rectangle([ox+6, oy+6, ox+25, oy+31], fill=MOUNTAIN_PALETTE["chasm_shadow"])
        # Arch stone lintel
        draw.arc([ox+6, oy+4, ox+25, oy+16], 180, 360, fill=MOUNTAIN_PALETTE["granite_light"])

    # Row 7: Collision Tile
    for c in range(COLS):
        ox, oy = c * 32, 7 * 32
        if c == 7:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=(220, 40, 20, 110), outline=(255, 60, 40, 220))
            draw.line([ox, oy, ox+31, oy+31], fill=(255, 60, 40, 200))
            draw.line([ox, oy+31, ox+31, oy], fill=(255, 60, 40, 200))
        else:
            draw.rectangle([ox, oy, ox+31, oy+31], fill=MOUNTAIN_PALETTE["granite_mid"])
            draw.point([ox+16, oy+16], fill=MOUNTAIN_PALETTE["granite_dark"])

def generate_tileset(name, draw_fn, out_path):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    img = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw_fn(draw)
    img.save(out_path, "PNG")
    print(f"Generated {name} tileset: {out_path} ({WIDTH}x{HEIGHT} px, 64 tiles)")

def main():
    print("Generating Wildbound Phase 14 Biome Tilesets...")
    generate_tileset("Desert", draw_desert_tiles, "assets/tilesets/desert/desert_tileset.png")
    generate_tileset("Snow", draw_snow_tiles, "assets/tilesets/snow/snow_tileset.png")
    generate_tileset("Swamp", draw_swamp_tiles, "assets/tilesets/swamp/swamp_tileset.png")
    generate_tileset("Mountain", draw_mountain_tiles, "assets/tilesets/mountain/mountain_tileset.png")
    print("All 4 biome tilesets generated successfully.")

if __name__ == "__main__":
    main()

