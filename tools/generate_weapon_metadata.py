#!/usr/bin/env python3
"""
Wildbound Phase 15 — Weapon Metadata & AMS Manifest Registration Tool
Generates:
  - Individual weapon JSON definitions in data/weapons/
  - Master weapon catalog in data/weapons/weapon_catalog.json
  - Held & Animation spritesheet metadata JSON files in assets/weapons/
  - Updates assets/manifests/asset_manifest.json with all weapon textures and sheets.
"""

import os
import json

WEAPONS_DATA = [
    {
        "id": "sword_wooden_training_001",
        "display_name": "Wooden Training Sword",
        "category": "sword",
        "subcategory": "training",
        "description": "A blunt training sword fashioned from seasoned birch wood. Ideal for sparring novices without lethal hazard.",
        "icon_file": "weapons/icons/sword_wooden_training_001_icon.png",
        "ground_file": "weapons/ground/sword_wooden_training_001_ground.png",
        "held_file": "weapons/held/sword_wooden_training_001_held.png",
        "anim_file": "weapons/animations/sword_wooden_training_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "sword_rusty_short_001",
        "display_name": "Rusty Short Sword",
        "category": "sword",
        "subcategory": "shortsword",
        "description": "A pitted iron shortsword retrieved from forgotten battlefield mud. Worn and chipped, but still functional.",
        "icon_file": "weapons/icons/sword_rusty_short_001_icon.png",
        "ground_file": "weapons/ground/sword_rusty_short_001_ground.png",
        "held_file": "weapons/held/sword_rusty_short_001_held.png",
        "anim_file": "weapons/animations/sword_rusty_short_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "sword_iron_001",
        "display_name": "Iron Sword",
        "category": "sword",
        "subcategory": "arming_sword",
        "description": "Standard double-edged iron arming sword with a solid crossguard and corded leather hilt wrap.",
        "icon_file": "weapons/icons/sword_iron_001_icon.png",
        "ground_file": "weapons/ground/sword_iron_001_ground.png",
        "held_file": "weapons/held/sword_iron_001_held.png",
        "anim_file": "weapons/animations/sword_iron_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "sword_steel_longsword_001",
        "display_name": "Steel Longsword",
        "category": "sword",
        "subcategory": "longsword",
        "description": "A master-forged steel longsword with a deep fuller, cruciform crossguard, and gilded balance pommel.",
        "icon_file": "weapons/icons/sword_steel_longsword_001_icon.png",
        "ground_file": "weapons/ground/sword_steel_longsword_001_ground.png",
        "held_file": "weapons/held/sword_steel_longsword_001_held.png",
        "anim_file": "weapons/animations/sword_steel_longsword_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "sword_curved_001",
        "display_name": "Curved Sword",
        "category": "sword",
        "subcategory": "saber",
        "description": "An exotic curved scimitar crafted from folded steel with a brass quillon and sculpted horn hilt.",
        "icon_file": "weapons/icons/sword_curved_001_icon.png",
        "ground_file": "weapons/ground/sword_curved_001_ground.png",
        "held_file": "weapons/held/sword_curved_001_held.png",
        "anim_file": "weapons/animations/sword_curved_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "sword_ancient_rune_001",
        "display_name": "Ancient Rune Sword",
        "category": "sword",
        "subcategory": "relic",
        "description": "A mythic relic sword forged of dark star-metal, etched with celestial runes that pulse with cyan arcane energy.",
        "icon_file": "weapons/icons/sword_ancient_rune_001_icon.png",
        "ground_file": "weapons/ground/sword_ancient_rune_001_ground.png",
        "held_file": "weapons/held/sword_ancient_rune_001_held.png",
        "anim_file": "weapons/animations/sword_ancient_rune_001_anim.png",
        "grip_anchor": {"x": 12.0, "y": 18.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "prep", "swing", "recovery"]
    },
    {
        "id": "bow_wooden_hunting_001",
        "display_name": "Wooden Hunting Bow",
        "category": "bow",
        "subcategory": "hunting_bow",
        "description": "A supple self-bow crafted from seasoned ash wood with waxed hemp bowstring, favored by woodsmen.",
        "icon_file": "weapons/icons/bow_wooden_hunting_001_icon.png",
        "ground_file": "weapons/ground/bow_wooden_hunting_001_ground.png",
        "held_file": "weapons/held/bow_wooden_hunting_001_held.png",
        "anim_file": "weapons/animations/bow_wooden_hunting_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "both_hands",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "draw", "hold", "release"]
    },
    {
        "id": "bow_longbow_001",
        "display_name": "Longbow",
        "category": "bow",
        "subcategory": "longbow",
        "description": "A tall, high-draw-weight yew longbow equipped with reinforced horn nocks and fine braided linen string.",
        "icon_file": "weapons/icons/bow_longbow_001_icon.png",
        "ground_file": "weapons/ground/bow_longbow_001_ground.png",
        "held_file": "weapons/held/bow_longbow_001_held.png",
        "anim_file": "weapons/animations/bow_longbow_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "both_hands",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "draw", "hold", "release"]
    },
    {
        "id": "bow_reinforced_001",
        "display_name": "Reinforced Bow",
        "category": "bow",
        "subcategory": "composite_bow",
        "description": "A heavy laminated composite bow reinforced with tempered iron bands and water-treated leather.",
        "icon_file": "weapons/icons/bow_reinforced_001_icon.png",
        "ground_file": "weapons/ground/bow_reinforced_001_ground.png",
        "held_file": "weapons/held/bow_reinforced_001_held.png",
        "anim_file": "weapons/animations/bow_reinforced_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "both_hands",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "draw", "hold", "release"]
    },
    {
        "id": "bow_recurve_001",
        "display_name": "Recurve Bow",
        "category": "bow",
        "subcategory": "recurve_bow",
        "description": "An agile equestrian recurve bow featuring backward-curving horn tips for compact power and fast draw cycles.",
        "icon_file": "weapons/icons/bow_recurve_001_icon.png",
        "ground_file": "weapons/ground/bow_recurve_001_ground.png",
        "held_file": "weapons/held/bow_recurve_001_held.png",
        "anim_file": "weapons/animations/bow_recurve_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "both_hands",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "draw", "hold", "release"]
    },
    {
        "id": "bow_ancient_rune_001",
        "display_name": "Ancient Rune Bow",
        "category": "bow",
        "subcategory": "runic_bow",
        "description": "An elven relic bow shaped from ancient ironwood, pulsating with verdant living runes and an ethereal energy string.",
        "icon_file": "weapons/icons/bow_ancient_rune_001_icon.png",
        "ground_file": "weapons/ground/bow_ancient_rune_001_ground.png",
        "held_file": "weapons/held/bow_ancient_rune_001_held.png",
        "anim_file": "weapons/animations/bow_ancient_rune_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 85, "up": 18, "left": 85, "right": 85},
        "hand": "both_hands",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["idle", "draw", "hold", "release"]
    },
    {
        "id": "arrow_iron_001",
        "display_name": "Iron Broadhead Arrow",
        "category": "ammunition",
        "subcategory": "arrow",
        "description": "Standard ash-shafted hunting arrow tipped with a forged steel broadhead and trimmed goose feather fletchings.",
        "icon_file": "weapons/icons/arrow_iron_001_icon.png",
        "ground_file": "weapons/ground/arrow_iron_001_ground.png",
        "held_file": "weapons/held/arrow_iron_001_held.png",
        "anim_file": "weapons/animations/arrow_iron_001_anim.png",
        "grip_anchor": {"x": 16.0, "y": 16.0},
        "render_order": {"down": 86, "up": 17, "left": 86, "right": 86},
        "hand": "right_hand",
        "directions": ["down", "up", "left", "right"],
        "animation_frames": ["flight_1", "flight_2", "flight_3", "impact"]
    }
]

