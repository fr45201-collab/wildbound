#!/usr/bin/env python3
"""
Master Storm Asset Generation & Manifest Registration Pipeline for Wildbound Phase 31.
"""

import os
import sys
import json
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
MANIFEST_PATH = os.path.join(PROJECT_ROOT, "assets/manifests/asset_manifest.json")

NEW_ASSETS = [
    {
        "id": "tex_storm_overlay",
        "path": "environment/storm/storm_overlay.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 512, "height": 512, "wrap": "repeat" }
    },
    {
        "id": "tex_corruption_cloud",
        "path": "environment/storm/corruption_cloud.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 256, "height": 256, "transparent": True }
    },
    {
        "id": "tex_storm_noise",
        "path": "environment/storm/storm_noise.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 256, "height": 256, "wrap": "repeat" }
    },
    {
        "id": "tex_storm_energy",
        "path": "environment/storm/storm_energy.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 256, "height": 256, "transparent": True }
    },
    {
        "id": "part_corruption_small",
        "path": "environment/storm/part_corruption_small.png",
        "type": "texture",
        "category": "particles",
        "metadata": { "width": 16, "height": 16, "transparent": True }
    },
    {
        "id": "part_corruption_medium",
        "path": "environment/storm/part_corruption_medium.png",
        "type": "texture",
        "category": "particles",
        "metadata": { "width": 32, "height": 32, "transparent": True }
    },
    {
        "id": "part_corruption_large",
        "path": "environment/storm/part_corruption_large.png",
        "type": "texture",
        "category": "particles",
        "metadata": { "width": 64, "height": 64, "transparent": True }
    },
    {
        "id": "part_energy_spark",
        "path": "environment/storm/part_energy_spark.png",
        "type": "texture",
        "category": "particles",
        "metadata": { "width": 16, "height": 16, "transparent": True }
    },
    {
        "id": "part_storm_debris",
        "path": "environment/storm/part_storm_debris.png",
        "type": "texture",
        "category": "particles",
        "metadata": { "width": 32, "height": 32, "transparent": True }
    },
    {
        "id": "tex_boundary_ring",
        "path": "environment/storm/boundary_ring.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 256, "height": 256, "transparent": True }
    },
    {
        "id": "tex_boundary_glow",
        "path": "environment/storm/boundary_glow.png",
        "type": "texture",
        "category": "environment",
        "metadata": { "width": 256, "height": 256, "transparent": True }
    },
    {
        "id": "icon_storm_approaching",
        "path": "ui/icons/storm/icon_storm_approaching.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "icon_zone_shrinking",
        "path": "ui/icons/storm/icon_zone_shrinking.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "icon_outside_zone",
        "path": "ui/icons/storm/icon_outside_zone.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "icon_return_zone",
        "path": "ui/icons/storm/icon_return_zone.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "icon_final_zone",
        "path": "ui/icons/storm/icon_final_zone.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "icon_safe_status",
        "path": "ui/icons/storm/icon_safe_status.png",
        "type": "texture",
        "category": "ui",
        "metadata": { "width": 48, "height": 48, "transparent": True }
    },
    {
        "id": "sfx_storm_warning",
        "path": "audio/sfx/events/sfx_storm_warning.wav",
        "type": "sound",
        "category": "audio",
        "metadata": { "format": "wav", "bus": "sfx" }
    },
    {
        "id": "sfx_storm_shrink",
        "path": "audio/sfx/events/sfx_storm_shrink.wav",
        "type": "sound",
        "category": "audio",
        "metadata": { "format": "wav", "bus": "sfx" }
    },
    {
        "id": "sfx_storm_damage",
        "path": "audio/sfx/combat/sfx_storm_damage.wav",
        "type": "sound",
        "category": "audio",
        "metadata": { "format": "wav", "bus": "combat" }
    },
    {
        "id": "amb_storm_rumble",
        "path": "audio/ambience/amb_storm_rumble.wav",
        "type": "sound",
        "category": "audio",
        "metadata": { "format": "wav", "bus": "environment", "loop": True }
    }
]

def run_script(script_name):
    script_path = os.path.join(SCRIPT_DIR, script_name)
    print(f"\n--- Running {script_name} ---")
    ret = subprocess.run([sys.executable, script_path], check=True)
    if ret.returncode != 0:
        raise RuntimeError(f"{script_name} failed with code {ret.returncode}")

def register_in_manifest():
    print(f"\n--- Registering assets in {MANIFEST_PATH} ---")
    with open(MANIFEST_PATH, "r", encoding="utf-8") as f:
        manifest = json.load(f)
        
    existing_ids = {a.get("id") for a in manifest.get("assets", [])}
    added_count = 0
    
    for a in NEW_ASSETS:
        if a["id"] not in existing_ids:
            manifest["assets"].append(a)
            existing_ids.add(a["id"])
            added_count += 1
            print(f"  + Registered: [{a['id']}] -> {a['path']}")
        else:
            print(f"  = Already exists: [{a['id']}]")
            
    with open(MANIFEST_PATH, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
        
    print(f"Updated manifest: {added_count} new assets registered. Total: {len(manifest['assets'])}")

def main():
    run_script("generate_storm_textures.py")
    run_script("generate_particle_textures.py")
    run_script("generate_boundary_effects.py")
    run_script("generate_storm_icons.py")
    run_script("generate_storm_audio.py")
    register_in_manifest()
    print("\nMaster Storm Asset Generation complete!")

if __name__ == "__main__":
    main()

