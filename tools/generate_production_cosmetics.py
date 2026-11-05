import os
import json
from PIL import Image, ImageDraw

FRAME_W = 32
FRAME_H = 48
COLS = 4
ROWS = 8
SHEET_W = COLS * FRAME_W  # 128
SHEET_H = ROWS * FRAME_H  # 384

DIRECTIONS = ["down", "up", "left", "right"]

def get_frame_offsets(anim_type, col, d):
    # Bobbing
    bob_y = 0
    if anim_type == "walk":
        if col in [1, 3]:
            bob_y = -1
    elif anim_type == "idle":
        if col in [1, 2]:
            bob_y = -1

    # Leg offsets for walk
    left_leg_dy = 0
    right_leg_dy = 0
    if anim_type == "walk":
        if col == 0:
            left_leg_dy = -2
            right_leg_dy = 2
        elif col == 1:
            left_leg_dy = 0
            right_leg_dy = 0
        elif col == 2:
            left_leg_dy = 2
            right_leg_dy = -2
        elif col == 3:
            left_leg_dy = 0
            right_leg_dy = 0

    return bob_y, left_leg_dy, right_leg_dy

def create_transparent_sheet():
    return Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))

def save_production_cosmetic(img, item_id, name, category, layer_name, occlusion_flags=None, tint=None, description=""):
    if occlusion_flags is None:
        occlusion_flags = []
    if tint is None:
        tint = [255, 255, 255, 255]

    # Map category to directory name plural
    cat_dir_map = {
        "hair": "hair",
        "shirt": "shirts",
        "pants": "pants",
        "shoes": "shoes",
        "hat": "hats",
        "headwear": "hats",
        "goggles": "goggles",
        "mask": "masks",
        "back": "backpacks",
        "backpack": "backpacks"
    }
    dir_category = cat_dir_map.get(category, category)

    rel_folder = f"characters/player/{dir_category}/{item_id}"
    full_folder = os.path.join("assets", rel_folder)
    os.makedirs(full_folder, exist_ok=True)

    png_path = os.path.join(full_folder, "sprite.png")
    sheet_json_path = os.path.join(full_folder, "spritesheet.json")
    meta_json_path = os.path.join(full_folder, "metadata.json")

    # 1. Save PNG
    img.save(png_path, "PNG")

    # 2. Spritesheet descriptor for AMS
    sheet_id = f"sheet_{item_id}"
    rel_tex_path = f"{rel_folder}/sprite.png".replace("\\", "/")
    sheet_data = {
        "id": sheet_id,
        "texture": rel_tex_path,
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(sheet_json_path, "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=4)

    # 3. Section 16 Cosmetic Metadata
    meta_data = {
        "id": item_id,
        "name": name,
        "category": category,
        "layer": layer_name,
        "description": description,
        "sprite": rel_tex_path,
        "spritesheet": sheet_id,
        "texture": f"tex_{item_id}",
        "compatible_directions": ["up", "down", "left", "right"],
        "compatible_animations": ["idle", "walk", "run"],
        "occlusion_flags": occlusion_flags,
        "anchors": {
            "body": [16, 48]
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 8 Production Asset",
            "license": "CC0 1.0 Universal",
            "attribution_required": False
        }
    }
    with open(meta_json_path, "w", encoding="utf-8") as f:
        json.dump(meta_data, f, indent=4)

    print(f"Generated [{category}] {item_id}: {png_path}")

# =============================================================================
# 1. HAIRSTYLES (5 Designs)
# =============================================================================

def generate_hair_short_001():
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_HAIR = (115, 65, 30, 255)       # Chestnut brown
    C_HILIGHT = (155, 95, 45, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "down":
            draw.rectangle((cx - 6, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            draw.rectangle((cx - 7, cy - 4, cx - 5, cy + 2), fill=C_HAIR)
            draw.rectangle((cx + 5, cy - 4, cx + 7, cy + 2), fill=C_HAIR)
            draw.line([(cx - 4, cy - 6), (cx + 4, cy - 6)], fill=C_HILIGHT)
        elif d == "up":
            draw.ellipse((cx - 6, cy - 7, cx + 6, cy + 5), fill=C_HAIR, outline=C_HILIGHT)
        elif d == "left":
            draw.rectangle((cx - 5, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            draw.rectangle((cx + 2, cy - 4, cx + 7, cy + 3), fill=C_HAIR)
            draw.line([(cx - 2, cy - 6), (cx + 4, cy - 6)], fill=C_HILIGHT)
        elif d == "right":
            draw.rectangle((cx - 6, cy - 7, cx + 5, cy - 2), fill=C_HAIR)
            draw.rectangle((cx - 7, cy - 4, cx - 2, cy + 3), fill=C_HAIR)
            draw.line([(cx - 4, cy - 6), (cx + 2, cy - 6)], fill=C_HILIGHT)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, "hair_short_001", "Short Adventurer", "hair", "head_hair",
                            description="Simple neat short adventurer haircut.")

def generate_hair_long_001():
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_HAIR = (35, 38, 48, 255)        # Raven black
    C_HILIGHT = (65, 75, 95, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "down":
            draw.rectangle((cx - 6, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            # Long front locks flowing down over shoulders
            draw.rectangle((cx - 8, cy - 3, cx - 5, cy + 8), fill=C_HAIR)
            draw.rectangle((cx + 5, cy - 3, cx + 8, cy + 8), fill=C_HAIR)
            draw.line([(cx - 3, cy - 6), (cx + 3, cy - 6)], fill=C_HILIGHT)
        elif d == "up":
            # Flowing long hair covering back of head and upper back
            draw.ellipse((cx - 7, cy - 7, cx + 7, cy + 9), fill=C_HAIR, outline=C_HILIGHT)
        elif d == "left":
            draw.rectangle((cx - 5, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            draw.rectangle((cx + 1, cy - 4, cx + 8, cy + 9), fill=C_HAIR)
        elif d == "right":
            draw.rectangle((cx - 6, cy - 7, cx + 5, cy - 2), fill=C_HAIR)
            draw.rectangle((cx - 8, cy - 4, cx - 1, cy + 9), fill=C_HAIR)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, "hair_long_001", "Long Wanderer", "hair", "head_hair",
                            description="Medium-length flowing dark locks.")

def generate_hair_messy_001():
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_HAIR = (165, 75, 35, 255)       # Auburn copper
    C_HILIGHT = (205, 115, 60, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "down":
            draw.rectangle((cx - 7, cy - 8, cx + 7, cy - 2), fill=C_HAIR)
            # Spiky strands
            draw.line([(cx - 6, cy - 9), (cx - 4, cy - 7)], fill=C_HILIGHT)
            draw.line([(cx - 1, cy - 10), (cx + 1, cy - 7)], fill=C_HILIGHT)
            draw.line([(cx + 4, cy - 9), (cx + 6, cy - 7)], fill=C_HILIGHT)
            draw.rectangle((cx - 8, cy - 4, cx - 6, cy + 2), fill=C_HAIR)
            draw.rectangle((cx + 6, cy - 4, cx + 8, cy + 2), fill=C_HAIR)
        elif d == "up":
            draw.ellipse((cx - 7, cy - 8, cx + 7, cy + 5), fill=C_HAIR, outline=C_HILIGHT)
            draw.point((cx - 2, cy - 9), fill=C_HAIR)
            draw.point((cx + 3, cy - 9), fill=C_HAIR)
        elif d in ["left", "right"]:
            draw.rectangle((cx - 6, cy - 8, cx + 6, cy - 2), fill=C_HAIR)
            ox = 2 if d == "left" else -7
            draw.rectangle((cx + ox, cy - 4, cx + ox + 5, cy + 4), fill=C_HAIR)
            draw.line([(cx - 3, cy - 9), (cx + 2, cy - 9)], fill=C_HILIGHT)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, "hair_messy_001", "Messy Explorer", "hair", "head_hair",
                            description="Unstructured adventurer spiky hair.")

def generate_hair_ponytail_001():
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_HAIR = (235, 195, 65, 255)      # Golden blonde
    C_HILIGHT = (255, 225, 110, 255)
    C_RIBBON = (195, 45, 55, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "down":
            draw.rectangle((cx - 6, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            draw.rectangle((cx - 7, cy - 3, cx - 5, cy + 4), fill=C_HAIR)
            draw.rectangle((cx + 5, cy - 3, cx + 7, cy + 4), fill=C_HAIR)
            draw.ellipse((cx + 5, cy - 6, cx + 9, cy + 3), fill=C_HAIR)
        elif d == "up":
            draw.ellipse((cx - 6, cy - 7, cx + 6, cy + 4), fill=C_HAIR, outline=C_HILIGHT)
            draw.rectangle((cx - 2, cy - 6, cx + 2, cy - 4), fill=C_RIBBON)
            draw.rectangle((cx - 2, cy - 4, cx + 2, cy + 9), fill=C_HAIR)
        elif d == "left":
            draw.rectangle((cx - 5, cy - 7, cx + 6, cy - 2), fill=C_HAIR)
            draw.rectangle((cx + 2, cy - 4, cx + 7, cy + 3), fill=C_HAIR)
            draw.rectangle((cx + 5, cy - 6, cx + 7, cy - 4), fill=C_RIBBON)
            draw.rectangle((cx + 7, cy - 5, cx + 10, cy + 5), fill=C_HAIR)
        elif d == "right":
            draw.rectangle((cx - 6, cy - 7, cx + 5, cy - 2), fill=C_HAIR)
            draw.rectangle((cx - 7, cy - 4, cx - 2, cy + 3), fill=C_HAIR)
            draw.rectangle((cx - 7, cy - 6, cx - 5, cy - 4), fill=C_RIBBON)
            draw.rectangle((cx - 10, cy - 5, cx - 7, cy + 5), fill=C_HAIR)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, "hair_ponytail_001", "High Ponytail", "hair", "head_hair",
                            description="Tied-back blonde ponytail with crimson ribbon.")

def generate_hair_curly_001():
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_HAIR = (70, 45, 25, 255)        # Dark curly brown
    C_HILIGHT = (105, 70, 40, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d in ["down", "up"]:
            draw.ellipse((cx - 8, cy - 8, cx + 8, cy + 5), fill=C_HAIR)
            draw.ellipse((cx - 7, cy - 9, cx - 3, cy - 5), fill=C_HILIGHT)
            draw.ellipse((cx - 1, cy - 10, cx + 3, cy - 6), fill=C_HILIGHT)
            draw.ellipse((cx + 3, cy - 9, cx + 7, cy - 5), fill=C_HILIGHT)
            if d == "down":
                # Clear face space
                draw.rectangle((cx - 4, cy - 1, cx + 4, cy + 5), fill=(0, 0, 0, 0))
        elif d in ["left", "right"]:
            draw.ellipse((cx - 7, cy - 8, cx + 7, cy + 5), fill=C_HAIR)
            draw.ellipse((cx - 3, cy - 9, cx + 3, cy - 5), fill=C_HILIGHT)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, "hair_curly_001", "Curly Hair", "hair", "head_hair",
                            description="Distinct textured curly hairstyle.")

# =============================================================================
# 2. SHIRTS (5 Designs)
# =============================================================================

def generate_shirt_generic(item_id, name, main_c, border_c, sleeve_c=None, has_armor=False, is_robe=False, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    if sleeve_c is None: sleeve_c = main_c
    C_BELT = (80, 48, 24, 255)
    C_BUCKLE = (220, 180, 50, 255)
    C_STEEL = (165, 175, 190, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)
        torso_top = by + 20 + bob_y
        torso_bot = by + (36 if is_robe else 32) + bob_y

        if d in ["down", "up"]:
            # Torso
            draw.rectangle((bx + 10, torso_top, bx + 21, torso_bot), fill=main_c, outline=border_c)
            # Belt
            draw.rectangle((bx + 10, by + 30 + bob_y, bx + 21, by + 32 + bob_y), fill=C_BELT)
            if d == "down":
                draw.rectangle((bx + 14, by + 30 + bob_y, bx + 17, by + 32 + bob_y), fill=C_BUCKLE)
                if has_armor:
                    # Steel chestplate
                    draw.rectangle((bx + 12, torso_top + 2, bx + 19, torso_top + 8), fill=C_STEEL)
            # Arms
            arm_y1 = torso_top + 2 + (-left_leg_dy if anim_type == "walk" else 0)
            arm_y2 = torso_top + 2 + (-right_leg_dy if anim_type == "walk" else 0)
            draw.rectangle((bx + 7, arm_y1, bx + 9, arm_y1 + 8), fill=sleeve_c, outline=border_c)
            draw.rectangle((bx + 22, arm_y2, bx + 24, arm_y2 + 8), fill=sleeve_c, outline=border_c)
            if has_armor:
                # Pauldrons
                draw.rectangle((bx + 6, torso_top, bx + 9, torso_top + 3), fill=C_STEEL)
                draw.rectangle((bx + 22, torso_top, bx + 25, torso_top + 3), fill=C_STEEL)
        else: # Left/Right
            draw.rectangle((bx + 11, torso_top, bx + 20, torso_bot), fill=main_c, outline=border_c)
            draw.rectangle((bx + 11, by + 30 + bob_y, bx + 20, by + 32 + bob_y), fill=C_BELT)
            arm_y = torso_top + 2
            arm_x = (bx + 10) if d == "left" else (bx + 19)
            draw.rectangle((arm_x, arm_y, arm_x + 3, arm_y + 8), fill=sleeve_c, outline=border_c)
            if has_armor:
                draw.rectangle((arm_x - 1, torso_top, arm_x + 4, torso_top + 3), fill=C_STEEL)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "shirt", "upper_body", description=desc)

def generate_all_shirts():
    generate_shirt_generic("shirt_tunic_001", "Forest Tunic", (45, 105, 52, 255), (32, 75, 38, 255),
                           desc="Simple classic forest fantasy tunic.")
    generate_shirt_generic("shirt_leather_001", "Leather Jacket", (105, 65, 35, 255), (75, 45, 25, 255),
                           desc="Explorer-style studded leather jacket.")
    generate_shirt_generic("shirt_armor_001", "Light Armor", (115, 105, 85, 255), (85, 75, 60, 255),
                           has_armor=True, desc="Lightweight steel breastplate and pauldrons.")
    generate_shirt_generic("shirt_traveler_001", "Traveler Shirt", (230, 220, 200, 255), (190, 180, 160, 255),
                           desc="Practical linen adventure clothing with rolled cuffs.")
    generate_shirt_generic("shirt_mage_001", "Mystic Robe", (85, 40, 120, 255), (210, 175, 50, 255),
                           sleeve_c=(65, 30, 95, 255), is_robe=True, desc="Arcane fantasy robe with gold runic trim.")

# =============================================================================
# 3. PANTS (5 Designs)
# =============================================================================

def generate_pants_generic(item_id, name, main_c, border_c, is_shorts=False, has_armor=False, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)
    C_STEEL = (155, 165, 180, 255)

    def draw_frame(bx, by, d, anim_type, col):
        _, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)
        reach = 6 if is_shorts else 9

        if d in ["down", "up"]:
            ll_y = by + 31 + left_leg_dy
            draw.rectangle((bx + 11, by + 31, bx + 14, ll_y + reach), fill=main_c, outline=border_c)
            rl_y = by + 31 + right_leg_dy
            draw.rectangle((bx + 17, by + 31, bx + 20, rl_y + reach), fill=main_c, outline=border_c)
            if has_armor:
                draw.rectangle((bx + 11, by + 33, bx + 14, by + 36), fill=C_STEEL)
                draw.rectangle((bx + 17, by + 33, bx + 20, by + 36), fill=C_STEEL)
        elif d in ["left", "right"]:
            l_y1 = by + 31 + left_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + reach), fill=main_c, outline=border_c)
            if has_armor:
                draw.rectangle((bx + 13, by + 33, bx + 18, by + 36), fill=C_STEEL)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "pants", "lower_body", description=desc)

def generate_all_pants():
    generate_pants_generic("pants_traveler_001", "Traveler Pants", (75, 50, 35, 255), (55, 35, 25, 255),
                           desc="Standard durable adventure trousers.")
    generate_pants_generic("pants_leather_001", "Leather Pants", (40, 42, 48, 255), (28, 30, 35, 255),
                           desc="Dark fitted leather trousers.")
    generate_pants_generic("pants_armored_001", "Armored Pants", (65, 70, 80, 255), (45, 50, 60, 255),
                           has_armor=True, desc="Lightweight steel-reinforced greave pants.")
    generate_pants_generic("pants_short_001", "Adventurer Shorts", (65, 80, 95, 255), (45, 60, 75, 255),
                           is_shorts=True, desc="Lightweight explorer shorts ending above knees.")
    generate_pants_generic("pants_mage_001", "Mystic Trousers", (70, 35, 95, 255), (190, 160, 45, 255),
                           desc="Flowing arcane violet trousers with gold stitch.")

# =============================================================================
# 4. SHOES (3 Designs)
# =============================================================================

def generate_shoes_generic(item_id, name, main_c, hilight_c, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        _, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)

        if d in ["down", "up"]:
            ll_y = by + 32 + left_leg_dy
            draw.rectangle((bx + 11, ll_y + 7, bx + 14, ll_y + 11), fill=main_c, outline=hilight_c)
            rl_y = by + 32 + right_leg_dy
            draw.rectangle((bx + 17, ll_y + 7, bx + 20, ll_y + 11), fill=main_c, outline=hilight_c)
        elif d == "left":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 12, l_y1 + 7, bx + 17, l_y1 + 11), fill=main_c, outline=hilight_c)
            if anim_type == "walk" and col in [0, 2]:
                l_y2 = by + 32 + right_leg_dy
                draw.rectangle((bx + 16, l_y2 + 7, bx + 20, l_y2 + 10), fill=hilight_c)
        elif d == "right":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 14, l_y1 + 7, bx + 19, l_y1 + 11), fill=main_c, outline=hilight_c)
            if anim_type == "walk" and col in [0, 2]:
                l_y2 = by + 32 + right_leg_dy
                draw.rectangle((bx + 11, l_y2 + 7, bx + 15, l_y2 + 10), fill=hilight_c)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "shoes", "footwear", description=desc)

def generate_all_shoes():
    generate_shoes_generic("shoes_boots_001", "Leather Boots", (60, 38, 22, 255), (85, 55, 32, 255),
                           desc="Standard sturdy adventurer leather boots.")
    generate_shoes_generic("shoes_armored_001", "Armored Boots", (160, 170, 185, 255), (115, 125, 140, 255),
                           desc="Protective steel armored sabatons.")
    generate_shoes_generic("shoes_light_001", "Light Shoes", (115, 85, 55, 255), (145, 110, 75, 255),
                           desc="Soft lightweight leather moccasin travel shoes.")

# =============================================================================
# 5. HATS (5 Designs)
# =============================================================================

def generate_hat_generic(item_id, name, hat_type, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    C_BROWN = (90, 60, 35, 255)
    C_GOLD = (220, 180, 50, 255)
    C_GREEN = (40, 85, 45, 255)
    C_STEEL = (155, 165, 180, 255)
    C_PURPLE = (75, 35, 110, 255)

    occlusion = []
    if hat_type == "helmet":
        occlusion = ["hair_hide_all"]

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if hat_type == "adventurer":
            # Wide brim fedora
            draw.rectangle((cx - 9, cy - 5, cx + 9, cy - 3), fill=C_BROWN)
            draw.rectangle((cx - 6, cy - 9, cx + 6, cy - 5), fill=C_BROWN)
            draw.line([(cx - 6, cy - 5), (cx + 6, cy - 5)], fill=C_GOLD)
        elif hat_type == "hood":
            # Explorer cloth hood
            if d == "down":
                draw.rectangle((cx - 7, cy - 8, cx + 7, cy - 1), fill=C_GREEN)
                draw.rectangle((cx - 8, cy - 2, cx - 6, cy + 6), fill=C_GREEN)
                draw.rectangle((cx + 6, cy - 2, cx + 8, cy + 6), fill=C_GREEN)
            elif d == "up":
                draw.rectangle((cx - 7, cy - 8, cx + 7, cy + 6), fill=C_GREEN)
                draw.point((cx, cy + 7), fill=C_GOLD)
            elif d == "left":
                draw.rectangle((cx - 6, cy - 8, cx + 7, cy - 1), fill=C_GREEN)
                draw.rectangle((cx + 2, cy - 2, cx + 7, cy + 6), fill=C_GREEN)
            elif d == "right":
                draw.rectangle((cx - 7, cy - 8, cx + 6, cy - 1), fill=C_GREEN)
                draw.rectangle((cx - 7, cy - 2, cx - 2, cy + 6), fill=C_GREEN)
        elif hat_type == "helmet":
            # Medieval iron bascinet
            draw.rectangle((cx - 7, cy - 8, cx + 7, cy + 4), fill=C_STEEL, outline=(110, 120, 135, 255))
            draw.rectangle((cx - 2, cy - 10, cx + 2, cy - 7), fill=C_GOLD)
            if d == "down":
                draw.rectangle((cx - 5, cy - 1, cx + 5, cy + 1), fill=(25, 25, 30, 255))
            elif d in ["left", "right"]:
                vx = cx - 5 if d == "left" else cx + 1
                draw.rectangle((vx, cy - 1, vx + 4, cy + 1), fill=(25, 25, 30, 255))
        elif hat_type == "wizard":
            # Pointed mystic wizard hat
            draw.rectangle((cx - 9, cy - 5, cx + 9, cy - 3), fill=C_PURPLE)
            draw.polygon([(cx - 5, cy - 5), (cx + 5, cy - 5), (cx, cy - 14)], fill=C_PURPLE)
            draw.rectangle((cx - 2, cy - 7, cx + 2, cy - 5), fill=C_GOLD)
        elif hat_type == "cap":
            # Traveler flat cap
            draw.rectangle((cx - 6, cy - 8, cx + 6, cy - 3), fill=(110, 75, 45, 255))
            if d == "down":
                draw.rectangle((cx - 7, cy - 4, cx + 7, cy - 2), fill=(85, 55, 30, 255))
            elif d == "left":
                draw.rectangle((cx - 8, cy - 4, cx + 4, cy - 2), fill=(85, 55, 30, 255))
            elif d == "right":
                draw.rectangle((cx - 4, cy - 4, cx + 8, cy - 2), fill=(85, 55, 30, 255))

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "hat", "headwear", occlusion_flags=occlusion, description=desc)

def generate_all_hats():
    generate_hat_generic("hat_adventurer_001", "Adventurer Hat", "adventurer", desc="Classic wide-brim fantasy explorer hat.")
    generate_hat_generic("hat_hood_001", "Explorer Hood", "hood", desc="Protective cloth cowl hood.")
    generate_hat_generic("hat_helmet_001", "Iron Helmet", "helmet", desc="Medieval steel helmet with visor slit.")
    generate_hat_generic("hat_wizard_001", "Mystic Hat", "wizard", desc="Pointed arcane wizard hat with gold brooch.")
    generate_hat_generic("hat_cap_001", "Traveler Cap", "cap", desc="Simple flat adventure travel cap.")

# =============================================================================
# 6. GOGGLES (3 Designs)
# =============================================================================

def generate_goggles_generic(item_id, name, g_type, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "up": return # Occluded from back view

        if g_type == "adventure":
            C_FRAME = (195, 145, 45, 255)
            C_LENS = (80, 210, 230, 255)
            C_STRAP = (50, 35, 25, 255)
            if d == "down":
                draw.line([(cx - 6, cy - 4), (cx + 6, cy - 4)], fill=C_STRAP)
                draw.rectangle((cx - 5, cy - 6, cx - 1, cy - 2), fill=C_FRAME, outline=C_LENS)
                draw.rectangle((cx + 1, cy - 6, cx + 5, cy - 2), fill=C_FRAME, outline=C_LENS)
            elif d == "left":
                draw.line([(cx - 4, cy - 4), (cx + 5, cy - 4)], fill=C_STRAP)
                draw.rectangle((cx - 6, cy - 6, cx - 2, cy - 2), fill=C_FRAME, outline=C_LENS)
            elif d == "right":
                draw.line([(cx - 5, cy - 4), (cx + 4, cy - 4)], fill=C_STRAP)
                draw.rectangle((cx + 2, cy - 6, cx + 6, cy - 2), fill=C_FRAME, outline=C_LENS)

        elif g_type == "steampunk":
            C_BRASS = (215, 160, 40, 255)
            C_AMBER = (245, 155, 35, 255)
            C_STRAP = (65, 45, 30, 255)
            if d == "down":
                draw.line([(cx - 6, cy - 4), (cx + 6, cy - 4)], fill=C_STRAP)
                draw.rectangle((cx - 6, cy - 6, cx - 1, cy - 2), fill=C_BRASS, outline=C_AMBER)
                draw.rectangle((cx + 1, cy - 6, cx + 6, cy - 2), fill=C_BRASS, outline=C_AMBER)
                draw.point((cx, cy - 4), fill=C_BRASS) # Center bridge gear
            elif d in ["left", "right"]:
                ox = -6 if d == "left" else 2
                draw.rectangle((cx + ox, cy - 6, cx + ox + 5, cy - 2), fill=C_BRASS, outline=C_AMBER)

        elif g_type == "snow":
            C_WHITE = (215, 220, 225, 255)
            C_POLAR = (40, 55, 75, 255)
            if d == "down":
                draw.rectangle((cx - 6, cy - 5, cx + 6, cy - 2), fill=C_WHITE)
                draw.rectangle((cx - 5, cy - 4, cx + 5, cy - 3), fill=C_POLAR)
            elif d in ["left", "right"]:
                ox = -6 if d == "left" else 1
                draw.rectangle((cx + ox, cy - 5, cx + ox + 6, cy - 2), fill=C_WHITE)
                draw.rectangle((cx + ox + 1, cy - 4, cx + ox + 5, cy - 3), fill=C_POLAR)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "goggles", "eyewear", description=desc)

def generate_all_goggles():
    generate_goggles_generic("goggles_adventure_001", "Adventurer Goggles", "adventure", desc="Classic explorer brass/leather goggles.")
    generate_goggles_generic("goggles_steampunk_001", "Mechanical Goggles", "steampunk", desc="Fantasy mechanical dual-lens goggles.")
    generate_goggles_generic("goggles_snow_001", "Snow Goggles", "snow", desc="Protective polarized snow eyewear.")

# =============================================================================
# 7. MASKS (3 Designs)
# =============================================================================

def generate_mask_generic(item_id, name, m_type, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    occlusion = []
    if m_type == "mystic":
        occlusion = ["mask_hide_goggles"]

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        cx = bx + 16
        cy = by + 14 + bob_y

        if d == "up": return # Occluded from back

        if m_type == "cloth":
            C_CLOTH = (45, 50, 55, 255)
            if d == "down":
                draw.polygon([(cx - 5, cy), (cx + 5, cy), (cx, cy + 5)], fill=C_CLOTH)
            elif d == "left":
                draw.polygon([(cx - 5, cy), (cx + 1, cy), (cx - 3, cy + 4)], fill=C_CLOTH)
            elif d == "right":
                draw.polygon([(cx + 5, cy), (cx - 1, cy), (cx + 3, cy + 4)], fill=C_CLOTH)

        elif m_type == "leather":
            C_LEATHER = (85, 52, 30, 255)
            C_STUD = (195, 175, 120, 255)
            if d == "down":
                draw.rectangle((cx - 5, cy, cx + 5, cy + 4), fill=C_LEATHER)
                draw.point((cx - 3, cy + 2), fill=C_STUD)
                draw.point((cx + 3, cy + 2), fill=C_STUD)
            elif d in ["left", "right"]:
                ox = -5 if d == "left" else 0
                draw.rectangle((cx + ox, cy, cx + ox + 5, cy + 4), fill=C_LEATHER)
                draw.point((cx + ox + 2, cy + 2), fill=C_STUD)

        elif m_type == "mystic":
            # Porcelain full-face mask
            C_IVORY = (235, 235, 240, 255)
            C_BLUE = (50, 110, 210, 255)
            if d == "down":
                draw.ellipse((cx - 5, cy - 4, cx + 5, cy + 5), fill=C_IVORY, outline=(180, 180, 190, 255))
                # Arcane runic slit markings
                draw.line([(cx - 3, cy - 1), (cx - 1, cy - 1)], fill=C_BLUE)
                draw.line([(cx + 1, cy - 1), (cx + 3, cy - 1)], fill=C_BLUE)
                draw.point((cx, cy + 2), fill=C_BLUE)
            elif d in ["left", "right"]:
                ox = -5 if d == "left" else 0
                draw.ellipse((cx + ox, cy - 4, cx + ox + 6, cy + 5), fill=C_IVORY)
                draw.line([(cx + ox + 2, cy - 1), (cx + ox + 4, cy - 1)], fill=C_BLUE)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "mask", "face_mask", occlusion_flags=occlusion, description=desc)

def generate_all_masks():
    generate_mask_generic("mask_cloth_001", "Cloth Mask", "cloth", desc="Simple dark cloth scout face covering.")
    generate_mask_generic("mask_leather_001", "Leather Mask", "leather", desc="Fantasy studded leather face covering.")
    generate_mask_generic("mask_mystic_001", "Mystic Mask", "mystic", desc="Ornate porcelain arcane mask (full face coverage).")

# =============================================================================
# 8. BACKPACKS (3 Designs)
# =============================================================================

def generate_backpack_generic(item_id, name, bp_type, desc=""):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    C_PACK = (115, 70, 38, 255)
    C_STRAP = (75, 45, 25, 255)
    C_BEDROLL = (55, 95, 60, 255)
    C_BUCKLE = (220, 180, 50, 255)

    if bp_type == "leather":
        C_PACK = (95, 48, 25, 255)
        C_STRAP = (65, 32, 18, 255)
    elif bp_type == "adventure":
        C_PACK = (75, 80, 60, 255) # Heavy canvas olive
        C_BEDROLL = (140, 115, 80, 255) # Canvas tent roll

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        torso_top = by + 20 + bob_y

        if d == "up":
            # Viewed from behind: full backpack on back
            draw.rectangle((bx + 10, torso_top + 1, bx + 21, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            if bp_type in ["traveler", "adventure"]:
                draw.rectangle((bx + 9, torso_top - 2, bx + 22, torso_top), fill=C_BEDROLL, outline=C_STRAP)
            draw.rectangle((bx + 12, torso_top + 4, bx + 14, torso_top + 6), fill=C_BUCKLE)
            draw.rectangle((bx + 17, torso_top + 4, bx + 19, torso_top + 6), fill=C_BUCKLE)
        elif d == "down":
            # Viewed from front: shoulder straps
            draw.rectangle((bx + 10, torso_top + 2, bx + 12, torso_top + 10), fill=C_STRAP)
            draw.rectangle((bx + 19, torso_top + 2, bx + 21, torso_top + 10), fill=C_STRAP)
            draw.rectangle((bx + 10, torso_top + 5, bx + 12, torso_top + 7), fill=C_BUCKLE)
            draw.rectangle((bx + 19, torso_top + 5, bx + 21, torso_top + 7), fill=C_BUCKLE)
        elif d == "left":
            draw.rectangle((bx + 19, torso_top + 2, bx + 24, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            if bp_type in ["traveler", "adventure"]:
                draw.rectangle((bx + 18, torso_top - 1, bx + 23, torso_top + 1), fill=C_BEDROLL)
            draw.line([(bx + 13, torso_top + 3), (bx + 19, torso_top + 5)], fill=C_STRAP)
        elif d == "right":
            draw.rectangle((bx + 7, torso_top + 2, bx + 12, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            if bp_type in ["traveler", "adventure"]:
                draw.rectangle((bx + 8, torso_top - 1, bx + 13, torso_top + 1), fill=C_BEDROLL)
            draw.line([(bx + 18, torso_top + 3), (bx + 12, torso_top + 5)], fill=C_STRAP)

    for r, d in enumerate(DIRECTIONS):
        for c in range(COLS): draw_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
        for c in range(COLS): draw_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

    save_production_cosmetic(img, item_id, name, "back", "back_accessory", description=desc)

def generate_all_backpacks():
    generate_backpack_generic("backpack_traveler_001", "Traveler Backpack", "traveler", desc="Standard explorer rucksack with green bedroll.")
    generate_backpack_generic("backpack_leather_001", "Leather Backpack", "leather", desc="Traditional compact leather pack with side buckles.")
    generate_backpack_generic("backpack_adventure_001", "Adventure Pack", "adventure", desc="Heavy expedition pack with canvas tent roll.")

# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    print("=" * 60)
    print(" Wildbound Phase 8 — Generating 32 Production Cosmetic Assets")
    print("=" * 60)

    generate_hair_short_001()
    generate_hair_long_001()
    generate_hair_messy_001()
    generate_hair_ponytail_001()
    generate_hair_curly_001()

    generate_all_shirts()
    generate_all_pants()
    generate_all_shoes()
    generate_all_hats()
    generate_all_goggles()
    generate_all_masks()
    generate_all_backpacks()

    print("=" * 60)
    print(" Successfully generated all 32 production cosmetic assets!")
    print("=" * 60)

if __name__ == "__main__":
    main()

