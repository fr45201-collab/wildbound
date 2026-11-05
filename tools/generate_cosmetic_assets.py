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

def save_spritesheet(img, sheet_id, rel_dir, filename_prefix, item_meta):
    full_dir = os.path.join("assets", rel_dir)
    os.makedirs(full_dir, exist_ok=True)

    png_path = os.path.join(full_dir, f"{filename_prefix}_spritesheet.png")
    json_path = os.path.join(full_dir, f"{filename_prefix}_spritesheet.json")
    item_json_path = os.path.join(full_dir, f"{filename_prefix}.json")

    # Save PNG
    img.save(png_path, "PNG")

    # Spritesheet metadata
    rel_tex_path = f"{rel_dir}/{filename_prefix}_spritesheet.png".replace("\\", "/")
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
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=4)

    # Item metadata
    item_data = {
        "id": item_meta["id"],
        "name": item_meta["name"],
        "category": item_meta["category"],
        "spritesheet": sheet_id,
        "texture": f"tex_{filename_prefix}",
        "occlusion_flags": item_meta.get("occlusion_flags", []),
        "tint": item_meta.get("tint", [255, 255, 255, 255]),
        "anchor_offset": item_meta.get("anchor_offset", [0.0, 0.0]),
        "visible": True
    }
    with open(item_json_path, "w", encoding="utf-8") as f:
        json.dump(item_data, f, indent=4)

    print(f"Generated: {png_path} & metadata ({sheet_id})")

# =============================================================================
# Generator Functions for Each Cosmetic Type
# =============================================================================