def main():
    print("Generating Weapon Metadata and Catalog...")
    os.makedirs("data/weapons/swords", exist_ok=True)
    os.makedirs("data/weapons/bows", exist_ok=True)
    os.makedirs("data/weapons/projectiles", exist_ok=True)

    # 1. Output Individual JSON files
    for w in WEAPONS_DATA:
        sub = "swords" if w["category"] == "sword" else ("bows" if w["category"] == "bow" else "projectiles")
        out_path = f"data/weapons/{sub}/{w['id']}.json"
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(w, f, indent=2)
        print(f"  + Wrote {out_path}")

    # 2. Master Catalog
    catalog = {
        "version": "1.0",
        "weapon_count": len(WEAPONS_DATA),
        "categories": ["sword", "bow", "ammunition"],
        "weapons": WEAPONS_DATA
    }
    with open("data/weapons/weapon_catalog.json", "w", encoding="utf-8") as f:
        json.dump(catalog, f, indent=2)
    print("  + Wrote data/weapons/weapon_catalog.json")

    # 3. Create spritesheet JSON metadata files for held and anim strips
    for w in WEAPONS_DATA:
        wid = w["id"]
        # Held spritesheet metadata
        held_meta = {
            "id": f"sheet_{wid}_held",
            "texture": w["held_file"],
            "frame_width": 32,
            "frame_height": 32,
            "columns": 4,
            "rows": 1,
            "frame_count": 4,
            "spacing": 0,
            "margin": 0,
            "origin": "center"
        }
        with open(f"assets/weapons/held/{wid}_held.json", "w", encoding="utf-8") as f:
            json.dump(held_meta, f, indent=2)

        # Animation spritesheet metadata
        anim_meta = {
            "id": f"sheet_{wid}_anim",
            "texture": w["anim_file"],
            "frame_width": 32,
            "frame_height": 32,
            "columns": 4,
            "rows": 1,
            "frame_count": 4,
            "spacing": 0,
            "margin": 0,
            "origin": "center"
        }
        with open(f"assets/weapons/animations/{wid}_anim.json", "w", encoding="utf-8") as f:
            json.dump(anim_meta, f, indent=2)

    # 4. Register in asset_manifest.json
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    existing_ids = {a["id"] for a in manifest["assets"]}

    new_assets = []
    for w in WEAPONS_DATA:
        wid = w["id"]
        cat_folder = "swords" if w["category"] == "sword" else ("bows" if w["category"] == "bow" else "projectiles")
        cat_name = f"weapons/{cat_folder}"

        # Icon Texture
        icon_id = f"tex_{wid}_icon"
        if icon_id not in existing_ids:
            new_assets.append({
                "id": icon_id,
                "file": w["icon_file"],
                "type": "texture",
                "category": f"weapons/icons",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 64,
                "height": 64,
                "status": "Approved"
            })
            existing_ids.add(icon_id)

        # Ground Texture
        ground_id = f"tex_{wid}_ground"
        if ground_id not in existing_ids:
            new_assets.append({
                "id": ground_id,
                "file": w["ground_file"],
                "type": "texture",
                "category": f"weapons/ground",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 32,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(ground_id)

        # Held Texture & Sheet
        held_tex_id = f"tex_{wid}_held"
        if held_tex_id not in existing_ids:
            new_assets.append({
                "id": held_tex_id,
                "file": w["held_file"],
                "type": "texture",
                "category": f"weapons/held",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 128,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(held_tex_id)

        held_sheet_id = f"sheet_{wid}_held"
        if held_sheet_id not in existing_ids:
            new_assets.append({
                "id": held_sheet_id,
                "file": f"weapons/held/{wid}_held.json",
                "type": "spritesheet",
                "category": f"weapons/held",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 128,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(held_sheet_id)

        # Anim Texture & Sheet
        anim_tex_id = f"tex_{wid}_anim"
        if anim_tex_id not in existing_ids:
            new_assets.append({
                "id": anim_tex_id,
                "file": w["anim_file"],
                "type": "texture",
                "category": f"weapons/animations",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 128,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(anim_tex_id)

        anim_sheet_id = f"sheet_{wid}_anim"
        if anim_sheet_id not in existing_ids:
            new_assets.append({
                "id": anim_sheet_id,
                "file": f"weapons/animations/{wid}_anim.json",
                "type": "spritesheet",
                "category": f"weapons/animations",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 128,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(anim_sheet_id)

    manifest["assets"].extend(new_assets)
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)

    print(f"AMS Registration: Added {len(new_assets)} new weapon assets. Total registered: {len(manifest['assets'])}")

if __name__ == "__main__":
    main()

