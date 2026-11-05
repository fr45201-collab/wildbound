# Wildbound — Gamepad Input Specification

## 1. Overview & Detection

Wildbound provides first-class gamepad support targeting standard dual-analog controllers (Xbox, PlayStation DualShock/DualSense, generic XInput/DirectInput controllers).

Gamepad presence is dynamically detected each frame via `IsGamepadAvailable(0)`. If a controller is disconnected or unavailable, input silently falls back to keyboard/mouse without throwing exceptions or generating undefined states.

---

## 2. Radial Deadzone Filtering

Standard axial deadzone filtering introduces rectangular deadbands that distort diagonal aim vectors and create sticky axes. Wildbound implements **continuous radial deadzones** on both thumbsticks:

```cpp
float magnitude = std::sqrt(x * x + y * y);
if (magnitude <= deadzone) {
    return Vector2{ 0.0f, 0.0f };
}

// Rescale magnitude smoothly from 0.0 to 1.0
float normalizedMag = (magnitude - deadzone) / (1.0f - deadzone);
normalizedMag = std::clamp(normalizedMag, 0.0f, 1.0f);

float scale = normalizedMag / magnitude;
return Vector2{ x * scale, y * scale };
```

### 2.1 Benefits of Radial Remapping
* **No Origin Jump**: Stick values begin at $0.0$ the instant the stick exits the deadzone boundary.
* **Full Range Preservation**: Deflections reaching the outer physical perimeter yield a true magnitude of $1.0$ in all $360^\circ$ directions.
* **Configurable Thresholds**: Default deadzone is set to `0.15f` (15%), tunable from `0.0f` to `0.5f` in DevUI Tab 14 or `input_bindings.json`.

---

## 3. Trigger Thresholding

Analog triggers (LT / RT on Xbox, L2 / R2 on PlayStation) report values ranging from $-1.0$ (fully released) to $+1.0$ (fully pressed) or $0.0$ to $1.0$ depending on raylib drivers.

Wildbound normalizes trigger readings into a clean $[0.0 \dots 1.0]$ interval:
* Triggers mapped as **Digital Actions** (e.g. `InputAction::Attack` on RT) activate when the analog value exceeds `triggerThreshold` (default: `0.30f`).
* Triggers mapped as **Analog Actions** (e.g. `InputAction::Aim` on LT) return the normalized continuous intensity.

---

## 4. Default Gamepad Layout

| Physical Control | Action Mapped | Context |
|:---|:---|:---|
| Left Thumbstick | `MoveUp`, `MoveDown`, `MoveLeft`, `MoveRight` | Gameplay |
| Right Thumbstick | Cursor / Ranged Aim Direction | Gameplay |
| Left Stick Click (L3) | `Sprint` | Gameplay |
| A / Cross | `UIConfirm` / Jump / Select | Menu & Gameplay |
| B / Circle | `Dodge` / `UICancel` | Gameplay & Menu |
| X / Square | `Interact` / `Reload` | Gameplay |
| Y / Triangle | Weapon Swap / Secondary Action | Gameplay |
| Right Trigger (RT) | `Attack` (Primary Attack / Fire) | Gameplay |
| Right Bumper (RB) | `HeavyAttack` (Power Attack) | Gameplay |
| Left Trigger (LT) | `Aim` (Ranged Aim / Zoom) | Gameplay |
| Left Bumper (LB) | Shield Stance / Guard | Gameplay |
| D-Pad Up | `QuickHeal` / `UINavUp` | Gameplay / UI |
| D-Pad Down | `MapToggle` / `UINavDown` | Gameplay / UI |
| D-Pad Left | `QuickSlot1` / `UINavLeft` | Gameplay / UI |
| D-Pad Right | `CharacterToggle` / `UINavRight` | Gameplay / UI |
| Back / Select | `InventoryToggle` | Gameplay |
| Start / Options | `Pause` | Gameplay & Menu |

