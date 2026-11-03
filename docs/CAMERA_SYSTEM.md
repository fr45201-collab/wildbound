# Wildbound 2D Camera System

## 1. Overview
`Camera2DSystem` (`src/graphics/Camera2D.hpp`) encapsulates and extends raylib's `Camera2D` structure with smooth target following, world boundary clamping, zoom constraints, and pixel-art stabilization.

## 2. Camera Modes
- **`Free` Mode**: Direct manual control via keyboard (WASD). Pan speed dynamically scales with zoom level to maintain consistent perceived movement speed.
- **`Follow` Mode**: Tracks a target position (or target pointer) using exponential smoothing. Ideal for following the player character or cinematic focal points.
- **`Fixed` Mode**: Locks the camera to a specified anchor coordinate in the world.

## 3. Smooth Follow Mechanism
Follow interpolation uses frame-rate independent exponential smoothing:
```cpp
float t = 1.0f - std::exp(-followSpeed * dt);
camera.target += (desiredTarget - camera.target) * t;
```
This avoids jitter, stuttering, and dependency on variable display refresh rates.

## 4. World Boundary Clamping
Configurable world boundaries (`Rectangle worldBounds`, e.g. `1600 x 1200`) prevent the camera from revealing areas outside the playable environment.

- **World Larger than Viewport**: The camera target is clamped so that the visible boundary never extends past the edges of `worldBounds`:
  ```cpp
  visibleWidth = virtualWidth / zoom;
  minX = worldBounds.x + visibleWidth * 0.5f;
  maxX = worldBounds.x + worldBounds.width - visibleWidth * 0.5f;
  target.x = std::clamp(target.x, minX, maxX);
  ```
- **World Smaller than Viewport**: When `worldBounds` width or height is smaller than the visible viewport, the camera automatically centers the world inside the viewport:
  ```cpp
  target.x = worldBounds.x + worldBounds.width * 0.5f;
  target.y = worldBounds.y + worldBounds.height * 0.5f;
  ```

## 5. Zoom Constraints & Pixel Snapping
- **Zoom Constraints**: Zoom is strictly clamped between `m_minZoom` (0.5x) and `m_maxZoom` (4.0x).
- **Pixel Snapping**: `GetRenderCamera()` rounds camera target and offset coordinates to exact integer pixels, guaranteeing pixel-perfect alignment.

