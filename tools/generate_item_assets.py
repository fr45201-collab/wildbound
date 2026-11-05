#!/usr/bin/env python3
"""
Wildbound Phase 16 — Item Asset Generation Tool
Generates 40 distinct items across 5 categories:
  - 8 Collectibles
  - 10 Consumables
  - 8 Crafting Materials
  - 7 Natural Resources
  - 7 Special Items

Each item receives:
  - 64x64 Inventory Icon (transparent background, circular UI badge frame, clear centered silhouette)
  - 32x32 World Representation (top-down perspective, transparent background, contact drop shadow)
"""

import os
import math
from PIL import Image, ImageDraw

def create_dirs(base="assets/items"):
    subdirs = [
        "icons",
        "world",
        "collectibles",
        "consumables",
        "materials",
        "resources",
        "special"
    ]
    for d in subdirs:
        os.makedirs(os.path.join(base, d), exist_ok=True)

def draw_badge(draw):
    # Circular UI badge framing
    draw.ellipse([6, 6, 58, 58], fill=(22, 26, 34, 160), outline=(45, 52, 66, 200), width=1)

def draw_shadow_32(draw):
    # Contact drop shadow for 32x32 world items
    draw.ellipse([6, 20, 26, 26], fill=(12, 15, 22, 120))

def draw_shadow_64(draw, bbox):
    # Subtle drop shadow for 64x64 icons
    ox, oy = 2, 2
    b = [bbox[0]+ox, bbox[1]+oy, bbox[2]+ox, bbox[3]+oy]
    draw.ellipse(b, fill=(10, 12, 18, 120))

# -------------------------------------------------------------
# Drawing functions for each item (icon 64x64, world 32x32)
# -------------------------------------------------------------

def gen_coin(draw_icon, draw_world, gold=True):
    fill = (255, 215, 0, 255) if gold else (220, 220, 225, 255)
    hi = (255, 245, 170, 255) if gold else (255, 255, 255, 255)
    sh = (180, 130, 0, 255) if gold else (140, 145, 150, 255)
    edge = (130, 90, 0, 255) if gold else (100, 105, 110, 255)

    # Icon (64x64)
    draw_badge(draw_icon)
    draw_icon.ellipse([18, 18, 46, 46], fill=edge)
    draw_icon.ellipse([20, 20, 44, 44], fill=fill, outline=sh, width=2)
    draw_icon.ellipse([22, 22, 42, 42], outline=hi, width=1)
    # Stamped emblem in center
    draw_icon.ellipse([28, 28, 36, 36], outline=sh, width=1)
    draw_icon.point([32, 32], fill=hi)

    # World (32x32)
    draw_shadow_32(draw_world)
    draw_world.ellipse([10, 14, 22, 22], fill=edge)
    draw_world.ellipse([11, 15, 21, 21], fill=fill)
    draw_world.point([15, 17], fill=hi)

def gen_gem(draw_icon, draw_world, color_type):
    # color_type: 'ruby', 'sapphire', 'emerald'
    if color_type == 'ruby':
        c_main, c_hi, c_sh = (220, 20, 60, 255), (255, 120, 140, 255), (139, 0, 0, 255)
    elif color_type == 'sapphire':
        c_main, c_hi, c_sh = (30, 144, 255, 255), (150, 210, 255, 255), (0, 0, 139, 255)
    else: # emerald
        c_main, c_hi, c_sh = (46, 204, 113, 255), (160, 250, 190, 255), (20, 110, 55, 255)

    # Icon 64x64
    draw_badge(draw_icon)
    # Faceted gem polygon
    if color_type == 'emerald': # Step cut rectangle
        draw_icon.polygon([(22, 18), (42, 18), (48, 26), (48, 38), (42, 46), (22, 46), (16, 38), (16, 26)], fill=c_sh)
        draw_icon.polygon([(24, 20), (40, 20), (45, 27), (45, 37), (40, 44), (24, 44), (19, 37), (19, 27)], fill=c_main)
        draw_icon.rectangle([25, 24, 39, 38], fill=c_hi)
        draw_icon.rectangle([27, 26, 37, 36], fill=c_main)
    else: # Brilliant cut diamond / heart
        draw_icon.polygon([(20, 22), (44, 22), (50, 30), (32, 50), (14, 30)], fill=c_sh)
        draw_icon.polygon([(22, 23), (42, 23), (47, 29), (32, 47), (17, 29)], fill=c_main)
        draw_icon.polygon([(26, 25), (38, 25), (42, 29), (32, 40), (22, 29)], fill=c_hi)
        draw_icon.polygon([(28, 27), (36, 27), (32, 36)], fill=(255, 255, 255, 230))

    # World 32x32
    draw_shadow_32(draw_world)
    draw_world.polygon([(12, 13), (20, 13), (23, 17), (16, 24), (9, 17)], fill=c_sh)
    draw_world.polygon([(13, 14), (19, 14), (21, 17), (16, 22), (11, 17)], fill=c_main)
    draw_world.point([15, 15], fill=c_hi)

