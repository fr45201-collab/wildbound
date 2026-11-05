# Wildbound — Keyboard and Mouse Input Specification

## 1. Keyboard Handling Architecture

Wildbound polls keyboard states via raylib's platform layer on every frame inside `InputManager::PollHardware()`.

### 1.1 Key State Evaluation
* `IsKeyPressed(key)`: Rising-edge transition (false $\to$ true in the current frame).
* `IsKeyDown(key)`: High-level continuous state (held down across multiple frames).
* `IsKeyReleased(key)`: Falling-edge transition (true $\to$ false in the current frame).

### 1.2 Modifier Combinations
The system supports modifier flags (`Mod_Shift`, `Mod_Ctrl`, `Mod_Alt`) stored as a bitmask in `InputBinding::modifiers`.
When evaluating a binding with modifier requirements:
```cpp
bool modifiersMatch = true;
if (b.modifiers & Mod_Shift) modifiersMatch &= (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
if (b.modifiers & Mod_Ctrl)  modifiersMatch &= (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
if (b.modifiers & Mod_Alt)   modifiersMatch &= (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
```
A binding only triggers if both the designated key and all required modifiers are simultaneously active.

---

## 2. Mouse Handling & Coordinate Space Pipelines

Mouse position queries are subject to geometric transformations before use by game systems:

```
[Native OS Window (e.g. 1920x1080)]
                |
                v  (via Viewport letterbox offset & scaling)
[Virtual Game Canvas (640x360)]
                |
                v  (via Camera2DSystem translation & zoom)
[World Space Coordinates (Tiles & Entities)]
```

### 2.1 Coordinate Conversion Formulas
1. **Window $\to$ Virtual**:
   $$\text{VirtualX} = \frac{\text{ScreenX} - \text{ViewportOffsetX}}{\text{ViewportScale}}$$
   $$\text{VirtualY} = \frac{\text{ScreenY} - \text{ViewportOffsetY}}{\text{ViewportScale}}$$
   Results are clamped to $[0 \dots 640]$ and $[0 \dots 360]$.

2. **Virtual $\to$ World**:
   $$\text{WorldX} = (\text{VirtualX} - \text{CameraOffsetX}) / \text{CameraZoom} + \text{CameraTargetX}$$
   $$\text{WorldY} = (\text{VirtualY} - \text{CameraOffsetY}) / \text{CameraZoom} + \text{CameraTargetY}$$

### 2.2 Mouse Buttons & Wheel
* `MOUSE_BUTTON_LEFT`: Primary attack, UI selection, item drag-and-drop.
* `MOUSE_BUTTON_RIGHT`: Heavy attack, ranged aim stance, quick-use context.
* `MOUSE_BUTTON_MIDDLE`: Developer panning / inspection.
* `Mouse Wheel`: Normalized vertical scroll delta captured each frame via `GetMouseWheelMove()`. Can be mapped as analog axes (e.g., hotbar cycling).

