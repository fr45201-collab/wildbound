#!/usr/bin/env python3
"""
Wildbound Phase 15 — Weapon Asset Generation Tool
Generates 11 distinct weapon asset collections (6 swords, 5 bows, 1 arrow supporting asset)
with:
  - 64x64 Inventory Icons
  - 32x32 Ground Representations (with contact shadow)
  - 128x32 Player-Held Sprites (4 directions: Down, Up, Left, Right at 32x32 each)
  - 128x32 Animation Pose Strips (4 key action frames at 32x32 each)
"""

import os
import math
from PIL import Image, ImageDraw

def create_dirs(base="assets/weapons"):
    subdirs = [
        "icons",
        "ground",
        "held",
        "animations",
        "swords/wooden_training_sword",
        "swords/rusty_short_sword",
        "swords/iron_sword",
        "swords/steel_longsword",
        "swords/curved_sword",
        "swords/ancient_rune_sword",
        "bows/wooden_hunting_bow",
        "bows/longbow",
        "bows/reinforced_bow",
        "bows/recurve_bow",
        "bows/ancient_rune_bow",
        "projectiles/arrow_iron_001"
    ]
    for d in subdirs:
        os.makedirs(os.path.join(base, d), exist_ok=True)

# Palette definitions
PALETTES = {
    "wood": {
        "blade": (166, 124, 82, 255),
        "blade_hi": (196, 154, 112, 255),
        "blade_shadow": (120, 80, 48, 255),
        "guard": (140, 94, 52, 255),
        "grip": (194, 178, 128, 255),
        "pommel": (100, 65, 35, 255),
        "string": (230, 222, 200, 255)
    },
    "rusty": {
        "blade": (117, 125, 117, 255),
        "blade_hi": (145, 155, 145, 255),
        "blade_shadow": (75, 82, 75, 255),
        "rust": (156, 78, 35, 255),
        "rust_dark": (110, 50, 20, 255),
        "guard": (80, 85, 80, 255),
        "grip": (90, 60, 40, 255),
        "pommel": (70, 75, 70, 255)
    },
    "iron": {
        "blade": (168, 177, 184, 255),
        "blade_hi": (207, 215, 220, 255),
        "blade_shadow": (96, 105, 112, 255),
        "guard": (70, 78, 86, 255),
        "guard_hi": (110, 120, 130, 255),
        "grip": (84, 50, 23, 255),
        "pommel": (90, 98, 106, 255)
    },
    "steel": {
        "blade": (224, 230, 237, 255),
        "blade_hi": (255, 255, 255, 255),
        "blade_shadow": (136, 152, 166, 255),
        "guard": (175, 190, 205, 255),
        "guard_hi": (220, 235, 248, 255),
        "grip": (43, 51, 61, 255),
        "pommel": (212, 175, 55, 255) # Gold accent
    },
    "curved": {
        "blade": (188, 202, 214, 255),
        "blade_hi": (230, 240, 248, 255),
        "blade_shadow": (141, 157, 182, 255),
        "guard": (197, 160, 89, 255),
        "guard_hi": (240, 205, 120, 255),
        "grip": (34, 34, 34, 255),
        "pommel": (197, 160, 89, 255)
    },
    "rune_sword": {
        "blade": (49, 55, 69, 255),
        "blade_hi": (80, 90, 110, 255),
        "blade_shadow": (28, 32, 40, 255),
        "rune": (0, 229, 255, 255),
        "rune_glow": (0, 229, 255, 120),
        "guard": (77, 83, 96, 255),
        "grip": (25, 28, 35, 255),
        "pommel": (0, 229, 255, 255)
    },
    "longbow": {
        "limb": (110, 55, 27, 255),
        "limb_hi": (139, 74, 38, 255),
        "limb_shadow": (81, 37, 13, 255),
        "nock": (220, 208, 179, 255),
        "grip": (50, 25, 10, 255),
        "string": (244, 241, 234, 255)
    },
    "reinforced_bow": {
        "limb": (90, 99, 110, 255),
        "limb_hi": (130, 142, 155, 255),
        "limb_shadow": (60, 66, 74, 255),
        "wood": (95, 60, 35, 255),
        "grip": (59, 34, 16, 255),
        "string": (210, 215, 220, 255)
    },
    "recurve_bow": {
        "limb": (160, 82, 45, 255),
        "limb_hi": (205, 133, 63, 255),
        "limb_shadow": (115, 55, 25, 255),
        "tip": (232, 228, 201, 255),
        "grip": (45, 45, 45, 255),
        "string": (255, 255, 255, 255)
    },
    "rune_bow": {
        "limb": (44, 62, 80, 255),
        "limb_hi": (70, 95, 120, 255),
        "limb_shadow": (25, 35, 45, 255),
        "rune": (56, 239, 125, 255),
        "rune_glow": (56, 239, 125, 140),
        "grip": (20, 30, 40, 255),
        "string": (168, 255, 120, 255)
    },
    "arrow": {
        "shaft": (166, 124, 82, 255),
        "head": (175, 185, 195, 255),
        "head_dark": (100, 110, 120, 255),
        "fletch": (240, 240, 245, 255),
        "fletch_shadow": (180, 185, 190, 255)
    }
}

