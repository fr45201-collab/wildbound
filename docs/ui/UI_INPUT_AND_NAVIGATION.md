# UI Input and Navigation Specification

## 1. Overview & Objectives

Wildbound's UI input system provides seamless, unified navigation across keyboard, mouse, and gamepads (Xbox, PlayStation, Generic DirectInput/XInput). The interface guarantees that:
1. Every menu, dialog, and slot grid can be completely operated using a gamepad or keyboard alone, without requiring mouse interaction.
2. Mouse cursor interaction smoothly overrides and synchronizes focus states without flickering or competing with gamepad focus.
3. Input contexts switch cleanly between gameplay simulation and UI modes.

---

## 2. Input Context State Machine

```
                 [GAMEPLAY CONTEXT]
              (WASD / Attack / Dodge)
                     |       ^
       Press Escape  |       | Press Escape / Resume
       or Open Inv   v       |
                   [UI CONTEXT]
           (Nav Up/Down, Select, Cancel)
```

### Context Interception Rules
When `UIManager::GetScreenState() != UIScreenState::None`:
- **Movement & Combat Suppressed**: Player character velocity is set to zero; weapon attacks, dodge rolls, and ability activations are blocked.
- **Directional UI Actions Routed**:
  - `UI_UP` $\to$ `KEY_UP`, `KEY_W`, `GAMEPAD_BUTTON_LEFT_FACE_UP`, `GAMEPAD_AXIS_LEFT_Y < -0.5`
  - `UI_DOWN` $\to$ `KEY_DOWN`, `KEY_S`, `GAMEPAD_BUTTON_LEFT_FACE_DOWN`, `GAMEPAD_AXIS_LEFT_Y > 0.5`
  - `UI_LEFT` $\to$ `KEY_LEFT`, `KEY_A`, `GAMEPAD_BUTTON_LEFT_FACE_LEFT`, `GAMEPAD_AXIS_LEFT_X < -0.5`
  - `UI_RIGHT` $\to$ `KEY_RIGHT`, `KEY_D`, `GAMEPAD_BUTTON_LEFT_FACE_RIGHT`, `GAMEPAD_AXIS_LEFT_X > 0.5`
  - `UI_SELECT` $\to$ `KEY_ENTER`, `KEY_SPACE`, `KEY_E`, `GAMEPAD_BUTTON_RIGHT_FACE_DOWN` (A / Cross)
  - `UI_CANCEL` $\to$ `KEY_ESCAPE`, `KEY_BACKSPACE`, `GAMEPAD_BUTTON_RIGHT_FACE_RIGHT` (B / Circle)
  - `UI_TAB_PREV` $\to$ `KEY_Q`, `GAMEPAD_BUTTON_LEFT_TRIGGER_1` (LB / L1)
  - `UI_TAB_NEXT` $\to$ `KEY_E`, `GAMEPAD_BUTTON_RIGHT_TRIGGER_1` (RB / R1)

---

## 3. Focus Synchronization Architecture

Each interactive sub-menu maintains internal focus:
- `MainMenu`: `m_selectedItemIndex` $\in [0..N-1]$
- `PauseMenu`: `m_selectedItemIndex` $\in [0..N-1]$
- `SettingsMenu`: `m_activeTab` $\in [0..3]$, `m_selectedOption` $\in [0..M-1]$
- `InventoryUI`: `m_selectedCol` $\in [0..5]$, `m_selectedRow` $\in [0..3]$

### Mouse Hover Override
When the player moves the mouse:
```cpp
Vector2 mouseVirtual = GetVirtualMousePosition();
if (Vector2Distance(mouseVirtual, m_lastMousePos) > 1.0f) {
    m_usingMouse = true;
    m_lastMousePos = mouseVirtual;
}

if (m_usingMouse) {
    for (int i = 0; i < itemCount; ++i) {
        if (CheckCollisionPointRec(mouseVirtual, itemBounds[i])) {
            m_selectedItemIndex = i;
            break;
        }
    }
}
```
When any directional gamepad button or arrow key is pressed, `m_usingMouse` is set to `false`, instantly locking focus back onto gamepad navigation highlights.

---

## 4. Rebinding Integration with `InputSettings`

The Controls tab in `SettingsMenu` queries `wildbound::input::InputSettings`:
- When rebinding is initiated for an action, the UI enters a modal polling state:
  `"Press any Key or Button to Rebind..."`
- The next incoming key event or gamepad button is recorded, validated against reserved system keys (e.g. `F1`, `F12`), and persisted to configuration storage.
- If conflicts arise, an on-screen warning toast notifies the user of the duplicate binding.

