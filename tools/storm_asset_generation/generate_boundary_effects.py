#!/usr/bin/env python3
"""
Procedural Safe-Zone Boundary Effect Generator for Wildbound Phase 31.
Generates boundary rings, edge glows, and energy perimeter textures.
"""

import os
import numpy as np
from PIL import Image

OUTPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../assets/environment/storm"))

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def generate_boundary_ring(size=256, ring_radius=110, ring_thickness=12):
    """Generates an energy perimeter ring with vibrant cyan/violet harmonics."""
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    cx, cy = (size - 1) / 2.0, (size - 1) / 2.0
    
    y, x = np.ogrid[:size, :size]
    d = np.sqrt((x - cx) ** 2 + (y - cy) ** 2)
    dist_from_ring = np.abs(d - ring_radius)
    
    mask = dist_from_ring <= (ring_thickness / 2.0)
    norm_dist = dist_from_ring / (ring_thickness / 2.0)
    
    # Core to rim falloff
    alpha = (255 * (1.0 - norm_dist ** 1.5) * mask).clip(0, 255).astype(np.uint8)
    r = (180 + 75 * (1.0 - norm_dist)).clip(0, 255).astype(np.uint8)
    g = (80 + 160 * (1.0 - norm_dist)).clip(0, 255).astype(np.uint8)
    b = (255 * np.ones_like(d)).clip(0, 255).astype(np.uint8)
    
    rgba[..., 0] = r
    rgba[..., 1] = g
    rgba[..., 2] = b
    rgba[..., 3] = alpha
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "boundary_ring.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({size}x{size})")

def generate_boundary_glow(size=256, ring_radius=110, glow_width=32):
    """Generates soft outer radial bloom around boundary."""
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    cx, cy = (size - 1) / 2.0, (size - 1) / 2.0
    
    y, x = np.ogrid[:size, :size]
    d = np.sqrt((x - cx) ** 2 + (y - cy) ** 2)
    dist_from_ring = np.abs(d - ring_radius)
    
    mask = dist_from_ring <= glow_width
    norm_dist = dist_from_ring / glow_width
    
    alpha = (140 * (1.0 - norm_dist ** 2) * mask).clip(0, 255).astype(np.uint8)
    r = 160
    g = 60
    b = 240
    
    rgba[..., 0] = r
    rgba[..., 1] = g
    rgba[..., 2] = b
    rgba[..., 3] = alpha
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "boundary_glow.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({size}x{size})")

def main():
    ensure_dir(OUTPUT_DIR)
    generate_boundary_ring(256)
    generate_boundary_glow(256)
    print("Boundary effect textures generated.")

if __name__ == "__main__":
    main()