def draw_sword_icon(draw, p, is_curved=False, is_rune=False, is_rusty=False, is_wood=False, is_long=False):
    # Sword Icon: 64x64, 45-degree angle from (14, 50) to (50, 14)
    # Background subtle radial weapon display frame
    draw.ellipse([8, 8, 56, 56], fill=(24, 28, 36, 160), outline=(48, 56, 70, 200), width=1)
    
    # Drop shadow
    for offset in [(1, 2), (2, 2)]:
        ox, oy = offset
        draw.line([14+ox, 50+oy, 48+ox, 16+oy], fill=(10, 12, 18, 120), width=3)

    # Blade line
    b_start = (26, 38)
    b_end = (52, 12) if is_long else (48, 16)
    
    if is_curved:
        # Curve points
        pts = [(24, 40), (32, 30), (42, 20), (50, 13)]
        for i in range(len(pts)-1):
            draw.line([pts[i], pts[i+1]], fill=p["blade_shadow"], width=4)
            draw.line([pts[i], pts[i+1]], fill=p["blade"], width=3)
            draw.line([(pts[i][0]-1, pts[i][1]-1), (pts[i+1][0]-1, pts[i+1][1]-1)], fill=p["blade_hi"], width=1)
    else:
        # Straight blade
        draw.line([b_start[0], b_start[1], b_end[0], b_end[1]], fill=p["blade_shadow"], width=4)
        draw.line([b_start[0], b_start[1], b_end[0], b_end[1]], fill=p["blade"], width=3)
        draw.line([b_start[0]-1, b_start[1]-1, b_end[0]-1, b_end[1]-1], fill=p["blade_hi"], width=1)
        # Tip
        draw.point([b_end[0], b_end[1]], fill=p["blade_hi"])

    # Rust spots or Rune engravings
    if is_rusty:
        draw.point([32, 32], fill=p["rust"])
        draw.point([33, 31], fill=p["rust_dark"])
        draw.point([40, 24], fill=p["rust"])
        draw.point([41, 25], fill=p["rust_dark"])
    elif is_rune:
        draw.line([30, 34, 44, 20], fill=p["rune_glow"], width=3)
        draw.line([32, 32, 42, 22], fill=p["rune"], width=1)
        draw.point([34, 30], fill=(255, 255, 255, 255))
        draw.point([40, 24], fill=(255, 255, 255, 255))
    elif is_wood:
        draw.line([30, 34, 44, 20], fill=p["blade_shadow"], width=1)

    # Crossguard (perpendicular to 45 deg, from (22, 34) to (30, 42))
    g1 = (19, 33)
    g2 = (33, 47)
    draw.line([g1[0], g1[1], g2[0], g2[1]], fill=p.get("guard_shadow", (30, 30, 30, 255)), width=4)
    draw.line([g1[0], g1[1], g2[0], g2[1]], fill=p["guard"], width=3)
    draw.line([g1[0]-1, g1[1]-1, g2[0]-1, g2[1]-1], fill=p.get("guard_hi", p["guard"]), width=1)

    # Handle / Grip: (26, 40) down to (18, 48)
    h_start = (25, 41)
    h_end = (16, 50)
    draw.line([h_start[0], h_start[1], h_end[0], h_end[1]], fill=p["grip"], width=3)

    # Pommel: (15, 51)
    draw.ellipse([12, 48, 17, 53], fill=p["pommel"])

