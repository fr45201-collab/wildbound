#!/usr/bin/env python3
"""
Procedural Pixel-Art Storm Icon Generator for Wildbound Phase 31.
Generates 48x48 transparent HUD icons with high contrast, crisp silhouettes, and no copyrighted symbols.
"""

import os
from PIL import Image, ImageDraw

OUTPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../assets/ui/icons/storm"))

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def draw_icon_storm_approaching(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Outer diamond badge background
    pad = 4
    pts = [(size//2, pad), (size - pad, size//2), (size//2, size - pad), (pad, size//2)]
    draw.polygon(pts, fill=(50, 20, 70, 220), outline=(230, 180, 50, 255))
    
    # Cloud silhouette in center
    draw.ellipse([14, 18, 28, 30], fill=(200, 150, 240, 255))
    draw.ellipse([22, 14, 34, 28], fill=(220, 170, 255, 255))
    draw.rectangle([16, 22, 32, 30], fill=(200, 150, 240, 255))
    
    # Small lightning bolt descending
    bolt = [(25, 28), (22, 34), (26, 34), (23, 40), (28, 33), (25, 33)]
    draw.polygon(bolt, fill=(255, 230, 80, 255))
    
    return img

def draw_icon_zone_shrinking(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Circular badge
    draw.ellipse([4, 4, size - 4, size - 4], fill=(30, 20, 60, 220), outline=(120, 70, 200, 255))
    
    # Outer ring
    draw.ellipse([10, 10, size - 10, size - 10], outline=(200, 100, 255, 255), width=2)
    # Inner safe ring
    draw.ellipse([18, 18, size - 18, size - 18], fill=(80, 40, 140, 255), outline=(100, 220, 255, 255), width=2)
    
    # Inward arrows (N, S, E, W)
    # North arrow pointing down
    draw.polygon([(24, 16), (21, 12), (27, 12)], fill=(255, 100, 100, 255))
    # South arrow pointing up
    draw.polygon([(24, 32), (21, 36), (27, 36)], fill=(255, 100, 100, 255))
    # West arrow pointing right
    draw.polygon([(16, 24), (12, 21), (12, 27)], fill=(255, 100, 100, 255))
    # East arrow pointing left
    draw.polygon([(32, 24), (36, 21), (36, 27)], fill=(255, 100, 100, 255))
    
    return img

def draw_icon_outside_zone(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Triangular hazard badge
    tri = [(24, 4), (44, 42), (4, 42)]
    draw.polygon(tri, fill=(70, 10, 20, 230), outline=(255, 60, 60, 255))
    
    # Exclamation mark
    draw.rectangle([22, 16, 26, 28], fill=(255, 255, 255, 255))
    draw.rectangle([22, 32, 26, 36], fill=(255, 255, 255, 255))
    
    return img

def draw_icon_return_zone(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Hexagonal badge
    draw.polygon([(24, 4), (42, 14), (42, 34), (24, 44), (6, 34), (6, 14)], 
                 fill=(20, 40, 60, 220), outline=(50, 180, 255, 255))
    
    # Large arrow pointing up-right into safe center
    arrow = [(14, 30), (28, 16), (28, 24), (32, 24), (32, 12), (20, 12), (20, 16), (28, 16)]
    draw.polygon([(14, 32), (24, 22), (24, 28), (34, 18), (32, 14), (20, 14), (26, 20), (16, 30)], fill=(80, 240, 180, 255))
    draw.ellipse([26, 12, 36, 22], outline=(100, 255, 200, 255), width=2)
    
    return img

def draw_icon_final_zone(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Red skull-shield badge
    draw.ellipse([4, 4, size - 4, size - 4], fill=(60, 10, 10, 230), outline=(255, 40, 40, 255))
    
    # Stylized critical target crosshair
    draw.ellipse([14, 14, 34, 34], outline=(255, 200, 50, 255), width=2)
    draw.line([24, 8, 24, 16], fill=(255, 255, 255, 255), width=2)
    draw.line([24, 32, 24, 40], fill=(255, 255, 255, 255), width=2)
    draw.line([8, 24, 16, 24], fill=(255, 255, 255, 255), width=2)
    draw.line([32, 24, 40, 24], fill=(255, 255, 255, 255), width=2)
    draw.ellipse([22, 22, 26, 26], fill=(255, 40, 40, 255))
    
    return img

def draw_icon_safe_status(size=48):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Green shield badge
    pts = [(24, 6), (40, 12), (38, 30), (24, 42), (10, 30), (8, 12)]
    draw.polygon(pts, fill=(10, 50, 25, 220), outline=(60, 220, 120, 255))
    
    # Checkmark inside
    check = [(16, 24), (22, 30), (32, 18), (34, 20), (22, 34), (14, 26)]
    draw.polygon(check, fill=(120, 255, 160, 255))
    
    return img

def main():
    ensure_dir(OUTPUT_DIR)
    
    icons = [
        ("icon_storm_approaching.png", draw_icon_storm_approaching()),
        ("icon_zone_shrinking.png", draw_icon_zone_shrinking()),
        ("icon_outside_zone.png", draw_icon_outside_zone()),
        ("icon_return_zone.png", draw_icon_return_zone()),
        ("icon_final_zone.png", draw_icon_final_zone()),
        ("icon_safe_status.png", draw_icon_safe_status())
    ]
    
    for fname, img in icons:
        path = os.path.join(OUTPUT_DIR, fname)
        img.save(path, format='PNG')
        print(f"Generated {path} (48x48)")

if __name__ == "__main__":
    main()

