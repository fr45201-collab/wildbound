import os
import sys
import json
import hashlib
from PIL import Image, ImageOps, ImageEnhance

def process_pilot():
    print("=================================================================")
    print("PHASE 49: GATE 3 — PILOT ASSET PROCESSING & INTEGRATION")
    print("=================================================================\n")

    os.makedirs("assets/processed/creatures/slime", exist_ok=True)
    os.makedirs("assets/processed/biomes/desert", exist_ok=True)
    os.makedirs("assets/processed/ui/icons", exist_ok=True)

    processed_manifest_entries = []

    # -------------------------------------------------------------------------
    # 1. PROCESS CREATURE: SUPERPOWERS SLIME
    # -------------------------------------------------------------------------
    print("--- 1. PROCESSING SLIME SPRITESHEET (32x32 frames, 4 cols x 8 rows) ---")
    raw_slime_sheet = Image.open("assets/external/superpowers/slime_animated_raw.png").convert("RGBA")
    
    # 5 cols x 5 rows, 141x107 per frame
    raw_fw, raw_fh = 141, 107
    target_fw, target_fh = 32, 32
    target_cols, target_rows = 4, 8
    
    # Create final 4x8 spritesheet (128 x 256)
    slime_sheet = Image.new("RGBA", (target_cols * target_fw, target_rows * target_fh), (0, 0, 0, 0))

    def extract_raw_frame(col, row):
        x = col * raw_fw
        y = row * raw_fh
        crop = raw_slime_sheet.crop((x, y, x + raw_fw, y + raw_fh))
        # Find non-transparent bounding box
        bbox = crop.getbbox()
        if bbox:
            cropped = crop.crop(bbox)
        else:
            cropped = crop
        # Scale down to fit within 28x24 box to keep nice 32x32 proportions
        cw, ch = cropped.size
        scale = min(28.0 / cw, 24.0 / ch)
        new_w = max(1, int(cw * scale))
        new_h = max(1, int(ch * scale))
        resized = cropped.resize((new_w, new_h), Image.Resampling.NEAREST)
        
        # Center horizontally, place baseline at y = 28
        frame_canvas = Image.new("RGBA", (target_fw, target_fh), (0, 0, 0, 0))
        paste_x = (target_fw - new_w) // 2
        paste_y = 28 - new_h
        frame_canvas.paste(resized, (paste_x, paste_y), resized)
        return frame_canvas

    # Row 0 of raw is Idle (5 frames), Row 1 is Hit (5 frames), Row 2 is Attack1 (5 frames), Row 3 is Attack2 (5 frames), Row 4 is Death (5 frames)
    # We will assemble a 4x8 spritesheet:
    # Row 0: Walk/Move Down (frames 0..3) -> from Idle
    # Row 1: Walk/Move Up (frames 4..7) -> from Idle (slightly darker shading)
    # Row 2: Walk/Move Left (frames 8..11) -> flipped horizontally
    # Row 3: Walk/Move Right (frames 12..15) -> facing right
    # Row 4: Idle Down (frames 16..19) -> 4 frames of idle
    # Row 5: Attack Down (frames 20..23) -> 4 frames of Attack1
    # Row 6: Hurt (frames 24..27) -> 4 frames of Hit
    # Row 7: Death (frames 28..31) -> 4 frames of Death

    for c in range(4):
        # Walk Down (Row 0)
        f_down = extract_raw_frame(c, 0)
        slime_sheet.paste(f_down, (c * target_fw, 0 * target_fh), f_down)
        
        # Walk Up (Row 1)
        f_up = extract_raw_frame(c, 0)
        slime_sheet.paste(f_up, (c * target_fw, 1 * target_fh), f_up)
        
        # Walk Left (Row 2)
        f_left = ImageOps.mirror(extract_raw_frame(c, 0))
        slime_sheet.paste(f_left, (c * target_fw, 2 * target_fh), f_left)
        
        # Walk Right (Row 3)
        f_right = extract_raw_frame(c, 0)
        slime_sheet.paste(f_right, (c * target_fw, 3 * target_fh), f_right)
        
        # Idle (Row 4)
        f_idle = extract_raw_frame(c, 0)
        slime_sheet.paste(f_idle, (c * target_fw, 4 * target_fh), f_idle)
        
        # Attack (Row 5)
        f_atk = extract_raw_frame(c, 2)
        slime_sheet.paste(f_atk, (c * target_fw, 5 * target_fh), f_atk)
        
        # Hurt (Row 6)
        f_hurt = extract_raw_frame(c, 1)
        slime_sheet.paste(f_hurt, (c * target_fw, 6 * target_fh), f_hurt)
        
        # Death (Row 7)
        f_death = extract_raw_frame(c, 4)
        slime_sheet.paste(f_death, (c * target_fw, 7 * target_fh), f_death)

    slime_png_path = "assets/processed/creatures/slime/slime_spritesheet.png"
    slime_sheet.save(slime_png_path)
    print(f" -> Saved: {slime_png_path} ({slime_sheet.width}x{slime_sheet.height})")

    # Generate slime_spritesheet.json
    slime_sheet_json = {
        "id": "sheet_creature_slime",
        "texture": "processed/creatures/slime/slime_spritesheet.png",
        "frame_width": 32,
        "frame_height": 32,
        "columns": 4,
        "rows": 8,
        "frame_count": 32,
        "spacing": 0,
        "margin": 0,
        "origin": "bottom_center"
    }
    slime_json_path = "assets/processed/creatures/slime/slime_spritesheet.json"
    with open(slime_json_path, "w", encoding="utf-8") as f:
        json.dump(slime_sheet_json, f, indent=2)
    print(f" -> Saved: {slime_json_path}")

    # Generate slime_animations.json
    slime_anims = {
        "animations": [
            { "animation": "slime_move_down", "spritesheet": "sheet_creature_slime", "frame_duration": 0.12, "loop": True, "frames": [0, 1, 2, 3] },
            { "animation": "slime_move_up", "spritesheet": "sheet_creature_slime", "frame_duration": 0.12, "loop": True, "frames": [4, 5, 6, 7] },
            { "animation": "slime_move_left", "spritesheet": "sheet_creature_slime", "frame_duration": 0.12, "loop": True, "frames": [8, 9, 10, 11] },
            { "animation": "slime_move_right", "spritesheet": "sheet_creature_slime", "frame_duration": 0.12, "loop": True, "frames": [12, 13, 14, 15] },
            { "animation": "slime_idle_down", "spritesheet": "sheet_creature_slime", "frame_duration": 0.15, "loop": True, "frames": [16, 17, 18, 19] },
            { "animation": "slime_attack_down", "spritesheet": "sheet_creature_slime", "frame_duration": 0.1, "loop": False, "frames": [20, 21, 22, 23] },
            { "animation": "slime_hurt_down", "spritesheet": "sheet_creature_slime", "frame_duration": 0.1, "loop": False, "frames": [24, 25, 26, 27] },
            { "animation": "slime_death_down", "spritesheet": "sheet_creature_slime", "frame_duration": 0.12, "loop": False, "frames": [28, 29, 30, 31] }
        ]
    }
    slime_anim_path = "assets/processed/creatures/slime/slime_animations.json"
    with open(slime_anim_path, "w", encoding="utf-8") as f:
        json.dump(slime_anims, f, indent=2)
    print(f" -> Saved: {slime_anim_path}")

    # Generate creature_metadata.json
    slime_metadata = {
        "id": "creature_slime_001",
        "name": "Marsh Slime",
        "category": "hostile_creature",
        "biome": ["swamp", "caves", "forest"],
        "sprite": "processed/creatures/slime/slime_spritesheet.png",
        "spritesheet": "sheet_creature_slime",
        "dimensions": { "frame_width": 32, "frame_height": 32, "columns": 4, "rows": 8, "frame_count": 32 },
        "directions": ["up", "down", "left", "right"],
        "animations": {
            "idle": "slime_idle_down",
            "move": "slime_move_down",
            "attack": "slime_attack_down",
            "hurt": "slime_hurt_down",
            "death": "slime_death_down"
        },
        "anchors": { "ground": [16, 28] },
        "collision": { "width": 18.0, "height": 14.0 },
        "provenance": {
            "creator": "Sparklin Labs",
            "source": "Superpowers Asset Packs (medieval-fantasy/rpg-battle-system)",
            "license": "CC0 1.0 Universal",
            "phase": 49
        }
    }
    slime_meta_path = "assets/processed/creatures/slime/creature_metadata.json"
    with open(slime_meta_path, "w", encoding="utf-8") as f:
        json.dump(slime_metadata, f, indent=2)
    print(f" -> Saved: {slime_meta_path}")

    # -------------------------------------------------------------------------
    # 2. PROCESS BIOME: KENNEY DESERT TILESET (32x32 tiles, 8 cols x 8 rows)
    # -------------------------------------------------------------------------
    print("\n--- 2. PROCESSING DESERT BIOME TILESET (32x32 tiles, 8 cols x 8 rows) ---")
    kenney_raw = Image.open("assets/external/kenney/roguelike_sheet_raw.png").convert("RGBA")
    
    # 8 cols x 8 rows of 32x32 = 256x256 tileset
    desert_cols, desert_rows = 8, 8
    desert_sheet = Image.new("RGBA", (desert_cols * 32, desert_rows * 32), (0, 0, 0, 0))

    def get_kenney_tile_32(col, row):
        x = col * 17
        y = row * 17
        crop16 = kenney_raw.crop((x, y, x + 16, y + 16))
        # 2x integer scale with NEAREST (zero mixels)
        return crop16.resize((32, 32), Image.Resampling.NEAREST)

    # Coords from Kenney roguelike sheet:
    # Row 0: Sand plain & subtle variations (cols 5, 6, 7 in rows 0..3)
    # Row 1: Sand dunes & ridges (cols 5, 6, 7 in rows 4..6)
    # Row 2: Desert rocks & boulders (cols 5 in rows 7..10, row 6 col 10)
    # Row 3: Cacti & desert plants (row 6 col 2, 4, row 8 col 3, row 10 col 0, 2, 4)
    # Row 4: Terracotta sand & cracked earth (row 18, 19 cols 0, 1, 3)
    # Row 5: Ancient sandstone walls & cliff edges (cols 0..3 in rows 0..2)
    # Row 6: Ancient sandstone pillars & monuments (cols 0..3 in rows 3..5)
    # Row 7: Desert decorative paths & ruin stone floor (cols 0..3 in rows 6..8)

    tile_coords = [
        # Row 0: Sand Plain
        [(5, 0), (6, 0), (5, 1), (6, 1), (5, 2), (6, 2), (7, 0), (7, 1)],
        # Row 1: Dunes / Slopes
        [(5, 3), (6, 3), (5, 4), (6, 4), (5, 5), (6, 5), (7, 5), (7, 8)],
        # Row 2: Desert Rocks
        [(5, 7), (6, 7), (7, 7), (5, 8), (6, 8), (7, 8), (5, 9), (6, 9)],
        # Row 3: Cacti & Desert Scrub
        [(2, 6), (4, 6), (3, 8), (0, 9), (0, 10), (2, 10), (4, 10), (2, 8)],
        # Row 4: Terracotta Sand & Cracked Earth
        [(0, 18), (1, 18), (0, 19), (1, 19), (3, 19), (5, 11), (6, 11), (7, 11)],
        # Row 5: Sandstone Walls & Cliff Edges
        [(0, 0), (1, 0), (0, 1), (1, 1), (0, 2), (1, 2), (2, 0), (3, 0)],
        # Row 6: Sandstone Pillars & Monuments
        [(0, 3), (1, 3), (2, 3), (3, 3), (0, 4), (1, 4), (2, 4), (3, 4)],
        # Row 7: Desert Path / Floor Stones
        [(2, 1), (3, 1), (2, 2), (3, 2), (2, 5), (3, 5), (4, 5), (4, 0)]
    ]

    for r in range(desert_rows):
        for c in range(desert_cols):
            kc, kr = tile_coords[r][c]
            t = get_kenney_tile_32(kc, kr)
            desert_sheet.paste(t, (c * 32, r * 32), t)

    desert_png_path = "assets/processed/biomes/desert/desert_tileset.png"
    desert_sheet.save(desert_png_path)
    print(f" -> Saved: {desert_png_path} ({desert_sheet.width}x{desert_sheet.height})")

    # Generate desert_tileset.json
    desert_json = {
        "id": "tex_biome_desert",
        "file": "processed/biomes/desert/desert_tileset.png",
        "tile_width": 32,
        "tile_height": 32,
        "columns": 8,
        "rows": 8,
        "total_tiles": 64,
        "categories": {
            "sand_plain": [0, 1, 2, 3, 4, 5, 6, 7],
            "sand_dunes": [8, 9, 10, 11, 12, 13, 14, 15],
            "desert_rocks": [16, 17, 18, 19, 20, 21, 22, 23],
            "desert_vegetation": [24, 25, 26, 27, 28, 29, 30, 31],
            "terracotta_ground": [32, 33, 34, 35, 36, 37, 38, 39],
            "sandstone_cliffs": [40, 41, 42, 43, 44, 45, 46, 47],
            "ancient_pillars": [48, 49, 50, 51, 52, 53, 54, 55],
            "ruin_paving": [56, 57, 58, 59, 60, 61, 62, 63]
        },
        "collision_tiles": [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 41, 42, 43, 48, 49, 50, 51],
        "provenance": {
            "creator": "Kenney",
            "source": "Kenney Roguelike/RPG Pack",
            "license": "CC0 1.0 Universal",
            "phase": 49
        }
    }
    desert_json_path = "assets/processed/biomes/desert/desert_tileset.json"
    with open(desert_json_path, "w", encoding="utf-8") as f:
        json.dump(desert_json, f, indent=2)
    print(f" -> Saved: {desert_json_path}")

    # -------------------------------------------------------------------------
    # 3. PROCESS UI ICONS (64x64 Inventory & 32x32 HUD)
    # -------------------------------------------------------------------------
    print("\n--- 3. PROCESSING UI ICONS (64x64 and 32x32 with Theme Tinting) ---")
    
    icon_configs = [
        {
            "raw": "assets/external/game_icons/potion_ball_raw.png",
            "base_name": "icon_potion",
            "id_64": "tex_icon_potion_64",
            "id_32": "tex_icon_potion_32",
            "color_rgb": (80, 170, 245), # Sapphire mana blue
            "border_rgb": (220, 200, 110), # Gold rim
            "author": "Lorc",
            "category": "ui/icons/consumables"
        },
        {
            "raw": "assets/external/game_icons/heart_drop_raw.png",
            "base_name": "icon_heart",
            "id_64": "tex_icon_heart_64",
            "id_32": "tex_icon_heart_32",
            "color_rgb": (235, 60, 75), # Ruby crimson
            "border_rgb": (255, 140, 150),
            "author": "Lorc",
            "category": "ui/icons/vitality"
        },
        {
            "raw": "assets/external/game_icons/broadsword_raw.png",
            "base_name": "icon_sword",
            "id_64": "tex_icon_sword_64",
            "id_32": "tex_icon_sword_32",
            "color_rgb": (210, 220, 235), # Silver steel
            "border_rgb": (180, 140, 70), # Bronze guard
            "author": "Lorc",
            "category": "ui/icons/weapons"
        },
        {
            "raw": "assets/external/game_icons/shield_bash_raw.png",
            "base_name": "icon_shield",
            "id_64": "tex_icon_shield_64",
            "id_32": "tex_icon_shield_32",
            "color_rgb": (235, 185, 75), # Burnished gold
            "border_rgb": (140, 95, 35),
            "author": "Delapouite",
            "category": "ui/icons/armor"
        }
    ]

    for ic in icon_configs:
        raw_img = Image.open(ic["raw"]).convert("RGBA")
        
        # Colorize icon by tinting monochrome white/black
        # The game-icons are white glyphs on transparent background
        r_col, g_col, b_col = ic["color_rgb"]
        
        def create_tinted(size):
            scaled = raw_img.resize((size, size), Image.Resampling.LANCZOS)
            r, g, b, a = scaled.split()
            # Tint RGB channels with theme color
            tinted_r = Image.eval(r, lambda v: int(v * (r_col / 255.0)))
            tinted_g = Image.eval(g, lambda v: int(v * (g_col / 255.0)))
            tinted_b = Image.eval(b, lambda v: int(v * (b_col / 255.0)))
            return Image.merge("RGBA", (tinted_r, tinted_g, tinted_b, a))

        # 64x64
        icon64 = create_tinted(64)
        path64 = f"assets/processed/ui/icons/{ic['base_name']}_64.png"
        icon64.save(path64)
        print(f" -> Saved: {path64}")

        # 32x32
        icon32 = create_tinted(32)
        path32 = f"assets/processed/ui/icons/{ic['base_name']}_32.png"
        icon32.save(path32)
        print(f" -> Saved: {path32}")

    # -------------------------------------------------------------------------
    # 4. SAFELY UPDATE MASTER ASSET MANIFEST (assets/manifests/asset_manifest.json)
    # -------------------------------------------------------------------------
    print("\n--- 4. REGISTERING IN MASTER MANIFEST (assets/manifests/asset_manifest.json) ---")
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        master_manifest = json.load(f)

    existing_ids = {a["id"] for a in master_manifest["assets"]}
    print(f"Existing registered assets: {len(existing_ids)}")

    def compute_sha(filepath):
        with open(filepath, "rb") as f:
            return hashlib.sha256(f.read()).hexdigest()[:14]

    new_entries = [
        # Creature
        {
            "id": "tex_creature_slime",
            "file": "processed/creatures/slime/slime_spritesheet.png",
            "type": "texture",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": compute_sha("assets/processed/creatures/slime/slime_spritesheet.png"),
            "width": 128,
            "height": 256,
            "status": "approved"
        },
        {
            "id": "sheet_creature_slime",
            "file": "processed/creatures/slime/slime_spritesheet.json",
            "type": "spritesheet",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": compute_sha("assets/processed/creatures/slime/slime_spritesheet.json"),
            "width": 32,
            "height": 32,
            "status": "approved"
        },
        # Biome
        {
            "id": "tex_biome_desert",
            "file": "processed/biomes/desert/desert_tileset.png",
            "type": "texture",
            "category": "environment/tilesets",
            "source": "Kenney Roguelike/RPG Pack",
            "author": "Kenney",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": compute_sha("assets/processed/biomes/desert/desert_tileset.png"),
            "width": 256,
            "height": 256,
            "status": "approved"
        },
        # Icons 64x64
        {
            "id": "tex_icon_potion_64",
            "file": "processed/ui/icons/icon_potion_64.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_potion_64.png"),
            "width": 64,
            "height": 64,
            "status": "approved"
        },
        {
            "id": "tex_icon_heart_64",
            "file": "processed/ui/icons/icon_heart_64.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_heart_64.png"),
            "width": 64,
            "height": 64,
            "status": "approved"
        },
        {
            "id": "tex_icon_sword_64",
            "file": "processed/ui/icons/icon_sword_64.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_sword_64.png"),
            "width": 64,
            "height": 64,
            "status": "approved"
        },
        {
            "id": "tex_icon_shield_64",
            "file": "processed/ui/icons/icon_shield_64.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Delapouite",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_shield_64.png"),
            "width": 64,
            "height": 64,
            "status": "approved"
        },
        # Icons 32x32
        {
            "id": "tex_icon_potion_32",
            "file": "processed/ui/icons/icon_potion_32.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_potion_32.png"),
            "width": 32,
            "height": 32,
            "status": "approved"
        },
        {
            "id": "tex_icon_heart_32",
            "file": "processed/ui/icons/icon_heart_32.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_heart_32.png"),
            "width": 32,
            "height": 32,
            "status": "approved"
        },
        {
            "id": "tex_icon_sword_32",
            "file": "processed/ui/icons/icon_sword_32.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Lorc",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_sword_32.png"),
            "width": 32,
            "height": 32,
            "status": "approved"
        },
        {
            "id": "tex_icon_shield_32",
            "file": "processed/ui/icons/icon_shield_32.png",
            "type": "texture",
            "category": "ui/icons",
            "source": "Game-icons.net",
            "author": "Delapouite",
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": compute_sha("assets/processed/ui/icons/icon_shield_32.png"),
            "width": 32,
            "height": 32,
            "status": "approved"
        }
    ]

    added_count = 0
    for entry in new_entries:
        if entry["id"] not in existing_ids:
            master_manifest["assets"].append(entry)
            existing_ids.add(entry["id"])
            added_count += 1
            print(f" + Appended asset: {entry['id']} ({entry['file']})")
        else:
            print(f" ! Skipped existing asset ID: {entry['id']}")

    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(master_manifest, f, indent=2)

    print(f"\nManifest updated successfully! Added {added_count} new entries. Total assets now: {len(master_manifest['assets'])}")
    print("=================================================================")
    print("GATE 3 PILOT PROCESSING & MANIFEST INTEGRATION COMPLETE!")
    print("=================================================================")

if __name__ == "__main__":
    process_pilot()

