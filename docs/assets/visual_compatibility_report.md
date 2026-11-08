# Wildbound — Visual Compatibility & Art Direction Evaluation Report

## 1. Objective

Every external asset must harmonize with Wildbound's established visual identity: **Advanced Atmospheric Pixel Art**.

This document defines the visual classification rules (`COMPATIBLE`, `ADAPTABLE`, `INCOMPATIBLE`) and art direction constraints.

---

## 2. Core Art Direction Constraints

| Visual Dimension | Specification | Verification Rule |
|---|---|---|
| **Perspective** | **3/4 Top-Down Oblique** | Visible front and top faces; ground plane oriented $0^\circ$. Reject orthographic side-scroller or true isometric $30^\circ$. |
| **Pixel Density** | **Strict 1:1 Zero-Mixels** | Internal canvas: $640 \times 360$ px integer scaled $2\times$ to $1280 \times 720$. All sprites must share identical texel size on screen. |
| **Tile Baseline** | **32 × 32 px Grid** | Environment terrain, walls, and ground decorations must align to 32×32 px tile bounds. |
| **Humanoid Baseline** | **32 × 48 px Frame** | Humanoid bodies, armor, and hairstyles must align to the 32×48 frame with foot anchor at $(16, 48)$. |
| **Lighting Direction** | **Top-Left $45^\circ$** | Highlights on top and left edges; shadows cast toward bottom-right. |
| **Outlines** | **Selective Dark Outline** | Darkened hue-shifted outline for characters; no stark pure-black outlines on soft environmental terrain. |
| **Color Palette** | **Dark Atmospheric Fantasy** | Desaturated midtones with high-contrast mystical highlights (corruption purple, ember orange, glacial blue). |

---

## 3. Classification Criteria

### 3.1 COMPATIBLE
Assets that meet Wildbound's perspective, pixel density, and palette without requiring hand-editing:
- High-contrast 32×32 UI icons.
- Top-down 32×32 medieval tilesets matching 3/4 oblique angle.
- Pixel-art particle spritesheets.

### 3.2 ADAPTABLE
Assets that fit Wildbound's perspective and theme but require non-destructive automated processing:
- 16×16 tilesets requiring integer $2\times$ nearest-neighbor scaling to 32×32.
- Sprites with solid color-key backgrounds requiring alpha extraction.
- Sprites with overly vibrant palettes requiring palette re-mapping to Lospec swatches.
- Multi-frame sheets requiring slicing and JSON frame descriptor generation.

### 3.3 INCOMPATIBLE
Assets that cannot be used without compromising visual cohesion:
- High-resolution textures (>64×64 for standard tiles) or anti-aliased vector renders.
- Side-scrolling platformer sprites (feet positioned at side view, horizontal walk cycles).
- Isometric tilted grid ($30^\circ / 60^\circ$) tilesets.
- Pre-rendered 3D models with smoothed lighting.
- AI-generated imagery with noisy pixels or distorted outlines.