def draw_bow_icon(draw, p, is_long=False, is_recurve=False, is_rune=False):
    # Bow Icon: 64x64. Bow curved limb facing left, string on right
    draw.ellipse([8, 8, 56, 56], fill=(24, 28, 36, 160), outline=(48, 56, 70, 200), width=1)
    
    # Shadow
    draw.arc([16, 12, 44, 52], 90, 270, fill=(10, 12, 18, 120), width=4)
    
    # Limb
    top_y = 10 if is_long else 14
    bot_y = 54 if is_long else 50
    cx = 24
    
    if is_recurve:
        # Recurve bends back at tips
        pts = [(38, top_y), (32, top_y+6), (22, 26), (20, 32), (22, 38), (32, bot_y-6), (38, bot_y)]
        for i in range(len(pts)-1):
            draw.line([pts[i], pts[i+1]], fill=p.get("limb_shadow", p.get("blade_shadow")), width=4)
            draw.line([pts[i], pts[i+1]], fill=p.get("limb", p.get("wood", (150, 100, 50, 255))), width=3)
            draw.line([(pts[i][0]-1, pts[i][1]), (pts[i+1][0]-1, pts[i+1][1])], fill=p.get("limb_hi", (200, 160, 100, 255)), width=1)
        # String
        draw.line([38, top_y, 38, bot_y], fill=p["string"], width=1)
    else:
        # Classical C-curve
        draw.arc([14, top_y, 42, bot_y], 90, 270, fill=p.get("limb_shadow", (50, 30, 10, 255)), width=4)
        draw.arc([14, top_y, 42, bot_y], 90, 270, fill=p.get("limb", (140, 90, 40, 255)), width=3)
        draw.arc([13, top_y, 41, bot_y], 90, 270, fill=p.get("limb_hi", (180, 130, 80, 255)), width=1)
        # Nocks & String
        draw.line([28, top_y, 28, bot_y], fill=p["string"], width=1)

    # Grip
    draw.rectangle([18, 29, 23, 35], fill=p["grip"])

    # Runes
    if is_rune:
        draw.point([20, 22], fill=p["rune"])
        draw.point([21, 23], fill=p["rune_glow"])
        draw.point([20, 42], fill=p["rune"])
        draw.point([21, 41], fill=p["rune_glow"])

def draw_arrow_icon(draw, p):
    draw.ellipse([8, 8, 56, 56], fill=(24, 28, 36, 160), outline=(48, 56, 70, 200), width=1)
    # 45-degree arrow
    draw.line([16, 48, 48, 16], fill=(10, 12, 18, 120), width=3) # shadow
    draw.line([15, 47, 47, 15], fill=p["shaft"], width=2)
    # Head
    draw.polygon([(46, 14), (52, 12), (50, 18), (47, 17)], fill=p["head"])
    # Fletching
    draw.polygon([(14, 46), (12, 50), (18, 48)], fill=p["fletch"])
    draw.polygon([(16, 44), (18, 48), (20, 42)], fill=p["fletch_shadow"])

