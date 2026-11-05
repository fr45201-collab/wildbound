# Wildbound — Input Debugging Guide

## 1. DevUI Tab 14: "Input & Controls"

The primary developer tool for real-time inspection, tuning, and testing of the input system is **Tab 14 ("Input & Controls")** in the Dear ImGui developer HUD (toggleable via `F1`).

```
+---------------------------------------------------------------------------------------+
|  DevUI Tab 14: Input & Controls                                                       |
+---------------------------------------------------------------------------------------+
| Active Device: Gamepad (Device 0: Xbox Controller Connected)                          |
| Mouse Screen:  ( 960.0, 540.0) | Virtual: ( 320.0, 180.0) | World: (1240.5, 876.2)    |
| Motion Vector: X: 0.707, Y: -0.707 (Magnitude: 1.000)                                 |
+---------------------------------------------------------------------------------------+
| Controller Deadzone: [====|=========] 0.15f                                           |
| Trigger Threshold:   [=======|======] 0.30f                                           |
+---------------------------------------------------------------------------------------+
| [Active Context Stack]                                                                |
|  - Priority 10: Gameplay (Active)                                                     |
+---------------------------------------------------------------------------------------+
| [Input Actions Table]                                                                 |
| Action       | Status   | Value | Primary Binding | Alternate Binding | Controls      |
| MoveUp       | HELD     | 1.00  | W               | LS_Y-             | [Rebind]      |
| Attack       | IDLE     | 0.00  | MouseLeft       | RT+               | [Rebind]      |
| Dodge        | PRESSED  | 1.00  | Space           | A / Cross         | [Rebind]      |
| Interact     | IDLE     | 0.00  | E               | X / Square        | [Rebind]      |
+---------------------------------------------------------------------------------------+
| [Quick Diagnostic Actions]                                                            |
|  [Simulate WASD Move]   [Simulate Attack]   [Reset to Defaults]   [Save to JSON]      |
+---------------------------------------------------------------------------------------+
```

---

## 2. Telemetry Metrics

1. **Active Device Telemetry**:
   - Identifies whether the player is currently driving input via Keyboard/Mouse or Gamepad.
   - Shows connection status and battery/controller index.
2. **Coordinate Telemetry**:
   - Displays real-time cursor values across Native Window, $640 \times 360$ Virtual Canvas, and World Space.
3. **Motion Vector & Analog Magnitude**:
   - Graphically or numerically reports combined stick/WASD direction and length.
4. **Context Stack Monitor**:
   - Shows active contexts and confirms whether modal blocking is properly suppressing lower contexts.

---

## 3. Automated Headless Testing (`test_input.exe`)

The full input test suite can be run headlessly via CTest:
```bash
ctest --test-dir build -C Release -R InputSystemTest --output-on-failure
```
Or run directly via command line:
```bash
.\build\Release\tests\test_input.exe
```

The test executable verifies:
1. `InputAction` enum mappings, conversions, and categories.
2. `InputBinding` validity, modifier masks, and display string formatters.
3. `GamepadInput` radial deadzone continuous math and trigger thresholds.
4. `InputBuffer` enqueueing, expiration windows, and single-consumption guarantees.
5. `InputContextManager` priority stack pushing/popping and modal masking.
6. `Viewport` mouse coordinate transformation from native screen to virtual canvas.
7. `InputSettings` rebinding, conflict detection, and JSON save/load roundtripping.
8. `InputManager` end-to-end action simulation and manual state injection.

