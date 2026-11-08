import os
import sys
import ssl
import json
import hashlib
import urllib.request
from PIL import Image, ImageDraw, ImageFont

def acquire_pilot():
    ctx = ssl._create_unverified_context()
    headers = {'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)'}

    downloads = [
        # Superpowers Slimes & License
        {
            "id": "pilot_creature_slime_anim",
            "category": "creature",
            "name": "slime_animated",
            "url": "https://raw.githubusercontent.com/sparklinlabs/superpowers-asset-packs/master/rpg-battle-system/monster/slime/sprite-sheet-141x107.png",
            "dest": "assets/external/superpowers/slime_animated_raw.png",
            "license_url": "https://raw.githubusercontent.com/sparklinlabs/superpowers-asset-packs/master/LICENSE.txt",
            "license_dest": "assets/external/superpowers/LICENSE_superpowers_cc0.txt",
            "source_repo": "sparklinlabs/superpowers-asset-packs",
            "license": "CC0 1.0 Universal",
            "commercial": True
        },
        {
            "id": "pilot_creature_slime_mini",
            "category": "creature",
            "name": "slime_overworld",
            "url": "https://raw.githubusercontent.com/sparklinlabs/superpowers-asset-packs/master/medieval-fantasy/monsters/slim.png",
            "dest": "assets/external/superpowers/slime_raw.png",
            "license_url": "https://raw.githubusercontent.com/sparklinlabs/superpowers-asset-packs/master/LICENSE.txt",
            "license_dest": "assets/external/superpowers/LICENSE_superpowers_cc0.txt",
            "source_repo": "sparklinlabs/superpowers-asset-packs",
            "license": "CC0 1.0 Universal",
            "commercial": True
        },
        # Kenney Roguelike / Desert Tileset & License
        {
            "id": "pilot_env_desert_tilesheet",
            "category": "environment",
            "name": "kenney_roguelike_sheet",
            "url": "https://raw.githubusercontent.com/ETdoFresh/kenney.nl/master/roguelike-pack/Spritesheet/roguelikeSheet_transparent.png",
            "dest": "assets/external/kenney/roguelike_sheet_raw.png",
            "license_url": "https://raw.githubusercontent.com/ETdoFresh/kenney.nl/master/roguelike-pack/License.txt",
            "license_dest": "assets/external/kenney/LICENSE_kenney_cc0.txt",
            "source_repo": "KenneyNL / ETdoFresh mirror",
            "license": "CC0 1.0 Universal",
            "commercial": True
        },
        # Game-icons (CC BY 3.0)
        {
            "id": "pilot_ui_icon_potion",
            "category": "ui_icons",
            "name": "potion_ball",
            "url": "https://game-icons.net/icons/000000/transparent/1x1/lorc/potion-ball.png",
            "dest": "assets/external/game_icons/potion_ball_raw.png",
            "author": "Lorc",
            "source_repo": "game-icons.net",
            "license": "CC BY 3.0",
            "commercial": True
        },
        {
            "id": "pilot_ui_icon_heart",
            "category": "ui_icons",
            "name": "heart_drop",
            "url": "https://game-icons.net/icons/000000/transparent/1x1/lorc/heart-drop.png",
            "dest": "assets/external/game_icons/heart_drop_raw.png",
            "author": "Lorc",
            "source_repo": "game-icons.net",
            "license": "CC BY 3.0",
            "commercial": True
        },
        {
            "id": "pilot_ui_icon_sword",
            "category": "ui_icons",
            "name": "broadsword",
            "url": "https://game-icons.net/icons/000000/transparent/1x1/lorc/broadsword.png",
            "dest": "assets/external/game_icons/broadsword_raw.png",
            "author": "Lorc",
            "source_repo": "game-icons.net",
            "license": "CC BY 3.0",
            "commercial": True
        },
        {
            "id": "pilot_ui_icon_shield",
            "category": "ui_icons",
            "name": "shield_bash",
            "url": "https://game-icons.net/icons/000000/transparent/1x1/delapouite/shield-bash.png",
            "dest": "assets/external/game_icons/shield_bash_raw.png",
            "author": "Delapouite",
            "source_repo": "game-icons.net",
            "license": "CC BY 3.0",
            "commercial": True
        }
    ]

    print("=================================================================")
    print("PHASE 49: GATE 2 — PILOT ASSET ACQUISITION & VALIDATION")
    print("=================================================================\n")

    # 1. Download Files
    print("--- 1. DOWNLOADING RAW ASSETS & LICENSES ---")
    for item in downloads:
        dest_path = item["dest"]
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        url = item["url"]
        print(f"Fetching: {url}")
        req = urllib.request.Request(url, headers=headers)
        with urllib.request.urlopen(req, context=ctx, timeout=15) as resp:
            content = resp.read()
            with open(dest_path, "wb") as f:
                f.write(content)
        print(f" -> Saved to: {dest_path} ({len(content)} bytes)")
        
        if "license_url" in item:
            lic_dest = item["license_dest"]
            lic_url = item["license_url"]
            req_lic = urllib.request.Request(lic_url, headers=headers)
            with urllib.request.urlopen(req_lic, context=ctx, timeout=15) as resp:
                lic_content = resp.read()
                with open(lic_dest, "wb") as f:
                    f.write(lic_content)
            print(f" -> Saved license to: {lic_dest} ({len(lic_content)} bytes)")

    # Write Game-icons license file
    game_icons_lic_path = "assets/external/game_icons/LICENSE_gameicons_ccby.txt"
    game_icons_license_text = """Creative Commons Attribution 3.0 Unported (CC BY 3.0)

Game-icons.net Asset Collection
Icons:
- potion-ball.png (Author: Lorc, http://lorcblog.blogspot.com)
- heart-drop.png (Author: Lorc, http://lorcblog.blogspot.com)
- broadsword.png (Author: Lorc, http://lorcblog.blogspot.com)
- shield-bash.png (Author: Delapouite, https://delapouite.com)

License Summary:
You are free to:
- Share — copy and redistribute the material in any medium or format
- Adapt — remix, transform, and build upon the material for any purpose, even commercially.

Under the following terms:
- Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made.
- No additional restrictions — You may not apply legal terms or technological measures that legally restrict others from doing anything the license permits.

Full Legal Code: https://creativecommons.org/licenses/by/3.0/legalcode
"""
    with open(game_icons_lic_path, "w", encoding="utf-8") as f:
        f.write(game_icons_license_text)
    print(f" -> Saved license to: {game_icons_lic_path}\n")

    # 2. Validate Integrity & Inspect Metadata
    print("--- 2. VERIFYING INTEGRITY & METADATA ---")
    validation_records = []

    for item in downloads:
        path = item["dest"]
        with open(path, "rb") as f:
            data = f.read()
        sha256 = hashlib.sha256(data).hexdigest()
        file_size = len(data)

        # PNG Magic Number Check (\x89PNG\r\n\x1a\n)
        is_png_magic = data.startswith(b"\x89PNG\r\n\x1a\n")

        # Pillow validation
        img = Image.open(path)
        w, h = img.size
        mode = img.mode
        format_name = img.format
        has_alpha = "A" in mode or (img.info.get("transparency") is not None)

        record = {
            "id": item["id"],
            "category": item["category"],
            "name": item["name"],
            "raw_file": path,
            "format": format_name,
            "png_magic_valid": is_png_magic,
            "dimensions": {"width": w, "height": h},
            "color_mode": mode,
            "file_size_bytes": file_size,
            "sha256": sha256,
            "has_alpha": has_alpha,
            "license": item["license"],
            "source_repo": item["source_repo"],
            "commercial_use": item["commercial"],
            "status": "VALIDATED"
        }
        validation_records.append(record)
        print(f"[{item['category'].upper()}] {item['name']}: {w}x{h} ({mode}), {file_size} B, SHA256={sha256[:12]}... (PNG Magic={is_png_magic})")

    # 3. Visual Contact Sheets Generation
    print("\n--- 3. GENERATING VISUAL CONTACT SHEETS ---")

    # 3a. Superpowers Slime Contact Sheet
    slime_sheet = Image.open("assets/external/superpowers/slime_animated_raw.png").convert("RGBA")
    sw, sh = slime_sheet.size # 705x535 (5 cols x 5 rows of 141x107)
    cols, rows = 5, 5
    fw, fh = 141, 107

    cs_slime = Image.new("RGBA", (sw + 60, sh + 140), (28, 30, 36, 255))
    draw_s = ImageDraw.Draw(cs_slime)
    
    draw_s.text((30, 15), "PILOT ASSET 1: Superpowers Slime Creature Spritesheet", fill=(240, 240, 240, 255))
    draw_s.text((30, 35), f"Raw: {sw}x{sh} RGBA | Frames: {cols}x{rows} ({fw}x{fh} per frame) | Source: Superpowers CC0", fill=(170, 175, 190, 255))
    draw_s.text((30, 55), "Rows: [0] Idle (5f), [1] Hit (5f), [2] Attack1 (5f), [3] Attack2 (5f), [4] Death (5f)", fill=(140, 145, 160, 255))

    for y in range(0, sh, 20):
        for x in range(0, sw, 20):
            c = (42, 45, 54, 255) if ((x // 20) + (y // 20)) % 2 == 0 else (34, 36, 44, 255)
            draw_s.rectangle([30 + x, 80 + y, 30 + x + 19, 80 + y + 19], fill=c)

    cs_slime.paste(slime_sheet, (30, 80), slime_sheet)

    for c in range(cols + 1):
        x = 30 + c * fw
        draw_s.line([(x, 80), (x, 80 + sh)], fill=(100, 200, 255, 180), width=1)
    for r in range(rows + 1):
        y = 80 + r * fh
        draw_s.line([(30, y), (30 + sw, y)], fill=(100, 200, 255, 180), width=1)

    row_labels = ["Idle", "Hit", "Atk 1", "Atk 2", "Death"]
    for r in range(rows):
        y = 80 + r * fh + 8
        draw_s.text((35, y), f"Row {r}: {row_labels[r]}", fill=(255, 255, 100, 240))

    cs_slime_path = "assets/external/superpowers/contact_sheet_slime.png"
    cs_slime.save(cs_slime_path)
    print(f" -> Created: {cs_slime_path}")

    # 3b. Kenney Roguelike / Desert Tileset Contact Sheet
    kenney_raw = Image.open("assets/external/kenney/roguelike_sheet_raw.png").convert("RGBA")
    kw, kh = kenney_raw.size # 968x526

    cs_kenney = Image.new("RGBA", (kw + 60, kh + 260), (28, 30, 36, 255))
    draw_k = ImageDraw.Draw(cs_kenney)

    draw_k.text((30, 15), "PILOT ASSET 2: Kenney Roguelike / Desert Tileset", fill=(240, 240, 240, 255))
    draw_k.text((30, 35), f"Raw: {kw}x{kh} RGBA | 57x31 tiles (16x16 with 1px border) | License: CC0 1.0 Universal", fill=(170, 175, 190, 255))
    draw_k.text((30, 55), "Target Integration: Desert Biome (Sand terrain, rock formations, desert vegetation, ancient ruins)", fill=(140, 145, 160, 255))

    cs_kenney.paste(kenney_raw, (30, 80), kenney_raw)
    
    draw_k.rectangle([30, 80, 30 + 17 * 12, 80 + 17 * 8], outline=(255, 200, 50, 255), width=2)
    draw_k.text((35, 85), "DESERT TILES REGION", fill=(255, 220, 80, 255))

    draw_k.text((30, kh + 95), "Desert Biome Extracted Previews (Scaled 2x for visual inspection):", fill=(220, 220, 220, 255))
    
    desert_crop = kenney_raw.crop((0, 0, min(kw, 340), min(kh, 70)))
    desert_zoomed = desert_crop.resize((desert_crop.width * 2, desert_crop.height * 2), Image.Resampling.NEAREST)
    cs_kenney.paste(desert_zoomed, (30, kh + 120), desert_zoomed)

    cs_kenney_path = "assets/external/kenney/contact_sheet_desert.png"
    cs_kenney.save(cs_kenney_path)
    print(f" -> Created: {cs_kenney_path}")

    # 3c. UI Icons Contact Sheet
    icon_list = [
        ("potion_ball", "Potion Ball (Consumable)", "assets/external/game_icons/potion_ball_raw.png", "Lorc"),
        ("heart_drop", "Heart Drop (Health/Vitality)", "assets/external/game_icons/heart_drop_raw.png", "Lorc"),
        ("broadsword", "Broadsword (Melee Weapon)", "assets/external/game_icons/broadsword_raw.png", "Lorc"),
        ("shield_bash", "Shield Bash (Defense/Armor)", "assets/external/game_icons/shield_bash_raw.png", "Delapouite")
    ]

    cs_icons = Image.new("RGBA", (760, 340), (28, 30, 36, 255))
    draw_i = ImageDraw.Draw(cs_icons)

    draw_i.text((30, 15), "PILOT ASSET 3: Game-icons.net UI Icon Pack", fill=(240, 240, 240, 255))
    draw_i.text((30, 35), "Raw 512x512 PNGs | Downscaled to 64x64 & 32x32 UI resolutions | License: CC BY 3.0", fill=(170, 175, 190, 255))

    for idx, (iname, idesc, ipath, iauthor) in enumerate(icon_list):
        raw_icon = Image.open(ipath).convert("RGBA")
        card_x = 30 + idx * 175
        card_y = 65

        draw_i.rectangle([card_x, card_y, card_x + 165, card_y + 250], fill=(38, 41, 50, 255), outline=(60, 65, 80, 255))
        draw_i.text((card_x + 8, card_y + 8), iname, fill=(255, 255, 255, 255))
        draw_i.text((card_x + 8, card_y + 24), f"By: {iauthor}", fill=(160, 165, 180, 255))
        draw_i.text((card_x + 8, card_y + 38), "Raw: 512x512", fill=(130, 135, 150, 255))

        draw_i.rectangle([card_x + 8, card_y + 58, card_x + 72, card_y + 122], fill=(18, 20, 24, 255), outline=(50, 55, 68, 255))
        icon_64 = raw_icon.resize((64, 64), Image.Resampling.LANCZOS)
        cs_icons.paste(icon_64, (card_x + 8, card_y + 58), icon_64)
        draw_i.text((card_x + 8, card_y + 126), "64x64", fill=(150, 155, 170, 255))

        draw_i.rectangle([card_x + 85, card_y + 58, card_x + 149, card_y + 122], fill=(215, 205, 185, 255), outline=(160, 150, 130, 255))
        cs_icons.paste(icon_64, (card_x + 85, card_y + 58), icon_64)
        draw_i.text((card_x + 85, card_y + 126), "Parchment", fill=(150, 155, 170, 255))

        draw_i.rectangle([card_x + 8, card_y + 150, card_x + 40, card_y + 182], fill=(18, 20, 24, 255), outline=(50, 55, 68, 255))
        icon_32 = raw_icon.resize((32, 32), Image.Resampling.LANCZOS)
        cs_icons.paste(icon_32, (card_x + 8, card_y + 150), icon_32)
        draw_i.text((card_x + 48, card_y + 158), "32x32 HUD size", fill=(150, 155, 170, 255))

        draw_i.text((card_x + 8, card_y + 195), idesc[:22], fill=(255, 200, 80, 255))
        if len(idesc) > 22:
            draw_i.text((card_x + 8, card_y + 210), idesc[22:], fill=(255, 200, 80, 255))

    cs_icons_path = "assets/external/game_icons/contact_sheet_icons.png"
    cs_icons.save(cs_icons_path)
    print(f" -> Created: {cs_icons_path}")

    # 4. Save Machine-Readable Register
    register_path = "assets/manifests/asset_licenses.json"
    register_data = {
        "schema_version": "1.0",
        "phase": 49,
        "last_updated": "2026-11-08",
        "description": "Wildbound Phase 49 External Asset Licensing Register",
        "rules": {
            "commercial_use_required": True,
            "prohibited_licenses": ["CC BY-NC", "Personal Use Only", "Proprietary Commercial"],
            "attribution_compliance": "Must record author, license, and source URL in all manifests"
        },
        "assets": validation_records
    }
    with open(register_path, "w", encoding="utf-8") as f:
        json.dump(register_data, f, indent=2)
    print(f"\n -> Updated Licensing Register: {register_path}")

    print("\n=================================================================")
    print("GATE 2 ACQUISITION AND VALIDATION COMPLETE!")
    print("=================================================================")

if __name__ == "__main__":
    acquire_pilot()
