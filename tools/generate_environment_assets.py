import os
import json
from PIL import Image, ImageDraw

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def save_metadata(meta_path, meta_data):
    ensure_dir(os.path.dirname(meta_path))
    with open(meta_path, "w", encoding="utf-8") as f:
        json.dump(meta_data, f, indent=2)

# Global list of generated assets to update manifest and tests
GENERATED_ASSETS = []

def register_asset(asset_id, name, category, subcategory, rel_png_path, w, h, anchor, collision, rendering, extra=None):
    full_png_path = os.path.join("assets", rel_png_path)
    meta_path = os.path.splitext(full_png_path)[0] + ".json"
    
    meta = {
        "id": asset_id,
        "name": name,
        "category": category,
        "subcategory": subcategory,
        "biomes": ["forest"],
        "sprite": rel_png_path.replace("\\", "/"),
        "dimensions": {
            "width": w,
            "height": h
        },
        "anchors": {
            "ground": anchor
        },
        "collision": collision,
        "rendering": rendering,
        "provenance": {
            "creator": "Wildbound Development Studio",
            "source": "Wildbound Phase 10 Production Environment Asset",
            "license": "CC0 1.0 Universal"
        }
    }
    if extra:
        meta.update(extra)
        
    save_metadata(meta_path, meta)
    
    GENERATED_ASSETS.append({
        "id": f"tex_{asset_id}",
        "asset_id": asset_id,
        "name": name,
        "file": rel_png_path.replace("\\", "/"),
        "category": f"environment/{subcategory}",
        "width": w,
        "height": h,
        "anchor": anchor,
        "collision": collision,
        "rendering": rendering,
        "extra": extra
    })

# Helper drawing primitives
def draw_ground_shadow(draw, cx, cy, rx, ry, alpha=100):
    draw.ellipse([cx - rx, cy - ry, cx + rx, cy + ry], fill=(20, 25, 35, alpha))

# =============================================================================
# 1. NATURE ASSETS (35)
# =============================================================================