def gen_crystal(draw_icon, draw_world):
    # Ancient Crystal Cluster: Violet and Cyan
    draw_badge(draw_icon)
    # 3 crystal shards
    draw_icon.polygon([(32, 14), (37, 28), (32, 48), (27, 28)], fill=(155, 89, 182, 255))
    draw_icon.polygon([(32, 14), (35, 28), (32, 48)], fill=(0, 229, 255, 255)) # cyan reflection
    draw_icon.polygon([(22, 24), (28, 34), (24, 48), (18, 38)], fill=(120, 60, 160, 255))
    draw_icon.polygon([(42, 22), (46, 34), (39, 48), (36, 32)], fill=(180, 120, 210, 255))
    draw_icon.point([32, 20], fill=(255, 255, 255, 255))

    draw_shadow_32(draw_world)
    draw_world.polygon([(16, 10), (19, 16), (16, 23), (13, 16)], fill=(155, 89, 182, 255))
    draw_world.polygon([(16, 10), (18, 16), (16, 23)], fill=(0, 229, 255, 255))
    draw_world.polygon([(11, 15), (14, 20), (12, 23)], fill=(120, 60, 160, 255))

def gen_relic(draw_icon, draw_world):
    # Ancient Bronze Relic Chalice with ruby
    draw_badge(draw_icon)
    draw_icon.polygon([(20, 20), (44, 20), (38, 34), (26, 34)], fill=(197, 160, 89, 255))
    draw_icon.rectangle([30, 34, 34, 44], fill=(140, 105, 50, 255)) # stem
    draw_icon.ellipse([24, 42, 40, 48], fill=(197, 160, 89, 255)) # base
    draw_icon.ellipse([30, 24, 34, 28], fill=(220, 20, 60, 255)) # gem

    draw_shadow_32(draw_world)
    draw_world.polygon([(12, 13), (20, 13), (18, 19), (14, 19)], fill=(197, 160, 89, 255))
    draw_world.rectangle([15, 19, 17, 23], fill=(140, 105, 50, 255))
    draw_world.ellipse([13, 22, 19, 25], fill=(197, 160, 89, 255))

def gen_emblem(draw_icon, draw_world):
    # Golden Emblem (Radiating Sun)
    draw_badge(draw_icon)
    draw_icon.ellipse([16, 16, 48, 48], fill=(180, 120, 0, 255))
    # Sun rays
    for i in range(8):
        a = i * 3.14159 / 4
        x1 = 32 + int(14 * 1.3 * math.cos(a))
        y1 = 32 + int(14 * 1.3 * math.sin(a))
        draw_icon.line([32, 32, x1, y1], fill=(255, 215, 0, 255), width=3)
    draw_icon.ellipse([22, 22, 42, 42], fill=(255, 215, 0, 255), outline=(255, 245, 170, 255), width=2)
    draw_icon.point([32, 32], fill=(255, 255, 255, 255))

    draw_shadow_32(draw_world)
    draw_world.ellipse([10, 13, 22, 23], fill=(255, 215, 0, 255), outline=(180, 120, 0, 255), width=1)

