import os
import json
import math
from PIL import Image, ImageDraw

FRAME_W = 32
FRAME_H = 48
COLS = 4
ROWS = 8
SHEET_W = COLS * FRAME_W  # 128
SHEET_H = ROWS * FRAME_H  # 384

DIRECTIONS = ["down", "up", "left", "right"]

def get_frame_offsets(anim_type, col, d):
    bob_y = 0
    if anim_type == "walk":
        if col in [1, 3]:
            bob_y = -1
    elif anim_type == "idle":
        if col in [1, 2]:
            bob_y = -1

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

def save_phase17_cosmetic(img, item_id, name, category, slot, occlusion_flags=None, tint=None, description=""):
    if occlusion_flags is None:
        occlusion_flags = []
    if tint is None:
        tint = [255, 255, 255, 255]

    cat_dir_map = {
        "clothing": "clothing",
        "armor": "armor",
        "pants": "pants",
        "footwear": "footwear",
        "shoes": "footwear",
        "headwear": "headwear",
        "hat": "headwear",
        "face_accessories": "face_accessories",
        "face": "face_accessories",
        "goggles": "face_accessories",
        "mask": "face_accessories",
        "backpacks": "backpacks",
        "back": "backpacks",
        "character_variations": "character_variations",
        "skin": "character_variations",
        "hair": "character_variations"
    }
    dir_category = cat_dir_map.get(category, category)

    rel_folder = f"cosmetics/{dir_category}/{item_id}"
    full_folder = os.path.join("assets", rel_folder)
    os.makedirs(full_folder, exist_ok=True)

    png_path = os.path.join(full_folder, "sprite.png")
    sheet_json_path = os.path.join(full_folder, "spritesheet.json")
    meta_json_path = os.path.join(full_folder, "metadata.json")

    # 1. Save PNG
    img.save(png_path, "PNG")

    # 2. Spritesheet descriptor for AMS
    sheet_id = f"sheet_{item_id}"
    tex_id = f"tex_{item_id}"
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

    # 3. Section 14 Equipment Metadata
    meta_data = {
        "id": item_id,
        "display_name": name,
        "category": category,
        "slot": slot,
        "description": description,
        "sprite": rel_tex_path,
        "spritesheet": sheet_id,
        "texture": tex_id,
        "compatible_directions": ["up", "down", "left", "right"],
        "compatible_animations": ["idle", "walk", "run"],
        "occlusion_flags": occlusion_flags,
        "anchors": {
            "body": [16, 48]
        },
        "render_order": slot,
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 17 Costumes & Armor Collection",
            "license": "CC0 1.0 Universal",
            "attribution_required": False
        }
    }
    with open(meta_json_path, "w", encoding="utf-8") as f:
        json.dump(meta_data, f, indent=4)

    return meta_data

