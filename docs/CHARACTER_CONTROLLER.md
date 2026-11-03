# Character Controller & Locomotion Mechanics

## 1. Overview

`CharacterController` bridges physical user hardware input (keyboard, gamepad) with entity locomotion physics. It supports multi-key chord inputs, manual input injection for headless testing, and diagonal velocity normalization.

---

## 2. Input Polling & Mapping

In standard gameplay mode, the controller polls hardware key states:
* **Horizontal Movement**:
  * Left: `KEY_A` or `KEY_LEFT` (`x = -1.0`)
  * Right: `KEY_D` or `KEY_RIGHT` (`x = +1.0`)
* **Vertical Movement**:
  * Up: `KEY_W` or `KEY_UP` (`y = -1.0`)
  * Down: `KEY_S` or `KEY_DOWN` (`y = +1.0`)
* **Speed Modifiers**:
  * Run: `KEY_LEFT_SHIFT` or `KEY_RIGHT_SHIFT`

Opposing inputs (e.g., pressing `A` and `D` simultaneously) cancel out cleanly to `0.0`.

---

## 3. Diagonal Normalization Math

In unnormalized 2D movement, moving along two axes simultaneously produces a diagonal displacement vector of magnitude:
$$|\vec{v}_{\text{raw}}| = \sqrt{1^2 + 1^2} = \sqrt{2} \approx 1.4142$$
This results in an unwanted **41.4% speed advantage** when moving diagonally.

`CharacterController` enforces strict normalization:
```cpp
float lengthSq = input.x * input.x + input.y * input.y;
if (lengthSq > 1.0f) {
    float invLength = 1.0f / std::sqrt(lengthSq);
    input.x *= invLength; // 1 / sqrt(2) ≈ 0.7071
    input.y *= invLength; // 1 / sqrt(2) ≈ 0.7071
}
```
Resulting movement velocities maintain an invariant speed:
$$|\vec{v}| = \sqrt{0.7071^2 + 0.7071^2} \times \text{speed} = 1.0 \times \text{speed}$$

---

## 4. Test Injection Mode

To facilitate deterministic automated unit tests without hardware window focus, `CharacterController` provides:
* `SetManualInput(Vector2 dir, bool runRequest)`: Bypasses raylib `IsKeyDown` checks and directly injects input vectors and sprint requests.
* `ClearManualInput()`: Reverts controller back to hardware polling.
