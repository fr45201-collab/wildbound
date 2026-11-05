#!/usr/bin/env python3
"""
Storm Asset Validation and Preview Contact Sheet Generator for Wildbound Phase 31.
"""

import os
import wave
import numpy as np
from PIL import Image, ImageDraw

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
ASSETS_ROOT = os.path.join(PROJECT_ROOT, "assets")

EXPECTED_FILES = [
    ("environment/storm/storm_overlay.png", 512, 512, True),
    ("environment/storm/corruption_cloud.png", 256, 256, True),
    ("environment/storm/storm_noise.png", 256, 256, True),
    ("environment/storm/storm_energy.png", 256, 256, True),
    ("environment/storm/part_corruption_small.png", 16, 16, True),
    ("environment/storm/part_corruption_medium.png", 32, 32, True),
    ("environment/storm/part_corruption_large.png", 64, 64, True),
    ("environment/storm/part_energy_spark.png", 16, 16, True),
    ("environment/storm/part_storm_debris.png", 32, 32, True),
    ("environment/storm/boundary_ring.png", 256, 256, True),
    ("environment/storm/boundary_glow.png", 256, 256, True),
    ("ui/icons/storm/icon_storm_approaching.png", 48, 48, True),
    ("ui/icons/storm/icon_zone_shrinking.png", 48, 48, True),
    ("ui/icons/storm/icon_outside_zone.png", 48, 48, True),
    ("ui/icons/storm/icon_return_zone.png", 48, 48, True),
    ("ui/icons/storm/icon_final_zone.png", 48, 48, True),
    ("ui/icons/storm/icon_safe_status.png", 48, 48, True),
]

EXPECTED_AUDIO = [
    "audio/sfx/events/sfx_storm_warning.wav",
    "audio/sfx/events/sfx_storm_shrink.wav",
    "audio/sfx/combat/sfx_storm_damage.wav",
    "audio/ambience/amb_storm_rumble.wav"
]

