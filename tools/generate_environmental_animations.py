import os
import json
import math
from PIL import Image, ImageDraw

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

GENERATED_ENTRIES = []

def save_json(path, data):
    ensure_dir(os.path.dirname(path))
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)

def register_spritesheet_asset(asset_id, name, rel_dir, frame_w, frame_h, cols, rows, frame_count, frame_duration=0.15, loop=True, category="animated_environment", subcategory=""):
    full_dir = os.path.join("assets", rel_dir)
    ensure_dir(full_dir)
    png_rel = f"{rel_dir}/{asset_id}.png".replace("\\", "/")
    sheet_rel = f"{rel_dir}/{asset_id}_spritesheet.json".replace("\\", "/")
    anim_rel = f"{rel_dir}/{asset_id}_animations.json".replace("\\", "/")
    
    # 1. Spritesheet JSON
    sheet_data = {
        "id": f"sheet_{asset_id}",
        "texture": png_rel,
        "frame_width": frame_w,
        "frame_height": frame_h,
        "columns": cols,
        "rows": rows,
        "frame_count": frame_count,
        "spacing": 0,
        "margin": 0,
        "origin": "center"
    }
    save_json(os.path.join("assets", sheet_rel), sheet_data)
    
    # 2. Animation JSON
    anim_clip_name = f"{asset_id}_loop"
    anim_data = {
        "animations": [
            {
                "animation": anim_clip_name,
                "spritesheet": f"sheet_{asset_id}",
                "frame_duration": frame_duration,
                "loop": loop,
                "frames": list(range(frame_count))
            }
        ]
    }
    save_json(os.path.join("assets", anim_rel), anim_data)
    
    # 3. Companion Metadata JSON
    meta_data = {
        "id": asset_id,
        "name": name,
        "category": category,
        "subcategory": subcategory,
        "biomes": ["forest"],
        "sprite": png_rel,
        "spritesheet": f"sheet_{asset_id}",
        "dimensions": {
            "frame_width": frame_w,
            "frame_height": frame_h,
            "columns": cols,
            "rows": rows,
            "frame_count": frame_count
        },
        "animation": {
            "name": anim_clip_name,
            "frame_duration": frame_duration,
            "loop": loop
        },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 11 Environmental Animation",
            "license": "CC0 1.0 Universal"
        }
    }
    save_json(os.path.join("assets", f"{rel_dir}/{asset_id}.json"), meta_data)
    
    GENERATED_ENTRIES.append({
        "tex_id": f"tex_{asset_id}",
        "sheet_id": f"sheet_{asset_id}",
        "clip_name": anim_clip_name,
        "png_file": png_rel,
        "sheet_file": sheet_rel,
        "anim_file": anim_rel,
        "width": frame_w * cols,
        "height": frame_h * rows,
        "frame_w": frame_w,
        "frame_h": frame_h,
        "frame_count": frame_count,
        "category": f"environment/{subcategory}" if subcategory else "environment"
    })

def register_static_texture(asset_id, name, rel_dir, w, h, category="atmosphere", subcategory=""):
    png_rel = f"{rel_dir}/{asset_id}.png".replace("\\", "/")
    meta_data = {
        "id": asset_id,
        "name": name,
        "category": category,
        "subcategory": subcategory,
        "biomes": ["forest"],
        "sprite": png_rel,
        "dimensions": { "width": w, "height": h },
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 11 Atmospheric Asset",
            "license": "CC0 1.0 Universal"
        }
    }
    save_json(os.path.join("assets", f"{rel_dir}/{asset_id}.json"), meta_data)
    
    GENERATED_ENTRIES.append({
        "tex_id": f"tex_{asset_id}",
        "sheet_id": None,
        "clip_name": None,
        "png_file": png_rel,
        "sheet_file": None,
        "anim_file": None,
        "width": w,
        "height": h,
        "frame_w": w,
        "frame_h": h,
        "frame_count": 1,
        "category": f"environment/{subcategory}" if subcategory else "environment"
    })

