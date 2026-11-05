#!/usr/bin/env python3
"""
Procedural Storm Particle Generator for Wildbound Phase 31.
Generates pixel-art particle textures for floating corruption motes, energy sparks, and storm debris.
"""

import os
import numpy as np
from PIL import Image

OUTPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../assets/environment/storm"))

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def generate_radial_particle(size, core_color, rim_color, falloff_exp=2.0):
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    cx, cy = (size - 1) / 2.0, (size - 1) / 2.0
    radius = size / 2.0
    
    for y in range(size):
        for x in range(size):
            d = np.sqrt((x - cx) ** 2 + (y - cy) ** 2) / radius
            if d <= 1.0:
                t = d ** falloff_exp
                alpha = int(255 * (1.0 - t))
                r = int(core_color[0] * (1.0 - d) + rim_color[0] * d)
                g = int(core_color[1] * (1.0 - d) + rim_color[1] * d)
                b = int(core_color[2] * (1.0 - d) + rim_color[2] * d)
                rgba[y, x] = [r, g, b, alpha]
                
    return Image.fromarray(rgba, mode='RGBA')

def generate_spark_particle(size):
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    cx, cy = (size - 1) / 2.0, (size - 1) / 2.0
    radius = size / 2.0
    
    for y in range(size):
        for x in range(size):
            dx = abs(x - cx) / radius
            dy = abs(y - cy) / radius
            d = np.sqrt(dx * dx + dy * dy)
            # Cross starburst
            cross = max(0.0, 1.0 - (dx * 3.0 + dy * 0.5)) + max(0.0, 1.0 - (dy * 3.0 + dx * 0.5))
            cross = min(1.0, cross)
            if d <= 1.0:
                alpha = int(255 * cross * (1.0 - d))
                rgba[y, x] = [230, 160, 255, alpha]
                
    return Image.fromarray(rgba, mode='RGBA')

def generate_debris_particle(size):
    np.random.seed(888)
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    cx, cy = int(size / 2), int(size / 2)
    
    # Irregular jagged ash / rock polygon
    angles = np.linspace(0, 2 * np.pi, 7, endpoint=False)
    radii = [size * 0.25 + np.random.uniform(-size * 0.08, size * 0.08) for _ in angles]
    
    # Rasterize simple polygon
    pts = [(int(cx + r * np.cos(a)), int(cy + r * np.sin(a))) for a, r in zip(angles, radii)]
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    from PIL import ImageDraw
    draw = ImageDraw.Draw(img)
    draw.polygon(pts, fill=(50, 20, 60, 230), outline=(130, 50, 160, 255))
    return img

def main():
    ensure_dir(OUTPUT_DIR)
    
    # 1. Small Corruption Mote (16x16)
    img_small = generate_radial_particle(16, (220, 140, 255), (120, 30, 180), falloff_exp=1.8)
    p_small = os.path.join(OUTPUT_DIR, "part_corruption_small.png")
    img_small.save(p_small, format='PNG')
    print(f"Generated {p_small} (16x16)")

    # 2. Medium Corruption Mote (32x32)
    img_med = generate_radial_particle(32, (240, 160, 255), (140, 40, 210), falloff_exp=2.0)
    p_med = os.path.join(OUTPUT_DIR, "part_corruption_medium.png")
    img_med.save(p_med, format='PNG')
    print(f"Generated {p_med} (32x32)")

    # 3. Large Glowing Mote (64x64)
    img_large = generate_radial_particle(64, (255, 200, 255), (160, 50, 230), falloff_exp=2.4)
    p_large = os.path.join(OUTPUT_DIR, "part_corruption_large.png")
    img_large.save(p_large, format='PNG')
    print(f"Generated {p_large} (64x64)")

    # 4. Energy Spark (16x16)
    img_spark = generate_spark_particle(16)
    p_spark = os.path.join(OUTPUT_DIR, "part_energy_spark.png")
    img_spark.save(p_spark, format='PNG')
    print(f"Generated {p_spark} (16x16)")

    # 5. Storm Debris (32x32)
    img_debris = generate_debris_particle(32)
    p_debris = os.path.join(OUTPUT_DIR, "part_storm_debris.png")
    img_debris.save(p_debris, format='PNG')
    print(f"Generated {p_debris} (32x32)")

if __name__ == "__main__":
    main()

