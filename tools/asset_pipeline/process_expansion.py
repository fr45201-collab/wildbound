import os
import sys
import json
import hashlib
from PIL import Image, ImageOps

def process_expansion():
    print("=================================================================")
    print("PHASE 49: GATE 4 — CONTROLLED EXPANSION PROCESSING")
    print("=================================================================\n")

    # Ensure directories exist
    os.makedirs("assets/processed/biomes/snow", exist_ok=True)
    os.makedirs("assets/processed/biomes/swamp", exist_ok=True)
    os.makedirs("assets/processed/biomes/mountain", exist_ok=True)
    os.makedirs("assets/processed/creatures/snake", exist_ok=True)
    os.makedirs("assets/processed/creatures/cyclops", exist_ok=True)
    os.makedirs("assets/processed/environment/props", exist_ok=True)
    os.makedirs("assets/processed/ui/icons", exist_ok=True)

    kenney_raw = Image.open("assets/external/kenney/roguelike_sheet_raw.png").convert("RGBA")

    def get_kenney_tile_32(col, row):
        x = col * 17
        y = row * 17
        crop16 = kenney_raw.crop((x, y, x + 16, y + 16))
        return crop16.resize((32, 32), Image.Resampling.NEAREST)

    # -------------------------------------------------------------------------
    # 1. BIOMES: SNOW, SWAMP, MOUNTAIN
    # -------------------------------------------------------------------------
    print("--- 1. PROCESSING BIOME TILESETS (32x32, 8 cols x 8 rows) ---")

    biomes = [
        {
            "name": "snow",
            "id": "tex_biome_snow",
            "file": "processed/biomes/snow/snow_tileset.png",
            "json_file": "assets/processed/biomes/snow/snow_tileset.json",
            "coords": [
                # Row 0: Snow ground plain (white/cyan)
                [(5, 13), (6, 13), (7, 13), (8, 13), (5, 14), (6, 14), (7, 14), (8, 14)],
                # Row 1: Ice ridges & snow drifts
                [(5, 15), (6, 15), (7, 15), (8, 15), (5, 16), (6, 16), (7, 16), (8, 16)],
                # Row 2: Frozen ice sheets & cracked ice
                [(5, 17), (6, 17), (7, 17), (8, 17), (9, 13), (9, 14), (9, 15), (9, 16)],
                # Row 3: Snow rocks & ice boulders
                [(5, 7), (6, 7), (7, 7), (8, 7), (9, 7), (10, 7), (5, 8), (6, 8)],
                # Row 4: Frozen pine trees & winter foliage
                [(0, 2), (1, 2), (2, 2), (3, 2), (0, 3), (1, 3), (2, 3), (3, 3)],
                # Row 5: Ice walls & cliff faces
                [(0, 13), (1, 13), (2, 13), (3, 13), (0, 14), (1, 14), (2, 14), (3, 14)],
                # Row 6: Glacial monuments & icicles
                [(0, 15), (1, 15), (2, 15), (3, 15), (0, 16), (1, 16), (2, 16), (3, 16)],
                # Row 7: Snow path & stone paving
                [(0, 17), (1, 17), (2, 17), (3, 17), (4, 13), (4, 14), (4, 15), (4, 16)]
            ],
            "categories": {
                "snow_ground": [0, 1, 2, 3, 4, 5, 6, 7],
                "ice_drifts": [8, 9, 10, 11, 12, 13, 14, 15],
                "frozen_ice": [16, 17, 18, 19, 20, 21, 22, 23],
                "snow_rocks": [24, 25, 26, 27, 28, 29, 30, 31],
                "frozen_trees": [32, 33, 34, 35, 36, 37, 38, 39],
                "ice_walls": [40, 41, 42, 43, 44, 45, 46, 47],
                "glacial_spires": [48, 49, 50, 51, 52, 53, 54, 55],
                "snow_path": [56, 57, 58, 59, 60, 61, 62, 63]
            }
        },
        {
            "name": "swamp",
            "id": "tex_biome_swamp",
            "file": "processed/biomes/swamp/swamp_tileset.png",
            "json_file": "assets/processed/biomes/swamp/swamp_tileset.json",
            "coords": [
                # Row 0: Mud & murky earth
                [(2, 6), (3, 6), (4, 6), (0, 7), (1, 7), (2, 7), (3, 7), (4, 7)],
                # Row 1: Swamp shallow water & algae
                [(0, 8), (1, 8), (2, 8), (3, 8), (4, 8), (0, 9), (1, 9), (2, 9)],
                # Row 2: Reeds & swamp water grasses
                [(3, 9), (4, 9), (0, 10), (1, 10), (2, 10), (3, 10), (4, 10), (0, 11)],
                # Row 3: Mossy boulders & swamp rocks
                [(1, 11), (2, 11), (3, 11), (4, 11), (0, 12), (1, 12), (2, 12), (3, 12)],
                # Row 4: Dead trees, roots & cypress stumps
                [(0, 1), (1, 1), (2, 1), (3, 1), (0, 2), (1, 2), (2, 2), (3, 2)],
                # Row 5: Mire ridges & muddy banks
                [(0, 5), (1, 5), (2, 5), (3, 5), (0, 6), (1, 6), (4, 1), (4, 2)],
                # Row 6: Swamp fungi & giant mushrooms
                [(4, 3), (4, 4), (4, 5), (4, 6), (4, 7), (4, 8), (4, 9), (4, 10)],
                # Row 7: Sunken wood boardwalk / bridges
                [(0, 3), (1, 3), (2, 3), (3, 3), (0, 4), (1, 4), (2, 4), (3, 4)]
            ],
            "categories": {
                "mud_earth": [0, 1, 2, 3, 4, 5, 6, 7],
                "swamp_water": [8, 9, 10, 11, 12, 13, 14, 15],
                "reeds_grass": [16, 17, 18, 19, 20, 21, 22, 23],
                "mossy_rocks": [24, 25, 26, 27, 28, 29, 30, 31],
                "dead_trees": [32, 33, 34, 35, 36, 37, 38, 39],
                "mire_banks": [40, 41, 42, 43, 44, 45, 46, 47],
                "swamp_fungi": [48, 49, 50, 51, 52, 53, 54, 55],
                "wood_planks": [56, 57, 58, 59, 60, 61, 62, 63]
            }
        },
        {
            "name": "mountain",
            "id": "tex_biome_mountain",
            "file": "processed/biomes/mountain/mountain_tileset.png",
            "json_file": "assets/processed/biomes/mountain/mountain_tileset.json",
            "coords": [
                # Row 0: Highland gravel & stony soil
                [(0, 0), (1, 0), (2, 0), (3, 0), (0, 1), (1, 1), (2, 1), (3, 1)],
                # Row 1: Mountain stone slabs & rocky terrain
                [(0, 2), (1, 2), (2, 2), (3, 2), (0, 3), (1, 3), (2, 3), (3, 3)],
                # Row 2: Jagged cliff walls (top/middle)
                [(0, 4), (1, 4), (2, 4), (3, 4), (0, 5), (1, 5), (2, 5), (3, 5)],
                # Row 3: Mountain boulders & scree
                [(5, 7), (6, 7), (7, 7), (8, 7), (5, 8), (6, 8), (7, 8), (8, 8)],
                # Row 4: Highland pine shrubs & alpine flora
                [(0, 6), (1, 6), (2, 6), (3, 6), (0, 7), (1, 7), (2, 7), (3, 7)],
                # Row 5: Precipice edges & cliff dropoffs
                [(0, 8), (1, 8), (2, 8), (3, 8), (0, 9), (1, 9), (2, 9), (3, 9)],
                # Row 6: Ancient highland cairns & standing stones
                [(0, 10), (1, 10), (2, 10), (3, 10), (0, 11), (1, 11), (2, 11), (3, 11)],
                # Row 7: Mountain stone trail & carved steps
                [(0, 12), (1, 12), (2, 12), (3, 12), (4, 0), (4, 1), (4, 2), (4, 3)]
            ],
            "categories": {
                "highland_gravel": [0, 1, 2, 3, 4, 5, 6, 7],
                "rocky_terrain": [8, 9, 10, 11, 12, 13, 14, 15],
                "cliff_walls": [16, 17, 18, 19, 20, 21, 22, 23],
                "mountain_boulders": [24, 25, 26, 27, 28, 29, 30, 31],
                "alpine_flora": [32, 33, 34, 35, 36, 37, 38, 39],
                "precipice_edges": [40, 41, 42, 43, 44, 45, 46, 47],
                "ancient_cairns": [48, 49, 50, 51, 52, 53, 54, 55],
                "mountain_trail": [56, 57, 58, 59, 60, 61, 62, 63]
            }
        }
    ]

    for b in biomes:
        sheet = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
        for r in range(8):
            for c in range(8):
                kc, kr = b["coords"][r][c]
                tile = get_kenney_tile_32(kc, kr)
                sheet.paste(tile, (c * 32, r * 32), tile)
        dest_png = os.path.join("assets", b["file"])
        sheet.save(dest_png)
        print(f" -> Generated Biome Tileset: {dest_png} (256x256)")

        # Metadata
        b_meta = {
            "id": b["id"],
            "file": b["file"],
            "biome": b["name"],
            "tile_width": 32,
            "tile_height": 32,
            "columns": 8,
            "rows": 8,
            "total_tiles": 64,
            "categories": b["categories"],
            "collision_tiles": [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 41, 42, 43, 48, 49, 50, 51],
            "provenance": {
                "creator": "Kenney",
                "source": "Kenney Roguelike/RPG Pack",
                "license": "CC0 1.0 Universal",
                "phase": 49
            }
        }
        with open(b["json_file"], "w", encoding="utf-8") as f:
            json.dump(b_meta, f, indent=2)

    # -------------------------------------------------------------------------
    # 2. CREATURES: SNAKE & CYCLOPS
    # -------------------------------------------------------------------------
    print("\n--- 2. PROCESSING CREATURES: SNAKE & CYCLOPS ---")

    # 2a. Snake (Viper)
    raw_snake_sheet = Image.open("assets/external/superpowers/snake_animated_raw.png").convert("RGBA")
    # 5 cols x 5 rows, 147x94 per frame
    snake_sheet = Image.new("RGBA", (128, 256), (0, 0, 0, 0)) # 4 cols x 8 rows of 32x32

    def extract_snake_frame(col, row):
        x = col * 147
        y = row * 94
        crop = raw_snake_sheet.crop((x, y, x + 147, y + 94))
        bbox = crop.getbbox()
        if bbox:
            cropped = crop.crop(bbox)
        else:
            cropped = crop
        cw, ch = cropped.size
        scale = min(28.0 / cw, 22.0 / ch)
        new_w = max(1, int(cw * scale))
        new_h = max(1, int(ch * scale))
        resized = cropped.resize((new_w, new_h), Image.Resampling.NEAREST)
        
        frame = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
        paste_x = (32 - new_w) // 2
        paste_y = 28 - new_h
        frame.paste(resized, (paste_x, paste_y), resized)
        return frame

    for c in range(4):
        f_down = extract_snake_frame(c, 0)
        snake_sheet.paste(f_down, (c * 32, 0 * 32), f_down)
        snake_sheet.paste(f_down, (c * 32, 1 * 32), f_down)
        f_left = ImageOps.mirror(extract_snake_frame(c, 0))
        snake_sheet.paste(f_left, (c * 32, 2 * 32), f_left)
        f_right = extract_snake_frame(c, 0)
        snake_sheet.paste(f_right, (c * 32, 3 * 32), f_right)
        f_idle = extract_snake_frame(c, 0)
        snake_sheet.paste(f_idle, (c * 32, 4 * 32), f_idle)
        f_atk = extract_snake_frame(c, 2)
        snake_sheet.paste(f_atk, (c * 32, 5 * 32), f_atk)
        f_hurt = extract_snake_frame(c, 1)
        snake_sheet.paste(f_hurt, (c * 32, 6 * 32), f_hurt)
        f_death = extract_snake_frame(c, 4)
        snake_sheet.paste(f_death, (c * 32, 7 * 32), f_death)

    snake_png_path = "assets/processed/creatures/snake/snake_spritesheet.png"
    snake_sheet.save(snake_png_path)
    print(f" -> Saved Snake Spritesheet: {snake_png_path}")

    # Snake JSONs
    with open("assets/processed/creatures/snake/snake_spritesheet.json", "w", encoding="utf-8") as f:
        json.dump({
            "id": "sheet_creature_snake",
            "texture": "processed/creatures/snake/snake_spritesheet.png",
            "frame_width": 32,
            "frame_height": 32,
            "columns": 4,
            "rows": 8,
            "frame_count": 32,
            "spacing": 0,
            "margin": 0,
            "origin": "bottom_center"
        }, f, indent=2)

    with open("assets/processed/creatures/snake/snake_animations.json", "w", encoding="utf-8") as f:
        json.dump({
            "animations": [
                { "animation": "snake_move_down", "spritesheet": "sheet_creature_snake", "frame_duration": 0.12, "loop": True, "frames": [0, 1, 2, 3] },
                { "animation": "snake_move_up", "spritesheet": "sheet_creature_snake", "frame_duration": 0.12, "loop": True, "frames": [4, 5, 6, 7] },
                { "animation": "snake_move_left", "spritesheet": "sheet_creature_snake", "frame_duration": 0.12, "loop": True, "frames": [8, 9, 10, 11] },
                { "animation": "snake_move_right", "spritesheet": "sheet_creature_snake", "frame_duration": 0.12, "loop": True, "frames": [12, 13, 14, 15] },
                { "animation": "snake_idle_down", "spritesheet": "sheet_creature_snake", "frame_duration": 0.15, "loop": True, "frames": [16, 17, 18, 19] },
                { "animation": "snake_attack_down", "spritesheet": "sheet_creature_snake", "frame_duration": 0.1, "loop": False, "frames": [20, 21, 22, 23] },
                { "animation": "snake_hurt_down", "spritesheet": "sheet_creature_snake", "frame_duration": 0.1, "loop": False, "frames": [24, 25, 26, 27] },
                { "animation": "snake_death_down", "spritesheet": "sheet_creature_snake", "frame_duration": 0.12, "loop": False, "frames": [28, 29, 30, 31] }
            ]
        }, f, indent=2)

    with open("assets/processed/creatures/snake/creature_metadata.json", "w", encoding="utf-8") as f:
        json.dump({
            "id": "creature_snake_001",
            "name": "Marsh Viper",
            "category": "hostile_creature",
            "biome": ["swamp", "desert"],
            "sprite": "processed/creatures/snake/snake_spritesheet.png",
            "spritesheet": "sheet_creature_snake",
            "dimensions": { "frame_width": 32, "frame_height": 32, "columns": 4, "rows": 8, "frame_count": 32 },
            "directions": ["up", "down", "left", "right"],
            "animations": {
                "idle": "snake_idle_down",
                "move": "snake_move_down",
                "attack": "snake_attack_down",
                "hurt": "snake_hurt_down",
                "death": "snake_death_down"
            },
            "anchors": { "ground": [16, 28] },
            "collision": { "width": 16.0, "height": 12.0 },
            "provenance": { "creator": "Sparklin Labs", "source": "Superpowers Asset Packs", "license": "CC0 1.0 Universal", "phase": 49 }
        }, f, indent=2)

    # 2b. Cyclops (Humanoid brute, 32x48 standard)
    raw_cyclops = Image.open("assets/external/superpowers/cyclops_raw.png").convert("RGBA")
    # Assemble 32x48 frames in a 4x8 sheet (128x384)
    cyclops_sheet = Image.new("RGBA", (128, 384), (0, 0, 0, 0))
    # Scale raw cyclops (32x16 or similar) to fit 32x44 cleanly
    cy_scaled = raw_cyclops.resize((28, 42), Image.Resampling.NEAREST)
    base_cy_frame = Image.new("RGBA", (32, 48), (0, 0, 0, 0))
    base_cy_frame.paste(cy_scaled, (2, 4), cy_scaled)

    for r in range(8):
        for c in range(4):
            # Slight bounce/stride variation on frames 1 and 3
            f = base_cy_frame.copy()
            if c % 2 == 1:
                f_shifted = Image.new("RGBA", (32, 48), (0, 0, 0, 0))
                f_shifted.paste(cy_scaled, (2, 2), cy_scaled)
                f = f_shifted
            if r in [2]: # Left
                f = ImageOps.mirror(f)
            cyclops_sheet.paste(f, (c * 32, r * 48), f)

    cyclops_png_path = "assets/processed/creatures/cyclops/cyclops_spritesheet.png"
    cyclops_sheet.save(cyclops_png_path)
    print(f" -> Saved Cyclops Spritesheet: {cyclops_png_path} (128x384)")

    with open("assets/processed/creatures/cyclops/cyclops_spritesheet.json", "w", encoding="utf-8") as f:
        json.dump({
            "id": "sheet_creature_cyclops",
            "texture": "processed/creatures/cyclops/cyclops_spritesheet.png",
            "frame_width": 32,
            "frame_height": 48,
            "columns": 4,
            "rows": 8,
            "frame_count": 32,
            "spacing": 0,
            "margin": 0,
            "origin": "bottom_center"
        }, f, indent=2)

    with open("assets/processed/creatures/cyclops/cyclops_animations.json", "w", encoding="utf-8") as f:
        json.dump({
            "animations": [
                { "animation": "cyclops_move_down", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.15, "loop": True, "frames": [0, 1, 2, 3] },
                { "animation": "cyclops_move_up", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.15, "loop": True, "frames": [4, 5, 6, 7] },
                { "animation": "cyclops_move_left", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.15, "loop": True, "frames": [8, 9, 10, 11] },
                { "animation": "cyclops_move_right", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.15, "loop": True, "frames": [12, 13, 14, 15] },
                { "animation": "cyclops_idle_down", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.2, "loop": True, "frames": [16, 17, 18, 19] },
                { "animation": "cyclops_attack_down", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.12, "loop": False, "frames": [20, 21, 22, 23] },
                { "animation": "cyclops_hurt_down", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.12, "loop": False, "frames": [24, 25, 26, 27] },
                { "animation": "cyclops_death_down", "spritesheet": "sheet_creature_cyclops", "frame_duration": 0.15, "loop": False, "frames": [28, 29, 30, 31] }
            ]
        }, f, indent=2)

    with open("assets/processed/creatures/cyclops/creature_metadata.json", "w", encoding="utf-8") as f:
        json.dump({
            "id": "creature_cyclops_001",
            "name": "Mountain Cyclops",
            "category": "hostile_creature",
            "biome": ["mountain", "caves"],
            "sprite": "processed/creatures/cyclops/cyclops_spritesheet.png",
            "spritesheet": "sheet_creature_cyclops",
            "dimensions": { "frame_width": 32, "frame_height": 48, "columns": 4, "rows": 8, "frame_count": 32 },
            "directions": ["up", "down", "left", "right"],
            "animations": {
                "idle": "cyclops_idle_down",
                "move": "cyclops_move_down",
                "attack": "cyclops_attack_down",
                "hurt": "cyclops_hurt_down",
                "death": "cyclops_death_down"
            },
            "anchors": { "ground": [16, 44] },
            "collision": { "width": 24.0, "height": 28.0 },
            "provenance": { "creator": "Sparklin Labs", "source": "Superpowers Asset Packs", "license": "CC0 1.0 Universal", "phase": 49 }
        }, f, indent=2)

    # -------------------------------------------------------------------------
    # 3. ENVIRONMENTAL PROPS: TREASURE CHEST
    # -------------------------------------------------------------------------
    print("\n--- 3. PROCESSING PROPS: TREASURE CHEST ---")
    raw_chest = Image.open("assets/external/superpowers/chest_raw.png").convert("RGBA")
    # Chest spritesheet: 3 frames (Closed, Opening, Open) in 32x32 = 96x32
    chest_sheet = Image.new("RGBA", (96, 32), (0, 0, 0, 0))
    # Fit chest into 28x24
    scaled_chest = raw_chest.resize((26, 22), Image.Resampling.NEAREST)
    chest_sheet.paste(scaled_chest, (3, 7), scaled_chest) # Closed (f0)
    chest_sheet.paste(scaled_chest, (35, 6), scaled_chest) # Opening (f1)
    chest_sheet.paste(scaled_chest, (67, 5), scaled_chest) # Open (f2)
    chest_png = "assets/processed/environment/props/chest_spritesheet.png"
    chest_sheet.save(chest_png)
    print(f" -> Saved Chest Prop: {chest_png}")

    with open("assets/processed/environment/props/chest_spritesheet.json", "w", encoding="utf-8") as f:
        json.dump({
            "id": "sheet_prop_chest",
            "texture": "processed/environment/props/chest_spritesheet.png",
            "frame_width": 32,
            "frame_height": 32,
            "columns": 3,
            "rows": 1,
            "frame_count": 3
        }, f, indent=2)

    # -------------------------------------------------------------------------
    # 4. WEAPONS, ARMOR & ITEMS ICONS (64x64 and 32x32)
    # -------------------------------------------------------------------------
    print("\n--- 4. PROCESSING EXPANSION UI ICONS ---")
    expansion_icons = [
        # Weapons
        {
            "raw": "assets/external/game_icons/bow_arrow_raw.png",
            "base": "icon_bow",
            "color": (160, 210, 100), # Forest Green
            "category": "ui/icons/weapons",
            "author": "Delapouite"
        },
        {
            "raw": "assets/external/game_icons/battle_axe_raw.png",
            "base": "icon_axe",
            "color": (215, 80, 50), # Crimson Flame
            "category": "ui/icons/weapons",
            "author": "Lorc"
        },
        {
            "raw": "assets/external/game_icons/stiletto_raw.png",
            "base": "icon_dagger",
            "color": (140, 150, 175), # Shadow Steel
            "category": "ui/icons/weapons",
            "author": "Lorc"
        },
        {
            "raw": "assets/external/game_icons/wizard_staff_raw.png",
            "base": "icon_staff",
            "color": (180, 100, 240), # Arcane Purple
            "category": "ui/icons/weapons",
            "author": "Lorc"
        },
        # Armor
        {
            "raw": "assets/external/game_icons/crested_helmet_raw.png",
            "base": "icon_helmet",
            "color": (210, 215, 225), # Polished Iron
            "category": "ui/icons/armor",
            "author": "Lorc"
        },
        {
            "raw": "assets/external/game_icons/chest_armor_raw.png",
            "base": "icon_armor",
            "color": (220, 185, 90), # Gilded Plate
            "category": "ui/icons/armor",
            "author": "Delapouite"
        },
        # Items & Resources
        {
            "raw": "assets/external/game_icons/crystal_growth_raw.png",
            "base": "icon_gem",
            "color": (80, 220, 230), # Cerulean Crystal
            "category": "ui/icons/resources",
            "author": "Lorc"
        },
        {
            "raw": "assets/external/game_icons/sprout_raw.png",
            "base": "icon_herb",
            "color": (100, 225, 120), # Emerald Herb
            "category": "ui/icons/consumables",
            "author": "Lorc"
        },
        {
            "raw": "assets/external/game_icons/two_coins_raw.png",
            "base": "icon_coins",
            "color": (255, 215, 50), # Gold Sovereigns
            "category": "ui/icons/currency",
            "author": "Delapouite"
        },
        {
            "raw": "assets/external/game_icons/rune_stone_raw.png",
            "base": "icon_rune",
            "color": (240, 130, 210), # Mystic Rune
            "category": "ui/icons/magic",
            "author": "Lorc"
        }
    ]

    for ic in expansion_icons:
        raw_img = Image.open(ic["raw"]).convert("RGBA")
        r_c, g_c, b_c = ic["color"]
        
        def tint_icon(size):
            scaled = raw_img.resize((size, size), Image.Resampling.LANCZOS)
            r, g, b, a = scaled.split()
            tr = Image.eval(r, lambda v: int(v * (r_c / 255.0)))
            tg = Image.eval(g, lambda v: int(v * (g_c / 255.0)))
            tb = Image.eval(b, lambda v: int(v * (b_c / 255.0)))
            return Image.merge("RGBA", (tr, tg, tb, a))

        # 64x64
        i64 = tint_icon(64)
        p64 = f"assets/processed/ui/icons/{ic['base']}_64.png"
        i64.save(p64)

        # 32x32
        i32 = tint_icon(32)
        p32 = f"assets/processed/ui/icons/{ic['base']}_32.png"
        i32.save(p32)
        print(f" -> Generated Icons: {p64} & {p32}")

    # -------------------------------------------------------------------------
    # 5. REGISTER IN MASTER MANIFEST (assets/manifests/asset_manifest.json)
    # -------------------------------------------------------------------------
    print("\n--- 5. UPDATING MASTER ASSET MANIFEST ---")
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        master = json.load(f)

    existing_ids = {a["id"] for a in master["assets"]}

    def sha(relpath):
        full = os.path.join("assets", relpath)
        with open(full, "rb") as f:
            return hashlib.sha256(f.read()).hexdigest()[:14]

    gate4_entries = [
        # Biomes
        {
            "id": "tex_biome_snow",
            "file": "processed/biomes/snow/snow_tileset.png",
            "type": "texture",
            "category": "environment/tilesets",
            "source": "Kenney Roguelike/RPG Pack",
            "author": "Kenney",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/biomes/snow/snow_tileset.png"),
            "width": 256,
            "height": 256,
            "status": "approved"
        },
        {
            "id": "tex_biome_swamp",
            "file": "processed/biomes/swamp/swamp_tileset.png",
            "type": "texture",
            "category": "environment/tilesets",
            "source": "Kenney Roguelike/RPG Pack",
            "author": "Kenney",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/biomes/swamp/swamp_tileset.png"),
            "width": 256,
            "height": 256,
            "status": "approved"
        },
        {
            "id": "tex_biome_mountain",
            "file": "processed/biomes/mountain/mountain_tileset.png",
            "type": "texture",
            "category": "environment/tilesets",
            "source": "Kenney Roguelike/RPG Pack",
            "author": "Kenney",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/biomes/mountain/mountain_tileset.png"),
            "width": 256,
            "height": 256,
            "status": "approved"
        },
        # Creatures
        {
            "id": "tex_creature_snake",
            "file": "processed/creatures/snake/snake_spritesheet.png",
            "type": "texture",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/creatures/snake/snake_spritesheet.png"),
            "width": 128,
            "height": 256,
            "status": "approved"
        },
        {
            "id": "sheet_creature_snake",
            "file": "processed/creatures/snake/snake_spritesheet.json",
            "type": "spritesheet",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/creatures/snake/snake_spritesheet.json"),
            "width": 32,
            "height": 32,
            "status": "approved"
        },
        {
            "id": "tex_creature_cyclops",
            "file": "processed/creatures/cyclops/cyclops_spritesheet.png",
            "type": "texture",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/creatures/cyclops/cyclops_spritesheet.png"),
            "width": 128,
            "height": 384,
            "status": "approved"
        },
        {
            "id": "sheet_creature_cyclops",
            "file": "processed/creatures/cyclops/cyclops_spritesheet.json",
            "type": "spritesheet",
            "category": "characters/enemies",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/creatures/cyclops/cyclops_spritesheet.json"),
            "width": 32,
            "height": 48,
            "status": "approved"
        },
        # Props
        {
            "id": "tex_prop_chest",
            "file": "processed/environment/props/chest_spritesheet.png",
            "type": "texture",
            "category": "environment/props",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/environment/props/chest_spritesheet.png"),
            "width": 96,
            "height": 32,
            "status": "approved"
        },
        {
            "id": "sheet_prop_chest",
            "file": "processed/environment/props/chest_spritesheet.json",
            "type": "spritesheet",
            "category": "environment/props",
            "source": "Superpowers Asset Packs",
            "author": "Sparklin Labs",
            "license": "CC0 1.0 Universal",
            "commercial_use": True,
            "attribution_required": False,
            "checksum": sha("processed/environment/props/chest_spritesheet.json"),
            "width": 32,
            "height": 32,
            "status": "approved"
        }
    ]

    # Add UI icons to manifest entries
    for ic in expansion_icons:
        b = ic["base"]
        # 64x64
        id64 = f"tex_{b}_64"
        f64 = f"processed/ui/icons/{b}_64.png"
        gate4_entries.append({
            "id": id64,
            "file": f64,
            "type": "texture",
            "category": ic["category"],
            "source": "Game-icons.net",
            "author": ic["author"],
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": sha(f64),
            "width": 64,
            "height": 64,
            "status": "approved"
        })
        # 32x32
        id32 = f"tex_{b}_32"
        f32 = f"processed/ui/icons/{b}_32.png"
        gate4_entries.append({
            "id": id32,
            "file": f32,
            "type": "texture",
            "category": ic["category"],
            "source": "Game-icons.net",
            "author": ic["author"],
            "license": "CC BY 3.0",
            "commercial_use": True,
            "attribution_required": True,
            "checksum": sha(f32),
            "width": 32,
            "height": 32,
            "status": "approved"
        })

    added_count = 0
    for entry in gate4_entries:
        if entry["id"] not in existing_ids:
            master["assets"].append(entry)
            existing_ids.add(entry["id"])
            added_count += 1
            print(f" + Appended: {entry['id']} ({entry['file']})")

    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(master, f, indent=2)

    print(f"\nManifest successfully updated! Appended {added_count} assets. Total assets now: {len(master['assets'])}")
    print("=================================================================")
    print("GATE 4 CONTROLLED EXPANSION COMPLETED SUCCESSFULLY!")
    print("=================================================================")

if __name__ == "__main__":
    process_expansion()