def generate_skin(skin_id, name, skin_color, shadow_color):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    C_UNDERWEAR = (50, 50, 70, 255)
    C_EYES = (30, 40, 50, 255)
    C_WHITE = (255, 255, 255, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)
        head_cx = bx + 16
        head_cy = by + 14 + bob_y
        torso_top = by + 20 + bob_y
        torso_bot = by + 32 + bob_y

        # 1. Legs
        if d in ["down", "up"]:
            ll_y = by + 32 + left_leg_dy
            draw.rectangle((bx + 11, by + 31, bx + 14, ll_y + 11), fill=skin_color, outline=shadow_color)
            rl_y = by + 32 + right_leg_dy
            draw.rectangle((bx + 17, by + 31, bx + 20, rl_y + 11), fill=skin_color, outline=shadow_color)
        elif d == "left":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + 11), fill=skin_color, outline=shadow_color)
        elif d == "right":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + 11), fill=skin_color, outline=shadow_color)

        # 2. Torso
        if d in ["down", "up"]:
            draw.rectangle((bx + 10, torso_top, bx + 21, torso_bot), fill=skin_color, outline=shadow_color)
            # Underwear trunks
            draw.rectangle((bx + 10, torso_bot - 4, bx + 21, torso_bot), fill=C_UNDERWEAR)
            # Arms
            arm_y1 = torso_top + 2 + (-left_leg_dy if anim_type == "walk" else 0)
            arm_y2 = torso_top + 2 + (-right_leg_dy if anim_type == "walk" else 0)
            draw.rectangle((bx + 7, arm_y1, bx + 9, arm_y1 + 10), fill=skin_color, outline=shadow_color)
            draw.rectangle((bx + 22, arm_y2, bx + 24, arm_y2 + 10), fill=skin_color, outline=shadow_color)
        else: # Left/Right
            draw.rectangle((bx + 11, torso_top, bx + 20, torso_bot), fill=skin_color, outline=shadow_color)
            draw.rectangle((bx + 11, torso_bot - 4, bx + 20, torso_bot), fill=C_UNDERWEAR)
            arm_y = torso_top + 2
            arm_x = (bx + 10) if d == "left" else (bx + 19)
            draw.rectangle((arm_x, arm_y, arm_x + 3, arm_y + 10), fill=skin_color, outline=shadow_color)

        # 3. Head
        draw.ellipse((head_cx - 6, head_cy - 6, head_cx + 6, head_cy + 6), fill=skin_color, outline=shadow_color)
        if d == "down":
            if anim_type == "idle" and col == 3:
                draw.line([(head_cx - 4, head_cy), (head_cx - 2, head_cy)], fill=C_EYES)
                draw.line([(head_cx + 2, head_cy), (head_cx + 4, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx - 4, head_cy - 1, head_cx - 2, head_cy + 1), fill=C_EYES)
                draw.rectangle((head_cx + 2, head_cy - 1, head_cx + 4, head_cy + 1), fill=C_EYES)
                draw.point((head_cx - 3, head_cy - 1), fill=C_WHITE)
                draw.point((head_cx + 3, head_cy - 1), fill=C_WHITE)
        elif d == "left":
            if anim_type == "idle" and col == 3:
                draw.line([(head_cx - 5, head_cy), (head_cx - 3, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx - 5, head_cy - 1, head_cx - 3, head_cy + 1), fill=C_EYES)
                draw.point((head_cx - 4, head_cy - 1), fill=C_WHITE)
        elif d == "right":
            if anim_type == "idle" and col == 3:
                draw.line([(head_cx + 3, head_cy), (head_cx + 5, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx + 3, head_cy - 1, head_cx + 5, head_cy + 1), fill=C_EYES)
                draw.point((head_cx + 4, head_cy - 1), fill=C_WHITE)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": skin_id,
        "name": name,
        "category": "skin"
    }
    save_spritesheet(img, f"sheet_{skin_id}", "characters/cosmetics/skin", skin_id, item_meta)

def generate_hair(hair_id, name, hair_color, highlight_color, style="short"):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        head_cx = bx + 16
        head_cy = by + 14 + bob_y

        if style == "short":
            if d == "down":
                draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 6, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx - 7, head_cy - 4, head_cx - 5, head_cy + 3), fill=hair_color)
                draw.rectangle((head_cx + 5, head_cy - 4, head_cx + 7, head_cy + 3), fill=hair_color)
                draw.line([(head_cx - 4, head_cy - 6), (head_cx + 4, head_cy - 6)], fill=highlight_color)
            elif d == "up":
                draw.ellipse((head_cx - 6, head_cy - 7, head_cx + 6, head_cy + 5), fill=hair_color, outline=highlight_color)
            elif d == "left":
                draw.rectangle((head_cx - 5, head_cy - 7, head_cx + 6, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx + 2, head_cy - 4, head_cx + 7, head_cy + 4), fill=hair_color)
                draw.line([(head_cx - 2, head_cy - 6), (head_cx + 5, head_cy - 6)], fill=highlight_color)
            elif d == "right":
                draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 5, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx - 7, head_cy - 4, head_cx - 2, head_cy + 4), fill=hair_color)
                draw.line([(head_cx - 5, head_cy - 6), (head_cx + 2, head_cy - 6)], fill=highlight_color)

        elif style == "ponytail":
            ribbon_color = (195, 45, 55, 255)
            if d == "down":
                # Front bangs
                draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 6, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx - 7, head_cy - 3, head_cx - 5, head_cy + 4), fill=hair_color)
                draw.rectangle((head_cx + 5, head_cy - 3, head_cx + 7, head_cy + 4), fill=hair_color)
                # Ponytail tuft peeking over right shoulder
                draw.ellipse((head_cx + 5, head_cy - 6, head_cx + 9, head_cy + 3), fill=hair_color)
            elif d == "up":
                # Back of head + high ponytail tail
                draw.ellipse((head_cx - 6, head_cy - 7, head_cx + 6, head_cy + 4), fill=hair_color, outline=highlight_color)
                draw.rectangle((head_cx - 2, head_cy - 6, head_cx + 2, head_cy - 4), fill=ribbon_color)
                draw.rectangle((head_cx - 2, head_cy - 4, head_cx + 2, head_cy + 9), fill=hair_color)
            elif d == "left":
                draw.rectangle((head_cx - 5, head_cy - 7, head_cx + 6, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx + 2, head_cy - 4, head_cx + 7, head_cy + 3), fill=hair_color)
                # Ponytail streaming back
                draw.rectangle((head_cx + 5, head_cy - 6, head_cx + 7, head_cy - 4), fill=ribbon_color)
                draw.rectangle((head_cx + 7, head_cy - 5, head_cx + 10, head_cy + 5), fill=hair_color)
            elif d == "right":
                draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 5, head_cy - 2), fill=hair_color)
                draw.rectangle((head_cx - 7, head_cy - 4, head_cx - 2, head_cy + 3), fill=hair_color)
                # Ponytail streaming back
                draw.rectangle((head_cx - 7, head_cy - 6, head_cx - 5, head_cy - 4), fill=ribbon_color)
                draw.rectangle((head_cx - 10, head_cy - 5, head_cx - 7, head_cy + 5), fill=hair_color)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": hair_id,
        "name": name,
        "category": "hair"
    }
    save_spritesheet(img, f"sheet_{hair_id}", "characters/cosmetics/hair", hair_id, item_meta)