def gen_potion(draw_icon, draw_world, p_type):
    # p_type: 'health_small', 'health_large', 'mana', 'stamina', 'antidote'
    draw_badge(draw_icon)
    if p_type == 'health_small':
        liq, cork = (231, 76, 60, 255), (160, 110, 60, 255)
        # Round bottle
        draw_icon.ellipse([20, 24, 44, 48], fill=(40, 50, 65, 200), outline=(180, 200, 220, 255), width=2)
        draw_icon.ellipse([22, 26, 42, 46], fill=liq)
        draw_icon.rectangle([28, 16, 36, 24], fill=(180, 200, 220, 255))
        draw_icon.rectangle([27, 12, 37, 17], fill=cork)
        draw_icon.point([26, 30], fill=(255, 255, 255, 255)) # shine
    elif p_type == 'health_large':
        liq, cork = (200, 30, 20, 255), (140, 95, 45, 255)
        # Tall ornate decanter
        draw_icon.polygon([(26, 20), (38, 20), (46, 34), (44, 50), (20, 50), (18, 34)], fill=(40, 50, 65, 200), outline=(220, 230, 240, 255), width=2)
        draw_icon.polygon([(27, 26), (37, 26), (43, 35), (42, 48), (22, 48), (21, 35)], fill=liq)
        # Silver bands
        draw_icon.line([20, 38, 44, 38], fill=(220, 225, 235, 255), width=2)
        draw_icon.rectangle([28, 14, 36, 20], fill=(220, 230, 240, 255))
        draw_icon.rectangle([27, 10, 37, 15], fill=cork)
        draw_icon.point([24, 32], fill=(255, 255, 255, 255))
    elif p_type == 'mana':
        liq, cork = (41, 128, 185, 255), (160, 110, 60, 255)
        # Cylindrical phial
        draw_icon.rectangle([24, 22, 40, 50], fill=(40, 50, 65, 200), outline=(180, 200, 220, 255), width=2)
        draw_icon.rectangle([26, 26, 38, 48], fill=liq)
        draw_icon.rectangle([28, 16, 36, 22], fill=(180, 200, 220, 255))
        draw_icon.rectangle([27, 12, 37, 17], fill=cork)
        draw_icon.point([28, 30], fill=(150, 220, 255, 255))
    elif p_type == 'stamina':
        liq, cork = (46, 204, 113, 255), (160, 110, 60, 255)
        # Triangular flask
        draw_icon.polygon([(32, 20), (46, 48), (18, 48)], fill=(40, 50, 65, 200), outline=(180, 200, 220, 255), width=2)
        draw_icon.polygon([(32, 26), (43, 46), (21, 46)], fill=liq)
        draw_icon.rectangle([29, 16, 35, 22], fill=(180, 200, 220, 255))
        draw_icon.rectangle([28, 12, 36, 17], fill=cork)
        draw_icon.point([28, 38], fill=(255, 255, 255, 255))
    else: # antidote
        liq, cork = (26, 188, 156, 255), (160, 110, 60, 255)
        # Square vial
        draw_icon.rectangle([22, 24, 42, 48], fill=(40, 50, 65, 200), outline=(180, 200, 220, 255), width=2)
        draw_icon.rectangle([24, 28, 40, 46], fill=liq)
        draw_icon.rectangle([28, 16, 36, 24], fill=(180, 200, 220, 255))
        draw_icon.rectangle([27, 12, 37, 17], fill=cork)
        draw_icon.point([27, 32], fill=(200, 255, 230, 255))

    # World 32x32
    draw_shadow_32(draw_world)
    draw_world.rectangle([12, 14, 20, 24], fill=(40, 50, 65, 200), outline=(180, 200, 220, 255), width=1)
    draw_world.rectangle([13, 17, 19, 23], fill=liq)
    draw_world.rectangle([14, 11, 18, 14], fill=(160, 110, 60, 255))

