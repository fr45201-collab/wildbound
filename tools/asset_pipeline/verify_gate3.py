import os
import json
import hashlib
from PIL import Image

def verify_gate3():
    print("=== GATE 3 PILOT INTEGRATION VERIFICATION ===")
    
    # 1. Manifest Verification
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    asset_map = {a["id"]: a for a in manifest["assets"]}
    
    expected_ids = [
        "tex_creature_slime",
        "sheet_creature_slime",
        "tex_biome_desert",
        "tex_icon_potion_64",
        "tex_icon_heart_64",
        "tex_icon_sword_64",
        "tex_icon_shield_64",
        "tex_icon_potion_32",
        "tex_icon_heart_32",
        "tex_icon_sword_32",
        "tex_icon_shield_32"
    ]

    all_ok = True
    for aid in expected_ids:
        if aid not in asset_map:
            print(f"[FAIL] Missing asset ID in manifest: {aid}")
            all_ok = False
            continue
        
        entry = asset_map[aid]
        full_path = os.path.join("assets", entry["file"])
        if not os.path.exists(full_path):
            print(f"[FAIL] File on disk missing: {full_path}")
            all_ok = False
            continue
        
        file_size = os.path.getsize(full_path)
        with open(full_path, "rb") as f:
            csum = hashlib.sha256(f.read()).hexdigest()[:14]
            
        if entry["checksum"] != csum:
            print(f"[FAIL] Checksum mismatch for {aid}: {entry['checksum']} vs {csum}")
            all_ok = False
            continue

        if entry["type"] == "texture":
            img = Image.open(full_path)
            if img.width != entry["width"] or img.height != entry["height"]:
                print(f"[FAIL] Dimension mismatch for {aid}: manifest {entry['width']}x{entry['height']} vs image {img.width}x{img.height}")
                all_ok = False
                continue

        print(f"[PASS] {aid} -> {entry['file']} ({file_size} B, {entry.get('width','-')}x{entry.get('height','-')}, SHA={csum})")

    # 2. Creature Metadata Verification
    meta_path = "assets/processed/creatures/slime/creature_metadata.json"
    anim_path = "assets/processed/creatures/slime/slime_animations.json"
    with open(meta_path, "r", encoding="utf-8") as f:
        meta = json.load(f)
    with open(anim_path, "r", encoding="utf-8") as f:
        anims = json.load(f)
        
    print(f"\n[PASS] Slime Creature Metadata: id='{meta['id']}', name='{meta['name']}', animations count={len(anims['animations'])}")

    # 3. Desert Tileset Verification
    dt_meta_path = "assets/processed/biomes/desert/desert_tileset.json"
    with open(dt_meta_path, "r", encoding="utf-8") as f:
        dmeta = json.load(f)
    print(f"[PASS] Desert Tileset Metadata: id='{dmeta['id']}', total_tiles={dmeta['total_tiles']}, categories={list(dmeta['categories'].keys())}")

    if all_ok:
        print("\n>>> ALL GATE 3 VERIFICATION CHECKS PASSED SUCCESSFULLY! <<<")
    else:
        print("\n>>> SOME CHECKS FAILED! <<<")
        sys.exit(1)

if __name__ == "__main__":
    verify_gate3()