def generate_shirt(shirt_id, name, main_color, accent_color, sleeve_color=None):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    if sleeve_color is None:
        sleeve_color = main_color
    C_BELT = (80, 48, 24, 255)
    C_BUCKLE = (220, 180, 50, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)
        torso_top = by + 20 + bob_y
        torso_bot = by + 32 + bob_y

        if d in ["down", "up"]:
            # Main Torso
            draw.rectangle((bx + 10, torso_top, bx + 21, torso_bot), fill=main_color, outline=accent_color)
            # Belt
            draw.rectangle((bx + 10, torso_bot - 4, bx + 21, torso_bot - 2), fill=C_BELT)
            if d == "down":
                draw.rectangle((bx + 14, torso_bot - 4, bx + 17, torso_bot - 2), fill=C_BUCKLE)
            # Sleeves / Arms
            arm_y1 = torso_top + 2 + (-left_leg_dy if anim_type == "walk" else 0)
            arm_y2 = torso_top + 2 + (-right_leg_dy if anim_type == "walk" else 0)
            draw.rectangle((bx + 7, arm_y1, bx + 9, arm_y1 + 8), fill=sleeve_color, outline=accent_color)
            draw.rectangle((bx + 22, arm_y2, bx + 24, arm_y2 + 8), fill=sleeve_color, outline=accent_color)
        else: # Left/Right
            draw.rectangle((bx + 11, torso_top, bx + 20, torso_bot), fill=main_color, outline=accent_color)
            draw.rectangle((bx + 11, torso_bot - 4, bx + 20, torso_bot - 2), fill=C_BELT)
            arm_y = torso_top + 2
            arm_x = (bx + 10) if d == "left" else (bx + 19)
            draw.rectangle((arm_x, arm_y, arm_x + 3, arm_y + 8), fill=sleeve_color, outline=accent_color)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": shirt_id,
        "name": name,
        "category": "shirt"
    }
    save_spritesheet(img, f"sheet_{shirt_id}", "characters/cosmetics/shirts", shirt_id, item_meta)

def generate_pants(pants_id, name, pants_color, highlight_color, is_shorts=False):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        _, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)
        reach = 6 if is_shorts else 9

        if d in ["down", "up"]:
            ll_y = by + 31 + left_leg_dy
            draw.rectangle((bx + 11, by + 31, bx + 14, ll_y + reach), fill=pants_color, outline=highlight_color)
            rl_y = by + 31 + right_leg_dy
            draw.rectangle((bx + 17, by + 31, bx + 20, rl_y + reach), fill=pants_color, outline=highlight_color)
        elif d in ["left", "right"]:
            l_y1 = by + 31 + left_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + reach), fill=pants_color, outline=highlight_color)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": pants_id,
        "name": name,
        "category": "pants"
    }
    save_spritesheet(img, f"sheet_{pants_id}", "characters/cosmetics/pants", pants_id, item_meta)