def gen_food(draw_icon, draw_world, f_type):
    # f_type: 'herb', 'bread', 'apple', 'meat', 'flask'
    draw_badge(draw_icon)
    if f_type == 'herb':
        draw_icon.line([24, 46, 38, 20], fill=(39, 174, 96, 255), width=3)
        draw_icon.ellipse([20, 22, 32, 32], fill=(46, 204, 113, 255))
        draw_icon.ellipse([30, 18, 42, 28], fill=(46, 204, 113, 255))
        draw_icon.ellipse([34, 28, 44, 38], fill=(39, 174, 96, 255))
        draw_icon.rectangle([27, 38, 33, 42], fill=(210, 180, 140, 255)) # twine
        
        draw_shadow_32(draw_world)
        draw_world.line([13, 23, 19, 13], fill=(39, 174, 96, 255), width=2)
        draw_world.ellipse([11, 14, 17, 19], fill=(46, 204, 113, 255))
    elif f_type == 'bread':
        draw_icon.ellipse([16, 22, 48, 44], fill=(160, 80, 20, 255))
        draw_icon.ellipse([17, 21, 47, 41], fill=(230, 126, 34, 255))
        # Scored slits
        draw_icon.line([24, 27, 27, 35], fill=(250, 210, 170, 255), width=2)
        draw_icon.line([31, 27, 34, 35], fill=(250, 210, 170, 255), width=2)
        draw_icon.line([38, 27, 41, 35], fill=(250, 210, 170, 255), width=2)

        draw_shadow_32(draw_world)
        draw_world.ellipse([9, 14, 23, 22], fill=(230, 126, 34, 255))
        draw_world.line([14, 16, 15, 19], fill=(250, 210, 170, 255), width=1)
        draw_world.line([18, 16, 19, 19], fill=(250, 210, 170, 255), width=1)
    elif f_type == 'apple':
        draw_icon.ellipse([18, 22, 46, 50], fill=(192, 57, 43, 255))
        draw_icon.ellipse([20, 20, 44, 48], fill=(231, 76, 60, 255))
        draw_icon.line([32, 14, 32, 22], fill=(120, 70, 30, 255), width=2) # stem
        draw_icon.ellipse([32, 14, 40, 20], fill=(46, 204, 113, 255)) # leaf
        draw_icon.point([26, 28], fill=(255, 180, 180, 255))

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 14, 21, 23], fill=(231, 76, 60, 255))
        draw_world.line([16, 11, 16, 14], fill=(120, 70, 30, 255), width=1)
    elif f_type == 'meat':
        # Shank on bone
        draw_icon.ellipse([18, 22, 42, 44], fill=(186, 74, 0, 255))
        draw_icon.ellipse([20, 20, 40, 42], fill=(211, 84, 0, 255))
        # Bone
        draw_icon.line([38, 38, 48, 48], fill=(245, 235, 225, 255), width=4)
        draw_icon.ellipse([46, 44, 50, 48], fill=(245, 235, 225, 255))
        draw_icon.ellipse([44, 46, 48, 50], fill=(245, 235, 225, 255))
        # Sear marks
        draw_icon.line([24, 26, 32, 34], fill=(100, 30, 10, 255), width=2)
        draw_icon.line([28, 24, 36, 32], fill=(100, 30, 10, 255), width=2)

        draw_shadow_32(draw_world)
        draw_world.ellipse([10, 14, 21, 22], fill=(211, 84, 0, 255))
        draw_world.line([19, 19, 24, 24], fill=(245, 235, 225, 255), width=2)
    else: # flask / canteen
        draw_icon.ellipse([18, 22, 46, 48], fill=(120, 66, 18, 255))
        draw_icon.ellipse([20, 24, 44, 46], fill=(160, 82, 45, 255))
        draw_icon.rectangle([28, 14, 36, 22], fill=(197, 160, 89, 255)) # brass cap
        draw_icon.arc([18, 22, 46, 48], 45, 225, fill=(93, 173, 226, 255), width=2)

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 14, 21, 22], fill=(160, 82, 45, 255))
        draw_world.rectangle([15, 11, 17, 14], fill=(197, 160, 89, 255))

