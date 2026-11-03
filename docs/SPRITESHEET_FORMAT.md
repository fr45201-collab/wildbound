# Wildbound Spritesheet Format Specification

## 1. Overview
Spritesheets in Wildbound represent multiple sprite frames packed into a single image file. Metadata descriptor files (`.json`) define the frame dimensions, packing grid, origin, and spacing.

## 2. JSON Schema Definition

```json
{
    "id": "player_spritesheet",
    "texture": "characters/player/player_spritesheet.png",
    "frame_width": 32,
    "frame_height": 32,
    "columns": 4,
    "rows": 1,
    "frame_count": 4,
    "spacing": 0,
    "margin": 0,
    "origin": "bottom_center"
}
```

### Field Definitions:
- `id` (string): Unique identifier for the spritesheet asset.
- `texture` (string): Relative file path to the source image under `assets/`.
- `frame_width` (integer): Width of each frame in pixels.
- `frame_height` (integer): Height of each frame in pixels.
- `columns` (integer): Number of horizontal frames in the grid.
- `rows` (integer): Number of vertical frames in the grid.
- `frame_count` (integer): Total accessible frames ($1 \le \text{count} \le \text{columns} \times \text{rows}$).
- `spacing` (integer, optional): Pixel padding between adjacent frames (default `0`).
- `margin` (integer, optional): Pixel border around the outer edge (default `0`).
- `origin` (string): Predefined anchor point:
  - `"top_left"`: $(0, 0)$
  - `"center"`: $(W/2, H/2)$
  - `"bottom_center"`: $(W/2, H)$ (Standard for ground entities/characters)
  - `"custom"`: User-specified offset.

## 3. Frame Calculation Formula
Given frame index $i$:
$$\text{col} = i \pmod{\text{columns}}$$
$$\text{row} = \lfloor i / \text{columns} \rfloor$$
$$\text{source.x} = \text{margin} + \text{col} \times (\text{frame\_width} + \text{spacing})$$
$$\text{source.y} = \text{margin} + \text{row} \times (\text{frame\_height} + \text{spacing})$$
$$\text{source.width} = \text{frame\_width}, \quad \text{source.height} = \text{frame\_height}$$

