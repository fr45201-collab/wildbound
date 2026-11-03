# Wildbound Coordinate Systems

## 1. Overview of Spaces
Wildbound defines three distinct coordinate systems to support pixel-perfect rendering across varied displays:

```
+----------------------------------------------------------------+
| Screen Space (Window Backbuffer: e.g. 1920x1080)               |
|                                                                |
|         +--------------------------------------------+         |
|         | Virtual Space (Internal Canvas: 640x360)   |         |
|         |                                            |         |
|         |      +------------------------------+      |         |
|         |      | World Space (e.g. 1600x1200) |      |         |
|         |      | Entities, Tilemaps, Physics  |      |         |
|         |      +------------------------------+      |         |
|         |                                            |         |
|         +--------------------------------------------+         |
|                                                                |
+----------------------------------------------------------------+
```

### A. World Space
- **Domain**: `(0, 0)` to `(worldWidth, worldHeight)` (arbitrary game world units).
- **Origin**: Top-left corner of the world map.
- **Used for**: Entities, tilemaps, physics colliders, camera target.

### B. Virtual Space
- **Domain**: `(0, 0)` to `(640, 360)`.
- **Origin**: Top-left corner of the internal render target canvas.
- **Used for**: In-game HUD, pixel-art text, UI frames, canvas drawing.

### C. Screen Space
- **Domain**: `(0, 0)` to `(windowWidth, windowHeight)` (e.g. 1280x720, 1920x1080).
- **Origin**: Top-left corner of the OS application window.
- **Used for**: Raw OS mouse input, developer ImGui overlay windows.

## 2. Transformation Functions

| Function | Signature | Purpose |
|---|---|---|
| `ScreenToVirtual` | `Viewport::ScreenToVirtual(Vector2)` | Maps window mouse position into virtual canvas pixels, accounting for letterbox offsets and scaling. |
| `VirtualToScreen` | `Viewport::VirtualToScreen(Vector2)` | Maps virtual canvas coordinates to native window backbuffer pixels. |
| `WorldToVirtual` | `Camera2DSystem::WorldToVirtual(Vector2)` | Projects world entity coordinates through the camera into the virtual canvas. |
| `VirtualToWorld` | `Camera2DSystem::VirtualToWorld(Vector2)` | Unprojects virtual canvas pixel coordinates into game world coordinates. |
| `WorldToScreen` | `Camera2DSystem::WorldToScreen(Vector2, Viewport)` | Full pipeline transformation from world space to native window coordinates. |
| `ScreenToWorld` | `Camera2DSystem::ScreenToWorld(Vector2, Viewport)` | Full pipeline transformation from native mouse position to game world coordinates. |