def gen_material(draw_icon, draw_world, m_type):
    draw_badge(draw_icon)
    if m_type == 'wood':
        # 3 cut logs stack
        draw_icon.rectangle([16, 30, 48, 42], fill=(120, 66, 18, 255))
        draw_icon.ellipse([14, 30, 22, 42], fill=(229, 152, 102, 255))
        draw_icon.rectangle([20, 20, 46, 32], fill=(140, 80, 25, 255))
        draw_icon.ellipse([18, 20, 26, 32], fill=(229, 152, 102, 255))
        
        draw_shadow_32(draw_world)
        draw_world.rectangle([9, 16, 23, 22], fill=(120, 66, 18, 255))
        draw_world.ellipse([8, 16, 13, 22], fill=(229, 152, 102, 255))
    elif m_type == 'stone':
        # Chiseled masonry block
        draw_icon.polygon([(18, 26), (34, 18), (48, 26), (48, 44), (32, 50), (18, 42)], fill=(127, 140, 141, 255))
        draw_icon.polygon([(18, 26), (34, 18), (48, 26), (32, 33)], fill=(189, 195, 199, 255)) # top face
        draw_icon.polygon([(18, 26), (32, 33), (32, 50), (18, 42)], fill=(149, 165, 166, 255)) # left face

        draw_shadow_32(draw_world)
        draw_world.polygon([(10, 16), (18, 12), (24, 16), (24, 23), (17, 26), (10, 22)], fill=(127, 140, 141, 255))
        draw_world.polygon([(10, 16), (18, 12), (24, 16), (17, 19)], fill=(189, 195, 199, 255))
    elif m_type == 'iron_ore':
        draw_icon.polygon([(20, 20), (38, 16), (48, 28), (44, 46), (24, 48), (14, 34)], fill=(66, 73, 73, 255))
        # Vein glints
        draw_icon.polygon([(26, 24), (34, 22), (32, 30), (24, 32)], fill=(166, 172, 175, 255))
        draw_icon.polygon([(34, 34), (42, 30), (40, 40), (32, 42)], fill=(166, 172, 175, 255))

        draw_shadow_32(draw_world)
        draw_world.polygon([(11, 14), (20, 12), (23, 18), (20, 23), (12, 22)], fill=(66, 73, 73, 255))
        draw_world.point([15, 16], fill=(166, 172, 175, 255))
    elif m_type == 'gold_ore':
        draw_icon.polygon([(20, 20), (38, 16), (48, 28), (44, 46), (24, 48), (14, 34)], fill=(93, 109, 126, 255))
        # Gold veins
        draw_icon.polygon([(26, 24), (34, 22), (32, 30), (24, 32)], fill=(241, 196, 15, 255))
        draw_icon.polygon([(34, 34), (42, 30), (40, 40), (32, 42)], fill=(243, 156, 18, 255))
        draw_icon.point([30, 26], fill=(255, 255, 255, 255))

        draw_shadow_32(draw_world)
        draw_world.polygon([(11, 14), (20, 12), (23, 18), (20, 23), (12, 22)], fill=(93, 109, 126, 255))
        draw_world.point([15, 16], fill=(241, 196, 15, 255))
    elif m_type == 'leather':
        # Rolled hide pelt
        draw_icon.ellipse([18, 20, 46, 46], fill=(160, 64, 0, 255))
        draw_icon.ellipse([20, 22, 44, 44], fill=(211, 84, 0, 255))
        draw_icon.line([20, 32, 44, 32], fill=(237, 187, 153, 255), width=3) # rope wrap

        draw_shadow_32(draw_world)
        draw_world.ellipse([10, 14, 22, 22], fill=(211, 84, 0, 255))
        draw_world.line([10, 18, 22, 18], fill=(237, 187, 153, 255), width=2)
    elif m_type == 'cloth':
        # Rolled bolt of linen
        draw_icon.rectangle([18, 22, 46, 44], fill=(213, 216, 220, 255))
        draw_icon.rectangle([20, 24, 44, 42], fill=(234, 236, 238, 255))
        draw_icon.ellipse([16, 22, 22, 44], fill=(171, 178, 185, 255))

        draw_shadow_32(draw_world)
        draw_world.rectangle([10, 14, 22, 22], fill=(234, 236, 238, 255))
        draw_world.ellipse([9, 14, 13, 22], fill=(171, 178, 185, 255))
    elif m_type == 'iron_ingot':
        # Trapezoidal metal bar
        draw_icon.polygon([(20, 26), (44, 26), (50, 42), (14, 42)], fill=(93, 109, 126, 255))
        draw_icon.polygon([(22, 28), (42, 28), (47, 40), (17, 40)], fill=(133, 146, 158, 255))
        draw_icon.line([22, 29, 42, 29], fill=(213, 219, 219, 255), width=1)

        draw_shadow_32(draw_world)
        draw_world.polygon([(11, 16), (21, 16), (24, 22), (8, 22)], fill=(133, 146, 158, 255))
    else: # gold_ingot
        draw_icon.polygon([(20, 26), (44, 26), (50, 42), (14, 42)], fill=(183, 149, 11, 255))
        draw_icon.polygon([(22, 28), (42, 28), (47, 40), (17, 40)], fill=(241, 196, 15, 255))
        draw_icon.line([22, 29, 42, 29], fill=(252, 243, 207, 255), width=1)
        # Hallmark stamp
        draw_icon.rectangle([30, 32, 34, 36], fill=(183, 149, 11, 255))

        draw_shadow_32(draw_world)
        draw_world.polygon([(11, 16), (21, 16), (24, 22), (8, 22)], fill=(241, 196, 15, 255))