def generate_shoes(shoes_id, name, shoe_color, highlight_color):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        _, left_leg_dy, right_leg_dy = get_frame_offsets(anim_type, col, d)

        if d in ["down", "up"]:
            ll_y = by + 32 + left_leg_dy
            draw.rectangle((bx + 11, ll_y + 7, bx + 14, ll_y + 11), fill=shoe_color, outline=highlight_color)
            rl_y = by + 32 + right_leg_dy
            draw.rectangle((bx + 17, rl_y + 7, bx + 20, rl_y + 11), fill=shoe_color, outline=highlight_color)
        elif d == "left":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 12, l_y1 + 7, bx + 17, l_y1 + 11), fill=shoe_color, outline=highlight_color)
            if anim_type == "walk" and col in [0, 2]:
                l_y2 = by + 32 + right_leg_dy
                draw.rectangle((bx + 16, l_y2 + 7, bx + 20, l_y2 + 10), fill=highlight_color)
        elif d == "right":
            l_y1 = by + 32 + left_leg_dy
            draw.rectangle((bx + 14, l_y1 + 7, bx + 19, l_y1 + 11), fill=shoe_color, outline=highlight_color)
            if anim_type == "walk" and col in [0, 2]:
                l_y2 = by + 32 + right_leg_dy
                draw.rectangle((bx + 11, l_y2 + 7, bx + 15, l_y2 + 10), fill=highlight_color)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": shoes_id,
        "name": name,
        "category": "shoes"
    }
    save_spritesheet(img, f"sheet_{shoes_id}", "characters/cosmetics/shoes", shoes_id, item_meta)

def generate_hat(hat_id, name, hat_color, trim_color, is_helm=False):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        head_cx = bx + 16
        head_cy = by + 14 + bob_y

        if is_helm:
            # Full Iron Helm
            if d == "down":
                draw.rectangle((head_cx - 7, head_cy - 8, head_cx + 7, head_cy + 4), fill=hat_color, outline=trim_color)
                # Eye visor slit
                draw.rectangle((head_cx - 5, head_cy - 1, head_cx + 5, head_cy + 1), fill=(25, 25, 30, 255))
                # Brass crest
                draw.rectangle((head_cx - 2, head_cy - 10, head_cx + 2, head_cy - 7), fill=(225, 185, 45, 255))
            elif d == "up":
                draw.rectangle((head_cx - 7, head_cy - 8, head_cx + 7, head_cy + 4), fill=hat_color, outline=trim_color)
                draw.rectangle((head_cx - 2, head_cy - 10, head_cx + 2, head_cy - 7), fill=(225, 185, 45, 255))
            elif d in ["left", "right"]:
                draw.rectangle((head_cx - 6, head_cy - 8, head_cx + 6, head_cy + 4), fill=hat_color, outline=trim_color)
                draw.rectangle((head_cx - 2, head_cy - 10, head_cx + 2, head_cy - 7), fill=(225, 185, 45, 255))
                vx = head_cx - 5 if d == "left" else head_cx + 1
                draw.rectangle((vx, head_cy - 1, vx + 4, head_cy + 1), fill=(25, 25, 30, 255))
        else:
            # Ranger Hood
            if d == "down":
                draw.rectangle((head_cx - 7, head_cy - 8, head_cx + 7, head_cy - 1), fill=hat_color, outline=trim_color)
                draw.rectangle((head_cx - 8, head_cy - 2, head_cx - 6, head_cy + 6), fill=hat_color)
                draw.rectangle((head_cx + 6, head_cy - 2, head_cx + 8, head_cy + 6), fill=hat_color)
            elif d == "up":
                draw.rectangle((head_cx - 7, head_cy - 8, head_cx + 7, head_cy + 6), fill=hat_color, outline=trim_color)
                draw.point((head_cx, head_cy + 7), fill=trim_color)
            elif d == "left":
                draw.rectangle((head_cx - 6, head_cy - 8, head_cx + 7, head_cy - 1), fill=hat_color, outline=trim_color)
                draw.rectangle((head_cx + 2, head_cy - 2, head_cx + 7, head_cy + 6), fill=hat_color)
            elif d == "right":
                draw.rectangle((head_cx - 7, head_cy - 8, head_cx + 6, head_cy - 1), fill=hat_color, outline=trim_color)
                draw.rectangle((head_cx - 7, head_cy - 2, head_cx - 2, head_cy + 6), fill=hat_color)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": hat_id,
        "name": name,
        "category": "headwear",
        "occlusion_flags": ["hair_hide_all"] if is_helm else []
    }
    save_spritesheet(img, f"sheet_{hat_id}", "characters/cosmetics/hats", hat_id, item_meta)