def draw_sword_ground(draw, p, is_curved=False, is_rune=False, is_rusty=False, is_wood=False):
    # 32x32 ground sprite: lying horizontally/slightly angled with soft shadow
    # Shadow
    draw.ellipse([4, 18, 28, 24], fill=(12, 15, 20, 110))
    # Blade
    draw.line([10, 18, 26, 18], fill=p["blade_shadow"], width=3)
    draw.line([10, 18, 26, 18], fill=p["blade"], width=2)
    draw.line([10, 17, 26, 17], fill=p["blade_hi"], width=1)
    # Guard
    draw.line([10, 15, 10, 21], fill=p["guard"], width=2)
    # Grip & Pommel
    draw.line([6, 18, 10, 18], fill=p["grip"], width=2)
    draw.point([5, 18], fill=p["pommel"])
    if is_rune:
        draw.point([16, 18], fill=p["rune"])
        draw.point([20, 18], fill=p["rune"])

def draw_bow_ground(draw, p, is_recurve=False, is_rune=False):
    # 32x32 ground sprite: bow unstrung or resting on ground
    draw.ellipse([4, 18, 28, 25], fill=(12, 15, 20, 110))
    draw.arc([6, 12, 26, 26], 180, 360, fill=p.get("limb_shadow", (50, 30, 10, 255)), width=3)
    draw.arc([6, 12, 26, 26], 180, 360, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([6, 19, 26, 19], fill=p["string"], width=1)
    draw.rectangle([14, 12, 18, 14], fill=p["grip"])

def draw_held_sword_strip(img, p, is_curved=False, is_rune=False):
    # 128x32 strip with 4 frames (each 32x32)
    # Frame 0 (0..31): Facing Down -> Weapon pointing Down-Right (in right hand)
    # Frame 1 (32..63): Facing Up -> Weapon pointing Up-Right (held upright behind shoulder)
    # Frame 2 (64..95): Facing Left -> Weapon pointing Left
    # Frame 3 (96..127): Facing Right -> Weapon pointing Right
    draw = ImageDraw.Draw(img)
    
    # F0: Down (X offset: 0)
    # Hand at (12, 18), blade downwards to (22, 28)
    draw.line([12, 18, 23, 27], fill=p["blade_shadow"], width=3)
    draw.line([12, 18, 23, 27], fill=p["blade"], width=2)
    draw.line([11, 17, 22, 26], fill=p["blade_hi"], width=1)
    draw.line([10, 20, 14, 16], fill=p["guard"], width=2)
    draw.line([8, 16, 11, 18], fill=p["grip"], width=2)
    draw.point([7, 15], fill=p["pommel"])
    if is_rune:
        draw.point([17, 22], fill=p["rune"])

    # F1: Up (X offset: 32)
    # Hand at (32+18, 18), blade pointing Up-Right to (32+26, 6)
    ox = 32
    draw.line([ox+18, 18, ox+26, 6], fill=p["blade_shadow"], width=3)
    draw.line([ox+18, 18, ox+26, 6], fill=p["blade"], width=2)
    draw.line([ox+17, 17, ox+25, 5], fill=p["blade_hi"], width=1)
    draw.line([ox+16, 16, ox+20, 20], fill=p["guard"], width=2)
    draw.line([ox+14, 21, ox+17, 19], fill=p["grip"], width=2)
    draw.point([ox+13, 22], fill=p["pommel"])
    if is_rune:
        draw.point([ox+22, 12], fill=p["rune"])

    # F2: Left (X offset: 64)
    # Hand at (64+18, 18), blade pointing horizontally left to (64+4, 18)
    ox = 64
    draw.line([ox+6, 18, ox+18, 18], fill=p["blade_shadow"], width=3)
    draw.line([ox+6, 18, ox+18, 18], fill=p["blade"], width=2)
    draw.line([ox+6, 17, ox+18, 17], fill=p["blade_hi"], width=1)
    draw.line([ox+18, 15, ox+18, 21], fill=p["guard"], width=2)
    draw.line([ox+18, 18, ox+22, 18], fill=p["grip"], width=2)
    draw.point([ox+23, 18], fill=p["pommel"])
    if is_rune:
        draw.point([ox+12, 18], fill=p["rune"])

    # F3: Right (X offset: 96)
    # Hand at (96+14, 18), blade pointing horizontally right to (96+26, 18)
    ox = 96
    draw.line([ox+14, 18, ox+26, 18], fill=p["blade_shadow"], width=3)
    draw.line([ox+14, 18, ox+26, 18], fill=p["blade"], width=2)
    draw.line([ox+14, 17, ox+26, 17], fill=p["blade_hi"], width=1)
    draw.line([ox+14, 15, ox+14, 21], fill=p["guard"], width=2)
    draw.line([ox+10, 18, ox+14, 18], fill=p["grip"], width=2)
    draw.point([ox+9, 18], fill=p["pommel"])
    if is_rune:
        draw.point([ox+20, 18], fill=p["rune"])

def draw_held_bow_strip(img, p, is_recurve=False, is_rune=False):
    draw = ImageDraw.Draw(img)
    # F0: Down (ox = 0)
    draw.arc([6, 8, 22, 26], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([14, 8, 14, 26], fill=p["string"], width=1)
    draw.rectangle([18, 15, 20, 19], fill=p["grip"])

    # F1: Up (ox = 32)
    ox = 32
    draw.arc([ox+10, 8, ox+26, 26], 90, 270, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+18, 8, ox+18, 26], fill=p["string"], width=1)
    draw.rectangle([ox+12, 15, ox+14, 19], fill=p["grip"])

    # F2: Left (ox = 64)
    ox = 64
    draw.arc([ox+4, 10, ox+22, 24], 90, 270, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+13, 10, ox+13, 24], fill=p["string"], width=1)
    draw.rectangle([ox+7, 15, ox+9, 19], fill=p["grip"])

    # F3: Right (ox = 96)
    ox = 96
    draw.arc([ox+10, 10, ox+28, 24], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+19, 10, ox+19, 24], fill=p["string"], width=1)
    draw.rectangle([ox+23, 15, ox+25, 19], fill=p["grip"])

