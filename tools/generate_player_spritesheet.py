import os
from PIL import Image, ImageDraw

def create_player_spritesheet():
    FRAME_W = 32
    FRAME_H = 48
    COLS = 4
    ROWS = 8

    SHEET_W = COLS * FRAME_W # 128
    SHEET_H = ROWS * FRAME_H # 384

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Color Palette for Fantasy Adventurer
    C_SKIN = (245, 200, 160, 255)
    C_SKIN_SHADOW = (215, 165, 125, 255)
    C_HAIR = (130, 75, 35, 255)
    C_HAIR_HIGHLIGHT = (165, 100, 50, 255)
    C_TUNIC = (55, 115, 60, 255)       # Forest green
    C_TUNIC_LIGHT = (75, 145, 80, 255)
    C_TUNIC_SHADOW = (38, 85, 42, 255)
    C_BELT = (90, 50, 25, 255)
    C_BUCKLE = (235, 195, 60, 255)
    C_PANTS = (45, 55, 75, 255)        # Navy dark trousers
    C_PANTS_LIGHT = (65, 80, 105, 255)
    C_BOOTS = (65, 40, 20, 255)        # Leather brown boots
    C_BOOTS_LIGHT = (90, 60, 30, 255)
    C_EYES = (30, 40, 50, 255)
    C_WHITE = (255, 255, 255, 255)
    C_PACK = (110, 70, 35, 255)        # Traveler's satchel / backpack

    def draw_character_frame(col, row, direction, anim_type, step_phase):
        # Base top-left of this frame
        bx = col * FRAME_W
        by = row * FRAME_H

        # Bobbing height for walking steps
        bob_y = 0
        if anim_type == "walk":
            if step_phase in [1, 3]:
                bob_y = -1
        elif anim_type == "idle":
            if step_phase in [1, 2]:
                bob_y = -1

        # Head center: (bx + 16, by + 14 + bob_y)
        head_cx = bx + 16
        head_cy = by + 14 + bob_y

        # Torso center: (bx + 16, by + 26 + bob_y)
        torso_top = by + 20 + bob_y
        torso_bot = by + 32 + bob_y

        # Leg offsets based on step_phase (0=neutral/contact1, 1=passing1, 2=contact2, 3=passing2)
        left_leg_dy = 0
        right_leg_dy = 0
        if anim_type == "walk":
            if step_phase == 0:
                left_leg_dy = -2
                right_leg_dy = 2
            elif step_phase == 1:
                left_leg_dy = 0
                right_leg_dy = 0
            elif step_phase == 2:
                left_leg_dy = 2
                right_leg_dy = -2
            elif step_phase == 3:
                left_leg_dy = 0
                right_leg_dy = 0

        # Draw Back items if facing UP
        if direction == "up":
            # Backpack on the back
            draw.rectangle((bx + 11, torso_top + 2, bx + 20, torso_top + 10), fill=C_PACK, outline=C_BELT)

        # 1. Legs and Boots
        if direction in ["down", "up"]:
            # Left leg
            ll_y = by + 32 + left_leg_dy
            draw.rectangle((bx + 11, by + 31, bx + 14, ll_y + 6), fill=C_PANTS)
            draw.rectangle((bx + 11, ll_y + 7, bx + 14, ll_y + 11), fill=C_BOOTS, outline=C_BOOTS_LIGHT)

            # Right leg
            rl_y = by + 32 + right_leg_dy
            draw.rectangle((bx + 17, by + 31, bx + 20, rl_y + 6), fill=C_PANTS)
            draw.rectangle((bx + 17, rl_y + 7, bx + 20, rl_y + 11), fill=C_BOOTS, outline=C_BOOTS_LIGHT)

        elif direction == "left":
            # Side profile legs
            l_y1 = by + 32 + left_leg_dy
            l_y2 = by + 32 + right_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + 6), fill=C_PANTS)
            draw.rectangle((bx + 12, l_y1 + 7, bx + 17, l_y1 + 11), fill=C_BOOTS, outline=C_BOOTS_LIGHT)
            if anim_type == "walk" and step_phase in [0, 2]:
                draw.rectangle((bx + 16, l_y2 + 7, bx + 20, l_y2 + 10), fill=C_BOOTS_LIGHT)

        elif direction == "right":
            # Side profile legs
            l_y1 = by + 32 + left_leg_dy
            l_y2 = by + 32 + right_leg_dy
            draw.rectangle((bx + 13, by + 31, bx + 18, l_y1 + 6), fill=C_PANTS)
            draw.rectangle((bx + 14, l_y1 + 7, bx + 19, l_y1 + 11), fill=C_BOOTS, outline=C_BOOTS_LIGHT)
            if anim_type == "walk" and step_phase in [0, 2]:
                draw.rectangle((bx + 11, l_y2 + 7, bx + 15, l_y2 + 10), fill=C_BOOTS_LIGHT)

        # 2. Torso & Tunic
        if direction in ["down", "up"]:
            draw.rectangle((bx + 10, torso_top, bx + 21, torso_bot), fill=C_TUNIC, outline=C_TUNIC_SHADOW)
            # Belt
            draw.rectangle((bx + 10, torso_bot - 4, bx + 21, torso_bot - 2), fill=C_BELT)
            if direction == "down":
                # Golden buckle
                draw.rectangle((bx + 14, torso_bot - 4, bx + 17, torso_bot - 2), fill=C_BUCKLE)
                # Tunic V-collar
                draw.polygon([(bx + 14, torso_top), (bx + 16, torso_top + 4), (bx + 17, torso_top)], fill=C_SKIN)
            # Arms
            arm_y1 = torso_top + 2 + (-left_leg_dy if anim_type == "walk" else 0)
            arm_y2 = torso_top + 2 + (-right_leg_dy if anim_type == "walk" else 0)
            draw.rectangle((bx + 7, arm_y1, bx + 9, arm_y1 + 8), fill=C_TUNIC, outline=C_TUNIC_SHADOW)
            draw.rectangle((bx + 7, arm_y1 + 8, bx + 9, arm_y1 + 10), fill=C_SKIN) # Hand
            draw.rectangle((bx + 22, arm_y2, bx + 24, arm_y2 + 8), fill=C_TUNIC, outline=C_TUNIC_SHADOW)
            draw.rectangle((bx + 22, arm_y2 + 8, bx + 24, arm_y2 + 10), fill=C_SKIN) # Hand
        else: # Left or Right
            draw.rectangle((bx + 11, torso_top, bx + 20, torso_bot), fill=C_TUNIC, outline=C_TUNIC_SHADOW)
            draw.rectangle((bx + 11, torso_bot - 4, bx + 20, torso_bot - 2), fill=C_BELT)
            # Arm swing
            arm_y = torso_top + 2
            arm_x = (bx + 10) if direction == "left" else (bx + 19)
            draw.rectangle((arm_x, arm_y, arm_x + 3, arm_y + 8), fill=C_TUNIC, outline=C_TUNIC_SHADOW)
            draw.rectangle((arm_x, arm_y + 8, arm_x + 3, arm_y + 10), fill=C_SKIN)

        # 3. Head & Hair
        draw.ellipse((head_cx - 6, head_cy - 6, head_cx + 6, head_cy + 6), fill=C_SKIN, outline=C_SKIN_SHADOW)

        if direction == "down":
            # Hair on top and sides
            draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 6, head_cy - 2), fill=C_HAIR)
            draw.rectangle((head_cx - 7, head_cy - 4, head_cx - 5, head_cy + 3), fill=C_HAIR)
            draw.rectangle((head_cx + 5, head_cy - 4, head_cx + 7, head_cy + 3), fill=C_HAIR)
            # Eyes
            if anim_type == "idle" and step_phase == 3:
                # Blink
                draw.line([(head_cx - 4, head_cy), (head_cx - 2, head_cy)], fill=C_EYES)
                draw.line([(head_cx + 2, head_cy), (head_cx + 4, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx - 4, head_cy - 1, head_cx - 2, head_cy + 1), fill=C_EYES)
                draw.rectangle((head_cx + 2, head_cy - 1, head_cx + 4, head_cy + 1), fill=C_EYES)
                draw.point((head_cx - 3, head_cy - 1), fill=C_WHITE)
                draw.point((head_cx + 3, head_cy - 1), fill=C_WHITE)

        elif direction == "up":
            # Hair covers back of head
            draw.ellipse((head_cx - 6, head_cy - 7, head_cx + 6, head_cy + 5), fill=C_HAIR, outline=C_HAIR_HIGHLIGHT)

        elif direction == "left":
            # Side hair
            draw.rectangle((head_cx - 5, head_cy - 7, head_cx + 6, head_cy - 2), fill=C_HAIR)
            draw.rectangle((head_cx + 2, head_cy - 4, head_cx + 7, head_cy + 4), fill=C_HAIR)
            # Single eye profile
            if anim_type == "idle" and step_phase == 3:
                draw.line([(head_cx - 5, head_cy), (head_cx - 3, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx - 5, head_cy - 1, head_cx - 3, head_cy + 1), fill=C_EYES)
                draw.point((head_cx - 4, head_cy - 1), fill=C_WHITE)

        elif direction == "right":
            # Side hair
            draw.rectangle((head_cx - 6, head_cy - 7, head_cx + 5, head_cy - 2), fill=C_HAIR)
            draw.rectangle((head_cx - 7, head_cy - 4, head_cx - 2, head_cy + 4), fill=C_HAIR)
            # Single eye profile
            if anim_type == "idle" and step_phase == 3:
                draw.line([(head_cx + 3, head_cy), (head_cx + 5, head_cy)], fill=C_EYES)
            else:
                draw.rectangle((head_cx + 3, head_cy - 1, head_cx + 5, head_cy + 1), fill=C_EYES)
                draw.point((head_cx + 4, head_cy - 1), fill=C_WHITE)

    # Generate all rows
    # Rows 0..3: Walking (Down, Up, Left, Right)
    directions = ["down", "up", "left", "right"]
    for r_idx, d in enumerate(directions):
        for col in range(COLS):
            draw_character_frame(col, r_idx, d, "walk", col)

    # Rows 4..7: Idle (Down, Up, Left, Right)
    for r_idx, d in enumerate(directions):
        for col in range(COLS):
            draw_character_frame(col, 4 + r_idx, d, "idle", col)

    os.makedirs("assets/characters/player", exist_ok=True)
    out_path = "assets/characters/player/player_spritesheet.png"
    img.save(out_path, "PNG")
    print(f"Generated {out_path} ({SHEET_W}x{SHEET_H}, 4x8 frames of {FRAME_W}x{FRAME_H}px)")

if __name__ == "__main__":
    create_player_spritesheet()