def gen_resource(draw_icon, draw_world, r_type):
    draw_badge(draw_icon)
    if r_type == 'red_mushroom':
        # Red capped toadstool with white spots
        draw_icon.rectangle([28, 34, 36, 48], fill=(235, 245, 251, 255)) # stem
        draw_icon.arc([16, 18, 48, 44], 180, 360, fill=(192, 57, 43, 255), width=14)
        draw_icon.ellipse([24, 22, 28, 26], fill=(255, 255, 255, 255))
        draw_icon.ellipse([36, 24, 40, 28], fill=(255, 255, 255, 255))

        draw_shadow_32(draw_world)
        draw_world.rectangle([14, 18, 18, 24], fill=(235, 245, 251, 255))
        draw_world.arc([9, 12, 23, 22], 180, 360, fill=(192, 57, 43, 255), width=7)
    elif r_type == 'blue_mushroom':
        draw_icon.rectangle([28, 34, 36, 48], fill=(174, 214, 241, 255))
        draw_icon.arc([16, 18, 48, 44], 180, 360, fill=(31, 97, 141, 255), width=14)
        draw_icon.ellipse([22, 24, 26, 28], fill=(84, 153, 199, 255))
        draw_icon.ellipse([34, 22, 38, 26], fill=(84, 153, 199, 255))

        draw_shadow_32(draw_world)
        draw_world.rectangle([14, 18, 18, 24], fill=(174, 214, 241, 255))
        draw_world.arc([9, 12, 23, 22], 180, 360, fill=(31, 97, 141, 255), width=7)
    elif r_type == 'flower':
        # Healing Flower with white petals and gold center
        draw_icon.line([32, 34, 32, 50], fill=(39, 174, 96, 255), width=3)
        for i in range(6):
            a = i * 3.14159 / 3
            px = 32 + int(10 * math.cos(a))
            py = 28 + int(10 * math.sin(a))
            draw_icon.ellipse([px-5, py-5, px+5, py+5], fill=(253, 254, 254, 255))
        draw_icon.ellipse([27, 23, 37, 33], fill=(247, 220, 111, 255))

        draw_shadow_32(draw_world)
        draw_world.line([16, 18, 16, 24], fill=(39, 174, 96, 255), width=2)
        draw_world.ellipse([12, 11, 20, 19], fill=(253, 254, 254, 255))
        draw_world.ellipse([14, 13, 18, 17], fill=(247, 220, 111, 255))
    elif r_type == 'herb':
        draw_icon.line([22, 48, 34, 20], fill=(30, 132, 73, 255), width=3)
        draw_icon.ellipse([24, 22, 36, 32], fill=(46, 204, 113, 255))
        draw_icon.ellipse([28, 28, 42, 38], fill=(39, 174, 96, 255))

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 13, 21, 22], fill=(46, 204, 113, 255))
    elif r_type == 'berries':
        # Bunch of purple/black berries
        draw_icon.ellipse([22, 28, 32, 38], fill=(74, 35, 90, 255))
        draw_icon.ellipse([30, 26, 40, 36], fill=(142, 68, 173, 255))
        draw_icon.ellipse([26, 34, 36, 44], fill=(74, 35, 90, 255))
        draw_icon.line([26, 26, 22, 18], fill=(39, 174, 96, 255), width=2) # stem
        draw_icon.ellipse([22, 16, 30, 22], fill=(46, 204, 113, 255)) # leaf

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 14, 21, 23], fill=(142, 68, 173, 255))
    elif r_type == 'cactus_fruit':
        # Magenta prickly pear
        draw_icon.ellipse([20, 20, 44, 46], fill=(146, 43, 33, 255))
        draw_icon.ellipse([22, 22, 42, 44], fill=(192, 57, 43, 255))
        draw_icon.line([30, 20, 34, 14], fill=(25, 111, 61, 255), width=3) # green stem
        draw_icon.point([28, 30], fill=(245, 183, 177, 255)) # spine

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 13, 21, 23], fill=(192, 57, 43, 255))
    else: # frozen_herb
        draw_icon.line([22, 48, 34, 20], fill=(93, 173, 226, 255), width=3)
        draw_icon.ellipse([24, 22, 36, 32], fill=(174, 214, 241, 255))
        draw_icon.ellipse([28, 28, 42, 38], fill=(235, 245, 251, 255))
        draw_icon.point([30, 26], fill=(255, 255, 255, 255))

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 13, 21, 22], fill=(174, 214, 241, 255))

