import os
import json
import hashlib
from PIL import Image

def verify_gate4():
    print("=== GATE 4 CONTROLLED EXPANSION VERIFICATION ===")
    
    with open("assets/manifests/asset_manifest.json", "r", encoding="utf-8") as f:
        manifest = json.load(f)

    asset_map = {a["id"]: a for a in manifest["assets"]}
    print(f"Total manifest assets: {len(manifest['assets'])}")
    
    gate4_ids = [
        # Gate 3 pilot
        "tex_creature_slime", "sheet_creature_slime", "tex_biome_desert",
        "tex_icon_potion_64", "tex_icon_heart_64", "tex_icon_sword_64", "tex_icon_shield_64",
        "tex_icon_potion_32", "tex_icon_heart_32", "tex_icon_sword_32", "tex_icon_shield_32",
        
        # Gate 4 biomes
        "tex_biome_snow", "tex_biome_swamp", "tex_biome_mountain",
        
        # Gate 4 creatures & props
        "tex_creature_snake", "sheet_creature_snake",
        "tex_creature_cyclops", "sheet_creature_cyclops",
        "tex_prop_chest", "sheet_prop_chest",
        
        # Gate 4 weapons & armor
        "tex_icon_bow_64", "tex_icon_bow_32",
        "tex_icon_axe_64", "tex_icon_axe_32",
        "tex_icon_dagger_64", "tex_icon_dagger_32",
        "tex_icon_staff_64", "tex_icon_staff_32",
        "tex_icon_helmet_64", "tex_icon_helmet_32",
        "tex_icon_armor_64", "tex_icon_armor_32",
        
        # Gate 4 items & resources
        "tex_icon_gem_64", "tex_icon_gem_32",
        "tex_icon_herb_64", "tex_icon_herb_32",
        "tex_icon_coins_64", "tex_icon_coins_32",
        "tex_icon_rune_64", "tex_icon_rune_32"
    ]

    all_ok = True
    for aid in gate4_ids:
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

    print(f"Verified {len(gate4_ids)} Phase 49 newly integrated assets across biomes, creatures, props, and UI!")
    if all_ok:
        print(">>> ALL GATE 4 VERIFICATION CHECKS PASSED (100% INTEGRITY)! <<<")
    else:
        print(">>> SOME VERIFICATION CHECKS FAILED! <<<")
        sys.exit(1)

if __name__ == "__main__":
    verify_gate4()

