# Camera Follow System & Smooth Tracking

## 1. Overview

The `Camera2DSystem` integrates directly with the player character entity to maintain a stable, centered viewpoint while respecting world boundaries and preventing pixel art jitter.

---

## 2. Follow Mathematics & Exponential Smoothing

In `CameraMode::Follow`, the camera tracks the player's ground anchor `m_position` using framerate-independent exponential smoothing:

$$\vec{T}_{t} = \vec{T}_{t-1} + (\vec{T}_{\text{target}} - \vec{T}_{t-1}) \times \left(1.0 - e^{-\lambda \Delta t}\right)$$

Where:
* $\vec{T}_t$: Current camera target position.
* $\vec{T}_{\text{target}}$: Follow target (player's logical position).
* $\lambda$: Follow speed coefficient (default: $8.0$).
* $\Delta t$: Variable delta time between frames.

---

## 3. World Boundary Clamping

The camera's visible viewport is constrained to the bounding rectangle of the active `Tilemap`:

```cpp
float halfVisibleW = (m_virtualWidth * 0.5f) / m_camera.zoom;
float halfVisibleH = (m_virtualHeight * 0.5f) / m_camera.zoom;

float minX = m_worldBounds.x + halfVisibleW;
float maxX = m_worldBounds.x + m_worldBounds.width - halfVisibleW;
m_camera.target.x = std::clamp(m_camera.target.x, minX, maxX);

float minY = m_worldBounds.y + halfVisibleH;
float maxY = m_worldBounds.y + m_worldBounds.height - halfVisibleH;
m_camera.target.y = std::clamp(m_camera.target.y, minY, maxY);
```

This guarantees the camera never reveals empty voids beyond the forest map perimeter.

---

## 4. Sub-Pixel Snapping for Pixel Art Stability

To avoid shimmering and blurring of high-frequency pixel art textures during smooth floating-point interpolation, `Camera2DSystem::GetRenderCamera()` rounds target and offset values to whole integers during the world rendering pass:

```cpp
Camera2D snapped = m_camera;
snapped.target.x = std::round(m_camera.target.x);
snapped.target.y = std::round(m_camera.target.y);
snapped.offset.x = std::round(m_camera.offset.x);
snapped.offset.y = std::round(m_camera.offset.y);
```