def draw_sword_anim_strip(img, p, is_rune=False):
    # 4 poses: Idle, Prep, Swing, Recovery
    draw = ImageDraw.Draw(img)
    
    # Pose 0: Idle (rest at side)
    ox = 0
    draw.line([ox+14, 16, ox+24, 24], fill=p["blade"], width=2)
    draw.line([ox+12, 18, ox+16, 14], fill=p["guard"], width=2)
    draw.line([ox+10, 14, ox+13, 16], fill=p["grip"], width=2)
    
    # Pose 1: Prep / Windup (raised high behind)
    ox = 32
    draw.line([ox+16, 18, ox+24, 8], fill=p["blade"], width=2)
    draw.line([ox+14, 16, ox+18, 20], fill=p["guard"], width=2)
    draw.line([ox+12, 21, ox+15, 19], fill=p["grip"], width=2)

    # Pose 2: Full Slash (forward horizontal slash with motion streak)
    ox = 64
    # Motion streak arc
    draw.arc([ox+4, 4, ox+28, 28], 300, 45, fill=(240, 245, 255, 160), width=2)
    draw.line([ox+8, 16, ox+26, 16], fill=p["blade_hi"], width=3)
    draw.line([ox+8, 14, ox+8, 20], fill=p["guard"], width=2)
    draw.line([ox+4, 16, ox+8, 16], fill=p["grip"], width=2)

    # Pose 3: Recovery / Follow-through (lowered forward angle)
    ox = 96
    draw.line([ox+14, 16, ox+26, 22], fill=p["blade"], width=2)
    draw.line([ox+12, 18, ox+16, 14], fill=p["guard"], width=2)
    draw.line([ox+10, 14, ox+13, 16], fill=p["grip"], width=2)

