import os
import json
from PIL import Image, ImageDraw

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

# =============================================================================
# 1. EXPAND GOBLIN & SKELETON ANIMATIONS & METADATA
# =============================================================================

def expand_goblin_and_skeleton():
    print("Expanding Goblin and Skeleton metadata...")

    # 1. Goblin Animations Expansion
    goblin_anim_path = "assets/characters/enemies/goblin/goblin_animations.json"
    with open(goblin_anim_path, "r", encoding="utf-8") as f:
        g_data = json.load(f)

    existing_anims = {a["animation"] for a in g_data["animations"]}

    # Add Run animations (rows 4..7 walk frames played faster at 0.09s per frame)
    run_configs = [
        ("goblin_run_down", [16, 17, 18, 19]),
        ("goblin_run_up", [20, 21, 22, 23]),
        ("goblin_run_left", [24, 25, 26, 27]),
        ("goblin_run_right", [28, 29, 30, 31]),
        ("goblin_hurt_up", [6, 4]),
        ("goblin_hurt_left", [10, 8]),
        ("goblin_hurt_right", [14, 12]),
        ("goblin_death_up", [5, 6, 7]),
        ("goblin_death_left", [9, 10, 11]),
        ("goblin_death_right", [13, 14, 15]),
    ]
    for anim_name, frames in run_configs:
        if anim_name not in existing_anims:
            g_data["animations"].append({
                "animation": anim_name,
                "spritesheet": "goblin_spritesheet",
                "frame_duration": 0.09 if "run" in anim_name else 0.15,
                "loop": "run" in anim_name,
                "frames": frames
            })

    with open(goblin_anim_path, "w", encoding="utf-8") as f:
        json.dump(g_data, f, indent=2)

    # Goblin Creature Metadata
    goblin_meta = {
        "id": "creature_goblin_001",
        "name": "Forest Goblin",
        "category": "hostile_creature",
        "biome": ["forest"],
        "sprite": "characters/enemies/goblin/goblin_spritesheet.png",
        "spritesheet": "goblin_spritesheet",
        "dimensions": {
            "frame_width": 32,
            "frame_height": 48,
            "columns": 4,
            "rows": 8,
            "frame_count": 32
        },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "goblin_idle_down",
            "walk": "goblin_walk_down",
            "run": "goblin_run_down",
            "attack": "goblin_attack_down",
            "hurt": "goblin_hurt_down",
            "death": "goblin_death_down"
        },
        "anchors": {
            "ground": [16, 48]
        },
        "collision": {
            "width": 16.0,
            "height": 12.0
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Original asset",
            "license": "CC0 1.0 Universal"
        }
    }
    with open("assets/characters/enemies/goblin/creature_metadata.json", "w", encoding="utf-8") as f:
        json.dump(goblin_meta, f, indent=2)

    # 2. Skeleton Animations Expansion
    skel_anim_path = "assets/characters/enemies/skeleton/skeleton_animations.json"
    with open(skel_anim_path, "r", encoding="utf-8") as f:
        s_data = json.load(f)

    existing_s_anims = {a["animation"] for a in s_data["animations"]}
    skel_run_configs = [
        ("skeleton_run_down", [16, 17, 18, 19]),
        ("skeleton_run_up", [20, 21, 22, 23]),
        ("skeleton_run_left", [24, 25, 26, 27]),
        ("skeleton_run_right", [28, 29, 30, 31]),
        ("skeleton_hurt_up", [6, 4]),
        ("skeleton_hurt_left", [10, 8]),
        ("skeleton_hurt_right", [14, 12]),
        ("skeleton_death_up", [5, 6, 7]),
        ("skeleton_death_left", [9, 10, 11]),
        ("skeleton_death_right", [13, 14, 15]),
    ]
    for anim_name, frames in skel_run_configs:
        if anim_name not in existing_s_anims:
            s_data["animations"].append({
                "animation": anim_name,
                "spritesheet": "skeleton_spritesheet",
                "frame_duration": 0.09 if "run" in anim_name else 0.15,
                "loop": "run" in anim_name,
                "frames": frames
            })

    with open(skel_anim_path, "w", encoding="utf-8") as f:
        json.dump(s_data, f, indent=2)

    # Skeleton Creature Metadata
    skel_meta = {
        "id": "creature_skeleton_001",
        "name": "Undead Skeleton",
        "category": "hostile_creature",
        "biome": ["forest", "ruins", "dungeon"],
        "sprite": "characters/enemies/skeleton/skeleton_spritesheet.png",
        "spritesheet": "skeleton_spritesheet",
        "dimensions": {
            "frame_width": 32,
            "frame_height": 48,
            "columns": 4,
            "rows": 8,
            "frame_count": 32
        },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "skeleton_idle_down",
            "walk": "skeleton_walk_down",
            "run": "skeleton_run_down",
            "attack": "skeleton_attack_down",
            "hurt": "skeleton_hurt_down",
            "death": "skeleton_death_down"
        },
        "anchors": {
            "ground": [16, 48]
        },
        "collision": {
            "width": 16.0,
            "height": 12.0
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Original asset",
            "license": "CC0 1.0 Universal"
        }
    }
    with open("assets/characters/enemies/skeleton/creature_metadata.json", "w", encoding="utf-8") as f:
        json.dump(skel_meta, f, indent=2)

    print("Goblin and Skeleton assets expanded successfully.")

# =============================================================================
# 2. WOLF GENERATOR
# =============================================================================