def generate_face(face_id, name, face_type):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        head_cx = bx + 16
        head_cy = by + 14 + bob_y

        if d == "up":
            return  # Not visible from behind

        if face_type == "eyepatch":
            C_PATCH = (25, 25, 30, 255)
            C_STRAP = (60, 45, 35, 255)
            if d == "down":
                draw.line([(head_cx - 6, head_cy - 3), (head_cx + 5, head_cy + 3)], fill=C_STRAP)
                draw.rectangle((head_cx - 5, head_cy - 2, head_cx - 1, head_cy + 2), fill=C_PATCH)
            elif d == "left":
                draw.line([(head_cx - 5, head_cy - 2), (head_cx + 4, head_cy + 2)], fill=C_STRAP)
                draw.rectangle((head_cx - 6, head_cy - 2, head_cx - 2, head_cy + 2), fill=C_PATCH)
            elif d == "right":
                draw.line([(head_cx - 4, head_cy + 2), (head_cx + 5, head_cy - 2)], fill=C_STRAP)
        elif face_type == "goggles":
            C_BRASS = (195, 145, 45, 255)
            C_LENS = (80, 210, 230, 255)
            C_STRAP = (50, 35, 25, 255)
            if d == "down":
                draw.line([(head_cx - 6, head_cy - 4), (head_cx + 6, head_cy - 4)], fill=C_STRAP)
                draw.rectangle((head_cx - 5, head_cy - 6, head_cx - 1, head_cy - 2), fill=C_BRASS, outline=C_LENS)
                draw.rectangle((head_cx + 1, head_cy - 6, head_cx + 5, head_cy - 2), fill=C_BRASS, outline=C_LENS)
            elif d == "left":
                draw.line([(head_cx - 4, head_cy - 4), (head_cx + 5, head_cy - 4)], fill=C_STRAP)
                draw.rectangle((head_cx - 6, head_cy - 6, head_cx - 2, head_cy - 2), fill=C_BRASS, outline=C_LENS)
            elif d == "right":
                draw.line([(head_cx - 5, head_cy - 4), (head_cx + 4, head_cy - 4)], fill=C_STRAP)
                draw.rectangle((head_cx + 2, head_cy - 6, head_cx + 6, head_cy - 2), fill=C_BRASS, outline=C_LENS)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": face_id,
        "name": name,
        "category": "face"
    }
    save_spritesheet(img, f"sheet_{face_id}", "characters/cosmetics/face", face_id, item_meta)