def draw_bow_anim_strip(img, p, is_rune=False):
    # 4 poses: Idle, Draw, FullDraw, Release
    draw = ImageDraw.Draw(img)
    
    # Pose 0: Idle
    ox = 0
    draw.arc([ox+6, 8, ox+22, 26], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+14, 8, ox+14, 26], fill=p["string"], width=1)
    
    # Pose 1: Draw (string pulled halfway)
    ox = 32
    draw.arc([ox+8, 8, ox+24, 26], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+16, 8, ox+12, 17], fill=p["string"], width=1)
    draw.line([ox+12, 17, ox+16, 26], fill=p["string"], width=1)

    # Pose 2: Full Draw with nocked arrow
    ox = 64
    draw.arc([ox+10, 8, ox+26, 26], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+18, 8, ox+8, 17], fill=p["string"], width=1)
    draw.line([ox+8, 17, ox+18, 26], fill=p["string"], width=1)
    # Arrow
    draw.line([ox+8, 17, ox+26, 17], fill=(210, 170, 120, 255), width=1)
    draw.point([ox+26, 17], fill=(180, 190, 200, 255))

    # Pose 3: Release (bow flexed forward, string straight, vibrating)
    ox = 96
    draw.arc([ox+6, 8, ox+22, 26], 270, 90, fill=p.get("limb", (140, 90, 40, 255)), width=2)
    draw.line([ox+14, 8, ox+14, 26], fill=(255, 255, 255, 220), width=1)
    # Whistle air puff
    draw.point([ox+26, 17], fill=(200, 240, 255, 180))
    draw.point([ox+28, 17], fill=(200, 240, 255, 120))