def build_all():
    catalog_items = []

    # =========================================================================
    # 1. CLOTHING COLLECTION (8 Designs)
    # =========================================================================
    clothing_configs = [
        ("clo_basic_traveler_001", "Basic Traveler Shirt", (210, 200, 180, 255), (130, 85, 45, 255), "Linen traveler shirt with laced neck and rolled brown cuffs."),
        ("clo_forest_tunic_001", "Forest Tunic", (45, 115, 55, 255), (30, 80, 40, 255), "Deep woodland green tunic with reinforced hem and brass buckle."),
        ("clo_desert_robe_001", "Desert Robe", (225, 210, 160, 255), (180, 160, 110, 255), "Flowing light linen desert robe offering sun and sand protection."),
        ("clo_snow_jacket_001", "Snow Jacket", (65, 105, 155, 255), (235, 240, 250, 255), "Heavy insulated winter parka with plush white fur collar trim."),
        ("clo_leather_jacket_001", "Leather Jacket", (105, 60, 35, 255), (75, 40, 20, 255), "Durable boiled cowhide jacket with brass buttons and lapel."),
        ("clo_mystic_robe_001", "Mystic Robe", (85, 45, 135, 255), (220, 185, 60, 255), "Arcane indigo vestment embroidered with gold runic script."),
        ("clo_royal_outfit_001", "Royal Outfit", (150, 30, 45, 255), (240, 215, 80, 255), "Velvet imperial tunic trimmed with ermine and gold thread."),
        ("clo_dark_adventurer_001", "Dark Adventurer Outfit", (38, 40, 48, 255), (70, 75, 88, 255), "Muted stealth adventurer tunic designed for night scouting.")
    ]

    for item_id, name, main_col, trim_col, desc in clothing_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_clothing_frame(bx, by, d, anim_type, col, m_col=main_col, t_col=trim_col):
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 26 + bob_y

            # Torso body (cx-6 to cx+6, cy-6 to cy+6)
            if d == "down":
                draw.rectangle((cx - 6, cy - 6, cx + 6, cy + 6), fill=m_col)
                # Collar / Neckline lace
                draw.line([(cx - 2, cy - 6), (cx + 2, cy - 6)], fill=t_col)
                draw.line([(cx, cy - 5), (cx, cy - 2)], fill=t_col)
                # Short/rolled sleeves
                draw.rectangle((cx - 8, cy - 5, cx - 6, cy + 1), fill=m_col)
                draw.rectangle((cx + 6, cy - 5, cx + 8, cy + 1), fill=m_col)
                draw.line([(cx - 6, cy + 6), (cx + 6, cy + 6)], fill=t_col)
            elif d == "up":
                draw.rectangle((cx - 6, cy - 6, cx + 6, cy + 6), fill=m_col)
                draw.rectangle((cx - 8, cy - 5, cx - 6, cy + 1), fill=m_col)
                draw.rectangle((cx + 6, cy - 5, cx + 8, cy + 1), fill=m_col)
                draw.line([(cx - 6, cy + 6), (cx + 6, cy + 6)], fill=t_col)
            elif d == "left":
                draw.rectangle((cx - 5, cy - 6, cx + 4, cy + 6), fill=m_col)
                draw.rectangle((cx - 7, cy - 5, cx - 4, cy + 2), fill=m_col)
                draw.line([(cx - 5, cy + 6), (cx + 4, cy + 6)], fill=t_col)
            elif d == "right":
                draw.rectangle((cx - 4, cy - 6, cx + 5, cy + 6), fill=m_col)
                draw.rectangle((cx + 4, cy - 5, cx + 7, cy + 2), fill=m_col)
                draw.line([(cx - 4, cy + 6), (cx + 5, cy + 6)], fill=t_col)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_clothing_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_clothing_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "clothing", "shirt", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 2. ARMOR COLLECTION (8 Designs)
    # =========================================================================
    armor_configs = [
        ("arm_leather_armor_001", "Leather Armor", (135, 80, 45, 255), (185, 125, 75, 255), "Toughened boiled leather cuirass with riveted shoulder straps."),
        ("arm_reinforced_leather_001", "Reinforced Leather Armor", (110, 65, 38, 255), (175, 180, 190, 255), "Studded leather breastplate reinforced with forged steel plates."),
        ("arm_iron_armor_001", "Iron Armor", (155, 160, 170, 255), (105, 110, 120, 255), "Solid cast iron breastplate with flared neck guard and arming cops."),
        ("arm_steel_armor_001", "Steel Armor", (210, 218, 230, 255), (145, 155, 170, 255), "Folded high-polish steel cuirass with central deflecting ridge."),
        ("arm_forest_guardian_001", "Forest Guardian Armor", (55, 130, 80, 255), (200, 230, 160, 255), "Living heartwood scale armor enchanted by ancient forest spirits."),
        ("arm_desert_armor_001", "Desert Armor", (215, 175, 75, 255), (145, 110, 40, 255), "Bronze-gilded lamellar cuirass tailored for searing dune combat."),
        ("arm_frost_armor_001", "Frost Armor", (125, 185, 235, 255), (220, 245, 255, 255), "Glacial steel cuirass radiating a crisp frost aura with fur rim."),
        ("arm_ancient_rune_001", "Ancient Rune Armor", (45, 48, 62, 255), (80, 220, 245, 255), "Celestial meteoric plate armor pulsating with luminous azure runes.")
    ]

    for item_id, name, plate_col, trim_col, desc in armor_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_armor_frame(bx, by, d, anim_type, col, p_col=plate_col, t_col=trim_col):
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 26 + bob_y

            if d == "down":
                # Breastplate
                draw.rectangle((cx - 7, cy - 6, cx + 7, cy + 5), fill=p_col)
                draw.line([(cx - 7, cy - 6), (cx + 7, cy - 6)], fill=t_col)
                # Central ridge
                draw.line([(cx, cy - 5), (cx, cy + 4)], fill=t_col)
                # Pauldrons (shoulders)
                draw.rectangle((cx - 9, cy - 6, cx - 6, cy - 1), fill=t_col)
                draw.rectangle((cx + 6, cy - 6, cx + 9, cy - 1), fill=t_col)
                # Lower rim
                draw.line([(cx - 6, cy + 5), (cx + 6, cy + 5)], fill=t_col)
            elif d == "up":
                # Backplate
                draw.rectangle((cx - 7, cy - 6, cx + 7, cy + 5), fill=p_col)
                draw.rectangle((cx - 9, cy - 6, cx - 6, cy - 1), fill=t_col)
                draw.rectangle((cx + 6, cy - 6, cx + 9, cy - 1), fill=t_col)
                draw.line([(cx - 6, cy - 6), (cx + 6, cy - 6)], fill=t_col)
                draw.line([(cx - 6, cy + 5), (cx + 6, cy + 5)], fill=t_col)
            elif d == "left":
                draw.rectangle((cx - 6, cy - 6, cx + 5, cy + 5), fill=p_col)
                draw.rectangle((cx - 8, cy - 6, cx - 5, cy - 1), fill=t_col)
                draw.line([(cx - 5, cy - 6), (cx + 4, cy - 6)], fill=t_col)
                draw.line([(cx - 5, cy + 5), (cx + 4, cy + 5)], fill=t_col)
            elif d == "right":
                draw.rectangle((cx - 5, cy - 6, cx + 6, cy + 5), fill=p_col)
                draw.rectangle((cx + 5, cy - 6, cx + 8, cy - 1), fill=t_col)
                draw.line([(cx - 4, cy - 6), (cx + 5, cy - 6)], fill=t_col)
                draw.line([(cx - 4, cy + 5), (cx + 5, cy + 5)], fill=t_col)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_armor_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_armor_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "armor", "armor", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 3. PANTS COLLECTION (6 Designs)
    # =========================================================================
    pants_configs = [
        ("clo_p_traveler_pants_001", "Traveler Pants", (110, 85, 60, 255), (75, 55, 38, 255), "Rugged canvas trousers built for long journeys across the wild."),
        ("clo_p_leather_pants_001", "Leather Pants", (75, 48, 30, 255), (50, 32, 20, 255), "Fitted dark leather riding breeches with reinforced knees."),
        ("clo_p_armored_pants_001", "Armored Pants", (125, 130, 140, 255), (75, 80, 90, 255), "Padded chausses fitted with steel poleyns and knee guards."),
        ("clo_p_desert_trousers_001", "Desert Trousers", (205, 185, 135, 255), (160, 140, 95, 255), "Lightweight loose-cut pleated trousers ideal for arid deserts."),
        ("clo_p_snow_trousers_001", "Snow Trousers", (90, 115, 140, 255), (200, 215, 230, 255), "Quilted winter trousers stuffed with down insulation."),
        ("clo_p_mystic_pants_001", "Mystic Pants", (65, 38, 95, 255), (180, 145, 45, 255), "Midnight violet scholar's breeches hemmed with gold silk.")
    ]

    for item_id, name, p_col, shadow_col, desc in pants_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_pants_frame(bx, by, d, anim_type, col, p_c=p_col, s_c=shadow_col):
            bob_y, left_dy, right_dy = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 34 + bob_y

            # Waist / Pelvis
            draw.rectangle((cx - 5, cy - 2, cx + 5, cy + 1), fill=p_c)

            if d in ["down", "up"]:
                # Left leg
                draw.rectangle((cx - 5, cy + 2, cx - 2, cy + 8 + left_dy), fill=p_c)
                # Right leg
                draw.rectangle((cx + 1, cy + 2, cx + 4, cy + 8 + right_dy), fill=p_c)
                # Inseam shadow
                draw.line([(cx - 1, cy + 2), (cx, cy + 4)], fill=s_c)
            elif d == "left":
                draw.rectangle((cx - 4, cy + 2, cx + 2, cy + 8 + left_dy), fill=p_c)
                if abs(right_dy) > 1:
                    draw.rectangle((cx - 1, cy + 2, cx + 3, cy + 8 + right_dy), fill=s_c)
            elif d == "right":
                draw.rectangle((cx - 2, cy + 2, cx + 4, cy + 8 + right_dy), fill=p_c)
                if abs(left_dy) > 1:
                    draw.rectangle((cx - 3, cy + 2, cx + 1, cy + 8 + left_dy), fill=s_c)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_pants_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_pants_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "pants", "pants", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 4. SHOES & BOOTS COLLECTION (5 Designs)
    # =========================================================================
    shoes_configs = [
        ("sho_leather_boots_001", "Leather Boots", (95, 55, 30, 255), (45, 25, 12, 255), "Mid-calf brown leather explorer boots with folded cuffs."),
        ("sho_iron_boots_001", "Iron Boots", (145, 150, 160, 255), (85, 90, 100, 255), "Heavy forged iron sabatons offering substantial foot protection."),
        ("sho_traveler_shoes_001", "Traveler Shoes", (140, 105, 75, 255), (70, 50, 35, 255), "Supple low-profile walking shoes tailored for swift travel."),
        ("sho_snow_boots_001", "Snow Boots", (70, 95, 120, 255), (230, 235, 245, 255), "Thick snow boots lined with fleece for arctic conditions."),
        ("sho_desert_sandals_001", "Desert Sandals", (165, 130, 90, 255), (115, 85, 55, 255), "Sturdy leather desert sandals with cross-straps and toe loops.")
    ]

    for item_id, name, b_col, sole_col, desc in shoes_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_shoes_frame(bx, by, d, anim_type, col, bc=b_col, sc=sole_col):
            bob_y, left_dy, right_dy = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 42 + bob_y

            if d in ["down", "up"]:
                # Left foot
                draw.rectangle((cx - 5, cy + left_dy, cx - 2, cy + 4 + left_dy), fill=bc)
                draw.line([(cx - 5, cy + 4 + left_dy), (cx - 2, cy + 4 + left_dy)], fill=sc)
                # Right foot
                draw.rectangle((cx + 1, cy + right_dy, cx + 4, cy + 4 + right_dy), fill=bc)
                draw.line([(cx + 1, cy + 4 + right_dy), (cx + 4, cy + 4 + right_dy)], fill=sc)
            elif d == "left":
                draw.rectangle((cx - 5, cy + left_dy, cx + 1, cy + 4 + left_dy), fill=bc)
                draw.line([(cx - 5, cy + 4 + left_dy), (cx + 1, cy + 4 + left_dy)], fill=sc)
            elif d == "right":
                draw.rectangle((cx - 1, cy + right_dy, cx + 5, cy + 4 + right_dy), fill=bc)
                draw.line([(cx - 1, cy + 4 + right_dy), (cx + 5, cy + 4 + right_dy)], fill=sc)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_shoes_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_shoes_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "footwear", "shoes", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 5. HEADWEAR COLLECTION (6 Designs)
    # =========================================================================
    headwear_configs = [
        ("hat_adventurer_hat_001", "Adventurer Hat", (115, 75, 45, 255), (60, 38, 22, 255), [], "Wide-brimmed felt hat with a dark band and traveling feather."),
        ("hat_explorer_hood_001", "Explorer Hood", (75, 95, 65, 255), (45, 60, 40, 255), [], "Woodland cowl hood draping smoothly over the shoulders."),
        ("hat_iron_helmet_001", "Iron Helmet", (155, 160, 170, 255), (100, 105, 115, 255), ["hair_hide_all"], "Forged iron kettle helmet with nose guard protecting the skull."),
        ("hat_steel_helmet_001", "Steel Helmet", (210, 218, 230, 255), (140, 150, 165, 255), ["hair_hide_all"], "Full-face polished knight's bascinet with eye slits."),
        ("hat_mystic_hat_001", "Mystic Hat", (85, 45, 135, 255), (225, 190, 65, 255), [], "Pointed indigo wizard hat adorned with celestial star embroidery."),
        ("hat_forest_ranger_hood_001", "Forest Ranger Hood", (45, 105, 55, 255), (160, 130, 65, 255), [], "Hunter's green hood fitted with a golden oak-leaf clasp.")
    ]

    for item_id, name, h_col, trim_col, occ_flags, desc in headwear_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_headwear_frame(bx, by, d, anim_type, col, hc=h_col, tc=trim_col):
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 12 + bob_y

            if "helmet" in item_id:
                # Full Helmet covering head
                draw.rectangle((cx - 7, cy - 8, cx + 7, cy + 4), fill=hc)
                draw.line([(cx - 7, cy - 8), (cx + 7, cy - 8)], fill=tc)
                if d == "down":
                    # Visor / Eye slot
                    draw.rectangle((cx - 5, cy - 1, cx + 5, cy + 1), fill=(30, 32, 40, 255))
                    draw.line([(cx, cy - 7), (cx, cy + 3)], fill=tc)
                elif d == "up":
                    draw.line([(cx, cy - 7), (cx, cy + 4)], fill=tc)
                elif d == "left":
                    draw.rectangle((cx - 7, cy - 1, cx - 1, cy + 1), fill=(30, 32, 40, 255))
                elif d == "right":
                    draw.rectangle((cx + 1, cy - 1, cx + 7, cy + 1), fill=(30, 32, 40, 255))
            elif "hat_mystic" in item_id:
                # Pointed cone + brim
                draw.ellipse((cx - 9, cy + 1, cx + 9, cy + 5), fill=hc, outline=tc)
                draw.polygon([(cx - 6, cy + 2), (cx + 6, cy + 2), (cx + 2, cy - 11)], fill=hc)
                draw.line([(cx - 6, cy + 2), (cx + 6, cy + 2)], fill=tc)
            elif "hood" in item_id:
                # Hood framing head
                draw.ellipse((cx - 8, cy - 7, cx + 8, cy + 6), fill=hc)
                if d == "down":
                    # Face cutout
                    draw.ellipse((cx - 5, cy - 3, cx + 5, cy + 4), fill=(0, 0, 0, 0))
                elif d in ["left", "right"]:
                    cut_x = cx - 5 if d == "left" else cx + 1
                    draw.rectangle((cut_x, cy - 2, cut_x + 4, cy + 3), fill=(0, 0, 0, 0))
            else:
                # Adventurer Hat (Brim + Crown)
                draw.ellipse((cx - 9, cy, cx + 9, cy + 4), fill=hc)
                draw.rectangle((cx - 6, cy - 6, cx + 6, cy + 1), fill=hc)
                draw.line([(cx - 6, cy), (cx + 6, cy)], fill=tc)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_headwear_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_headwear_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "headwear", "headwear", occlusion_flags=occ_flags, description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 6. FACE ACCESSORIES (5 Designs)
    # =========================================================================
    face_configs = [
        ("fac_cloth_mask_001", "Cloth Mask", "mask", (40, 42, 50, 255), [], "Black assassin cloth bandana concealing mouth and nose."),
        ("fac_leather_mask_001", "Leather Mask", "mask", (110, 65, 35, 255), [], "Hardened leather combat respirator mask with brass rivets."),
        ("fac_mystic_mask_001", "Mystic Mask", "mask", (245, 245, 250, 255), ["mask_hide_goggles"], "Ornate porcelain mask painted with arcane sigils."),
        ("fac_adventurer_goggles_001", "Adventurer Goggles", "goggles", (185, 140, 60, 255), [], "Brass explorer goggles with amber-tinted lenses."),
        ("fac_mechanical_goggles_001", "Mechanical Goggles", "goggles", (145, 150, 160, 255), [], "Complex steampunk multi-lens goggles with optical gears.")
    ]

    for item_id, name, subcat, m_col, occ_flags, desc in face_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_face_frame(bx, by, d, anim_type, col, mc=m_col, is_goggles=(subcat=="goggles")):
            if d == "up":
                return # Face items invisible from behind
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 14 + bob_y

            if is_goggles:
                # Goggles across eyes (cy-2 to cy+1)
                if d == "down":
                    draw.rectangle((cx - 6, cy - 2, cx - 1, cy + 1), fill=mc)
                    draw.rectangle((cx + 1, cy - 2, cx + 6, cy + 1), fill=mc)
                    draw.line([(cx - 1, cy - 1), (cx + 1, cy - 1)], fill=(50, 50, 50, 255))
                    # Lens tint
                    draw.point((cx - 3, cy), fill=(90, 200, 255, 255))
                    draw.point((cx + 4, cy), fill=(90, 200, 255, 255))
                elif d == "left":
                    draw.rectangle((cx - 6, cy - 2, cx - 1, cy + 1), fill=mc)
                    draw.point((cx - 3, cy), fill=(90, 200, 255, 255))
                elif d == "right":
                    draw.rectangle((cx + 1, cy - 2, cx + 6, cy + 1), fill=mc)
                    draw.point((cx + 4, cy), fill=(90, 200, 255, 255))
            else:
                # Mask across lower face (cy+1 to cy+6)
                if d == "down":
                    draw.rectangle((cx - 5, cy + 1, cx + 5, cy + 6), fill=mc)
                    draw.line([(cx - 5, cy + 6), (cx + 5, cy + 6)], fill=(20, 20, 25, 255))
                elif d == "left":
                    draw.rectangle((cx - 5, cy + 1, cx + 1, cy + 6), fill=mc)
                elif d == "right":
                    draw.rectangle((cx - 1, cy + 1, cx + 5, cy + 6), fill=mc)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_face_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_face_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "face_accessories", subcat, occlusion_flags=occ_flags, description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 7. BACKPACK COLLECTION (5 Designs)
    # =========================================================================
    backpack_configs = [
        ("bag_small_traveler_001", "Small Traveler Backpack", (140, 105, 75, 255), (90, 65, 45, 255), "Compact canvas daypack with single brass-buckled flap."),
        ("bag_leather_backpack_001", "Leather Backpack", (105, 60, 35, 255), (65, 35, 20, 255), "Sturdy saddle-leather expedition backpack with utility pouches."),
        ("bag_adventurer_backpack_001", "Adventurer Backpack", (135, 80, 45, 255), (210, 180, 130, 255), "Traveler pack with rolled woolen bedroll strapped on top."),
        ("bag_explorer_backpack_001", "Explorer Backpack", (80, 100, 75, 255), (190, 160, 100, 255), "Large field pack fitted with map tube and brass compass pouch."),
        ("bag_military_backpack_001", "Military-Style Backpack", (65, 75, 55, 255), (45, 50, 38, 255), "Heavy-duty reinforced tactical ruck with modular gear straps.")
    ]

    for item_id, name, b_col, trim_col, desc in backpack_configs:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_backpack_frame(bx, by, d, anim_type, col, bc=b_col, tc=trim_col):
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 26 + bob_y

            if d == "up":
                # Full backpack seen from behind (cx-7 to cx+7, cy-6 to cy+6)
                draw.rectangle((cx - 7, cy - 6, cx + 7, cy + 6), fill=bc)
                draw.rectangle((cx - 6, cy - 5, cx + 6, cy - 2), fill=tc) # Top flap
                draw.rectangle((cx - 6, cy, cx - 2, cy + 5), fill=tc)     # Left pouch
                draw.rectangle((cx + 2, cy, cx + 6, cy + 5), fill=tc)     # Right pouch
                draw.line([(cx - 7, cy + 6), (cx + 7, cy + 6)], fill=(25, 25, 25, 255))
            elif d == "down":
                # Shoulder straps peaking over front of chest
                draw.line([(cx - 5, cy - 6), (cx - 5, cy + 2)], fill=tc, width=1)
                draw.line([(cx + 5, cy - 6), (cx + 5, cy + 2)], fill=tc, width=1)
            elif d == "left":
                # Side view: backpack sticks out to the right (behind back)
                draw.rectangle((cx + 4, cy - 5, cx + 8, cy + 5), fill=bc)
                draw.rectangle((cx + 4, cy - 5, cx + 8, cy - 2), fill=tc)
                draw.line([(cx - 4, cy - 5), (cx - 4, cy + 2)], fill=tc) # Front strap
            elif d == "right":
                # Side view: backpack sticks out to the left (behind back)
                draw.rectangle((cx - 8, cy - 5, cx - 4, cy + 5), fill=bc)
                draw.rectangle((cx - 8, cy - 5, cx - 4, cy - 2), fill=tc)
                draw.line([(cx + 4, cy - 5), (cx + 4, cy + 2)], fill=tc) # Front strap

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_backpack_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_backpack_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "backpacks", "backpack", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # 8. CHARACTER VARIATIONS (7 Designs)
    # =========================================================================
    # Skins: Pale, Dark, Olive
    skin_variants = [
        ("var_skin_pale_001", "Pale Skin Appearance", (255, 230, 215, 255), (230, 195, 180, 255), "Fair alabaster skin tone option for player customization."),
        ("var_skin_dark_001", "Umber Skin Appearance", (105, 65, 45, 255), (75, 45, 30, 255), "Deep umber rich skin tone option for player customization."),
        ("var_skin_olive_001", "Olive Skin Appearance", (195, 160, 125, 255), (155, 120, 90, 255), "Warm sun-kissed olive complexion for player customization.")
    ]

    for item_id, name, skin_c, shadow_c, desc in skin_variants:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_skin_frame(bx, by, d, anim_type, col, sc=skin_c, shc=shadow_c):
            bob_y, left_dy, right_dy = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy_head = by + 14 + bob_y
            cy_torso = by + 26 + bob_y
            cy_legs = by + 34 + bob_y

            # Head
            draw.ellipse((cx - 6, cy_head - 6, cx + 6, cy_head + 6), fill=sc)
            if d == "down":
                # Eyes
                draw.point((cx - 2, cy_head), fill=(40, 40, 50, 255))
                draw.point((cx + 2, cy_head), fill=(40, 40, 50, 255))
            elif d == "left":
                draw.point((cx - 3, cy_head), fill=(40, 40, 50, 255))
            elif d == "right":
                draw.point((cx + 3, cy_head), fill=(40, 40, 50, 255))

            # Torso base
            draw.rectangle((cx - 5, cy_torso - 5, cx + 5, cy_torso + 5), fill=sc)

            # Arms
            if d in ["down", "up"]:
                draw.rectangle((cx - 8, cy_torso - 4, cx - 6, cy_torso + 4), fill=sc)
                draw.rectangle((cx + 6, cy_torso - 4, cx + 8, cy_torso + 4), fill=sc)
                # Legs base
                draw.rectangle((cx - 4, cy_legs, cx - 2, cy_legs + 8 + left_dy), fill=sc)
                draw.rectangle((cx + 2, cy_legs, cx + 4, cy_legs + 8 + right_dy), fill=sc)
            elif d == "left":
                draw.rectangle((cx - 6, cy_torso - 4, cx - 4, cy_torso + 4), fill=sc)
                draw.rectangle((cx - 3, cy_legs, cx + 1, cy_legs + 8 + left_dy), fill=sc)
            elif d == "right":
                draw.rectangle((cx + 4, cy_torso - 4, cx + 6, cy_torso + 4), fill=sc)
                draw.rectangle((cx - 1, cy_legs, cx + 3, cy_legs + 8 + right_dy), fill=sc)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_skin_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_skin_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "character_variations", "skin", description=desc)
        catalog_items.append(meta)

    # Hair Variations: Braided Red, Spiky Black
    hair_variants = [
        ("var_hair_braided_red_001", "Braided Crimson Hair", (175, 45, 30, 255), (215, 80, 55, 255), "Long warrior braids woven in vibrant flame-red hair."),
        ("var_hair_spiky_black_001", "Spiky Obsidian Hair", (28, 30, 36, 255), (60, 65, 78, 255), "Dynamic stylized spiky adventurer haircut in deep obsidian.")
    ]

    for item_id, name, h_col, hi_col, desc in hair_variants:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_hair_frame(bx, by, d, anim_type, col, hc=h_col, hic=hi_col, is_braided=("braided" in item_id)):
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 14 + bob_y

            if is_braided:
                if d == "down":
                    draw.rectangle((cx - 6, cy - 7, cx + 6, cy - 2), fill=hc)
                    draw.rectangle((cx - 8, cy - 3, cx - 5, cy + 9), fill=hc) # Left braid
                    draw.rectangle((cx + 5, cy - 3, cx + 8, cy + 9), fill=hc) # Right braid
                    draw.line([(cx - 4, cy - 6), (cx + 4, cy - 6)], fill=hic)
                elif d == "up":
                    draw.ellipse((cx - 6, cy - 7, cx + 6, cy + 8), fill=hc)
                    draw.rectangle((cx - 3, cy + 4, cx + 3, cy + 12), fill=hc) # Rear central braid
                elif d == "left":
                    draw.rectangle((cx - 5, cy - 7, cx + 6, cy - 2), fill=hc)
                    draw.rectangle((cx + 2, cy - 3, cx + 6, cy + 10), fill=hc)
                elif d == "right":
                    draw.rectangle((cx - 6, cy - 7, cx + 5, cy - 2), fill=hc)
                    draw.rectangle((cx - 6, cy - 3, cx - 2, cy + 10), fill=hc)
            else:
                # Spiky Black Hair
                if d == "down":
                    draw.polygon([(cx - 7, cy - 3), (cx - 6, cy - 8), (cx - 3, cy - 5),
                                  (cx, cy - 9), (cx + 3, cy - 5), (cx + 6, cy - 8),
                                  (cx + 7, cy - 3), (cx + 6, cy + 1), (cx - 6, cy + 1)], fill=hc)
                    draw.line([(cx - 3, cy - 5), (cx, cy - 7)], fill=hic)
                elif d == "up":
                    draw.ellipse((cx - 7, cy - 8, cx + 7, cy + 4), fill=hc)
                    draw.polygon([(cx - 5, cy - 8), (cx, cy - 11), (cx + 5, cy - 8)], fill=hc)
                elif d == "left":
                    draw.polygon([(cx - 6, cy + 1), (cx - 7, cy - 6), (cx - 2, cy - 9),
                                  (cx + 5, cy - 7), (cx + 6, cy + 2)], fill=hc)
                elif d == "right":
                    draw.polygon([(cx + 6, cy + 1), (cx + 7, cy - 6), (cx + 2, cy - 9),
                                  (cx - 5, cy - 7), (cx - 6, cy + 2)], fill=hc)

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_hair_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_hair_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "character_variations", "hair", description=desc)
        catalog_items.append(meta)

    # Face Details: Warpaint, Eyepatch
    detail_variants = [
        ("var_face_warpaint_001", "Azure Hunter Warpaint", (40, 180, 240, 255), "Tribal azure warpaint stripes painted across cheeks and nose."),
        ("var_face_eyepatch_001", "Veteran Leather Eyepatch", (30, 32, 40, 255), "Weathered black leather eyepatch covering the right eye.")
    ]

    for item_id, name, d_col, desc in detail_variants:
        img = create_transparent_sheet()
        draw = ImageDraw.Draw(img)

        def draw_detail_frame(bx, by, d, anim_type, col, dc=d_col, is_warpaint=("warpaint" in item_id)):
            if d == "up":
                return
            bob_y, _, _ = get_frame_offsets(anim_type, col, d)
            cx = bx + 16
            cy = by + 14 + bob_y

            if is_warpaint:
                if d == "down":
                    draw.line([(cx - 5, cy + 1), (cx - 2, cy + 2)], fill=dc, width=1)
                    draw.line([(cx + 2, cy + 2), (cx + 5, cy + 1)], fill=dc, width=1)
                elif d == "left":
                    draw.line([(cx - 4, cy + 1), (cx - 1, cy + 2)], fill=dc, width=1)
                elif d == "right":
                    draw.line([(cx + 1, cy + 2), (cx + 4, cy + 1)], fill=dc, width=1)
            else:
                # Eyepatch over right eye
                if d == "down":
                    draw.rectangle((cx + 1, cy - 1, cx + 4, cy + 2), fill=dc)
                    draw.line([(cx - 4, cy - 3), (cx + 5, cy + 2)], fill=(20, 20, 20, 255))
                elif d == "right":
                    draw.rectangle((cx + 1, cy - 1, cx + 4, cy + 2), fill=dc)
                    draw.line([(cx - 2, cy - 3), (cx + 5, cy + 2)], fill=(20, 20, 20, 255))

        for r, d in enumerate(DIRECTIONS):
            for c in range(COLS): draw_detail_frame(c * FRAME_W, r * FRAME_H, d, "walk", c)
            for c in range(COLS): draw_detail_frame(c * FRAME_W, (4 + r) * FRAME_H, d, "idle", c)

        meta = save_phase17_cosmetic(img, item_id, name, "character_variations", "face", description=desc)
        catalog_items.append(meta)

    # =========================================================================
    # Master Catalog
    # =========================================================================
    os.makedirs("data/cosmetics", exist_ok=True)
    catalog_path = "data/cosmetics/costume_catalog.json"
    catalog_data = {
        "version": "1.0.0",
        "total_cosmetics": len(catalog_items),
        "categories": {
            "clothing": 8,
            "armor": 8,
            "pants": 6,
            "footwear": 5,
            "headwear": 6,
            "face_accessories": 5,
            "backpacks": 5,
            "character_variations": 7
        },
        "cosmetics": catalog_items
    }
    with open(catalog_path, "w", encoding="utf-8") as f:
        json.dump(catalog_data, f, indent=4)

    print(f"\nSuccessfully generated {len(catalog_items)} Phase 17 cosmetic assets!")
    print(f"Master catalog written to: {catalog_path}")

    # =========================================================================
    # Update Asset Manifest (asset_manifest.json)
    # =========================================================================
    manifest_path = "assets/manifests/asset_manifest.json"
    if os.path.exists(manifest_path):
        with open(manifest_path, "r", encoding="utf-8") as f:
            manifest = json.load(f)

        # Remove temporary erroneous keys if present
        manifest.pop("textures", None)
        manifest.pop("spritesheets", None)

        existing_asset_ids = {a["id"] for a in manifest.get("assets", [])}

        new_assets = []
        for item in catalog_items:
            item_id = item["id"]
            cat = item["category"]
            tex_id = item["texture"]
            sheet_id = item["spritesheet"]
            sprite_file = item["sprite"]
            sheet_file = sprite_file.replace("sprite.png", "spritesheet.json")

            # Texture entry
            if tex_id not in existing_asset_ids:
                new_assets.append({
                    "id": tex_id,
                    "file": sprite_file,
                    "type": "texture",
                    "category": f"cosmetics/{cat}",
                    "source": "Wildbound Development Studio",
                    "author": "Wildbound Art Team",
                    "license": "CC0 1.0 Universal",
                    "commercial_use": True,
                    "attribution_required": False,
                    "checksum": f"c_{item_id}_tex",
                    "width": SHEET_W,
                    "height": SHEET_H,
                    "status": "approved"
                })
                existing_asset_ids.add(tex_id)

            # Spritesheet entry
            if sheet_id not in existing_asset_ids:
                new_assets.append({
                    "id": sheet_id,
                    "file": sheet_file,
                    "type": "spritesheet",
                    "category": f"cosmetics/{cat}",
                    "source": "Wildbound Development Studio",
                    "author": "Wildbound Art Team",
                    "license": "CC0 1.0 Universal",
                    "commercial_use": True,
                    "attribution_required": False,
                    "checksum": f"c_{item_id}_sheet",
                    "width": FRAME_W,
                    "height": FRAME_H,
                    "status": "approved"
                })
                existing_asset_ids.add(sheet_id)

        manifest["assets"].extend(new_assets)

        with open(manifest_path, "w", encoding="utf-8") as f:
            json.dump(manifest, f, indent=2)

        total_assets = len(manifest.get("assets", []))
        print(f"Updated {manifest_path}: added {len(new_assets)} assets.")
        print(f"Total registered AMS assets now: {total_assets}")

if __name__ == "__main__":
    build_all()
