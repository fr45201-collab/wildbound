#!/usr/bin/env python3
"""
Procedural Storm Texture Generator for Wildbound Phase 31.
Generates seamless corruption storm textures, atmospheric clouds, and noise maps.
"""

import os
import math
import numpy as np
from PIL import Image

OUTPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../assets/environment/storm"))

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def generate_noise(w, h, scale=16.0, seed=42):
    np.random.seed(seed)
    # Generate multi-octave value noise
    grid_w = max(4, int(w / scale))
    grid_h = max(4, int(h / scale))
    base = np.random.rand(grid_h, grid_w)
    
    # Bilinear interpolation
    img = Image.fromarray((base * 255).astype(np.uint8), mode='L')
    img = img.resize((w, h), Image.Resampling.BICUBIC)
    arr1 = np.array(img, dtype=np.float32) / 255.0
    
    # Octave 2
    base2 = np.random.rand(grid_h * 2, grid_w * 2)
    img2 = Image.fromarray((base2 * 255).astype(np.uint8), mode='L').resize((w, h), Image.Resampling.BICUBIC)
    arr2 = np.array(img2, dtype=np.float32) / 255.0
    
    # Combine octaves
    combined = 0.7 * arr1 + 0.3 * arr2
    return combined

def generate_storm_overlay(w=512, h=512):
    """Generates seamless dark purple/magenta corruption storm atmospheric texture."""
    noise = generate_noise(w, h, scale=32.0, seed=101)
    noise2 = generate_noise(w, h, scale=16.0, seed=202)
    
    rgba = np.zeros((h, w, 4), dtype=np.uint8)
    
    # Corruption palette: Deep violet (#240638) to vivid magenta-purple (#7b1fa2)
    r = (36 + 87 * noise + 20 * noise2).clip(0, 255).astype(np.uint8)
    g = (6 + 25 * noise).clip(0, 255).astype(np.uint8)
    b = (56 + 106 * noise + 35 * noise2).clip(0, 255).astype(np.uint8)
    # Translucent atmospheric density (alpha 120-190)
    a = (110 + 75 * noise).clip(0, 255).astype(np.uint8)
    
    rgba[..., 0] = r
    rgba[..., 1] = g
    rgba[..., 2] = b
    rgba[..., 3] = a
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "storm_overlay.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({w}x{h})")

def generate_corruption_cloud(w=256, h=256):
    """Generates billowy translucent purple corruption cloud sprite."""
    noise = generate_noise(w, h, scale=24.0, seed=303)
    
    # Radial falloff from center
    y, x = np.ogrid[:h, :w]
    cx, cy = w / 2.0, h / 2.0
    dist = np.sqrt((x - cx) ** 2 + (y - cy) ** 2)
    radius = w * 0.45
    falloff = (1.0 - (dist / radius)).clip(0.0, 1.0)
    falloff = falloff ** 1.5
    
    rgba = np.zeros((h, w, 4), dtype=np.uint8)
    r = (80 + 120 * noise).clip(0, 255).astype(np.uint8)
    g = (15 + 40 * noise).clip(0, 255).astype(np.uint8)
    b = (120 + 110 * noise).clip(0, 255).astype(np.uint8)
    a = (210 * falloff * noise).clip(0, 255).astype(np.uint8)
    
    rgba[..., 0] = r
    rgba[..., 1] = g
    rgba[..., 2] = b
    rgba[..., 3] = a
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "corruption_cloud.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({w}x{h})")

def generate_storm_noise(w=256, h=256):
    """Generates procedural high-contrast turbulence noise for shader/distortion."""
    noise = generate_noise(w, h, scale=12.0, seed=404)
    rgba = np.zeros((h, w, 4), dtype=np.uint8)
    val = (noise * 255).clip(0, 255).astype(np.uint8)
    rgba[..., 0] = val
    rgba[..., 1] = val
    rgba[..., 2] = val
    rgba[..., 3] = 255
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "storm_noise.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({w}x{h})")

def generate_storm_energy(w=256, h=256):
    """Generates arcane electric filaments and energy distortion."""
    noise1 = generate_noise(w, h, scale=18.0, seed=505)
    noise2 = generate_noise(w, h, scale=9.0, seed=606)
    
    # Sharp filaments via abs(sin(noise))
    filaments = np.abs(np.sin((noise1 + noise2) * 8.0))
    filaments = np.power(1.0 - filaments, 3.0)
    
    rgba = np.zeros((h, w, 4), dtype=np.uint8)
    r = (180 + 75 * filaments).clip(0, 255).astype(np.uint8)
    g = (40 + 80 * filaments).clip(0, 255).astype(np.uint8)
    b = (220 + 35 * filaments).clip(0, 255).astype(np.uint8)
    a = (255 * filaments).clip(0, 255).astype(np.uint8)
    
    rgba[..., 0] = r
    rgba[..., 1] = g
    rgba[..., 2] = b
    rgba[..., 3] = a
    
    img = Image.fromarray(rgba, mode='RGBA')
    path = os.path.join(OUTPUT_DIR, "storm_energy.png")
    img.save(path, format='PNG')
    print(f"Generated {path} ({w}x{h})")

def main():
    ensure_dir(OUTPUT_DIR)
    generate_storm_overlay(512, 512)
    generate_corruption_cloud(256, 256)
    generate_storm_noise(256, 256)
    generate_storm_energy(256, 256)
    print("All storm textures successfully generated.")

if __name__ == "__main__":
    main()

