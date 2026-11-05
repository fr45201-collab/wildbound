import os
import json
from PIL import Image, ImageDraw

def generate_goblin():
    FRAME_W = 32
    FRAME_H = 48
    COLS = 4
    ROWS = 8

    SHEET_W = COLS * FRAME_W # 128
    SHEET_H = ROWS * FRAME_H # 384

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Goblin Palette
    C_SKIN = (95, 145, 65, 255)         # Goblin green
    C_SKIN_SHADOW = (65, 105, 45, 255)
    C_TUNIC = (110, 75, 45, 255)        # Ragged leather tunic
    C_TUNIC_SHADOW = (80, 50, 30, 255)
    C_BELT = (45, 30, 20, 255)
    C_EYES = (235, 60, 45, 255)         # Glowing red/orange goblin eyes
    C_WOOD = (130, 85, 45, 255)         # Wood club
    C_LEATHER = (70, 45, 25, 255)

    def draw_goblin_frame(col, row, direction, anim_type, phase):
        bx = col * FRAME_W
        by = row * FRAME_H

        bob_y = 0
        if anim_type in ["walk", "attack"] and phase in [1, 3]:
            bob_y = -1

        # Death anim: collapse down
        if anim_type == "death":
            bob_y = phase * 3

        head_cx = bx + 16
        head_cy = by + 16 + bob_y

        # Head / Ears
        draw.ellipse([head_cx - 6, head_cy - 6, head_cx + 6, head_cy + 6], fill=C_SKIN)
        if direction in ["down", "up"]:
            # Pointed ears on both sides
            draw.polygon([(head_cx - 7, head_cy - 1), (head_cx - 11, head_cy - 4), (head_cx - 7, head_cy + 2)], fill=C_SKIN_SHADOW)
            draw.polygon([(head_cx + 7, head_cy - 1), (head_cx + 11, head_cy - 4), (head_cx + 7, head_cy + 2)], fill=C_SKIN_SHADOW)
        elif direction == "left":
            draw.polygon([(head_cx + 5, head_cy - 1), (head_cx + 10, head_cy - 4), (head_cx + 5, head_cy + 2)], fill=C_SKIN_SHADOW)
        elif direction == "right":
            draw.polygon([(head_cx - 5, head_cy - 1), (head_cx - 10, head_cy - 4), (head_cx - 5, head_cy + 2)], fill=C_SKIN_SHADOW)

        # Eyes
        if direction == "down":
            draw.point((head_cx - 3, head_cy), fill=C_EYES)
            draw.point((head_cx + 3, head_cy), fill=C_EYES)
        elif direction == "left":
            draw.point((head_cx - 4, head_cy), fill=C_EYES)
        elif direction == "right":
            draw.point((head_cx + 4, head_cy), fill=C_EYES)

        # Torso
        torso_top = by + 22 + bob_y
        torso_bot = by + 34 + bob_y
        draw.rectangle([bx + 11, torso_top, bx + 21, torso_bot], fill=C_TUNIC)
        draw.rectangle([bx + 10, torso_bot - 4, bx + 22, torso_bot - 1], fill=C_BELT)

        # Legs / Feet
        l_dy = 0
        r_dy = 0
        if anim_type == "walk":
            if phase == 0: l_dy = -2; r_dy = 2
            elif phase == 2: l_dy = 2; r_dy = -2

        draw.rectangle([bx + 11, torso_bot, bx + 15, by + 44 + l_dy + bob_y], fill=C_LEATHER)
        draw.rectangle([bx + 17, torso_bot, bx + 21, by + 44 + r_dy + bob_y], fill=C_LEATHER)

        # Weapon / Attack
        if anim_type == "attack":
            # Draw swinging wooden club
            swing_offset = phase * 4
            if direction == "down":
                draw.rectangle([bx + 20, torso_top + swing_offset, bx + 24, torso_top + 14 + swing_offset], fill=C_WOOD)
            elif direction == "up":
                draw.rectangle([bx + 20, torso_top - 6 - swing_offset, bx + 24, torso_top + 8 - swing_offset], fill=C_WOOD)
            elif direction == "left":
                draw.rectangle([bx + 6 - swing_offset, torso_top + 2, bx + 16 - swing_offset, torso_top + 6], fill=C_WOOD)
            elif direction == "right":
                draw.rectangle([bx + 16 + swing_offset, torso_top + 2, bx + 26 + swing_offset, torso_top + 6], fill=C_WOOD)
        else:
            # Idle club at side
            if direction in ["down", "right"]:
                draw.rectangle([bx + 21, torso_top + 4, bx + 24, torso_top + 14], fill=C_WOOD)
            elif direction == "left":
                draw.rectangle([bx + 8, torso_top + 4, bx + 11, torso_top + 14], fill=C_WOOD)

    # Row 0: Idle Down (4 frames)
    for c in range(4): draw_goblin_frame(c, 0, "down", "idle", c)
    # Row 1: Idle Up (4 frames)
    for c in range(4): draw_goblin_frame(c, 1, "up", "idle", c)
    # Row 2: Idle Left (4 frames)
    for c in range(4): draw_goblin_frame(c, 2, "left", "idle", c)
    # Row 3: Idle Right (4 frames)
    for c in range(4): draw_goblin_frame(c, 3, "right", "idle", c)
    # Row 4: Walk Down (4 frames)
    for c in range(4): draw_goblin_frame(c, 4, "down", "walk", c)
    # Row 5: Walk Up (4 frames)
    for c in range(4): draw_goblin_frame(c, 5, "up", "walk", c)
    # Row 6: Walk Left (4 frames)
    for c in range(4): draw_goblin_frame(c, 6, "left", "walk", c)
    # Row 7: Walk Right (4 frames)
    for c in range(4): draw_goblin_frame(c, 7, "right", "walk", c)

    out_dir = "assets/characters/enemies/goblin"
    os.makedirs(out_dir, exist_ok=True)
    out_png = os.path.join(out_dir, "goblin_spritesheet.png")
    img.save(out_png)
    print(f"Generated goblin spritesheet: {out_png}")

    # Build JSON metadata
    meta = {
        "id": "goblin_spritesheet",
        "texture": "characters/enemies/goblin/goblin_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "goblin_spritesheet.json"), "w") as f:
        json.dump(meta, f, indent=2)

    # Build animations metadata
    anims = {
        "animations": [
            { "animation": "goblin_idle_down", "spritesheet": "goblin_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "goblin_idle_up", "spritesheet": "goblin_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "goblin_idle_left", "spritesheet": "goblin_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "goblin_idle_right", "spritesheet": "goblin_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "goblin_walk_down", "spritesheet": "goblin_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "goblin_walk_up", "spritesheet": "goblin_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "goblin_walk_left", "spritesheet": "goblin_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "goblin_walk_right", "spritesheet": "goblin_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "goblin_attack_down", "spritesheet": "goblin_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [1, 17, 18, 0] },
            { "animation": "goblin_attack_up", "spritesheet": "goblin_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [5, 21, 22, 4] },
            { "animation": "goblin_attack_left", "spritesheet": "goblin_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [9, 25, 26, 8] },
            { "animation": "goblin_attack_right", "spritesheet": "goblin_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [13, 29, 30, 12] },
            { "animation": "goblin_hurt_down", "spritesheet": "goblin_spritesheet", "frame_duration": 0.15, "loop": False, "frames": [2, 0] },
            { "animation": "goblin_death_down", "spritesheet": "goblin_spritesheet", "frame_duration": 0.25, "loop": False, "frames": [1, 2, 3] }
        ]
    }
    with open(os.path.join(out_dir, "goblin_animations.json"), "w") as f:
        json.dump(anims, f, indent=2)

def generate_skeleton():
    FRAME_W = 32
    FRAME_H = 48
    COLS = 4
    ROWS = 8

    SHEET_W = COLS * FRAME_W
    SHEET_H = ROWS * FRAME_H

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Skeleton Palette
    C_BONE = (230, 225, 215, 255)
    C_BONE_SHADOW = (180, 175, 160, 255)
    C_DARK = (40, 40, 45, 255)
    C_IRON = (150, 160, 170, 255)
    C_RUST = (145, 75, 45, 255)

    def draw_skeleton_frame(col, row, direction, anim_type, phase):
        bx = col * FRAME_W
        by = row * FRAME_H

        bob_y = 0
        if anim_type in ["walk", "attack"] and phase in [1, 3]:
            bob_y = -1

        head_cx = bx + 16
        head_cy = by + 14 + bob_y

        # Skull
        draw.ellipse([head_cx - 5, head_cy - 6, head_cx + 5, head_cy + 5], fill=C_BONE)
        # Eye sockets
        if direction == "down":
            draw.rectangle([head_cx - 3, head_cy - 1, head_cx - 2, head_cy + 1], fill=C_DARK)
            draw.rectangle([head_cx + 2, head_cy - 1, head_cx + 3, head_cy + 1], fill=C_DARK)
            draw.rectangle([head_cx - 2, head_cy + 3, head_cx + 2, head_cy + 4], fill=C_BONE_SHADOW)
        elif direction == "left":
            draw.rectangle([head_cx - 4, head_cy - 1, head_cx - 2, head_cy + 1], fill=C_DARK)
        elif direction == "right":
            draw.rectangle([head_cx + 2, head_cy - 1, head_cx + 4, head_cy + 1], fill=C_DARK)

        # Spine & Ribs
        draw.line([bx + 16, by + 20 + bob_y, bx + 16, by + 34 + bob_y], fill=C_BONE, width=2)
        draw.line([bx + 12, by + 23 + bob_y, bx + 20, by + 23 + bob_y], fill=C_BONE)
        draw.line([bx + 13, by + 26 + bob_y, bx + 19, by + 26 + bob_y], fill=C_BONE)
        draw.line([bx + 14, by + 29 + bob_y, bx + 18, by + 29 + bob_y], fill=C_BONE)
        # Pelvis
        draw.rectangle([bx + 13, by + 33 + bob_y, bx + 19, by + 35 + bob_y], fill=C_BONE)

        # Legs
        l_dy = 0
        r_dy = 0
        if anim_type == "walk":
            if phase == 0: l_dy = -2; r_dy = 2
            elif phase == 2: l_dy = 2; r_dy = -2

        draw.line([bx + 14, by + 35 + bob_y, bx + 14, by + 45 + l_dy + bob_y], fill=C_BONE)
        draw.line([bx + 18, by + 35 + bob_y, bx + 18, by + 45 + r_dy + bob_y], fill=C_BONE)

        # Sword
        if direction in ["down", "right"]:
            draw.line([bx + 21, by + 22 + bob_y, bx + 21, by + 38 + bob_y], fill=C_IRON, width=2)
            draw.line([bx + 19, by + 26 + bob_y, bx + 23, by + 26 + bob_y], fill=C_RUST)
        elif direction == "left":
            draw.line([bx + 10, by + 22 + bob_y, bx + 10, by + 38 + bob_y], fill=C_IRON, width=2)
            draw.line([bx + 8, by + 26 + bob_y, bx + 12, by + 26 + bob_y], fill=C_RUST)
        elif direction == "up":
            draw.line([bx + 21, by + 18 + bob_y, bx + 21, by + 34 + bob_y], fill=C_IRON, width=2)

    # Fill rows
    for c in range(4): draw_skeleton_frame(c, 0, "down", "idle", c)
    for c in range(4): draw_skeleton_frame(c, 1, "up", "idle", c)
    for c in range(4): draw_skeleton_frame(c, 2, "left", "idle", c)
    for c in range(4): draw_skeleton_frame(c, 3, "right", "idle", c)
    for c in range(4): draw_skeleton_frame(c, 4, "down", "walk", c)
    for c in range(4): draw_skeleton_frame(c, 5, "up", "walk", c)
    for c in range(4): draw_skeleton_frame(c, 6, "left", "walk", c)
    for c in range(4): draw_skeleton_frame(c, 7, "right", "walk", c)

    out_dir = "assets/characters/enemies/skeleton"
    os.makedirs(out_dir, exist_ok=True)
    out_png = os.path.join(out_dir, "skeleton_spritesheet.png")
    img.save(out_png)
    print(f"Generated skeleton spritesheet: {out_png}")

    meta = {
        "id": "skeleton_spritesheet",
        "texture": "characters/enemies/skeleton/skeleton_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "skeleton_spritesheet.json"), "w") as f:
        json.dump(meta, f, indent=2)

    anims = {
        "animations": [
            { "animation": "skeleton_idle_down", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "skeleton_idle_up", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "skeleton_idle_left", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "skeleton_idle_right", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "skeleton_walk_down", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "skeleton_walk_up", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "skeleton_walk_left", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "skeleton_walk_right", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.15, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "skeleton_attack_down", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [1, 17, 18, 0] },
            { "animation": "skeleton_attack_up", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [5, 21, 22, 4] },
            { "animation": "skeleton_attack_left", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [9, 25, 26, 8] },
            { "animation": "skeleton_attack_right", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [13, 29, 30, 12] },
            { "animation": "skeleton_hurt_down", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.15, "loop": False, "frames": [2, 0] },
            { "animation": "skeleton_death_down", "spritesheet": "skeleton_spritesheet", "frame_duration": 0.25, "loop": False, "frames": [1, 2, 3] }
        ]
    }
    with open(os.path.join(out_dir, "skeleton_animations.json"), "w") as f:
        json.dump(anims, f, indent=2)

if __name__ == "__main__":
    generate_goblin()
    generate_skeleton()
    print("Enemy spritesheets generation complete.")

