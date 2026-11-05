import os
import sys
import json

# Ensure local imports work cleanly
script_dir = os.path.dirname(os.path.abspath(__file__))
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

from generators.music import generate_music
from generators.ui import generate_ui_sfx
from generators.footsteps import generate_footsteps
from generators.weapons import generate_weapons
from generators.combat import generate_combat
from generators.creatures import generate_creatures
from generators.environmental import generate_environmental
from generators.interactions import generate_interactions
from generators.events import generate_events
from validation.validate_audio import validate_all_audio

def build_all_audio():
    print("=" * 70)
    print("WILDBOUND PHASE 28: PROCEDURAL AUDIO GENERATION PIPELINE")
    print("=" * 70)

    all_audio = []

    print("[1/9] Generating Music Tracks (11 tracks)...")
    all_audio.extend(generate_music())

    print("[2/9] Generating UI SFX (15 sounds)...")
    all_audio.extend(generate_ui_sfx())

    print("[3/9] Generating Footstep System (24 variations across 8 surfaces)...")
    all_audio.extend(generate_footsteps())

    print("[4/9] Generating Weapon & Shield Audio (16 sounds)...")
    all_audio.extend(generate_weapons())

    print("[5/9] Generating Combat Audio & Flesh Impacts (7 sounds)...")
    all_audio.extend(generate_combat())

    print("[6/9] Generating Creature Audio (23 sounds for 5 archetypes + wildlife)...")
    all_audio.extend(generate_creatures())

    print("[7/9] Generating Environmental Ambience & Biome Loops (16 tracks)...")
    all_audio.extend(generate_environmental())

    print("[8/9] Generating Interactions Audio (8 sounds)...")
    all_audio.extend(generate_interactions())

    print("[9/9] Generating Match Events & Stingers (5 sounds)...")
    all_audio.extend(generate_events())

    print(f"\nGenerated Total Audio Assets: {len(all_audio)}")

    # 1. Master Audio Catalog (Updated with 8 Buses)
    os.makedirs("data/audio", exist_ok=True)
    catalog_path = "data/audio/audio_catalog.json"
    buses = ["master", "music", "ui", "sfx", "environment", "combat", "creatures", "voice"]

    catalog_data = {
        "version": "1.1.0",
        "total_assets": len(all_audio),
        "buses": buses,
        "assets": all_audio
    }
    with open(catalog_path, "w", encoding="utf-8") as f:
        json.dump(catalog_data, f, indent=4)
    print(f"-> Master catalog updated: {catalog_path} ({len(all_audio)} entries)")

    # 2. Biome Audio Configuration (Expanded with 6 Biomes + Rivers/Waterfalls)
    biome_audio_path = "data/audio/biome_audio.json"
    biome_data = {
        "biomes": {
            "forest": {
                "music": "mus_forest",
                "ambient_layers": ["amb_forest_wind", "amb_forest_birds", "amb_forest_river"],
                "default_volume": 0.70
            },
            "desert": {
                "music": "mus_desert",
                "ambient_layers": ["amb_desert_wind"],
                "default_volume": 0.75
            },
            "snow": {
                "music": "mus_snow",
                "ambient_layers": ["amb_snow_blizzard"],
                "default_volume": 0.70
            },
            "swamp": {
                "music": "mus_swamp",
                "ambient_layers": ["amb_swamp_insects"],
                "default_volume": 0.75
            },
            "mountain": {
                "music": "mus_mountain",
                "ambient_layers": ["amb_mountain_wind", "amb_waterfall_heavy"],
                "default_volume": 0.75
            },
            "village": {
                "music": "mus_village",
                "ambient_layers": ["amb_village_market", "amb_village_blacksmith", "amb_indoor_fireplace"],
                "default_volume": 0.70
            }
        }
    }
    with open(biome_audio_path, "w", encoding="utf-8") as f:
        json.dump(biome_data, f, indent=4)
    print(f"-> Biome configurations updated: {biome_audio_path}")

    # 3. Update Asset Management System (AMS) Manifest
    manifest_path = "assets/manifests/asset_manifest.json"
    if os.path.exists(manifest_path):
        with open(manifest_path, "r", encoding="utf-8") as f:
            manifest = json.load(f)

        existing_asset_ids = {a["id"] for a in manifest.get("assets", [])}
        new_assets = []

        for item in all_audio:
            aid = item["id"]
            if aid not in existing_asset_ids:
                new_assets.append({
                    "id": aid,
                    "file": item["file"],
                    "type": "audio",
                    "category": f"audio/{item['category']}",
                    "source": "Wildbound Procedural Audio Generator",
                    "author": "Wildbound Audio Team",
                    "license": "CC0 1.0 Universal",
                    "commercial_use": True,
                    "attribution_required": False,
                    "checksum": f"c_{aid}_audio",
                    "duration": item["duration"],
                    "status": "approved"
                })
                existing_asset_ids.add(aid)

        manifest["assets"].extend(new_assets)
        with open(manifest_path, "w", encoding="utf-8") as f:
            json.dump(manifest, f, indent=2)
        print(f"-> Asset manifest updated: added {len(new_assets)} new entries.")

    # 4. Run automated validation
    print("\nRunning automated audio integrity validation...")
    valid = validate_all_audio("assets/audio")
    if valid:
        print("[SUCCESS] All generated audio assets verified!")
    else:
        print("[ERROR] Some audio files failed validation!")

if __name__ == "__main__":
    build_all_audio()