def generate_wolf():
    print("Generating Wolf assets...")
    out_dir = "assets/characters/enemies/wolf"
    ensure_dir(out_dir)

    FRAME_W = 48
    FRAME_H = 40
    COLS = 4
    ROWS = 8
    SHEET_W = COLS * FRAME_W # 192
    SHEET_H = ROWS * FRAME_H # 320

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    C_FUR = (105, 100, 95, 255)         # Timber wolf slate-grey fur
    C_FUR_SHADOW = (75, 70, 65, 255)
    C_FUR_LIGHT = (145, 140, 135, 255)  # Underbelly light fur
    C_NOSE = (35, 30, 30, 255)
    C_EYES = (235, 195, 45, 255)        # Predatory amber eyes
    C_MOUTH = (185, 55, 60, 255)

    def draw_wolf_frame(c, r, d, anim_type, phase):
        bx = c * FRAME_W
        by = r * FRAME_H

        bob_y = 0
        if anim_type == "walk" and phase in [1, 3]:
            bob_y = -1
        elif anim_type == "run":
            bob_y = -2 if phase in [1, 2] else 0

        # Center torso anchor
        cx = bx + 24
        cy = by + 22 + bob_y

        leg_dy1 = -2 if phase == 0 else (2 if phase == 2 else 0)
        leg_dy2 = 2 if phase == 0 else (-2 if phase == 2 else 0)
        if anim_type == "run":
            leg_dy1 *= 2
            leg_dy2 *= 2

        if d == "down":
            # Head facing camera
            draw.ellipse([cx - 8, cy - 10, cx + 8, cy + 4], fill=C_FUR, outline=C_FUR_SHADOW)
            # Pointed ears
            draw.polygon([(cx - 7, cy - 9), (cx - 10, cy - 16), (cx - 4, cy - 10)], fill=C_FUR_SHADOW)
            draw.polygon([(cx + 4, cy - 10), (cx + 10, cy - 16), (cx + 7, cy - 9)], fill=C_FUR_SHADOW)
            # Eyes & Muzzle
            draw.point((cx - 3, cy - 5), fill=C_EYES)
            draw.point((cx + 3, cy - 5), fill=C_EYES)
            draw.ellipse([cx - 4, cy - 3, cx + 4, cy + 3], fill=C_FUR_LIGHT)
            draw.point((cx, cy - 1), fill=C_NOSE)
            # Front Legs
            draw.rectangle([cx - 7, cy + 4, cx - 4, by + 37 + leg_dy1], fill=C_FUR_SHADOW)
            draw.rectangle([cx + 4, cy + 4, cx + 7, by + 37 + leg_dy2], fill=C_FUR_SHADOW)

        elif d == "up":
            # Viewed from behind: body + tail + back of head and pointed ears
            draw.ellipse([cx - 7, cy - 8, cx + 7, cy + 6], fill=C_FUR, outline=C_FUR_SHADOW)
            draw.polygon([(cx - 7, cy - 8), (cx - 9, cy - 15), (cx - 4, cy - 9)], fill=C_FUR_SHADOW)
            draw.polygon([(cx + 4, cy - 9), (cx + 9, cy - 15), (cx + 7, cy - 8)], fill=C_FUR_SHADOW)
            # Bushy tail hanging down
            draw.polygon([(cx - 3, cy + 4), (cx + 3, cy + 4), (cx, cy + 14)], fill=C_FUR_LIGHT)
            # Hind Legs
            draw.rectangle([cx - 8, cy + 4, cx - 5, by + 37 + leg_dy1], fill=C_FUR_SHADOW)
            draw.rectangle([cx + 5, cy + 4, cx + 8, by + 37 + leg_dy2], fill=C_FUR_SHADOW)

        elif d in ["left", "right"]:
            # Horizontal four-legged quad profile
            fx = 1 if d == "right" else -1
            # Body torso
            draw.ellipse([cx - 14, cy - 6, cx + 14, cy + 6], fill=C_FUR, outline=C_FUR_SHADOW)
            # Underbelly light patch
            draw.ellipse([cx - 8, cy, cx + 8, cy + 5], fill=C_FUR_LIGHT)
            # Head & Snout
            head_x = cx + (12 * fx)
            draw.ellipse([head_x - 6, cy - 11, head_x + 6, cy + 1], fill=C_FUR)
            # Snout
            draw.polygon([(head_x, cy - 6), (head_x + (8 * fx), cy - 2), (head_x, cy + 1)], fill=C_FUR_LIGHT)
            draw.point((head_x + (7 * fx), cy - 2), fill=C_NOSE)
            draw.point((head_x + (2 * fx), cy - 5), fill=C_EYES)
            # Ear
            draw.polygon([(head_x - 3, cy - 10), (head_x, cy - 16), (head_x + 3, cy - 10)], fill=C_FUR_SHADOW)
            # Bushy tail
            tail_x = cx - (13 * fx)
            draw.polygon([(tail_x, cy - 2), (tail_x - (8 * fx), cy + 4), (tail_x, cy + 5)], fill=C_FUR_SHADOW)
            # Legs
            draw.rectangle([cx - 10, cy + 3, cx - 7, by + 37 + leg_dy1], fill=C_FUR_SHADOW)
            draw.rectangle([cx + 7, cy + 3, cx + 10, by + 37 + leg_dy2], fill=C_FUR_SHADOW)

    directions = ["down", "up", "left", "right"]
    # Rows 0..3: Idle
    for r, d in enumerate(directions):
        for c in range(COLS): draw_wolf_frame(c, r, d, "idle", c)
    # Rows 4..7: Walk
    for r, d in enumerate(directions):
        for c in range(COLS): draw_wolf_frame(c, 4 + r, d, "walk", c)

    png_path = os.path.join(out_dir, "wolf_spritesheet.png")
    img.save(png_path, "PNG")

    # Spritesheet Descriptor
    sheet_data = {
        "id": "wolf_spritesheet",
        "texture": "characters/enemies/wolf/wolf_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "wolf_spritesheet.json"), "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=2)

    # Animation Clips
    anim_data = {
        "animations": [
            { "animation": "wolf_idle_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "wolf_idle_up", "spritesheet": "wolf_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "wolf_idle_left", "spritesheet": "wolf_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "wolf_idle_right", "spritesheet": "wolf_spritesheet", "frame_duration": 0.2, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "wolf_walk_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.14, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "wolf_walk_up", "spritesheet": "wolf_spritesheet", "frame_duration": 0.14, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "wolf_walk_left", "spritesheet": "wolf_spritesheet", "frame_duration": 0.14, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "wolf_walk_right", "spritesheet": "wolf_spritesheet", "frame_duration": 0.14, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "wolf_run_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.08, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "wolf_run_up", "spritesheet": "wolf_spritesheet", "frame_duration": 0.08, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "wolf_run_left", "spritesheet": "wolf_spritesheet", "frame_duration": 0.08, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "wolf_run_right", "spritesheet": "wolf_spritesheet", "frame_duration": 0.08, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "wolf_attack_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [0, 17, 18, 0] },
            { "animation": "wolf_attack_up", "spritesheet": "wolf_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [4, 21, 22, 4] },
            { "animation": "wolf_attack_left", "spritesheet": "wolf_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [8, 25, 26, 8] },
            { "animation": "wolf_attack_right", "spritesheet": "wolf_spritesheet", "frame_duration": 0.1, "loop": False, "frames": [12, 29, 30, 12] },
            { "animation": "wolf_hurt_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.15, "loop": False, "frames": [2, 0] },
            { "animation": "wolf_death_down", "spritesheet": "wolf_spritesheet", "frame_duration": 0.25, "loop": False, "frames": [1, 2, 3] }
        ]
    }
    with open(os.path.join(out_dir, "wolf_animations.json"), "w", encoding="utf-8") as f:
        json.dump(anim_data, f, indent=2)

    # Creature Metadata
    meta = {
        "id": "creature_wolf_001",
        "name": "Forest Wolf",
        "category": "hostile_creature",
        "biome": ["forest"],
        "sprite": "characters/enemies/wolf/wolf_spritesheet.png",
        "spritesheet": "wolf_spritesheet",
        "dimensions": {
            "frame_width": FRAME_W,
            "frame_height": FRAME_H,
            "columns": COLS,
            "rows": ROWS,
            "frame_count": COLS * ROWS
        },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "wolf_idle_down",
            "walk": "wolf_walk_down",
            "run": "wolf_run_down",
            "attack": "wolf_attack_down",
            "hurt": "wolf_hurt_down",
            "death": "wolf_death_down"
        },
        "anchors": {
            "ground": [24, 40]
        },
        "collision": {
            "width": 24.0,
            "height": 14.0
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 9 Production Creature",
            "license": "CC0 1.0 Universal"
        }
    }
    with open(os.path.join(out_dir, "creature_metadata.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    # Gameplay Config
    gameplay_cfg = {
        "id": "wolf",
        "name": "Forest Wolf",
        "type": "beast",
        "faction": "monsters",
        "spritesheet": "wolf_spritesheet",
        "stats": { "walk_speed": 75.0, "run_speed": 135.0, "normalize_diagonal": True },
        "health": { "max_health": 45.0, "initial_health": 45.0, "regeneration_enabled": False, "regeneration_rate": 0.0 },
        "collision": { "enabled": True, "width": 22.0, "height": 14.0, "offset_x": 0.0, "offset_y": 0.0 },
        "combat": { "default_attack": "wolf_bite", "attack_cooldown": 0.85, "attack_range": 30.0 },
        "ai": { "behavior_script": "scripts/gameplay/ai/enemy_base.lua", "detection_radius": 160.0, "attack_radius": 34.0, "retreat_health_percent": 0.15, "decision_interval": 0.15, "patrol_radius": 100.0 },
        "animations": {
            "idle_down": "wolf_idle_down", "idle_up": "wolf_idle_up", "idle_left": "wolf_idle_left", "idle_right": "wolf_idle_right",
            "walk_down": "wolf_walk_down", "walk_up": "wolf_walk_up", "walk_left": "wolf_walk_left", "walk_right": "wolf_walk_right",
            "attack_down": "wolf_attack_down", "attack_up": "wolf_attack_up", "attack_left": "wolf_attack_left", "attack_right": "wolf_attack_right",
            "hurt_down": "wolf_hurt_down", "death_down": "wolf_death_down"
        }
    }
    with open("data/characters/enemies/wolf.json", "w", encoding="utf-8") as f:
        json.dump(gameplay_cfg, f, indent=2)

    print("Wolf generated successfully.")

# =============================================================================
# 3. WILD BOAR GENERATOR
# =============================================================================

def generate_wild_boar():
    print("Generating Wild Boar assets...")
    out_dir = "assets/characters/enemies/wild_boar"
    ensure_dir(out_dir)

    FRAME_W = 48
    FRAME_H = 40
    COLS = 4
    ROWS = 8
    SHEET_W = COLS * FRAME_W # 192
    SHEET_H = ROWS * FRAME_H # 320

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    C_BODY = (85, 55, 35, 255)         # Russet/dark bristle brown
    C_SHADOW = (55, 35, 20, 255)
    C_SNOUT = (130, 85, 60, 255)
    C_TUSK = (245, 240, 220, 255)       # Ivory tusks
    C_EYES = (30, 20, 20, 255)
    C_BRISTLE = (45, 30, 15, 255)       # Spine ridge mane

    def draw_boar_frame(c, r, d, anim_type, phase):
        bx = c * FRAME_W
        by = r * FRAME_H

        bob_y = 0
        if anim_type == "walk" and phase in [1, 3]: bob_y = -1
        elif anim_type == "run": bob_y = -2 if phase in [1, 2] else 0

        cx = bx + 24
        cy = by + 23 + bob_y

        leg_dy1 = -2 if phase == 0 else (2 if phase == 2 else 0)
        leg_dy2 = 2 if phase == 0 else (-2 if phase == 2 else 0)

        if d == "down":
            # Broad massive head & snout
            draw.ellipse([cx - 10, cy - 8, cx + 10, cy + 6], fill=C_BODY, outline=C_SHADOW)
            # Bristle mane
            draw.line([(cx - 4, cy - 11), (cx + 4, cy - 11)], fill=C_BRISTLE)
            # Tusks protruding upward
            draw.polygon([(cx - 7, cy + 1), (cx - 9, cy - 4), (cx - 6, cy - 1)], fill=C_TUSK)
            draw.polygon([(cx + 7, cy + 1), (cx + 9, cy - 4), (cx + 6, cy - 1)], fill=C_TUSK)
            # Snout disc
            draw.ellipse([cx - 5, cy, cx + 5, cy + 6], fill=C_SNOUT)
            draw.point((cx - 2, cy + 3), fill=C_EYES)
            draw.point((cx + 2, cy + 3), fill=C_EYES)
            # Eyes
            draw.point((cx - 5, cy - 3), fill=C_EYES)
            draw.point((cx + 5, cy - 3), fill=C_EYES)
            # Sturdy stout legs
            draw.rectangle([cx - 8, cy + 6, cx - 4, by + 37 + leg_dy1], fill=C_SHADOW)
            draw.rectangle([cx + 4, cy + 6, cx + 8, by + 37 + leg_dy2], fill=C_SHADOW)

        elif d == "up":
            # Viewed from behind: massive rounded rump
            draw.ellipse([cx - 11, cy - 8, cx + 11, cy + 8], fill=C_BODY, outline=C_SHADOW)
            draw.line([(cx, cy - 11), (cx, cy - 3)], fill=C_BRISTLE)
            # Small tail
            draw.line([(cx, cy + 8), (cx + 2, cy + 14)], fill=C_SHADOW)
            draw.rectangle([cx - 9, cy + 6, cx - 5, by + 37 + leg_dy1], fill=C_SHADOW)
            draw.rectangle([cx + 5, cy + 6, cx + 9, by + 37 + leg_dy2], fill=C_SHADOW)

        elif d in ["left", "right"]:
            fx = 1 if d == "right" else -1
            # Bulky body
            draw.ellipse([cx - 15, cy - 9, cx + 12, cy + 7], fill=C_BODY, outline=C_SHADOW)
            # Spine bristles
            draw.line([(cx - 10, cy - 11), (cx + 6, cy - 11)], fill=C_BRISTLE)
            # Massive head & curved tusk
            hx = cx + (12 * fx)
            draw.polygon([(hx - (4*fx), cy - 7), (hx + (10*fx), cy + 3), (hx - (4*fx), cy + 7)], fill=C_SNOUT)
            # Upward curving tusk
            draw.polygon([(hx + (6*fx), cy + 4), (hx + (10*fx), cy - 2), (hx + (8*fx), cy + 4)], fill=C_TUSK)
            draw.point((hx + (2*fx), cy - 3), fill=C_EYES)
            # Legs
            draw.rectangle([cx - 11, cy + 5, cx - 7, by + 37 + leg_dy1], fill=C_SHADOW)
            draw.rectangle([cx + 4, cy + 5, cx + 8, by + 37 + leg_dy2], fill=C_SHADOW)

    directions = ["down", "up", "left", "right"]
    for r, d in enumerate(directions):
        for c in range(COLS): draw_boar_frame(c, r, d, "idle", c)
    for r, d in enumerate(directions):
        for c in range(COLS): draw_boar_frame(c, 4 + r, d, "walk", c)

    png_path = os.path.join(out_dir, "wild_boar_spritesheet.png")
    img.save(png_path, "PNG")

    # Spritesheet Descriptor
    sheet_data = {
        "id": "wild_boar_spritesheet",
        "texture": "characters/enemies/wild_boar/wild_boar_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "wild_boar_spritesheet.json"), "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=2)

    # Animations
    anim_data = {
        "animations": [
            { "animation": "boar_idle_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.22, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "boar_idle_up", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.22, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "boar_idle_left", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.22, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "boar_idle_right", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.22, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "boar_walk_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.16, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "boar_walk_up", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.16, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "boar_walk_left", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.16, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "boar_walk_right", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.16, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "boar_run_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "boar_run_up", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "boar_run_left", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "boar_run_right", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "boar_attack_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [0, 17, 18, 0] },
            { "animation": "boar_attack_up", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [4, 21, 22, 4] },
            { "animation": "boar_attack_left", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [8, 25, 26, 8] },
            { "animation": "boar_attack_right", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [12, 29, 30, 12] },
            { "animation": "boar_hurt_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.15, "loop": False, "frames": [2, 0] },
            { "animation": "boar_death_down", "spritesheet": "wild_boar_spritesheet", "frame_duration": 0.25, "loop": False, "frames": [1, 2, 3] }
        ]
    }
    with open(os.path.join(out_dir, "wild_boar_animations.json"), "w", encoding="utf-8") as f:
        json.dump(anim_data, f, indent=2)

    # Creature Metadata
    meta = {
        "id": "creature_boar_001",
        "name": "Wild Boar",
        "category": "hostile_creature",
        "biome": ["forest"],
        "sprite": "characters/enemies/wild_boar/wild_boar_spritesheet.png",
        "spritesheet": "wild_boar_spritesheet",
        "dimensions": {
            "frame_width": FRAME_W,
            "frame_height": FRAME_H,
            "columns": COLS,
            "rows": ROWS,
            "frame_count": COLS * ROWS
        },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "boar_idle_down",
            "walk": "boar_walk_down",
            "run": "boar_run_down",
            "attack": "boar_attack_down",
            "hurt": "boar_hurt_down",
            "death": "boar_death_down"
        },
        "anchors": {
            "ground": [24, 40]
        },
        "collision": {
            "width": 26.0,
            "height": 16.0
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 9 Production Creature",
            "license": "CC0 1.0 Universal"
        }
    }
    with open(os.path.join(out_dir, "creature_metadata.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    # Gameplay Config
    gameplay_cfg = {
        "id": "wild_boar",
        "name": "Wild Boar",
        "type": "beast",
        "faction": "monsters",
        "spritesheet": "wild_boar_spritesheet",
        "stats": { "walk_speed": 55.0, "run_speed": 125.0, "normalize_diagonal": True },
        "health": { "max_health": 70.0, "initial_health": 70.0, "regeneration_enabled": False, "regeneration_rate": 0.0 },
        "collision": { "enabled": True, "width": 26.0, "height": 16.0, "offset_x": 0.0, "offset_y": 0.0 },
        "combat": { "default_attack": "boar_tusk_charge", "attack_cooldown": 1.2, "attack_range": 32.0 },
        "ai": { "behavior_script": "scripts/gameplay/ai/enemy_base.lua", "detection_radius": 130.0, "attack_radius": 36.0, "retreat_health_percent": 0.1, "decision_interval": 0.2, "patrol_radius": 70.0 },
        "animations": {
            "idle_down": "boar_idle_down", "idle_up": "boar_idle_up", "idle_left": "boar_idle_left", "idle_right": "boar_idle_right",
            "walk_down": "boar_walk_down", "walk_up": "boar_walk_up", "walk_left": "boar_walk_left", "walk_right": "boar_walk_right",
            "attack_down": "boar_attack_down", "attack_up": "boar_attack_up", "attack_left": "boar_attack_left", "attack_right": "boar_attack_right",
            "hurt_down": "boar_hurt_down", "death_down": "boar_death_down"
        }
    }
    with open("data/characters/enemies/wild_boar.json", "w", encoding="utf-8") as f:
        json.dump(gameplay_cfg, f, indent=2)

    print("Wild Boar generated successfully.")

# =============================================================================
# 4. BAT GENERATOR
# =============================================================================

def generate_bat():
    print("Generating Bat assets...")
    out_dir = "assets/characters/enemies/bat"
    ensure_dir(out_dir)

    FRAME_W = 32
    FRAME_H = 32
    COLS = 4
    ROWS = 8
    SHEET_W = COLS * FRAME_W # 128
    SHEET_H = ROWS * FRAME_H # 256

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    C_BODY = (40, 35, 45, 255)         # Dark slate/charcoal bat body
    C_WING = (60, 50, 70, 255)         # Leathery purple-grey wing membrane
    C_WING_BONE = (30, 25, 35, 255)
    C_EYES = (245, 50, 45, 255)        # Glowing crimson eyes

    def draw_bat_frame(c, r, d, anim_type, phase):
        bx = c * FRAME_W
        by = r * FRAME_H

        # Flapping wing cycle (phase 0=spread high, 1=level, 2=downstroke, 3=level)
        wing_dy = -4 if phase == 0 else (4 if phase == 2 else 0)

        cx = bx + 16
        cy = by + 16

        # Furry body core
        draw.ellipse([cx - 4, cy - 5, cx + 4, cy + 5], fill=C_BODY)

        # Ears & Glowing Eyes
        if d in ["down", "left", "right"]:
            draw.polygon([(cx - 4, cy - 4), (cx - 6, cy - 10), (cx - 2, cy - 5)], fill=C_WING_BONE)
            draw.polygon([(cx + 2, cy - 5), (cx + 6, cy - 10), (cx + 4, cy - 4)], fill=C_WING_BONE)
            if d == "down":
                draw.point((cx - 2, cy - 2), fill=C_EYES)
                draw.point((cx + 2, cy - 2), fill=C_EYES)
            elif d == "left":
                draw.point((cx - 3, cy - 2), fill=C_EYES)
            elif d == "right":
                draw.point((cx + 3, cy - 2), fill=C_EYES)
        elif d == "up":
            draw.polygon([(cx - 4, cy - 4), (cx - 5, cy - 9), (cx - 2, cy - 5)], fill=C_WING_BONE)
            draw.polygon([(cx + 2, cy - 5), (cx + 5, cy - 9), (cx + 4, cy - 4)], fill=C_WING_BONE)

        # Wings flapping left and right
        # Left Wing
        draw.polygon([(cx - 3, cy), (cx - 14, cy + wing_dy), (cx - 10, cy + 6 + wing_dy), (cx - 2, cy + 3)], fill=C_WING, outline=C_WING_BONE)
        # Right Wing
        draw.polygon([(cx + 3, cy), (cx + 14, cy + wing_dy), (cx + 10, cy + 6 + wing_dy), (cx + 2, cy + 3)], fill=C_WING, outline=C_WING_BONE)

    directions = ["down", "up", "left", "right"]
    for r, d in enumerate(directions):
        for c in range(COLS): draw_bat_frame(c, r, d, "fly", c)
    for r, d in enumerate(directions):
        for c in range(COLS): draw_bat_frame(c, 4 + r, d, "move", c)

    png_path = os.path.join(out_dir, "bat_spritesheet.png")
    img.save(png_path, "PNG")

    # Spritesheet Descriptor
    sheet_data = {
        "id": "bat_spritesheet",
        "texture": "characters/enemies/bat/bat_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "bat_spritesheet.json"), "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=2)

    # Animations
    anim_data = {
        "animations": [
            { "animation": "bat_fly_down", "spritesheet": "bat_spritesheet", "frame_duration": 0.12, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "bat_fly_up", "spritesheet": "bat_spritesheet", "frame_duration": 0.12, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "bat_fly_left", "spritesheet": "bat_spritesheet", "frame_duration": 0.12, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "bat_fly_right", "spritesheet": "bat_spritesheet", "frame_duration": 0.12, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "bat_move_down", "spritesheet": "bat_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "bat_move_up", "spritesheet": "bat_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [20, 21, 22, 23] },
            { "animation": "bat_move_left", "spritesheet": "bat_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [24, 25, 26, 27] },
            { "animation": "bat_move_right", "spritesheet": "bat_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [28, 29, 30, 31] },
            { "animation": "bat_attack_down", "spritesheet": "bat_spritesheet", "frame_duration": 0.08, "loop": False, "frames": [0, 17, 18, 0] },
            { "animation": "bat_attack_up", "spritesheet": "bat_spritesheet", "frame_duration": 0.08, "loop": False, "frames": [4, 21, 22, 4] },
            { "animation": "bat_attack_left", "spritesheet": "bat_spritesheet", "frame_duration": 0.08, "loop": False, "frames": [8, 25, 26, 8] },
            { "animation": "bat_attack_right", "spritesheet": "bat_spritesheet", "frame_duration": 0.08, "loop": False, "frames": [12, 29, 30, 12] },
            { "animation": "bat_hurt_down", "spritesheet": "bat_spritesheet", "frame_duration": 0.15, "loop": False, "frames": [2, 0] },
            { "animation": "bat_death_down", "spritesheet": "bat_spritesheet", "frame_duration": 0.25, "loop": False, "frames": [1, 2, 3] }
        ]
    }
    with open(os.path.join(out_dir, "bat_animations.json"), "w", encoding="utf-8") as f:
        json.dump(anim_data, f, indent=2)

    # Creature Metadata
    meta = {
        "id": "creature_bat_001",
        "name": "Dusk Bat",
        "category": "hostile_creature",
        "biome": ["forest", "caves"],
        "sprite": "characters/enemies/bat/bat_spritesheet.png",
        "spritesheet": "bat_spritesheet",
        "dimensions": {
            "frame_width": FRAME_W,
            "frame_height": FRAME_H,
            "columns": COLS,
            "rows": ROWS,
            "frame_count": COLS * ROWS
        },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "bat_fly_down",
            "fly": "bat_fly_down",
            "move": "bat_move_down",
            "attack": "bat_attack_down",
            "hurt": "bat_hurt_down",
            "death": "bat_death_down"
        },
        "anchors": {
            "ground": [16, 28]
        },
        "collision": {
            "width": 14.0,
            "height": 14.0
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 9 Production Creature",
            "license": "CC0 1.0 Universal"
        }
    }
    with open(os.path.join(out_dir, "creature_metadata.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    # Gameplay Config
    gameplay_cfg = {
        "id": "bat",
        "name": "Dusk Bat",
        "type": "flying_beast",
        "faction": "monsters",
        "spritesheet": "bat_spritesheet",
        "stats": { "walk_speed": 60.0, "run_speed": 115.0, "normalize_diagonal": True },
        "health": { "max_health": 30.0, "initial_health": 30.0, "regeneration_enabled": False, "regeneration_rate": 0.0 },
        "collision": { "enabled": True, "width": 14.0, "height": 14.0, "offset_x": 0.0, "offset_y": 0.0 },
        "combat": { "default_attack": "bat_bite", "attack_cooldown": 0.75, "attack_range": 24.0 },
        "ai": { "behavior_script": "scripts/gameplay/ai/enemy_base.lua", "detection_radius": 150.0, "attack_radius": 28.0, "retreat_health_percent": 0.2, "decision_interval": 0.15, "patrol_radius": 90.0 },
        "animations": {
            "idle_down": "bat_fly_down", "idle_up": "bat_fly_up", "idle_left": "bat_fly_left", "idle_right": "bat_fly_right",
            "walk_down": "bat_move_down", "walk_up": "bat_move_up", "walk_left": "bat_move_left", "walk_right": "bat_move_right",
            "attack_down": "bat_attack_down", "attack_up": "bat_attack_up", "attack_left": "bat_attack_left", "attack_right": "bat_attack_right",
            "hurt_down": "bat_hurt_down", "death_down": "bat_death_down"
        }
    }
    with open("data/characters/enemies/bat.json", "w", encoding="utf-8") as f:
        json.dump(gameplay_cfg, f, indent=2)

    print("Bat generated successfully.")

# =============================================================================
# 5. SMALL BIRD GENERATOR (Ambient Wildlife)
# =============================================================================

def generate_small_bird():
    print("Generating Small Bird ambient wildlife...")
    out_dir = "assets/characters/wildlife/birds/small_bird"
    ensure_dir(out_dir)

    FRAME_W = 24
    FRAME_H = 24
    COLS = 4
    ROWS = 4
    SHEET_W = COLS * FRAME_W # 96
    SHEET_H = ROWS * FRAME_H # 96

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    C_PLUMAGE = (65, 125, 205, 255)    # Bluebird plumage
    C_BREAST = (235, 140, 50, 255)     # Warm rufous breast
    C_BEAK = (235, 185, 45, 255)
    C_EYE = (25, 25, 30, 255)
    C_WING = (45, 85, 155, 255)

    def draw_bird_frame(c, r, anim_type, phase):
        bx = c * FRAME_W
        by = r * FRAME_H
        cx = bx + 12
        cy = by + 12

        if anim_type == "idle":
            # Perching bird looking around
            bob = -1 if phase in [1, 2] else 0
            draw.ellipse([cx - 5, cy - 4 + bob, cx + 5, cy + 6 + bob], fill=C_PLUMAGE)
            draw.ellipse([cx - 4, cy + bob, cx + 2, cy + 5 + bob], fill=C_BREAST)
            # Head & Beak
            draw.point((cx - 3, cy - 2 + bob), fill=C_EYE)
            draw.polygon([(cx - 5, cy - 1 + bob), (cx - 8, cy + bob), (cx - 5, cy + 1 + bob)], fill=C_BEAK)
            # Folded wing
            draw.ellipse([cx - 1, cy - 2 + bob, cx + 6, cy + 4 + bob], fill=C_WING)
            # Tail
            draw.line([(cx + 4, cy + 4 + bob), (cx + 9, cy + 7 + bob)], fill=C_WING)

        elif anim_type == "fly":
            # Flight with flapping wings (phase 0=up, 1=level, 2=down, 3=level)
            w_dy = -4 if phase == 0 else (4 if phase == 2 else 0)
            draw.ellipse([cx - 4, cy - 3, cx + 4, cy + 4], fill=C_PLUMAGE)
            draw.polygon([(cx - 4, cy - 1), (cx - 7, cy), (cx - 4, cy + 1)], fill=C_BEAK)
            # Wings
            draw.polygon([(cx, cy - 1), (cx - 2, cy + w_dy - 4), (cx + 6, cy + w_dy - 2)], fill=C_WING)
            draw.line([(cx + 3, cy + 2), (cx + 8, cy + 3)], fill=C_WING)

        elif anim_type == "takeoff":
            draw.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], fill=C_PLUMAGE)
            draw.polygon([(cx - 3, cy - 2), (cx - 6, cy - 8), (cx + 4, cy - 6)], fill=C_WING)

        elif anim_type == "landing":
            draw.ellipse([cx - 4, cy - 2, cx + 4, cy + 6], fill=C_PLUMAGE)
            draw.polygon([(cx - 3, cy), (cx - 6, cy - 4), (cx + 4, cy - 2)], fill=C_WING)

    # Row 0: Idle
    for c in range(COLS): draw_bird_frame(c, 0, "idle", c)
    # Row 1: Fly
    for c in range(COLS): draw_bird_frame(c, 1, "fly", c)
    # Row 2: Takeoff
    for c in range(COLS): draw_bird_frame(c, 2, "takeoff", c)
    # Row 3: Landing
    for c in range(COLS): draw_bird_frame(c, 3, "landing", c)

    png_path = os.path.join(out_dir, "small_bird_spritesheet.png")
    img.save(png_path, "PNG")

    # Spritesheet Descriptor
    sheet_data = {
        "id": "small_bird_spritesheet",
        "texture": "characters/wildlife/birds/small_bird/small_bird_spritesheet.png",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    with open(os.path.join(out_dir, "small_bird_spritesheet.json"), "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=2)

    # Animations
    anim_data = {
        "animations": [
            { "animation": "bird_idle", "spritesheet": "small_bird_spritesheet", "frame_duration": 0.25, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "bird_fly", "spritesheet": "small_bird_spritesheet", "frame_duration": 0.1, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "bird_takeoff", "spritesheet": "small_bird_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [8, 9, 10, 11] },
            { "animation": "bird_landing", "spritesheet": "small_bird_spritesheet", "frame_duration": 0.12, "loop": False, "frames": [12, 13, 14, 15] }
        ]
    }
    with open(os.path.join(out_dir, "small_bird_animations.json"), "w", encoding="utf-8") as f:
        json.dump(anim_data, f, indent=2)

    # Creature Metadata
    meta = {
        "id": "creature_bird_001",
        "name": "Small Songbird",
        "category": "ambient_wildlife",
        "biome": ["forest", "meadow"],
        "sprite": "characters/wildlife/birds/small_bird/small_bird_spritesheet.png",
        "spritesheet": "small_bird_spritesheet",
        "dimensions": { "frame_width": FRAME_W, "frame_height": FRAME_H, "columns": COLS, "rows": ROWS, "frame_count": COLS * ROWS },
        "directions": ["down", "left", "right"],
        "animations": {
            "idle": "bird_idle",
            "fly": "bird_fly",
            "takeoff": "bird_takeoff",
            "landing": "bird_landing"
        },
        "anchors": { "ground": [12, 22] },
        "provenance": { "creator": "Wildbound Development Studio", "source": "Wildbound Phase 9 Production Wildlife", "license": "CC0 1.0 Universal" }
    }
    with open(os.path.join(out_dir, "creature_metadata.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    print("Small Bird generated successfully.")

# =============================================================================
# 6. BUTTERFLIES GENERATOR (3 Variations)
# =============================================================================

def generate_butterfly(variant_id, name, wing_color, hilight_color):
    print(f"Generating Butterfly variant: {variant_id}...")
    out_dir = f"assets/characters/wildlife/butterflies/{variant_id}"
    ensure_dir(out_dir)

    FRAME_W = 16
    FRAME_H = 16
    COLS = 4
    ROWS = 2
    SHEET_W = COLS * FRAME_W # 64
    SHEET_H = ROWS * FRAME_H # 32

    img = Image.new("RGBA", (SHEET_W, SHEET_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    C_BODY = (35, 30, 30, 255)

    def draw_butterfly_frame(c, r, anim_type, phase):
        bx = c * FRAME_W
        by = r * FRAME_H
        cx = bx + 8
        cy = by + 8

        # Body line
        draw.line([(cx, cy - 4), (cx, cy + 4)], fill=C_BODY)

        if anim_type == "resting":
            # Wings partially closed/fluttering
            w_span = 3 if phase in [0, 2] else 5
            draw.ellipse([cx - w_span, cy - 4, cx, cy + 3], fill=wing_color, outline=hilight_color)
            draw.ellipse([cx, cy - 4, cx + w_span, cy + 3], fill=wing_color, outline=hilight_color)
        elif anim_type == "fly":
            # Full fluttering wing stroke (phase 0=open, 1=mid, 2=closed, 3=mid)
            span = 6 if phase == 0 else (4 if phase in [1, 3] else 2)
            draw.ellipse([cx - span, cy - 5, cx, cy + 4], fill=wing_color, outline=hilight_color)
            draw.ellipse([cx, cy - 5, cx + span, cy + 4], fill=wing_color, outline=hilight_color)

    # Row 0: Resting
    for c in range(COLS): draw_butterfly_frame(c, 0, "resting", c)
    # Row 1: Fly
    for c in range(COLS): draw_butterfly_frame(c, 1, "fly", c)

    sheet_filename = f"{variant_id}_spritesheet.png"
    png_path = os.path.join(out_dir, sheet_filename)
    img.save(png_path, "PNG")

    sheet_id = f"{variant_id}_spritesheet"
    sheet_data = {
        "id": sheet_id,
        "texture": f"characters/wildlife/butterflies/{variant_id}/{sheet_filename}",
        "frame_width": FRAME_W,
        "frame_height": FRAME_H,
        "columns": COLS,
        "rows": ROWS,
        "frame_count": COLS * ROWS,
        "spacing": 0,
        "margin": 0,
        "origin": "center"
    }
    with open(os.path.join(out_dir, f"{variant_id}_spritesheet.json"), "w", encoding="utf-8") as f:
        json.dump(sheet_data, f, indent=2)

    # Animations
    anim_data = {
        "animations": [
            { "animation": f"{variant_id}_resting", "spritesheet": sheet_id, "frame_duration": 0.2, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": f"{variant_id}_fly", "spritesheet": sheet_id, "frame_duration": 0.08, "loop": True, "frames": [4, 5, 6, 7] }
        ]
    }
    with open(os.path.join(out_dir, f"{variant_id}_animations.json"), "w", encoding="utf-8") as f:
        json.dump(anim_data, f, indent=2)

    # Creature Metadata
    meta = {
        "id": f"creature_{variant_id}",
        "name": name,
        "category": "ambient_wildlife",
        "biome": ["forest", "meadow", "river"],
        "sprite": f"characters/wildlife/butterflies/{variant_id}/{sheet_filename}",
        "spritesheet": sheet_id,
        "dimensions": { "frame_width": FRAME_W, "frame_height": FRAME_H, "columns": COLS, "rows": ROWS, "frame_count": COLS * ROWS },
        "directions": ["omni"],
        "animations": {
            "resting": f"{variant_id}_resting",
            "fly": f"{variant_id}_fly"
        },
        "anchors": { "ground": [8, 8] },
        "provenance": { "creator": "Wildbound Development Studio", "source": "Wildbound Phase 9 Production Wildlife", "license": "CC0 1.0 Universal" }
    }
    with open(os.path.join(out_dir, "creature_metadata.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    print(f"Butterfly variant {variant_id} generated successfully.")

# =============================================================================
# MAIN ENTRY POINT
# =============================================================================

def main():
    print("=" * 60)
    print(" Wildbound Phase 9 — Generating Creature & Wildlife Assets")
    print("=" * 60)

    # Category A: Hostile / Combat-capable creatures
    expand_goblin_and_skeleton()
    generate_wolf()
    generate_wild_boar()
    generate_bat()

    # Category B: Ambient wildlife
    generate_small_bird()
    generate_butterfly("forest_butterfly", "Forest Butterfly", (40, 195, 90, 255), (90, 245, 140, 255))
    generate_butterfly("blue_butterfly", "Blue Butterfly", (40, 130, 230, 255), (110, 195, 255, 255))
    generate_butterfly("golden_butterfly", "Golden Butterfly", (240, 180, 30, 255), (255, 225, 90, 255))

    print("=" * 60)
    print(" All Phase 9 Creature and Wildlife Assets Generated Successfully!")
    print("=" * 60)

if __name__ == "__main__":
    main()