def generate_oak_trees():
    configs = [
        ("tree_oak_001", "Small Oak Tree", 48, 64, [24, 60], {"enabled": True, "shape": "rectangle", "offset": [16, 52], "size": [16, 10]}, 16, 20),
        ("tree_oak_002", "Medium Oak Tree", 64, 80, [32, 74], {"enabled": True, "shape": "rectangle", "offset": [22, 64], "size": [20, 12]}, 22, 28),
        ("tree_oak_003", "Large Oak Tree", 80, 96, [40, 90], {"enabled": True, "shape": "rectangle", "offset": [28, 78], "size": [24, 14]}, 28, 36),
        ("tree_oak_004", "Ancient Oak Tree", 112, 128, [56, 120], {"enabled": True, "shape": "rectangle", "offset": [38, 104], "size": [36, 18]}, 40, 50),
    ]
    for asset_id, name, w, h, anchor, col, trunk_w, canopy_r in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Ground shadow
        draw_ground_shadow(draw, anchor[0], anchor[1] - 2, trunk_w + 8, 8, 120)
        
        # Trunk
        tx0 = anchor[0] - trunk_w // 2
        tx1 = anchor[0] + trunk_w // 2
        ty0 = h // 2 - 4
        ty1 = anchor[1]
        
        # Sprawling roots
        draw.polygon([(tx0 - 4, ty1), (tx0 + 2, ty0), (tx1 - 2, ty0), (tx1 + 4, ty1)], fill=(90, 60, 35, 255))
        # Trunk texture
        draw.line([tx0 + 4, ty0 + 6, tx0 + 2, ty1 - 2], fill=(60, 40, 20, 255), width=2)
        draw.line([tx1 - 4, ty0 + 8, tx1 - 2, ty1 - 2], fill=(120, 85, 50, 255), width=2)
        
        # Canopy clumps
        cx = anchor[0]
        cy = ty0 - canopy_r // 3
        # Background shadow foliage
        draw.ellipse([cx - canopy_r - 2, cy - canopy_r + 4, cx + canopy_r + 2, cy + canopy_r + 8], fill=(20, 55, 28, 255))
        # Mid foliage
        draw.ellipse([cx - canopy_r, cy - canopy_r, cx + canopy_r, cy + canopy_r], fill=(45, 105, 52, 255))
        # Foliage clumps
        draw.ellipse([cx - canopy_r * 0.7, cy - canopy_r * 0.8, cx + canopy_r * 0.2, cy + canopy_r * 0.1], fill=(60, 130, 70, 255))
        draw.ellipse([cx - canopy_r * 0.1, cy - canopy_r * 0.7, cx + canopy_r * 0.8, cy + canopy_r * 0.2], fill=(70, 150, 80, 255))
        draw.ellipse([cx - canopy_r * 0.5, cy - canopy_r * 0.3, cx + canopy_r * 0.5, cy + canopy_r * 0.7], fill=(35, 80, 40, 255))
        # Highlights
        draw.ellipse([cx - canopy_r * 0.4, cy - canopy_r * 0.6, cx, cy - canopy_r * 0.2], fill=(95, 185, 105, 255))

        out_path = f"assets/environment/nature/trees/oak/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "trees/oak", f"environment/nature/trees/oak/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_pine_trees():
    configs = [
        ("tree_pine_001", "Small Pine Tree", 32, 64, [16, 60], {"enabled": True, "shape": "rectangle", "offset": [10, 52], "size": [12, 10]}),
        ("tree_pine_002", "Medium Pine Tree", 48, 80, [24, 76], {"enabled": True, "shape": "rectangle", "offset": [16, 66], "size": [16, 12]}),
        ("tree_pine_003", "Large Pine Tree", 64, 96, [32, 90], {"enabled": True, "shape": "rectangle", "offset": [22, 78], "size": [20, 14]}),
    ]
    for asset_id, name, w, h, anchor, col in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        cx = anchor[0]
        cy = anchor[1]
        
        # Shadow
        draw_ground_shadow(draw, cx, cy - 2, 14, 6, 120)
        
        # Trunk
        draw.rectangle([cx - 3, cy - 18, cx + 3, cy], fill=(75, 50, 30, 255))
        draw.line([cx, cy - 18, cx, cy], fill=(50, 32, 18, 255))
        
        # Tiered conical foliage (3 tiers)
        tier_h = (cy - 12) // 3
        # Tier 3 (bottom)
        draw.polygon([(cx - w//2 + 4, cy - 12), (cx + w//2 - 4, cy - 12), (cx, cy - 12 - tier_h * 1.5)], fill=(20, 60, 35, 255))
        draw.polygon([(cx - w//2 + 6, cy - 14), (cx, cy - 14), (cx, cy - 12 - tier_h * 1.4)], fill=(30, 85, 50, 255))
        # Tier 2 (middle)
        t2_y = cy - 12 - tier_h
        draw.polygon([(cx - w//2 + 8, t2_y), (cx + w//2 - 8, t2_y), (cx, t2_y - tier_h * 1.5)], fill=(25, 75, 42, 255))
        draw.polygon([(cx - w//2 + 10, t2_y - 2), (cx, t2_y - 2), (cx, t2_y - tier_h * 1.4)], fill=(40, 105, 60, 255))
        # Tier 1 (top crown)
        t1_y = t2_y - tier_h
        draw.polygon([(cx - w//3 + 2, t1_y), (cx + w//3 - 2, t1_y), (cx, 4)], fill=(30, 90, 50, 255))
        draw.polygon([(cx - w//3 + 4, t1_y - 2), (cx, t1_y - 2), (cx, 6)], fill=(55, 135, 75, 255))

        out_path = f"assets/environment/nature/trees/pine/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "trees/pine", f"environment/nature/trees/pine/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_bushes():
    configs = [
        ("bush_small_001", "Small Bush", 24, 24, [12, 22], {"enabled": False}),
        ("bush_round_001", "Round Bush", 32, 32, [16, 28], {"enabled": True, "shape": "rectangle", "offset": [8, 20], "size": [16, 10]}),
        ("bush_dense_001", "Dense Bush", 40, 32, [20, 28], {"enabled": True, "shape": "rectangle", "offset": [8, 18], "size": [24, 12]}),
        ("bush_flowering_001", "Flowering Bush", 32, 32, [16, 28], {"enabled": True, "shape": "rectangle", "offset": [8, 20], "size": [16, 10]}),
        ("bush_large_001", "Large Bush", 48, 36, [24, 32], {"enabled": True, "shape": "rectangle", "offset": [9, 20], "size": [30, 14]}),
    ]
    for asset_id, name, w, h, anchor, col in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx = anchor[0]
        cy = anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 2, w//2 - 2, 5, 110)
        
        # Shadow underbush
        draw.ellipse([2, 6, w - 2, h - 4], fill=(25, 65, 30, 255))
        # Main foliage
        draw.ellipse([4, 4, w - 4, h - 6], fill=(45, 110, 50, 255))
        # Light clusters
        draw.ellipse([6, 6, w//2 + 2, h//2 + 2], fill=(65, 145, 65, 255))
        draw.ellipse([w//3, 4, w - 6, h//2 + 4], fill=(75, 160, 75, 255))
        
        if "flowering" in asset_id:
            # Add flower petals
            petals = [(cx - 6, cy - 14), (cx + 5, cy - 12), (cx - 2, cy - 8), (cx + 8, cy - 16), (cx - 8, cy - 6)]
            for px, py in petals:
                draw.ellipse([px - 2, py - 2, px + 2, py + 2], fill=(240, 180, 210, 255))
                draw.point((px, py), fill=(255, 230, 100, 255))
                
        out_path = f"assets/environment/nature/bushes/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "bushes", f"environment/nature/bushes/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_grass():
    configs = [
        ("grass_small_001", "Small Grass Patch", 16, 16, [8, 14]),
        ("grass_dense_001", "Dense Grass Patch", 24, 20, [12, 18]),
        ("grass_tall_001", "Tall Grass Tuft", 24, 32, [12, 30]),
        ("grass_dry_001", "Dry Grass Patch", 20, 20, [10, 18]),
        ("grass_flowering_001", "Flowering Grass", 24, 24, [12, 22]),
        ("grass_forest_001", "Lush Forest Grass", 32, 24, [16, 22]),
    ]
    for asset_id, name, w, h, anchor in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        col_main = (55, 135, 55, 255)
        col_dark = (35, 90, 35, 255)
        col_light = (85, 175, 80, 255)
        if "dry" in asset_id:
            col_main = (165, 135, 55, 255)
            col_dark = (115, 90, 35, 255)
            col_light = (205, 175, 85, 255)
            
        # Draw grass blades
        num_blades = w // 3
        for i in range(num_blades):
            bx = 3 + i * 3
            by = cy
            bh = (h * 0.6) if "tall" in asset_id else (h * 0.45)
            curve = (i % 3 - 1) * 3
            draw.line([(bx, by), (bx + curve, by - bh)], fill=col_dark, width=2)
            draw.line([(bx, by), (bx + curve, by - bh)], fill=col_main, width=1)
            draw.point((bx + curve, int(by - bh)), fill=col_light)
            
        if "flowering" in asset_id:
            for fx, fy in [(cx - 4, cy - 10), (cx + 5, cy - 12)]:
                draw.ellipse([fx-1, fy-1, fx+1, fy+1], fill=(255, 255, 255, 255))
                draw.point((fx, fy), fill=(255, 215, 0, 255))

        out_path = f"assets/environment/nature/grass/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "grass", f"environment/nature/grass/{asset_id}.png",
                       w, h, anchor, {"enabled": False}, {"layer": "ground_decor", "y_sort": False})

def generate_flowers():
    configs = [
        ("flower_white_001", "White Flowers", 16, 16, [8, 14], (240, 240, 250, 255), (255, 220, 50, 255)),
        ("flower_yellow_001", "Yellow Buttercups", 16, 16, [8, 14], (245, 210, 40, 255), (200, 120, 20, 255)),
        ("flower_red_001", "Red Poppies", 16, 16, [8, 14], (220, 45, 45, 255), (50, 20, 20, 255)),
        ("flower_blue_001", "Bluebells", 16, 16, [8, 14], (55, 110, 220, 255), (220, 240, 255, 255)),
        ("flower_wildflower_001", "Wildflower Cluster", 24, 20, [12, 18], None, None),
    ]
    for asset_id, name, w, h, anchor, petal_col, center_col in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        # Stems
        draw.line([cx - 3, cy, cx - 3, cy - 6], fill=(40, 100, 40, 255))
        draw.line([cx + 3, cy, cx + 3, cy - 7], fill=(40, 100, 40, 255))
        draw.line([cx, cy, cx, cy - 8], fill=(50, 120, 50, 255))
        
        if petal_col:
            for fx, fy in [(cx - 3, cy - 7), (cx + 3, cy - 8), (cx, cy - 9)]:
                draw.ellipse([fx - 2, fy - 2, fx + 2, fy + 2], fill=petal_col)
                draw.point((fx, fy), fill=center_col)
        else:
            # Cluster of mixed colors
            colors = [
                ((220, 50, 50, 255), (255, 220, 50, 255)),
                ((60, 120, 230, 255), (255, 255, 255, 255)),
                ((240, 210, 40, 255), (180, 100, 20, 255)),
                ((230, 140, 210, 255), (255, 255, 180, 255))
            ]
            for idx, (fx, fy) in enumerate([(cx - 6, cy - 7), (cx + 5, cy - 8), (cx - 1, cy - 10), (cx + 2, cy - 6)]):
                pc, cc = colors[idx % len(colors)]
                draw.ellipse([fx - 2, fy - 2, fx + 2, fy + 2], fill=pc)
                draw.point((fx, fy), fill=cc)

        out_path = f"assets/environment/nature/flowers/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "flowers", f"environment/nature/flowers/{asset_id}.png",
                       w, h, anchor, {"enabled": False}, {"layer": "ground_decor", "y_sort": False})

def generate_rocks():
    configs = [
        ("rock_small_001", "Small Stone", 16, 16, [8, 14], {"enabled": False}),
        ("rock_medium_001", "Medium Rock", 24, 20, [12, 18], {"enabled": True, "shape": "rectangle", "offset": [4, 10], "size": [16, 9]}),
        ("rock_boulder_001", "Large Boulder", 48, 40, [24, 36], {"enabled": True, "shape": "rectangle", "offset": [6, 18], "size": [36, 18]}),
        ("rock_flat_001", "Flat Stepping Stone", 32, 20, [16, 18], {"enabled": False}),
        ("rock_mossy_001", "Mossy Rock", 32, 28, [16, 26], {"enabled": True, "shape": "rectangle", "offset": [5, 14], "size": [22, 12]}),
    ]
    for asset_id, name, w, h, anchor, col in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        # Shadow
        draw_ground_shadow(draw, cx, cy - 2, w//2 - 2, 4, 110)
        
        # Stone silhouette
        draw.ellipse([2, 4, w - 2, h - 3], fill=(80, 85, 95, 255))
        # Shading
        draw.polygon([(2, cy), (w//2, 4), (w - 2, cy), (w//2, h - 3)], fill=(110, 115, 125, 255))
        draw.ellipse([4, 6, w//2 + 2, h//2 + 2], fill=(145, 150, 160, 255))
        # Crack lines
        draw.line([cx - 2, cy - 6, cx + 4, cy], fill=(55, 60, 70, 255), width=1)
        
        if "mossy" in asset_id:
            draw.ellipse([4, 5, w//2, h//2 - 2], fill=(60, 120, 50, 255))
            draw.ellipse([w//3, 4, w - 8, h//3 + 4], fill=(75, 145, 60, 255))

        out_path = f"assets/environment/nature/rocks/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "rocks", f"environment/nature/rocks/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_mushrooms():
    configs = [
        ("mushroom_small_001", "Small Mushroom", 16, 16, [8, 14], (210, 200, 180, 255), False),
        ("mushroom_red_001", "Red Toadstool", 16, 20, [8, 18], (215, 45, 45, 255), True),
        ("mushroom_brown_001", "Brown Mushroom", 20, 20, [10, 18], (125, 80, 45, 255), False),
        ("mushroom_cluster_001", "Mushroom Cluster", 24, 20, [12, 18], (180, 120, 70, 255), False),
    ]
    for asset_id, name, w, h, anchor, cap_col, has_spots in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, 5, 3, 100)
        
        if "cluster" in asset_id:
            # 3 small mushrooms
            for ox, oy, scale in [(-5, 0, 0.7), (4, -1, 0.8), (0, -3, 1.0)]:
                draw.rectangle([cx + ox - 1, cy + oy - 6, cx + ox + 1, cy + oy], fill=(220, 215, 200, 255))
                draw.ellipse([cx + ox - 4, cy + oy - 10, cx + ox + 4, cy + oy - 4], fill=cap_col)
        else:
            # Single mushroom
            draw.rectangle([cx - 2, cy - 8, cx + 2, cy], fill=(225, 220, 210, 255))
            draw.ellipse([cx - 6, cy - 12, cx + 6, cy - 4], fill=cap_col)
            draw.arc([cx - 6, cy - 12, cx + 6, cy - 4], 0, 360, fill=(40, 20, 20, 255))
            if has_spots:
                for sx, sy in [(cx - 3, cy - 9), (cx + 2, cy - 10), (cx, cy - 7)]:
                    draw.point((sx, sy), fill=(255, 255, 255, 255))

        out_path = f"assets/environment/nature/mushrooms/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "mushrooms", f"environment/nature/mushrooms/{asset_id}.png",
                       w, h, anchor, {"enabled": False}, {"layer": "ground_decor", "y_sort": False})

def generate_logs():
    configs = [
        ("log_branch_001", "Small Fallen Branch", 24, 16, [12, 14], {"enabled": False}),
        ("log_medium_001", "Medium Fallen Log", 48, 24, [24, 20], {"enabled": True, "shape": "rectangle", "offset": [4, 12], "size": [40, 10]}),
        ("log_trunk_001", "Large Fallen Tree Trunk", 64, 32, [32, 28], {"enabled": True, "shape": "rectangle", "offset": [5, 14], "size": [54, 14]}),
    ]
    for asset_id, name, w, h, anchor, col in configs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 2, w//2 - 2, 4, 110)
        
        # Log trunk cylinder
        y0 = cy - 8
        y1 = cy
        draw.rectangle([6, y0, w - 8, y1], fill=(90, 60, 35, 255))
        # Bark lines
        draw.line([8, y0 + 2, w - 10, y0 + 2], fill=(60, 40, 20, 255))
        draw.line([10, y0 + 5, w - 12, y0 + 5], fill=(115, 80, 45, 255))
        # Cut end ring (right side)
        draw.ellipse([w - 10, y0, w - 4, y1], fill=(180, 145, 95, 255))
        draw.ellipse([w - 9, y0 + 2, w - 5, y1 - 2], outline=(140, 105, 65, 255))
        
        if "trunk" in asset_id:
            # Moss patches
            draw.rectangle([14, y0 - 1, 26, y0 + 2], fill=(65, 130, 55, 255))
            draw.rectangle([34, y0 - 1, 44, y0 + 1], fill=(65, 130, 55, 255))

        out_path = f"assets/environment/nature/logs/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "nature", "logs", f"environment/nature/logs/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

# =============================================================================
# 2. BUILDING ASSETS (18)
# =============================================================================

def draw_building_base(draw, w, h, anchor, wall_w, wall_h, roof_w, roof_h, wall_col, roof_col, door_pos=None, windows=None):
    cx = anchor[0]
    cy = anchor[1]
    
    # Ground shadow
    draw_ground_shadow(draw, cx, cy - 2, wall_w // 2 + 6, 8, 120)
    
    # Walls
    wx0 = cx - wall_w // 2
    wx1 = cx + wall_w // 2
    wy1 = cy - 2
    wy0 = wy1 - wall_h
    draw.rectangle([wx0, wy0, wx1, wy1], fill=wall_col)
    # Timber frame / stone texture lines
    draw.line([wx0, wy0, wx0, wy1], fill=(40, 30, 20, 255), width=2)
    draw.line([wx1, wy0, wx1, wy1], fill=(40, 30, 20, 255), width=2)
    draw.line([wx0, wy1, wx1, wy1], fill=(40, 30, 20, 255), width=2)
    
    # Roof (pitched overhang)
    rx0 = cx - roof_w // 2
    rx1 = cx + roof_w // 2
    ry1 = wy0 + 4
    ry0 = ry1 - roof_h
    # Triangular gable / pitched roof
    draw.polygon([(rx0, ry1), (rx1, ry1), (cx, ry0)], fill=roof_col)
    # Roof highlight & shingle lines
    draw.polygon([(rx0 + 2, ry1 - 2), (cx, ry1 - 2), (cx, ry0 + 3)], fill=(min(255, roof_col[0] + 30), min(255, roof_col[1] + 25), min(255, roof_col[2] + 25), 255))
    draw.line([(rx0, ry1), (cx, ry0), (rx1, ry1)], fill=(40, 25, 20, 255), width=2)
    
    # Door
    if door_pos:
        dx, dy, dw, dh = door_pos
        draw.rectangle([dx - dw//2, dy - dh, dx + dw//2, dy], fill=(60, 40, 25, 255))
        draw.rectangle([dx - dw//2, dy - dh, dx + dw//2, dy], outline=(30, 20, 10, 255))
        draw.point((dx + dw//2 - 2, dy - dh//2), fill=(220, 180, 50, 255)) # brass handle
        
    # Windows
    if windows:
        for wx, wy in windows:
            draw.rectangle([wx - 3, wy - 4, wx + 3, wy + 4], fill=(70, 130, 180, 255))
            draw.rectangle([wx - 3, wy - 4, wx + 3, wy + 4], outline=(40, 30, 20, 255))
            draw.line([wx, wy - 4, wx, wy + 4], fill=(40, 30, 20, 255))
            draw.line([wx - 3, wy, wx + 3, wy], fill=(40, 30, 20, 255))

def generate_houses():
    # 3 Small houses
    small_houses = [
        ("house_cottage_001", "Wooden Cottage", 80, 80, [40, 76], [64, 40], (160, 120, 80, 255), (160, 60, 45, 255)),
        ("house_stone_small_001", "Small Stone House", 80, 80, [40, 76], [64, 40], (130, 135, 145, 255), (65, 75, 95, 255)),
        ("house_village_001", "Forest Village House", 96, 88, [48, 84], [76, 44], (175, 135, 90, 255), (170, 70, 50, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in small_houses:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        door = (cx, cy - 2, 10, 16)
        windows = [(cx - 16, cy - 16), (cx + 16, cy - 16)]
        draw_building_base(draw, w, h, anchor, col_size[0], 32, col_size[0] + 12, 34, wall_c, roof_c, door, windows)
        
        out_path = f"assets/environment/buildings/houses/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "houses", f"environment/buildings/houses/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx, cy], "door": [cx, cy - 2], "windows": windows,
                        "roof": {"width": col_size[0] + 12, "height": 34},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

    # 2 Large houses
    large_houses = [
        ("house_residence_large_001", "Large Wooden Residence", 112, 96, [56, 92], [92, 48], (165, 125, 85, 255), (170, 65, 45, 255)),
        ("house_twostory_001", "Two-Story Medieval House", 120, 112, [60, 108], [96, 54], (140, 145, 155, 255), (60, 70, 90, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in large_houses:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        door = (cx - 10, cy - 2, 12, 20)
        windows = [(cx - 30, cy - 20), (cx + 18, cy - 20), (cx + 36, cy - 20), (cx - 20, cy - 44), (cx + 20, cy - 44)]
        draw_building_base(draw, w, h, anchor, col_size[0], 48, col_size[0] + 14, 42, wall_c, roof_c, door, windows)
        
        out_path = f"assets/environment/buildings/houses/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "houses", f"environment/buildings/houses/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx - 10, cy], "door": [cx - 10, cy - 2], "windows": windows,
                        "roof": {"width": col_size[0] + 14, "height": 42},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

def generate_cabins():
    cabins = [
        ("cabin_forest_001", "Forest Cabin", 72, 72, [36, 68], [56, 36], (140, 95, 60, 255), (110, 75, 45, 255)),
        ("cabin_hunting_001", "Hunting Cabin", 72, 72, [36, 68], [56, 36], (130, 90, 55, 255), (100, 65, 40, 255)),
        ("cabin_woodland_001", "Remote Woodland Cabin", 80, 76, [40, 72], [64, 38], (145, 100, 65, 255), (115, 80, 50, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in cabins:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        # Chimney on right
        draw.rectangle([cx + 18, cy - 54, cx + 24, cy - 28], fill=(90, 95, 105, 255))
        draw.rectangle([cx + 17, cy - 56, cx + 25, cy - 53], fill=(70, 75, 85, 255))
        
        door = (cx - 8, cy - 2, 10, 16)
        windows = [(cx + 12, cy - 14)]
        draw_building_base(draw, w, h, anchor, col_size[0], 28, col_size[0] + 10, 28, wall_c, roof_c, door, windows)
        
        if "hunting" in asset_id:
            # Antlers above door
            draw.line([cx - 12, cy - 24, cx - 8, cy - 20], fill=(220, 210, 190, 255), width=2)
            draw.line([cx - 4, cy - 24, cx - 8, cy - 20], fill=(220, 210, 190, 255), width=2)

        out_path = f"assets/environment/buildings/cabins/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "cabins", f"environment/buildings/cabins/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx - 8, cy], "door": [cx - 8, cy - 2], "windows": windows,
                        "roof": {"width": col_size[0] + 10, "height": 28},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

def generate_shops_and_blacksmith():
    # 2 Shops
    shops = [
        ("shop_general_001", "General Store", 96, 88, [48, 84], [76, 44], (170, 130, 85, 255), (180, 50, 45, 255)),
        ("shop_market_001", "Market Shop Stall", 88, 80, [44, 76], [70, 40], (160, 120, 80, 255), (45, 110, 165, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in shops:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        door = (cx - 12, cy - 2, 12, 18)
        windows = [(cx + 16, cy - 16)]
        draw_building_base(draw, w, h, anchor, col_size[0], 34, col_size[0] + 12, 34, wall_c, roof_c, door, windows)
        
        # Shop striped awning over window
        ax0 = cx + 6
        ax1 = cx + 26
        ay0 = cy - 22
        ay1 = cy - 16
        draw.rectangle([ax0, ay0, ax1, ay1], fill=(230, 220, 200, 255))
        for sx in range(ax0, ax1, 4):
            draw.rectangle([sx, ay0, sx + 2, ay1], fill=roof_c)

        out_path = f"assets/environment/buildings/shops/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "shops", f"environment/buildings/shops/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx - 12, cy], "door": [cx - 12, cy - 2], "windows": windows,
                        "roof": {"width": col_size[0] + 12, "height": 34},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

    # 1 Blacksmith
    bs_id = "blacksmith_forge_001"
    bs_w, bs_h = 112, 96
    bs_anchor = [56, 92]
    bs_col = [90, 48]
    img = Image.new("RGBA", (bs_w, bs_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    cx, cy = bs_anchor[0], bs_anchor[1]
    
    # Large stone forge chimney
    draw.rectangle([cx - 36, cy - 64, cx - 22, cy - 24], fill=(95, 100, 110, 255))
    draw.rectangle([cx - 38, cy - 66, cx - 20, cy - 63], fill=(65, 70, 80, 255))
    
    door = (cx + 10, cy - 2, 12, 18)
    windows = [(cx + 28, cy - 16)]
    draw_building_base(draw, bs_w, bs_h, bs_anchor, bs_col[0], 36, bs_col[0] + 12, 36, (120, 125, 135, 255), (70, 75, 85, 255), door, windows)
    
    # Outdoor forge anvil & coal bin
    draw.rectangle([cx - 32, cy - 16, cx - 20, cy - 6], fill=(50, 50, 55, 255)) # coal bin
    draw.rectangle([cx - 16, cy - 12, cx - 8, cy - 4], fill=(70, 75, 85, 255)) # anvil block
    draw.line([cx - 18, cy - 12, cx - 6, cy - 12], fill=(160, 170, 180, 255), width=3) # anvil horn
    
    out_path = f"assets/environment/buildings/blacksmith/{bs_id}.png"
    ensure_dir(os.path.dirname(out_path))
    img.save(out_path)
    register_asset(bs_id, "Village Blacksmith Forge", "building", "blacksmith", f"environment/buildings/blacksmith/{bs_id}.png",
                   bs_w, bs_h, bs_anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - bs_col[0]//2, cy - bs_col[1]], "size": bs_col},
                   {"layer": "solid_structures", "y_sort": True},
                   {"entrance": [cx + 10, cy], "door": [cx + 10, cy - 2], "windows": windows,
                    "roof": {"width": bs_col[0] + 12, "height": 36},
                    "interior_compatibility": {"supported": True, "map_reference": f"interior_{bs_id}"}})

def generate_storage_and_watchtowers():
    # 2 Storage buildings
    storage = [
        ("storage_shed_001", "Wooden Storage Shed", 64, 64, [32, 60], [50, 32], (150, 110, 70, 255), (105, 75, 45, 255)),
        ("storage_warehouse_001", "Supply Warehouse", 96, 88, [48, 84], [78, 44], (155, 115, 75, 255), (145, 55, 40, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in storage:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        door = (cx, cy - 2, 16, 20)
        draw_building_base(draw, w, h, anchor, col_size[0], 30, col_size[0] + 8, 28, wall_c, roof_c, door, None)
        
        out_path = f"assets/environment/buildings/storage/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "storage", f"environment/buildings/storage/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx, cy], "door": [cx, cy - 2], "windows": [],
                        "roof": {"width": col_size[0] + 8, "height": 28},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

    # 2 Watchtowers
    watchtowers = [
        ("watchtower_wood_001", "Wooden Watchtower", 64, 112, [32, 108], [40, 36], (135, 95, 60, 255), (95, 65, 40, 255)),
        ("watchtower_stone_001", "Stone Watchtower", 64, 120, [32, 116], [44, 40], (125, 130, 140, 255), (65, 70, 80, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, wall_c, roof_c in watchtowers:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 2, 24, 8, 120)
        
        # Tall shaft
        bx0 = cx - col_size[0] // 2 + 4
        bx1 = cx + col_size[0] // 2 - 4
        by1 = cy - 2
        by0 = by1 - 70
        draw.rectangle([bx0, by0, bx1, by1], fill=wall_c)
        draw.line([bx0, by0, bx0, by1], fill=(40, 30, 20, 255), width=2)
        draw.line([bx1, by0, bx1, by1], fill=(40, 30, 20, 255), width=2)
        
        # Doorway at base
        draw.rectangle([cx - 5, cy - 18, cx + 5, cy - 2], fill=(40, 25, 15, 255))
        
        # Observation platform
        px0 = cx - 22
        px1 = cx + 22
        py1 = by0
        py0 = py1 - 16
        draw.rectangle([px0, py0, px1, py1], fill=wall_c)
        draw.rectangle([px0, py0, px1, py1], outline=(30, 25, 20, 255))
        # Windows/slits
        draw.rectangle([cx - 10, py0 + 4, cx - 4, py1 - 4], fill=(20, 20, 25, 255))
        draw.rectangle([cx + 4, py0 + 4, cx + 10, py1 - 4], fill=(20, 20, 25, 255))
        
        # Top pyramid roof
        draw.polygon([(px0 - 2, py0), (px1 + 2, py0), (cx, py0 - 20)], fill=roof_c)
        draw.line([(px0 - 2, py0), (cx, py0 - 20), (px1 + 2, py0)], fill=(30, 20, 15, 255), width=2)

        out_path = f"assets/environment/buildings/watchtowers/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "watchtowers", f"environment/buildings/watchtowers/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"entrance": [cx, cy], "door": [cx, cy - 2], "windows": [[cx - 7, py0 + 8], [cx + 7, py0 + 8]],
                        "roof": {"width": 48, "height": 20},
                        "interior_compatibility": {"supported": True, "map_reference": f"interior_{asset_id}"}})

def generate_ruins():
    ruins = [
        ("ruin_wall_broken_001", "Broken Stone Wall", 64, 40, [32, 36], [52, 16]),
        ("ruin_building_abandoned_001", "Abandoned Stone Ruin", 96, 80, [48, 76], [74, 40]),
        ("ruin_tower_small_001", "Small Ruined Tower", 64, 88, [32, 84], [44, 36]),
    ]
    for asset_id, name, w, h, anchor, col_size in ruins:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 2, col_size[0]//2 + 4, 6, 110)
        
        # Crumbled stone walls
        wx0 = cx - col_size[0]//2
        wx1 = cx + col_size[0]//2
        wy1 = cy - 2
        wy0 = wy1 - col_size[1]
        
        draw.rectangle([wx0, wy0, wx1, wy1], fill=(110, 115, 125, 255))
        # Jagged crumbling top
        for k in range(wx0, wx1, 8):
            rag_h = (k % 5) * 3
            draw.rectangle([k, wy0 - rag_h, k + 8, wy0], fill=(125, 130, 140, 255))
        # Stone brick lines
        for by in range(wy0, wy1, 6):
            draw.line([wx0, by, wx1, by], fill=(70, 75, 85, 255))
        # Moss & cracks
        draw.ellipse([wx0 + 6, cy - 14, wx0 + 18, cy - 4], fill=(55, 110, 50, 255))
        draw.ellipse([wx1 - 20, cy - 18, wx1 - 8, cy - 6], fill=(55, 110, 50, 255))

        out_path = f"assets/environment/buildings/ruins/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "building", "ruins", f"environment/buildings/ruins/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True},
                       {"interior_compatibility": {"supported": False}})

# =============================================================================
# 3. ENVIRONMENT DETAILS & PROPS (24)
# =============================================================================

def generate_fences():
    fences = [
        ("fence_straight_001", "Wooden Straight Fence", 32, 24, [16, 22], [30, 8]),
        ("fence_corner_001", "Wooden Corner Fence", 32, 32, [16, 28], [24, 24]),
        ("fence_gate_001", "Wooden Fence Gate", 32, 24, [16, 22], [30, 8]),
        ("fence_broken_001", "Broken Wooden Fence", 32, 24, [16, 22], [18, 8]),
    ]
    for asset_id, name, w, h, anchor, col_size in fences:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, w//2 - 2, 3, 90)
        
        col_wood = (145, 105, 65, 255)
        col_dark = (95, 65, 35, 255)
        
        if "corner" in asset_id:
            # Vertical post corner
            draw.rectangle([cx - 4, cy - 24, cx + 4, cy], fill=col_wood)
            draw.rectangle([cx - 4, cy - 24, cx + 4, cy], outline=col_dark)
            # Two orthogonal rails
            draw.line([2, cy - 16, cx, cy - 16], fill=col_dark, width=3)
            draw.line([cx, cy - 16, cx, cy - 6], fill=col_dark, width=3)
        elif "gate" in asset_id:
            # Posts
            draw.rectangle([2, cy - 18, 6, cy], fill=col_wood)
            draw.rectangle([w - 6, cy - 18, w - 2, cy], fill=col_wood)
            # Gate cross
            draw.rectangle([8, cy - 16, w - 8, cy - 4], fill=col_wood)
            draw.line([8, cy - 16, w - 8, cy - 4], fill=col_dark, width=2)
            draw.line([8, cy - 4, w - 8, cy - 16], fill=col_dark, width=2)
            draw.point((w//2, cy - 10), fill=(220, 180, 50, 255))
        elif "broken" in asset_id:
            draw.rectangle([2, cy - 18, 6, cy], fill=col_wood)
            draw.line([6, cy - 14, 18, cy - 6], fill=col_wood, width=3)
        else:
            # Straight fence
            draw.rectangle([2, cy - 18, 6, cy], fill=col_wood)
            draw.rectangle([w - 6, cy - 18, w - 2, cy], fill=col_wood)
            draw.rectangle([cx - 2, cy - 18, cx + 2, cy], fill=col_wood)
            draw.line([2, cy - 14, w - 2, cy - 14], fill=col_wood, width=3)
            draw.line([2, cy - 7, w - 2, cy - 7], fill=col_wood, width=3)

        out_path = f"assets/environment/props/fences/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "fences", f"environment/props/fences/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "environment", "y_sort": True})

def generate_barrels_and_crates():
    # Barrels
    barrels = [
        ("barrel_standard_001", "Standard Barrel", 20, 24, [10, 22], [14, 12]),
        ("barrel_small_001", "Small Keg", 16, 18, [8, 16], [12, 10]),
        ("barrel_banded_001", "Metal-Banded Barrel", 22, 26, [11, 24], [16, 12]),
    ]
    for asset_id, name, w, h, anchor, col_size in barrels:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, w//2 - 2, 4, 110)
        draw.ellipse([cx - w//2 + 2, cy - h + 4, cx + w//2 - 2, cy], fill=(130, 85, 45, 255))
        draw.rectangle([cx - w//2 + 2, cy - h + 8, cx + w//2 - 2, cy - 4], fill=(145, 95, 50, 255))
        # Metal hoops
        draw.line([cx - w//2 + 2, cy - h + 10, cx + w//2 - 2, cy - h + 10], fill=(80, 85, 95, 255), width=2)
        draw.line([cx - w//2 + 2, cy - 6, cx + w//2 - 2, cy - 6], fill=(80, 85, 95, 255), width=2)
        # Top rim
        draw.ellipse([cx - w//2 + 3, cy - h + 4, cx + w//2 - 3, cy - h + 10], fill=(110, 70, 35, 255))

        out_path = f"assets/environment/props/barrels/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "barrels", f"environment/props/barrels/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "environment", "y_sort": True})

    # Crates
    crates = [
        ("crate_small_001", "Small Wooden Crate", 20, 20, [10, 18], [16, 14]),
        ("crate_large_001", "Large Cargo Crate", 32, 32, [16, 28], [26, 20]),
        ("crate_reinforced_001", "Reinforced Storage Crate", 28, 28, [14, 25], [22, 18]),
    ]
    for asset_id, name, w, h, anchor, col_size in crates:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, w//2 - 2, 4, 110)
        draw.rectangle([cx - col_size[0]//2, cy - col_size[1], cx + col_size[0]//2, cy], fill=(160, 115, 70, 255))
        draw.rectangle([cx - col_size[0]//2, cy - col_size[1], cx + col_size[0]//2, cy], outline=(95, 65, 35, 255))
        # Cross diagonals
        draw.line([cx - col_size[0]//2, cy - col_size[1], cx + col_size[0]//2, cy], fill=(115, 80, 45, 255))
        draw.line([cx - col_size[0]//2, cy, cx + col_size[0]//2, cy - col_size[1]], fill=(115, 80, 45, 255))
        
        if "reinforced" in asset_id:
            # Iron corner brackets
            for kx, ky in [(cx - col_size[0]//2, cy - col_size[1]), (cx + col_size[0]//2, cy - col_size[1]),
                           (cx - col_size[0]//2, cy), (cx + col_size[0]//2, cy)]:
                draw.rectangle([kx - 2, ky - 2, kx + 2, ky + 2], fill=(90, 95, 105, 255))

        out_path = f"assets/environment/props/crates/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "crates", f"environment/props/crates/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "environment", "y_sort": True})

def generate_wells_and_lamps():
    # Wells
    wells = [
        ("well_stone_001", "Stone Village Well", 36, 40, [18, 36], [26, 22], (125, 130, 140, 255), (145, 105, 65, 255)),
        ("well_wooden_001", "Wooden Roofed Well", 36, 44, [18, 40], [26, 22], (145, 105, 65, 255), (160, 65, 45, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, base_c, roof_c in wells:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 2, 16, 6, 120)
        # Stone/wood base
        draw.ellipse([cx - 13, cy - 16, cx + 13, cy], fill=base_c)
        draw.ellipse([cx - 10, cy - 14, cx + 10, cy - 4], fill=(30, 50, 70, 255)) # water surface
        # Posts
        draw.line([cx - 11, cy - 10, cx - 11, cy - 26], fill=(100, 70, 40, 255), width=2)
        draw.line([cx + 11, cy - 10, cx + 11, cy - 26], fill=(100, 70, 40, 255), width=2)
        # Roof canopy
        draw.polygon([(cx - 15, cy - 24), (cx + 15, cy - 24), (cx, cy - 36)], fill=roof_c)

        out_path = f"assets/environment/props/wells/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "wells", f"environment/props/wells/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "solid_structures", "y_sort": True})

    # Lamps
    lamps = [
        ("lamp_post_001", "Wooden Post Street Lamp", 20, 48, [10, 46], {"enabled": True, "shape": "rectangle", "offset": [5, 36], "size": [10, 10]}),
        ("lamp_wall_001", "Wall-Mounted Lantern", 16, 24, [8, 22], {"enabled": False}),
        ("lamp_hanging_001", "Hanging Chain Lantern", 16, 32, [8, 30], {"enabled": False}),
    ]
    for asset_id, name, w, h, anchor, col in lamps:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        if "post" in asset_id:
            draw_ground_shadow(draw, cx, cy - 1, 6, 3, 100)
            draw.rectangle([cx - 2, cy - 36, cx + 2, cy], fill=(120, 85, 50, 255))
            # Iron bracket
            draw.line([cx, cy - 36, cx + 5, cy - 38], fill=(60, 60, 65, 255), width=2)
            # Lantern
            draw.rectangle([cx + 2, cy - 44, cx + 8, cy - 36], fill=(255, 225, 120, 255))
            draw.rectangle([cx + 2, cy - 44, cx + 8, cy - 36], outline=(50, 50, 55, 255))
        elif "wall" in asset_id:
            draw.line([cx - 4, cy - 12, cx + 2, cy - 12], fill=(50, 50, 55, 255), width=2)
            draw.rectangle([cx - 1, cy - 18, cx + 5, cy - 10], fill=(255, 225, 120, 255))
            draw.rectangle([cx - 1, cy - 18, cx + 5, cy - 10], outline=(50, 50, 55, 255))
        else:
            # Hanging chain
            draw.line([cx, 2, cx, cy - 14], fill=(80, 85, 95, 255), width=1)
            draw.rectangle([cx - 3, cy - 14, cx + 3, cy - 4], fill=(255, 225, 120, 255))
            draw.rectangle([cx - 3, cy - 14, cx + 3, cy - 4], outline=(50, 50, 55, 255))

        out_path = f"assets/environment/props/lamps/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "lamps", f"environment/props/lamps/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_signs():
    signs = [
        ("sign_direction_001", "Village Direction Signpost", 24, 36, [12, 34], {"enabled": True, "shape": "rectangle", "offset": [8, 26], "size": [8, 8]}),
        ("sign_shop_001", "Hanging Shop Signboard", 24, 28, [12, 26], {"enabled": False}),
        ("sign_noticeboard_001", "Wooden Notice Board", 36, 36, [18, 34], {"enabled": True, "shape": "rectangle", "offset": [4, 24], "size": [28, 10]}),
        ("sign_warning_001", "Warning Signpost", 20, 32, [10, 30], {"enabled": True, "shape": "rectangle", "offset": [6, 22], "size": [8, 8]}),
    ]
    for asset_id, name, w, h, anchor, col in signs:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, w//3, 3, 100)
        
        if "direction" in asset_id:
            draw.rectangle([cx - 2, cy - 24, cx + 2, cy], fill=(120, 85, 50, 255))
            # Right arrow board
            draw.polygon([(cx - 2, cy - 22), (cx + 8, cy - 22), (cx + 10, cy - 18), (cx + 8, cy - 14), (cx - 2, cy - 14)], fill=(160, 120, 75, 255))
            # Left arrow board
            draw.polygon([(cx + 2, cy - 13), (cx - 8, cy - 13), (cx - 10, cy - 9), (cx - 8, cy - 5), (cx + 2, cy - 5)], fill=(160, 120, 75, 255))
        elif "noticeboard" in asset_id:
            # 2 posts
            draw.rectangle([cx - 12, cy - 24, cx - 9, cy], fill=(110, 75, 45, 255))
            draw.rectangle([cx + 9, cy - 24, cx + 12, cy], fill=(110, 75, 45, 255))
            # Board
            draw.rectangle([cx - 15, cy - 26, cx + 15, cy - 10], fill=(145, 105, 65, 255))
            # Notice papers
            draw.rectangle([cx - 12, cy - 23, cx - 6, cy - 13], fill=(235, 225, 205, 255))
            draw.rectangle([cx - 2, cy - 24, cx + 4, cy - 14], fill=(235, 225, 205, 255))
            draw.rectangle([cx + 6, cy - 22, cx + 12, cy - 15], fill=(235, 225, 205, 255))
        elif "warning" in asset_id:
            draw.rectangle([cx - 2, cy - 20, cx + 2, cy], fill=(110, 75, 45, 255))
            # Diamond warning board
            draw.polygon([(cx, cy - 28), (cx + 8, cy - 20), (cx, cy - 12), (cx - 8, cy - 20)], fill=(225, 75, 45, 255))
            # Skull / mark in white
            draw.point((cx, cy - 20), fill=(255, 255, 255, 255))
        else:
            # Shop sign
            draw.line([4, 4, 18, 4], fill=(60, 60, 65, 255), width=2)
            draw.rectangle([6, 8, 16, 20], fill=(155, 115, 75, 255))
            draw.rectangle([6, 8, 16, 20], outline=(95, 65, 35, 255))

        out_path = f"assets/environment/props/signs/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "signs", f"environment/props/signs/{asset_id}.png",
                       w, h, anchor, col, {"layer": "environment", "y_sort": True})

def generate_bridges_and_benches():
    # Bridges
    bridges = [
        ("bridge_wood_small_001", "Small Wooden Footbridge", 64, 32, [32, 28], (150, 110, 70, 255)),
        ("bridge_wood_large_001", "Large Timber Bridge", 96, 48, [48, 44], (140, 100, 60, 255)),
        ("bridge_stone_001", "Arched Stone Bridge", 96, 48, [48, 44], (125, 130, 140, 255)),
    ]
    for asset_id, name, w, h, anchor, deck_col in bridges:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        # Bridge deck spanning horizontally
        y0 = cy - 20
        y1 = cy
        draw.rectangle([4, y0, w - 4, y1], fill=deck_col)
        # Planks / stone blocks
        for bx in range(6, w - 4, 6):
            draw.line([bx, y0, bx, y1], fill=(min(255, deck_col[0] - 30), min(255, deck_col[1] - 30), min(255, deck_col[2] - 30), 255), width=1)
        # Side railings
        draw.line([4, y0 - 3, w - 4, y0 - 3], fill=(80, 55, 30, 255) if "wood" in asset_id else (90, 95, 105, 255), width=2)
        draw.line([4, y1 + 1, w - 4, y1 + 1], fill=(80, 55, 30, 255) if "wood" in asset_id else (90, 95, 105, 255), width=2)

        out_path = f"assets/environment/props/bridges/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "bridges", f"environment/props/bridges/{asset_id}.png",
                       w, h, anchor, {"enabled": False, "traversable": True},
                       {"layer": "ground_decor", "y_sort": False})

    # Benches
    benches = [
        ("bench_wooden_001", "Village Wooden Bench", 32, 20, [16, 18], [26, 10], (145, 105, 65, 255)),
        ("bench_stone_001", "Carved Stone Bench", 32, 20, [16, 18], [26, 10], (125, 130, 140, 255)),
    ]
    for asset_id, name, w, h, anchor, col_size, bench_col in benches:
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        cx, cy = anchor[0], anchor[1]
        
        draw_ground_shadow(draw, cx, cy - 1, w//2 - 2, 4, 100)
        # Legs
        draw.rectangle([cx - 10, cy - 8, cx - 7, cy], fill=(min(255, bench_col[0] - 35), min(255, bench_col[1] - 35), min(255, bench_col[2] - 35), 255))
        draw.rectangle([cx + 7, cy - 8, cx + 10, cy], fill=(min(255, bench_col[0] - 35), min(255, bench_col[1] - 35), min(255, bench_col[2] - 35), 255))
        # Seat slab
        draw.rectangle([cx - 13, cy - 12, cx + 13, cy - 8], fill=bench_col)
        # Backrest
        draw.rectangle([cx - 13, cy - 16, cx + 13, cy - 13], fill=bench_col)
        draw.line([cx - 10, cy - 13, cx - 10, cy - 8], fill=bench_col, width=2)
        draw.line([cx + 10, cy - 13, cx + 10, cy - 8], fill=bench_col, width=2)

        out_path = f"assets/environment/props/benches/{asset_id}.png"
        ensure_dir(os.path.dirname(out_path))
        img.save(out_path)
        register_asset(asset_id, name, "prop", "benches", f"environment/props/benches/{asset_id}.png",
                       w, h, anchor, {"enabled": True, "shape": "rectangle", "offset": [cx - col_size[0]//2, cy - col_size[1]], "size": col_size},
                       {"layer": "environment", "y_sort": True})

# =============================================================================
# 4. MANIFEST UPDATE
# =============================================================================

def update_asset_manifest():
    manifest_path = "assets/manifests/asset_manifest.json"
    print(f"Updating asset manifest: {manifest_path}...")
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)
        
    existing_ids = {a["id"] for a in manifest["assets"]}
    
    added_count = 0
    for a in GENERATED_ASSETS:
        tex_id = a["id"]
        if tex_id not in existing_ids:
            manifest["assets"].append({
                "id": tex_id,
                "file": a["file"],
                "type": "texture",
                "category": a["category"],
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercial_use": True,
                "attribution_required": False,
                "checksum": f"{a['asset_id']}_chk",
                "width": a["width"],
                "height": a["height"],
                "status": "approved"
            })
            existing_ids.add(tex_id)
            added_count += 1
            
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=4)
        
    print(f"Added {added_count} new environment textures to manifest. Total entries: {len(manifest['assets'])}")

def main():
    print("Generating Wildbound Phase 10 Environment Asset Collection (77 Assets)...")
    generate_oak_trees()
    generate_pine_trees()
    generate_bushes()
    generate_grass()
    generate_flowers()
    generate_rocks()
    generate_mushrooms()
    generate_logs()
    
    generate_houses()
    generate_cabins()
    generate_shops_and_blacksmith()
    generate_storage_and_watchtowers()
    generate_ruins()
    
    generate_fences()
    generate_barrels_and_crates()
    generate_wells_and_lamps()
    generate_signs()
    generate_bridges_and_benches()
    
    print(f"Total environment assets generated: {len(GENERATED_ASSETS)}")
    assert len(GENERATED_ASSETS) == 77, f"Expected 77 assets, got {len(GENERATED_ASSETS)}"
    
    update_asset_manifest()
    print("Environment asset generation complete!")

if __name__ == "__main__":
    main()