def main():
    print("Generating Wildbound Phase 15 Weapon Asset Collection...")
    create_dirs()

    weapons = [
        # Swords
        {
            "id": "sword_wooden_training_001",
            "type": "sword",
            "folder": "swords/wooden_training_sword",
            "palette": PALETTES["wood"],
            "is_wood": True
        },
        {
            "id": "sword_rusty_short_001",
            "type": "sword",
            "folder": "swords/rusty_short_sword",
            "palette": PALETTES["rusty"],
            "is_rusty": True
        },
        {
            "id": "sword_iron_001",
            "type": "sword",
            "folder": "swords/iron_sword",
            "palette": PALETTES["iron"]
        },
        {
            "id": "sword_steel_longsword_001",
            "type": "sword",
            "folder": "swords/steel_longsword",
            "palette": PALETTES["steel"],
            "is_long": True
        },
        {
            "id": "sword_curved_001",
            "type": "sword",
            "folder": "swords/curved_sword",
            "palette": PALETTES["curved"],
            "is_curved": True
        },
        {
            "id": "sword_ancient_rune_001",
            "type": "sword",
            "folder": "swords/ancient_rune_sword",
            "palette": PALETTES["rune_sword"],
            "is_rune": True
        },
        # Bows
        {
            "id": "bow_wooden_hunting_001",
            "type": "bow",
            "folder": "bows/wooden_hunting_bow",
            "palette": PALETTES["wood"]
        },
        {
            "id": "bow_longbow_001",
            "type": "bow",
            "folder": "bows/longbow",
            "palette": PALETTES["longbow"],
            "is_long": True
        },
        {
            "id": "bow_reinforced_001",
            "type": "bow",
            "folder": "bows/reinforced_bow",
            "palette": PALETTES["reinforced_bow"]
        },
        {
            "id": "bow_recurve_001",
            "type": "bow",
            "folder": "bows/recurve_bow",
            "palette": PALETTES["recurve_bow"],
            "is_recurve": True
        },
        {
            "id": "bow_ancient_rune_001",
            "type": "bow",
            "folder": "bows/ancient_rune_bow",
            "palette": PALETTES["rune_bow"],
            "is_rune": True
        },
        # Optional Supporting Asset: Arrow
        {
            "id": "arrow_iron_001",
            "type": "arrow",
            "folder": "projectiles/arrow_iron_001",
            "palette": PALETTES["arrow"]
        }
    ]

    for w in weapons:
        wid = w["id"]
        wtype = w["type"]
        pal = w["palette"]
        
        # 1. Inventory Icon (64x64)
        icon_img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(icon_img)
        if wtype == "sword":
            draw_sword_icon(draw, pal,
                            is_curved=w.get("is_curved", False),
                            is_rune=w.get("is_rune", False),
                            is_rusty=w.get("is_rusty", False),
                            is_wood=w.get("is_wood", False),
                            is_long=w.get("is_long", False))
        elif wtype == "bow":
            draw_bow_icon(draw, pal,
                          is_long=w.get("is_long", False),
                          is_recurve=w.get("is_recurve", False),
                          is_rune=w.get("is_rune", False))
        elif wtype == "arrow":
            draw_arrow_icon(draw, pal)
            
        icon_path = f"assets/weapons/icons/{wid}_icon.png"
        icon_img.save(icon_path)
        # Also copy to weapon folder
        icon_img.save(f"assets/weapons/{w['folder']}/icon.png")

        # 2. Ground Representation (32x32)
        ground_img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
        draw_g = ImageDraw.Draw(ground_img)
        if wtype == "sword":
            draw_sword_ground(draw_g, pal,
                              is_curved=w.get("is_curved", False),
                              is_rune=w.get("is_rune", False),
                              is_rusty=w.get("is_rusty", False),
                              is_wood=w.get("is_wood", False))
        elif wtype == "bow":
            draw_bow_ground(draw_g, pal,
                            is_recurve=w.get("is_recurve", False),
                            is_rune=w.get("is_rune", False))
        elif wtype == "arrow":
            draw_g.line([6, 16, 26, 16], fill=pal["shaft"], width=2)
            draw.polygon([(24, 14), (28, 16), (24, 18)], fill=pal["head"])

        ground_path = f"assets/weapons/ground/{wid}_ground.png"
        ground_img.save(ground_path)
        ground_img.save(f"assets/weapons/{w['folder']}/ground.png")

        # 3. Held Representation (128x32 - 4 directions)
        held_img = Image.new("RGBA", (128, 32), (0, 0, 0, 0))
        if wtype == "sword":
            draw_held_sword_strip(held_img, pal,
                                  is_curved=w.get("is_curved", False),
                                  is_rune=w.get("is_rune", False))
        elif wtype == "bow":
            draw_held_bow_strip(held_img, pal,
                                is_recurve=w.get("is_recurve", False),
                                is_rune=w.get("is_rune", False))
        elif wtype == "arrow":
            # 4 directional arrow sprites
            d_h = ImageDraw.Draw(held_img)
            # Down
            d_h.line([16, 6, 16, 26], fill=pal["shaft"], width=2)
            # Up
            d_h.line([32+16, 26, 32+16, 6], fill=pal["shaft"], width=2)
            # Left
            d_h.line([64+26, 16, 64+6, 16], fill=pal["shaft"], width=2)
            # Right
            d_h.line([96+6, 16, 96+26, 16], fill=pal["shaft"], width=2)

        held_path = f"assets/weapons/held/{wid}_held.png"
        held_img.save(held_path)
        held_img.save(f"assets/weapons/{w['folder']}/held.png")

        # 4. Animation Strip (128x32 - 4 poses)
        anim_img = Image.new("RGBA", (128, 32), (0, 0, 0, 0))
        if wtype == "sword":
            draw_sword_anim_strip(anim_img, pal, is_rune=w.get("is_rune", False))
        elif wtype == "bow":
            draw_bow_anim_strip(anim_img, pal, is_rune=w.get("is_rune", False))
        elif wtype == "arrow":
            # Arrow flight trail strip
            d_a = ImageDraw.Draw(anim_img)
            for i in range(4):
                ox = i * 32
                d_a.line([ox+6, 16, ox+26, 16], fill=pal["shaft"], width=2)
                d_a.polygon([(ox+24, 14), (ox+28, 16), (ox+24, 18)], fill=pal["head"])

        anim_path = f"assets/weapons/animations/{wid}_anim.png"
        anim_img.save(anim_path)
        anim_img.save(f"assets/weapons/{w['folder']}/anim.png")

        print(f"  + Generated weapon: {wid} ({wtype})")

    print("Weapon asset production complete: 11 weapons + 1 arrow generated.")

if __name__ == "__main__":
    main()