def gen_special(draw_icon, draw_world, s_type):
    draw_badge(draw_icon)
    if s_type == 'ancient_key':
        # Ornate golden key
        draw_icon.ellipse([24, 16, 40, 32], outline=(212, 172, 13, 255), width=4)
        draw_icon.line([32, 30, 32, 50], fill=(212, 172, 13, 255), width=4)
        draw_icon.line([32, 42, 40, 42], fill=(212, 172, 13, 255), width=3)
        draw_icon.line([32, 48, 42, 48], fill=(212, 172, 13, 255), width=3)

        draw_shadow_32(draw_world)
        draw_world.ellipse([12, 10, 20, 18], outline=(212, 172, 13, 255), width=2)
        draw_world.line([16, 17, 16, 25], fill=(212, 172, 13, 255), width=2)
        draw_world.line([16, 23, 20, 23], fill=(212, 172, 13, 255), width=2)
    elif s_type == 'rusty_key':
        # Pitted iron skeleton key
        draw_icon.ellipse([24, 16, 40, 32], outline=(110, 44, 0, 255), width=4)
        draw_icon.line([32, 30, 32, 50], fill=(135, 54, 0, 255), width=4)
        draw_icon.line([32, 44, 38, 44], fill=(135, 54, 0, 255), width=3)

        draw_shadow_32(draw_world)
        draw_world.ellipse([12, 10, 20, 18], outline=(135, 54, 0, 255), width=2)
        draw_world.line([16, 17, 16, 25], fill=(135, 54, 0, 255), width=2)
        draw_world.line([16, 23, 19, 23], fill=(135, 54, 0, 255), width=2)
    elif s_type == 'scroll':
        # Rolled parchment with red ribbon
        draw_icon.rectangle([20, 18, 44, 46], fill=(245, 203, 167, 255))
        draw_icon.ellipse([18, 18, 22, 46], fill=(237, 187, 153, 255))
        draw_icon.ellipse([42, 18, 46, 46], fill=(237, 187, 153, 255))
        draw_icon.line([30, 18, 34, 46], fill=(192, 57, 43, 255), width=3) # ribbon

        draw_shadow_32(draw_world)
        draw_world.rectangle([10, 13, 22, 23], fill=(245, 203, 167, 255))
        draw_world.line([15, 13, 17, 23], fill=(192, 57, 43, 255), width=2)
    elif s_type == 'treasure_map':
        # Unfurled parchment with X
        draw_icon.polygon([(16, 20), (46, 16), (48, 46), (18, 48)], fill=(245, 203, 167, 255))
        draw_icon.line([24, 28, 34, 38], fill=(192, 57, 43, 255), width=3) # Red X
        draw_icon.line([34, 28, 24, 38], fill=(192, 57, 43, 255), width=3)
        draw_icon.arc([20, 20, 42, 42], 0, 180, fill=(186, 74, 0, 255), width=1)

        draw_shadow_32(draw_world)
        draw_world.polygon([(9, 14), (23, 12), (24, 23), (10, 24)], fill=(245, 203, 167, 255))
        draw_world.line([14, 16, 18, 20], fill=(192, 57, 43, 255), width=1)
        draw_world.line([18, 16, 14, 20], fill=(192, 57, 43, 255), width=1)
    elif s_type == 'rune_stone':
        # Dark stone with cyan glowing rune
        draw_icon.ellipse([18, 18, 46, 46], fill=(44, 62, 80, 255), outline=(52, 73, 94, 255), width=2)
        # Cyan rune symbol
        draw_icon.line([32, 24, 32, 40], fill=(0, 229, 255, 255), width=3)
        draw_icon.line([26, 28, 38, 28], fill=(0, 229, 255, 255), width=3)
        draw_icon.line([28, 36, 36, 36], fill=(0, 229, 255, 255), width=2)

        draw_shadow_32(draw_world)
        draw_world.ellipse([10, 13, 22, 23], fill=(44, 62, 80, 255))
        draw_world.line([16, 15, 16, 21], fill=(0, 229, 255, 255), width=2)
    elif s_type == 'amulet':
        # Gold amulet with blue eye
        draw_icon.arc([22, 12, 42, 30], 180, 360, fill=(212, 172, 13, 255), width=2) # chain
        draw_icon.ellipse([22, 24, 42, 46], fill=(212, 172, 13, 255))
        draw_icon.ellipse([26, 28, 38, 42], fill=(41, 128, 185, 255))
        draw_icon.point([32, 34], fill=(255, 255, 255, 255))

        draw_shadow_32(draw_world)
        draw_world.ellipse([11, 14, 21, 23], fill=(212, 172, 13, 255))
        draw_world.ellipse([13, 16, 19, 21], fill=(41, 128, 185, 255))
    else: # artifact
        # Pulsing obsidian dodecahedron
        draw_icon.polygon([(32, 14), (46, 24), (46, 42), (32, 50), (18, 42), (18, 24)], fill=(27, 38, 49, 255))
        draw_icon.polygon([(32, 20), (42, 28), (42, 38), (32, 44), (22, 38), (22, 28)], fill=(142, 68, 173, 255))
        draw_icon.point([32, 32], fill=(210, 180, 222, 255))

        draw_shadow_32(draw_world)
        draw_world.polygon([(16, 11), (22, 15), (22, 22), (16, 25), (10, 22), (10, 15)], fill=(27, 38, 49, 255))
        draw_world.point([16, 18], fill=(142, 68, 173, 255))

# -------------------------------------------------------------
# Main production dispatcher
# -------------------------------------------------------------

