# Wildbound Rendering Architecture

## 1. Overview
The Wildbound rendering architecture is engineered for authentic, pixel-perfect 2D presentation at a fixed internal virtual resolution of **640 x 360**, scaled to the user's window (default **1280 x 720**, 2x scale) with strict aspect-ratio preservation and letterboxing.

```
                      +-----------------------------+
                      |       Game World Data       |
                      +-----------------------------+
                                     |
                                     v
                       [ World Rendering Pass ]
                       - Camera2DSystem Transform
                       - Snapped Pixel Coordinates
                       - Ground, Objects, Effects
                                     |
                                     v
                  +-------------------------------------+
                  |   Virtual Canvas (RenderTexture2D)  |
                  |             640 x 360               |
                  |     (TEXTURE_FILTER_POINT)          |
                  +-------------------------------------+
                                     |
                                     v
                         [ In-Game UI Pass ]
                         - Screen-space HUD
                         - Pixel-aligned Text / Frames
                                     |
                                     v
                    [ Viewport Presentation Pass ]
                    - Aspect Ratio Calculation
                    - Centered Destination Viewport
                    - Letterbox / Pillarbox Borders
                                     |
                                     v
                  +-------------------------------------+
                  |           Window Backbuffer         |
                  |             1280 x 720              |
                  +-------------------------------------+
                                     |
                                     v
                       [ Developer DevUI Pass ]
                       - Dear ImGui & rlImGui
                       - Diagnostics & Metrics (F1)
```

## 2. Core Rendering Components

### `Viewport` (`src/graphics/Viewport.hpp`)
- Calculates uniform scale factor `min(windowW / virtualW, windowH / virtualH)`.
- Positions the scaled destination rectangle `m_destRect` centered inside the window backbuffer.
- Handles arbitrary window resizing without distorting or stretching the pixel grid.
- Converts coordinates bi-directionally between Screen and Virtual canvas spaces.

### `RenderTexture` (`src/graphics/RenderTexture.hpp`)
- RAII wrapper encapsulating raylib's `RenderTexture2D`.
- Automatically enforces `TEXTURE_FILTER_POINT` (nearest-neighbor) upon creation and recreation.
- Safe lifecycle binding via `Begin()` and `End()`.

### `Renderer` (`src/graphics/Renderer.hpp`)
- Coordinates the virtual render canvas, viewport calculations, camera passes, and backbuffer presentation.
- Keeps rendering hardware logic completely separated from game simulation logic.

## 3. Rendering Layers
The engine establishes standard rendering layers (ordered back-to-front):
0. **`Background`**: Sky, distant parallax layers, deep terrain fills.
1. **`Ground`**: Base terrain tiles, paths, water surfaces.
2. **`GroundDecorations`**: Foliage, flowers, cracks, footprints.
3. **`WorldObjects`**: Trees, rocks, ruins, buildings, chests.
4. **`Characters`**: Player character, NPCs, wildlife, enemies.
5. **`ForegroundObjects`**: Tree crowns, canopy shadows, architectural roofs.
6. **`GameEffects`**: Particle systems, spell impacts, weather overlays.
7. **`GameUI`**: In-game HUD, dialog boxes, hotbars (rendered in virtual canvas space).
8. **`DebugOverlay`**: Developer diagnostics (Dear ImGui) rendered in native window space.

## 4. Pixel-Art Rendering Rules
- **No Fractional Sub-Pixel Rendering**: Camera target and offset coordinates are rounded to integer pixels before rendering, eliminating sub-pixel jitter and shimmering.
- **Strict Point Filtering**: Textures maintain crisp square pixels without bilinear blur or bleeding.
- **Aspect Ratio Integrity**: Non-16:9 displays automatically receive clean letterbox or pillarbox bars.

