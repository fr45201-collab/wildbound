# Wildbound — Asset Processing & Contact Sheet Generation Guide

## 1. Objective

This guide outlines the technical pipeline and algorithms for normalizing raw downloaded external assets into engine-ready spritesheets, frames, and contact sheets.

All processing operations are executed in Python via `tools/asset_pipeline/` using **Pillow 10.3** and **NumPy 1.24**.

---

## 2. Core Processing Operations

### 2.1 Color Mode & Premultiplied Alpha Normalization
Many external PNGs are authored in Indexed Palette mode (`P`) or standard `RGB` with opaque background keys (e.g. magenta `#FF00FF` or black `#000000`).
- **Operation**:
  ```python
  from PIL import Image
  img = Image.open(raw_path)
  if img.mode != 'RGBA':
      img = img.convert('RGBA')
  ```
- Any color-key transparency (e.g. pure `#FF00FF`) is converted to true alpha transparency:
  ```python
  datas = img.getdata()
  new_data = []
  for item in datas:
      if item[0] == 255 and item[1] == 0 and item[2] == 255:
          new_data.append((255, 255, 255, 0))
      else:
          new_data.append(item)
  img.putdata(new_data)
  ```

### 2.2 Sprite Slicing & Grid Extraction
Atlas sheets containing multiple frames or tiles are parsed into sub-images:
- **Tile Slicing**: Slices 32×32 or 16×16 grids into discrete PNG frames:
  ```python
  box = (col * tile_w, row * tile_h, (col + 1) * tile_w, (row + 1) * tile_h)
  tile = img.crop(box)
  ```
- **Nearest-Neighbor Scaling**: If 16×16 tiles require integer 2× upscaling to match Wildbound's 32×32 baseline, `Image.Resampling.NEAREST` is strictly enforced to eliminate pixel blurring (zero mixels).

### 2.3 Contact Sheet Generation
For visual inspection prior to game integration, a montage contact sheet is generated with an overlaid grid and coordinate labels:
- Transparent checkerboard backdrop (8×8 alternating gray/white).
- Bounding boxes around each frame.
- Frame index numbers rendered onto the contact sheet.
- Output saved as `preview_contact_sheet.png` alongside the processed files.

### 2.4 Metadata Authoring
For each processed spritesheet, a corresponding JSON metadata descriptor is generated:
```json
{
  "id": "sheet_desert_tiles",
  "texture": "tex_desert_tiles",
  "frame_width": 32,
  "frame_height": 32,
  "columns": 8,
  "rows": 8,
  "total_frames": 64,
  "origin_x": 0.0,
  "origin_y": 0.0
}
```

---

## 3. Preservation Invariant

The processing script strictly outputs to `assets/processed/<category>/<asset_name>/` and **never modifies or overwrites any file in `assets/external/` or `assets/`**.