def main():
    print("Generating Wildbound Phase 16 Item Asset Collection (40 items)...")
    create_dirs()

    items = [
        # Collectibles (8)
        {"id": "col_gold_coin_001", "cat": "collectibles", "gen": lambda di, dw: gen_coin(di, dw, gold=True)},
        {"id": "col_silver_coin_001", "cat": "collectibles", "gen": lambda di, dw: gen_coin(di, dw, gold=False)},
        {"id": "col_ruby_001", "cat": "collectibles", "gen": lambda di, dw: gen_gem(di, dw, 'ruby')},
        {"id": "col_sapphire_001", "cat": "collectibles", "gen": lambda di, dw: gen_gem(di, dw, 'sapphire')},
        {"id": "col_emerald_001", "cat": "collectibles", "gen": lambda di, dw: gen_gem(di, dw, 'emerald')},
        {"id": "col_ancient_crystal_001", "cat": "collectibles", "gen": gen_crystal},
        {"id": "col_ancient_relic_001", "cat": "collectibles", "gen": gen_relic},
        {"id": "col_golden_emblem_001", "cat": "collectibles", "gen": gen_emblem},

        # Consumables (10)
        {"id": "con_health_potion_small_001", "cat": "consumables", "gen": lambda di, dw: gen_potion(di, dw, 'health_small')},
        {"id": "con_health_potion_large_001", "cat": "consumables", "gen": lambda di, dw: gen_potion(di, dw, 'health_large')},
        {"id": "con_mana_potion_001", "cat": "consumables", "gen": lambda di, dw: gen_potion(di, dw, 'mana')},
        {"id": "con_stamina_potion_001", "cat": "consumables", "gen": lambda di, dw: gen_potion(di, dw, 'stamina')},
        {"id": "con_antidote_001", "cat": "consumables", "gen": lambda di, dw: gen_potion(di, dw, 'antidote')},
        {"id": "con_healing_herb_001", "cat": "consumables", "gen": lambda di, dw: gen_food(di, dw, 'herb')},
        {"id": "con_bread_001", "cat": "consumables", "gen": lambda di, dw: gen_food(di, dw, 'bread')},
        {"id": "con_apple_001", "cat": "consumables", "gen": lambda di, dw: gen_food(di, dw, 'apple')},
        {"id": "con_cooked_meat_001", "cat": "consumables", "gen": lambda di, dw: gen_food(di, dw, 'meat')},
        {"id": "con_water_flask_001", "cat": "consumables", "gen": lambda di, dw: gen_food(di, dw, 'flask')},

        # Crafting Materials (8)
        {"id": "mat_wood_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'wood')},
        {"id": "mat_stone_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'stone')},
        {"id": "mat_iron_ore_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'iron_ore')},
        {"id": "mat_gold_ore_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'gold_ore')},
        {"id": "mat_leather_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'leather')},
        {"id": "mat_cloth_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'cloth')},
        {"id": "mat_iron_ingot_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'iron_ingot')},
        {"id": "mat_gold_ingot_001", "cat": "materials", "gen": lambda di, dw: gen_material(di, dw, 'gold_ingot')},

        # Natural Resources (7)
        {"id": "res_red_mushroom_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'red_mushroom')},
        {"id": "res_blue_mushroom_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'blue_mushroom')},
        {"id": "res_healing_flower_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'flower')},
        {"id": "res_forest_herb_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'herb')},
        {"id": "res_wild_berries_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'berries')},
        {"id": "res_desert_cactus_fruit_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'cactus_fruit')},
        {"id": "res_frozen_herb_001", "cat": "resources", "gen": lambda di, dw: gen_resource(di, dw, 'frozen_herb')},

        # Special Items (7)
        {"id": "spc_ancient_key_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'ancient_key')},
        {"id": "spc_rusty_key_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'rusty_key')},
        {"id": "spc_ancient_scroll_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'scroll')},
        {"id": "spc_treasure_map_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'treasure_map')},
        {"id": "spc_rune_stone_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'rune_stone')},
        {"id": "spc_ancient_amulet_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'amulet')},
        {"id": "spc_mysterious_artifact_001", "cat": "special", "gen": lambda di, dw: gen_special(di, dw, 'artifact')},
    ]

    for item in items:
        iid = item["id"]
        cat = item["cat"]
        gen_fn = item["gen"]

        # Icon 64x64
        icon_img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
        draw_i = ImageDraw.Draw(icon_img)

        # World 32x32
        world_img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
        draw_w = ImageDraw.Draw(world_img)

        gen_fn(draw_i, draw_w)

        # Save to primary icons/ and world/ directories
        icon_path = f"assets/items/icons/{iid}_icon.png"
        world_path = f"assets/items/world/{iid}_world.png"
        icon_img.save(icon_path)
        world_img.save(world_path)

        # Also save to category folder
        cat_dir = f"assets/items/{cat}/{iid}"
        os.makedirs(cat_dir, exist_ok=True)
        icon_img.save(f"{cat_dir}/icon.png")
        world_img.save(f"{cat_dir}/world.png")

        print(f"  + Generated item: {iid} [{cat}]")

    print(f"Successfully generated all {len(items)} items (40 icons + 40 world sprites).")

if __name__ == "__main__":
    main()