def generate_backpack(pack_id, name):
    img = create_transparent_sheet()
    draw = ImageDraw.Draw(img)

    C_PACK = (115, 70, 38, 255)
    C_STRAP = (75, 45, 25, 255)
    C_BEDROLL = (55, 95, 60, 255)
    C_BUCKLE = (220, 180, 50, 255)

    def draw_frame(bx, by, d, anim_type, col):
        bob_y, _, _ = get_frame_offsets(anim_type, col, d)
        torso_top = by + 20 + bob_y

        if d == "up":
            # Viewed from behind: full backpack on back
            draw.rectangle((bx + 10, torso_top + 1, bx + 21, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            # Bedroll on top
            draw.rectangle((bx + 9, torso_top - 2, bx + 22, torso_top), fill=C_BEDROLL, outline=C_STRAP)
            # Buckles
            draw.rectangle((bx + 12, torso_top + 4, bx + 14, torso_top + 6), fill=C_BUCKLE)
            draw.rectangle((bx + 17, torso_top + 4, bx + 19, torso_top + 6), fill=C_BUCKLE)
        elif d == "down":
            # Viewed from front: just shoulder straps
            draw.rectangle((bx + 10, torso_top + 2, bx + 12, torso_top + 10), fill=C_STRAP)
            draw.rectangle((bx + 19, torso_top + 2, bx + 21, torso_top + 10), fill=C_STRAP)
            draw.rectangle((bx + 10, torso_top + 5, bx + 12, torso_top + 7), fill=C_BUCKLE)
            draw.rectangle((bx + 19, torso_top + 5, bx + 21, torso_top + 7), fill=C_BUCKLE)
        elif d == "left":
            # Viewed from left side: backpack protruding from right (back)
            draw.rectangle((bx + 19, torso_top + 2, bx + 24, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            draw.rectangle((bx + 18, torso_top - 1, bx + 23, torso_top + 1), fill=C_BEDROLL)
            draw.line([(bx + 13, torso_top + 3), (bx + 19, torso_top + 5)], fill=C_STRAP)
        elif d == "right":
            # Viewed from right side: backpack protruding from left (back)
            draw.rectangle((bx + 7, torso_top + 2, bx + 12, torso_top + 10), fill=C_PACK, outline=C_STRAP)
            draw.rectangle((bx + 8, torso_top - 1, bx + 13, torso_top + 1), fill=C_BEDROLL)
            draw.line([(bx + 18, torso_top + 3), (bx + 12, torso_top + 5)], fill=C_STRAP)

    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, r_idx * FRAME_H, d, "walk", col)
    for r_idx, d in enumerate(DIRECTIONS):
        for col in range(COLS):
            draw_frame(col * FRAME_W, (4 + r_idx) * FRAME_H, d, "idle", col)

    item_meta = {
        "id": pack_id,
        "name": name,
        "category": "back"
    }
    save_spritesheet(img, f"sheet_{pack_id}", "characters/cosmetics/back", pack_id, item_meta)

# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    print("Generating Wildbound Phase 7 Modular Character Cosmetic Assets...")

    # 1. Skin
    generate_skin("skin_default", "Default Skin", (245, 200, 160, 255), (215, 165, 125, 255))
    generate_skin("skin_tan", "Sun-Kissed Tan Skin", (205, 145, 95, 255), (170, 115, 70, 255))

    # 2. Hair
    generate_hair("hair_short_brown", "Short Brown Hair", (115, 65, 30, 255), (155, 95, 45, 255), "short")
    generate_hair("hair_ponytail_blonde", "Blonde Ponytail", (235, 195, 65, 255), (255, 225, 110, 255), "ponytail")

    # 3. Shirts
    generate_shirt("shirt_forest_tunic", "Forest Tunic", (45, 105, 52, 255), (32, 75, 38, 255))
    generate_shirt("shirt_adventurer_vest", "Adventurer Vest", (35, 75, 145, 255), (25, 55, 110, 255), (235, 230, 215, 255))

    # 4. Pants
    generate_pants("pants_traveler_trousers", "Traveler Trousers", (75, 50, 35, 255), (55, 35, 25, 255), is_shorts=False)
    generate_pants("pants_scout_shorts", "Scout Shorts", (65, 80, 95, 255), (45, 60, 75, 255), is_shorts=True)

    # 5. Shoes
    generate_shoes("shoes_leather_boots", "Leather Boots", (55, 35, 20, 255), (75, 50, 30, 255))
    generate_shoes("shoes_iron_greaves", "Iron Greaves", (165, 175, 190, 255), (115, 125, 140, 255))

    # 6. Headwear
    generate_hat("hat_ranger_hood", "Ranger Hood", (40, 85, 45, 255), (28, 62, 32, 255), is_helm=False)
    generate_hat("hat_iron_helm", "Iron Helmet", (155, 165, 180, 255), (110, 120, 135, 255), is_helm=True)

    # 7. Face
    generate_face("face_eyepatch", "Rogue Eyepatch", "eyepatch")
    generate_face("face_explorer_goggles", "Explorer Goggles", "goggles")

    # 8. Back
    generate_backpack("back_traveler_backpack", "Traveler Backpack")

    print("All 15 cosmetic assets generated successfully!")

if __name__ == "__main__":
    main()