def validate():
    print("==================================================")
    print("VALIDATING WILDBOUND PHASE 31 STORM ASSETS")
    print("==================================================")
    
    passed = 0
    failed = 0
    
    # 1. Image validation
    for rel_path, exp_w, exp_h, exp_alpha in EXPECTED_FILES:
        full_path = os.path.join(ASSETS_ROOT, rel_path)
        if not os.path.exists(full_path):
            print(f"[FAIL] Missing file: {rel_path}")
            failed += 1
            continue
            
        try:
            with Image.open(full_path) as img:
                w, h = img.size
                if w != exp_w or h != exp_h:
                    print(f"[FAIL] {rel_path}: Size mismatch {w}x{h} (expected {exp_w}x{exp_h})")
                    failed += 1
                    continue
                if exp_alpha and img.mode != 'RGBA':
                    print(f"[FAIL] {rel_path}: Mode is {img.mode} (expected RGBA)")
                    failed += 1
                    continue
                # Check for completely blank/zero images
                arr = np.array(img)
                if arr.max() == 0:
                    print(f"[FAIL] {rel_path}: Image is completely blank/black!")
                    failed += 1
                    continue
                if exp_alpha and arr[..., 3].max() == 0:
                    print(f"[FAIL] {rel_path}: Alpha channel is completely transparent!")
                    failed += 1
                    continue
                    
                print(f"[PASS] {rel_path} ({w}x{h}, {img.mode}) - OK")
                passed += 1
        except Exception as e:
            print(f"[FAIL] {rel_path}: Corrupt image ({e})")
            failed += 1

    # 2. Audio validation
    for rel_path in EXPECTED_AUDIO:
        full_path = os.path.join(ASSETS_ROOT, rel_path)
        if not os.path.exists(full_path):
            print(f"[FAIL] Missing audio file: {rel_path}")
            failed += 1
            continue
        try:
            with wave.open(full_path, 'rb') as wf:
                channels = wf.getnchannels()
                rate = wf.getframerate()
                frames = wf.getnframes()
                dur = frames / float(rate)
                if dur <= 0.05:
                    print(f"[FAIL] {rel_path}: Duration too short ({dur:.2f}s)")
                    failed += 1
                    continue
                print(f"[PASS] {rel_path} ({dur:.2f}s, {rate}Hz, {channels}ch) - OK")
                passed += 1
        except Exception as e:
            print(f"[FAIL] {rel_path}: Corrupt WAV ({e})")
            failed += 1

    # 3. Create Contact Sheet
    preview_path = os.path.join(ASSETS_ROOT, "environment/storm/storm_assets_preview.png")
    sheet_w, sheet_h = 1024, 768
    sheet = Image.new("RGBA", (sheet_w, sheet_h), (25, 20, 35, 255))
    draw = ImageDraw.Draw(sheet)
    draw.text((20, 20), "WILDBOUND PHASE 31: CORRUPTION STORM ASSET CONTACT SHEET", fill=(240, 200, 255, 255))
    
    # Place overlay scaled down
    p_overlay = os.path.join(ASSETS_ROOT, "environment/storm/storm_overlay.png")
    if os.path.exists(p_overlay):
        with Image.open(p_overlay) as im:
            sheet.paste(im.resize((256, 256)), (20, 60))
            draw.text((20, 325), "Storm Overlay (512x512)", fill=(200, 200, 200, 255))

    # Place cloud
    p_cloud = os.path.join(ASSETS_ROOT, "environment/storm/corruption_cloud.png")
    if os.path.exists(p_cloud):
        with Image.open(p_cloud) as im:
            sheet.paste(im, (300, 60), mask=im)
            draw.text((300, 325), "Corruption Cloud (256x256)", fill=(200, 200, 200, 255))

    # Place boundary ring
    p_ring = os.path.join(ASSETS_ROOT, "environment/storm/boundary_ring.png")
    if os.path.exists(p_ring):
        with Image.open(p_ring) as im:
            sheet.paste(im, (580, 60), mask=im)
            draw.text((580, 325), "Boundary Ring (256x256)", fill=(200, 200, 200, 255))

    # Place particles row
    draw.text((20, 360), "Storm Particle Sprites:", fill=(240, 200, 255, 255))
    parts = [
        ("part_corruption_small.png", 20, 390),
        ("part_corruption_medium.png", 60, 390),
        ("part_corruption_large.png", 120, 390),
        ("part_energy_spark.png", 210, 390),
        ("part_storm_debris.png", 250, 390),
    ]
    for p_name, px, py in parts:
        fp = os.path.join(ASSETS_ROOT, "environment/storm", p_name)
        if os.path.exists(fp):
            with Image.open(fp) as im:
                sheet.paste(im, (px, py), mask=im)

    # Place HUD icons row
    draw.text((20, 480), "Storm HUD Warning & Status Icons (48x48):", fill=(240, 200, 255, 255))
    icons = [
        ("icon_storm_approaching.png", 20, 510, "Approaching"),
        ("icon_zone_shrinking.png", 160, 510, "Shrinking"),
        ("icon_outside_zone.png", 300, 510, "Outside"),
        ("icon_return_zone.png", 440, 510, "Return"),
        ("icon_final_zone.png", 580, 510, "Final"),
        ("icon_safe_status.png", 720, 510, "Safe"),
    ]
    for iname, ix, iy, label in icons:
        fp = os.path.join(ASSETS_ROOT, "ui/icons/storm", iname)
        if os.path.exists(fp):
            with Image.open(fp) as im:
                sheet.paste(im, (ix, iy), mask=im)
                draw.text((ix, iy + 55), label, fill=(180, 180, 180, 255))

    sheet.save(preview_path, format='PNG')
    print(f"\nSaved preview contact sheet: {preview_path}")
    
    print("\n==================================================")
    print(f"VALIDATION SUMMARY: {passed} PASSED, {failed} FAILED")
    print("==================================================")
    return failed == 0

if __name__ == "__main__":
    import sys
    success = validate()
    sys.exit(0 if success else 1)