# =============================================================================
# 1. WATER ANIMATIONS (5 Spritesheets)
# =============================================================================

def generate_water_animations():
    water_defs = [
        ("water_pond_anim", "Small Pond Water Animation", 32, 32, 4, 0.18, "water"),
        ("water_river_anim", "Flowing River Water Animation", 32, 32, 4, 0.14, "water"),
        ("water_lake_anim", "Lake Surface Water Animation", 32, 32, 4, 0.20, "water"),
        ("water_flow_anim", "Rapid Water Current Animation", 32, 32, 4, 0.12, "water"),
        ("water_ripple_anim", "Water Ring Ripple Animation", 32, 32, 4, 0.15, "water"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in water_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            # Deep blue water base
            draw.rectangle([ox, 0, ox + fw, fh], fill=(35, 75, 140, 220))
            
            if "pond" in asset_id:
                # Concentric soft caustics
                r = 6 + (f * 3) % 12
                draw.ellipse([ox + fw//2 - r, fh//2 - r//2, ox + fw//2 + r, fh//2 + r//2], outline=(75, 140, 220, 180), width=1)
                draw.ellipse([ox + fw//2 - r//2, fh//2 - r//4, ox + fw//2 + r//2, fh//2 + r//4], outline=(140, 195, 255, 160), width=1)
            elif "river" in asset_id:
                # Directional diagonal wave lines
                shift = (f * 8) % 32
                for ly in [6, 16, 26]:
                    draw.line([ox + shift - 10, ly, ox + shift + 12, ly + 2], fill=(85, 160, 240, 200), width=2)
                    draw.line([ox + shift + 2, ly + 1, ox + shift + 8, ly + 2], fill=(200, 230, 255, 220), width=1)
            elif "lake" in asset_id:
                # Gentle horizontal specular reflections
                shift = (f * 4) % 16
                draw.line([ox + 4 + shift, 8, ox + 18 + shift, 8], fill=(120, 180, 255, 180), width=1)
                draw.line([ox + 12 - shift, 18, ox + 28 - shift, 18], fill=(120, 180, 255, 180), width=1)
                draw.line([ox + 8 + shift, 24, ox + 22 + shift, 24], fill=(150, 210, 255, 180), width=1)
            elif "flow" in asset_id:
                # Rapid current with foam caps
                shift = (f * 10) % 32
                draw.line([ox + shift - 16, 10, ox + shift + 14, 10], fill=(220, 240, 255, 240), width=2)
                draw.line([ox + shift - 8, 22, ox + shift + 20, 22], fill=(220, 240, 255, 240), width=2)
                draw.point((ox + shift, 11), fill=(255, 255, 255, 255))
            elif "ripple" in asset_id:
                # Expanding circular ripple ring
                r = 2 + f * 4
                alpha = int(220 * (1.0 - (f / 4.0)))
                draw.ellipse([ox + fw//2 - r, fh//2 - r//2, ox + fw//2 + r, fh//2 + r//2], outline=(160, 215, 255, alpha), width=1)

        out_path = f"assets/environment/animations/water/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/animations/water", fw, fh, frames, 1, frames, dur, True, "animated_environment", "water")

# =============================================================================
# 2. WATERFALL ANIMATIONS (3 Spritesheets)
# =============================================================================

def generate_waterfall_animations():
    wf_defs = [
        ("waterfall_small_anim", "Small Waterfall Animation", 32, 48, 6, 0.11, "waterfalls"),
        ("waterfall_medium_anim", "Medium Waterfall Animation", 48, 64, 6, 0.10, "waterfalls"),
        ("waterfall_large_anim", "Large Waterfall Animation", 64, 80, 6, 0.09, "waterfalls"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in wf_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            # Dark cliff water chute background
            draw.rectangle([ox + 4, 2, ox + fw - 4, fh - 6], fill=(40, 85, 145, 220))
            
            # Cascading vertical flow streaks
            for s in range(4):
                streak_x = ox + 8 + s * ((fw - 16) // 3)
                phase_y = (f * 8 + s * 6) % (fh - 16)
                draw.line([streak_x, 2 + phase_y, streak_x, 14 + phase_y], fill=(130, 190, 255, 230), width=2)
                draw.line([streak_x + 1, 4 + phase_y, streak_x + 1, 10 + phase_y], fill=(235, 245, 255, 255), width=1)
                
            # Foaming base basin
            splash_y = fh - 10
            draw.ellipse([ox + 2, splash_y, ox + fw - 2, fh - 2], fill=(220, 240, 255, 240))
            # Spray droplets
            drop_x = ox + (f * 9) % (fw - 6) + 3
            draw.point((drop_x, splash_y - 3), fill=(255, 255, 255, 255))
            draw.point((ox + fw - drop_x + ox, splash_y - 2), fill=(240, 250, 255, 255))

        out_path = f"assets/environment/animations/waterfalls/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/animations/waterfalls", fw, fh, frames, 1, frames, dur, True, "animated_environment", "waterfalls")

# =============================================================================
# 3. FIRE ANIMATIONS (3 Spritesheets)
# =============================================================================

def generate_fire_animations():
    fire_defs = [
        ("fire_small_anim", "Small Torch Fire Animation", 16, 24, 6, 0.10, "fire"),
        ("fire_medium_anim", "Medium Campfire Flame Animation", 24, 32, 6, 0.09, "fire"),
        ("fire_large_anim", "Large Bonfire Flame Animation", 32, 48, 6, 0.08, "fire"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in fire_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            cx = ox + fw // 2
            base_y = fh - 4
            
            # Subtle flame flicker shift
            shift_x = int(math.sin(f * 1.2) * 2)
            flame_h = fh - 8 + (f % 3) * 2
            
            # Outer Red Core
            draw.polygon([
                (cx - fw//3, base_y),
                (cx + fw//3, base_y),
                (cx + fw//4 + shift_x, base_y - flame_h * 0.5),
                (cx + shift_x, base_y - flame_h),
                (cx - fw//4 + shift_x, base_y - flame_h * 0.5)
            ], fill=(225, 50, 25, 230))
            
            # Mid Orange Flame
            draw.polygon([
                (cx - fw//4, base_y),
                (cx + fw//4, base_y),
                (cx + shift_x, base_y - flame_h * 0.8),
                (cx - shift_x, base_y - flame_h * 0.4)
            ], fill=(255, 140, 20, 240))
            
            # Inner Yellow/White Heart
            draw.polygon([
                (cx - fw//6, base_y),
                (cx + fw//6, base_y),
                (cx + shift_x, base_y - flame_h * 0.5)
            ], fill=(255, 235, 100, 255))
            
            # Floating ember
            ember_y = base_y - flame_h - ((f * 3) % 6)
            draw.point((cx + shift_x * 2, int(ember_y)), fill=(255, 200, 50, 255))

        out_path = f"assets/environment/animations/fire/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/animations/fire", fw, fh, frames, 1, frames, dur, True, "animated_environment", "fire")

# =============================================================================
# 4. GRASS WIND ANIMATIONS (4 Spritesheets)
# =============================================================================

def generate_grass_wind_animations():
    grass_defs = [
        ("grass_small_wind_anim", "Small Grass Wind Sway", 16, 16, 4, 0.20, "grass"),
        ("grass_tall_wind_anim", "Tall Grass Wind Wave", 24, 32, 4, 0.18, "grass"),
        ("grass_dense_wind_anim", "Dense Grass Wind Sway", 24, 20, 4, 0.22, "grass"),
        ("grass_flowering_wind_anim", "Flowering Grass Wind Sway", 24, 24, 4, 0.20, "grass"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in grass_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            # Sinusoidal wind deflection (-2 to +2 px)
            sway = int(math.sin(f * (math.pi / 2.0)) * 2.5)
            
            num_blades = fw // 3
            for b in range(num_blades):
                bx = ox + 3 + b * 3
                by = fh - 2
                bh = fh * 0.65
                tip_x = bx + sway + (b % 2)
                draw.line([(bx, by), (tip_x, int(by - bh))], fill=(40, 105, 45, 255), width=2)
                draw.line([(bx, by), (tip_x, int(by - bh))], fill=(65, 150, 65, 255), width=1)
                draw.point((tip_x, int(by - bh)), fill=(95, 185, 85, 255))
                
            if "flowering" in asset_id:
                fx = ox + fw // 2 + sway
                fy = int(fh * 0.4)
                draw.ellipse([fx - 2, fy - 2, fx + 2, fy + 2], fill=(255, 255, 255, 255))
                draw.point((fx, fy), fill=(255, 220, 50, 255))

        out_path = f"assets/environment/animations/grass/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/animations/grass", fw, fh, frames, 1, frames, dur, True, "animated_environment", "grass")

# =============================================================================
# 5. TREE MOVEMENT & FOLIAGE ANIMATIONS (4 Spritesheets)
# =============================================================================

def generate_tree_movement_animations():
    tree_defs = [
        ("tree_oak_breeze_anim", "Oak Canopy Breeze Overlay", 48, 48, 4, 0.22, "trees"),
        ("tree_pine_breeze_anim", "Pine Branch Breeze Sway", 32, 48, 4, 0.24, "trees"),
        ("bush_rustle_anim", "Bush Wind Rustle Animation", 32, 32, 4, 0.18, "trees"),
        ("leaves_falling_anim", "Falling Woodland Leaves", 32, 32, 6, 0.14, "trees"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in tree_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            cx = ox + fw // 2
            cy = fh // 2
            sway = int(math.sin(f * (math.pi / 2.0)) * 2.0)
            
            if "oak" in asset_id:
                # Foliage clump breathing / swaying
                draw.ellipse([cx - 18 + sway, cy - 14, cx + 18 + sway, cy + 14], fill=(50, 115, 55, 210))
                draw.ellipse([cx - 12 + sway, cy - 12, cx + 8 + sway, cy + 8], fill=(70, 145, 75, 230))
                draw.ellipse([cx - 4 + sway, cy - 8, cx + 14 + sway, cy + 4], fill=(90, 175, 90, 240))
            elif "pine" in asset_id:
                # Conifer branch tip flex
                draw.polygon([(cx - 12, cy + 10), (cx + 12 + sway, cy + 10), (cx, cy - 16)], fill=(30, 80, 48, 220))
                draw.polygon([(cx - 8, cy + 4), (cx + 8 + sway, cy + 4), (cx, cy - 12)], fill=(45, 110, 65, 240))
            elif "bush" in asset_id:
                # Bush clump shimmer
                draw.ellipse([cx - 12 + sway, cy - 10, cx + 12 + sway, cy + 10], fill=(55, 125, 58, 220))
                draw.ellipse([cx - 6 + sway, cy - 6, cx + 6 + sway, cy + 6], fill=(80, 160, 80, 240))
            elif "leaves" in asset_id:
                # 3 falling leaves with tumbling coordinates
                for l in range(3):
                    lx = ox + 6 + l * 10 + int(math.sin(f + l) * 4)
                    ly = (f * 5 + l * 9) % fh
                    draw.ellipse([lx - 2, ly - 1, lx + 2, ly + 1], fill=(195, 140, 45, 230))
                    draw.point((lx, ly), fill=(240, 190, 60, 255))

        out_path = f"assets/environment/animations/trees/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/animations/trees", fw, fh, frames, 1, frames, dur, True, "animated_environment", "trees")

# =============================================================================
# 6. CLOUDS (5 Static Textures)
# =============================================================================

def generate_clouds():
    cloud_defs = [
        ("cloud_small", "Small Cumulus Cloud", 64, 32),
        ("cloud_medium", "Medium Cumulus Cloud", 96, 48),
        ("cloud_large", "Large Atmospheric Cloud", 128, 64),
        ("cloud_thin", "Thin Wispy Cirrus Cloud", 80, 24),
        ("cloud_cluster", "Cloud Cluster Formation", 112, 56),
    ]
    for asset_id, name, w, h in cloud_defs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Soft billowy cloud ellipses
        cy = h // 2
        col_shadow = (200, 215, 235, 150)
        col_mid = (235, 245, 255, 180)
        col_white = (255, 255, 255, 210)
        
        # Shadow bottom
        draw.ellipse([4, cy - 4, w - 4, h - 2], fill=col_shadow)
        # Mid layer
        draw.ellipse([8, cy - 10, w - 8, h - 6], fill=col_mid)
        # Puffs
        num_puffs = 4
        for p in range(num_puffs):
            px = 12 + p * (w // num_puffs)
            pr = (h // 2) - 2
            draw.ellipse([px - pr, cy - pr, px + pr, cy + pr], fill=col_white)

        out_path = f"assets/environment/atmosphere/clouds/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_static_texture(asset_id, name, "environment/atmosphere/clouds", w, h, "atmosphere", "clouds")

# =============================================================================
# 7. FOG LAYERS (4 Textures)
# =============================================================================

def generate_fog_layers():
    fog_defs = [
        ("fog_light_layer", "Light Forest Fog Overlay", 128, 128, 60),
        ("fog_dense_layer", "Dense Forest Fog Bank", 128, 128, 120),
        ("fog_ground_drift", "Ground Mist Drift", 128, 64, 90),
        ("fog_distant_haze", "Distant Atmospheric Haze", 128, 128, 50),
    ]
    for asset_id, name, w, h, base_alpha in fog_defs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        for y in range(h):
            row_alpha = int(base_alpha * (0.6 + 0.4 * math.sin(y * 0.08)))
            draw.line([0, y, w, y], fill=(225, 235, 245, row_alpha))

        out_path = f"assets/environment/atmosphere/fog/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_static_texture(asset_id, name, "environment/atmosphere/fog", w, h, "atmosphere", "fog")

# =============================================================================
# 8. SMOKE ANIMATIONS (3 Spritesheets)
# =============================================================================

def generate_smoke_animations():
    smoke_defs = [
        ("smoke_chimney_anim", "Wispy Chimney Smoke", 16, 32, 6, 0.15, "smoke"),
        ("smoke_campfire_anim", "Campfire Thermal Smoke", 24, 40, 6, 0.14, "smoke"),
        ("smoke_plume_anim", "Smoldering Ruin Smoke Plume", 32, 48, 6, 0.12, "smoke"),
    ]
    for asset_id, name, fw, fh, frames, dur, subcat in smoke_defs:
        sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        
        for f in range(frames):
            ox = f * fw
            cx = ox + fw // 2
            
            # Rising smoke puffs that expand and fade
            for puff in range(3):
                phase = (f + puff * 2) % frames
                py = fh - 4 - int((phase / float(frames)) * (fh - 8))
                pr = 2 + int(phase * 1.5)
                shift = int(math.sin(phase * 1.0) * 3)
                alpha = int(180 * (1.0 - (phase / float(frames))))
                draw.ellipse([cx - pr + shift, py - pr, cx + pr + shift, py + pr], fill=(210, 215, 225, alpha))

        out_path = f"assets/environment/atmosphere/smoke/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        sheet.save(out_path)
        register_spritesheet_asset(asset_id, name, "environment/atmosphere/smoke", fw, fh, frames, 1, frames, dur, True, "atmosphere", "smoke")

# =============================================================================
# 9. AIRPLANE FLYOVER (1 Spritesheet)
# =============================================================================

def generate_airplane_flyover():
    asset_id = "airplane_recon_001"
    name = "Reconnaissance Biplane Flyover"
    fw, fh = 48, 48
    frames = 4
    sheet = Image.new("RGBA", (fw * frames, fh), (0, 0, 0, 0))
    draw = ImageDraw.Draw(sheet)
    
    for f in range(frames):
        ox = f * fw
        cx = ox + fw // 2
        cy = fh // 2
        
        # Fuselage (military olive drab)
        draw.polygon([(cx - 4, cy - 16), (cx + 4, cy - 16), (cx + 3, cy + 14), (cx - 3, cy + 14)], fill=(85, 100, 65, 255))
        draw.polygon([(cx - 2, cy + 14), (cx + 2, cy + 14), (cx, cy + 20)], fill=(70, 85, 55, 255)) # tail rudder
        # Cockpit canopy (cyan glass)
        draw.rectangle([cx - 2, cy - 6, cx + 2, cy], fill=(120, 200, 240, 255))
        # Wings (broad biplane wings)
        draw.rectangle([cx - 20, cy - 8, cx + 20, cy - 4], fill=(95, 115, 75, 255))
        draw.line([cx - 20, cy - 8, cx + 20, cy - 8], fill=(130, 150, 105, 255))
        # Tail horizontal stabilizers
        draw.rectangle([cx - 9, cy + 14, cx + 9, cy + 17], fill=(85, 100, 65, 255))
        
        # Spinning propeller animation (blur line rotating)
        prop_angle = f * (math.pi / 2.0)
        px0 = cx - int(math.cos(prop_angle) * 8)
        py0 = cy - 18 - int(math.sin(prop_angle) * 2)
        px1 = cx + int(math.cos(prop_angle) * 8)
        py1 = cy - 18 + int(math.sin(prop_angle) * 2)
        draw.line([px0, py0, px1, py1], fill=(220, 225, 235, 220), width=2)
        draw.point((cx, cy - 18), fill=(40, 40, 45, 255)) # nose spinner

    out_path = f"assets/environment/events/airplanes/{asset_id}.png"
    ensure_dir(os.path.dirname(out_path))
    sheet.save(out_path)
    register_spritesheet_asset(asset_id, name, "environment/events/airplanes", fw, fh, frames, 1, frames, 0.06, True, "event", "airplanes")

# =============================================================================
# MANIFEST UPDATE
# =============================================================================

def update_manifest():
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)
        
    existing_ids = {a["id"] for a in manifest["assets"]}
    added_count = 0
    
    for entry in GENERATED_ENTRIES:
        # Texture entry
        if entry["tex_id"] not in existing_ids:
            manifest["assets"].append({
                "id": entry["tex_id"],
                "file": entry["png_file"],
                "type": "texture",
                "category": entry["category"],
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercial_use": True,
                "attribution_required": False,
                "checksum": f"{entry['tex_id']}_chk",
                "width": entry["width"],
                "height": entry["height"],
                "status": "approved"
            })
            existing_ids.add(entry["tex_id"])
            added_count += 1
            
        # Spritesheet entry if present
        if entry["sheet_id"] and entry["sheet_id"] not in existing_ids:
            manifest["assets"].append({
                "id": entry["sheet_id"],
                "file": entry["sheet_file"],
                "type": "spritesheet",
                "category": entry["category"],
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercial_use": True,
                "attribution_required": False,
                "checksum": f"{entry['sheet_id']}_chk",
                "width": entry["frame_w"],
                "height": entry["frame_h"],
                "status": "approved"
            })
            existing_ids.add(entry["sheet_id"])
            added_count += 1
            
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=4)
        
    print(f"Updated manifest: added {added_count} entries. Total in manifest: {len(manifest['assets'])}")

def main():
    print("Generating Wildbound Phase 11 Environmental Animation Assets...")
    generate_water_animations()
    generate_waterfall_animations()
    generate_fire_animations()
    generate_grass_wind_animations()
    generate_tree_movement_animations()
    generate_clouds()
    generate_fog_layers()
    generate_smoke_animations()
    generate_airplane_flyover()
    
    print(f"Generated {len(GENERATED_ENTRIES)} environmental asset entries.")
    update_manifest()
    print("Done!")

if __name__ == "__main__":
    main()

