# Wildbound — Input Rebinding and Settings Specification

## 1. Rebinding State Machine

Wildbound features a non-blocking rebinding state machine managed by `InputSettings` and rendered via DevUI Tab 14 or in-game Settings screens:

```
[Normal Listening State]
          |
          | User clicks "Rebind Primary" or "Rebind Alternate"
          v
[Awaiting Input Capture State]
  * targetAction = selectedAction
  * isAlternate = true/false
          |
          | Physical Key, Mouse Button, or Gamepad Input Detected
          v
[Conflict Evaluation & Resolution]
  * Check for duplicate bindings
  * If duplicate found -> unbind existing or flag warning
          |
          v
[Apply New Binding & Save JSON]
          |
          v
[Return to Normal Listening State]
```

### 1.1 Protected Actions
Cancellation inputs (such as `Escape` on Keyboard or `B` on Gamepad) can either cancel the remapping process or be bound if explicitly confirmed, preventing users from locking themselves out of menu navigation.

---

## 2. Conflict Detection & Resolution

When a new binding is submitted:
```cpp
auto conflict = DetectConflict(newBinding, targetAction);
if (conflict.has_value()) {
    // Conflict detected with conflict.value()
    // Default policy: Clear conflicting slot to avoid duplicate action triggers
    UnbindAction(conflict.value());
}
```

---

## 3. Persistence & JSON Schema

All user control customizations persist in `config/input_bindings.json`. If the file is missing or corrupted, the system falls back to hardcoded defaults without halting the engine.

### 3.1 JSON Schema Example
```json
{
  "version": 1,
  "deadzone": 0.15000000596046448,
  "triggerThreshold": 0.30000001192092896,
  "bindings": {
    "MoveUp": {
      "primary": { "device": "Keyboard", "source": "ButtonOrKey", "code": 87, "axis": 0, "axisDir": 0, "modifiers": 0 },
      "alternate": { "device": "Gamepad", "source": "Axis", "code": 0, "axis": 1, "axisDir": -1, "modifiers": 0 }
    },
    "Attack": {
      "primary": { "device": "Mouse", "source": "ButtonOrKey", "code": 0, "axis": 0, "axisDir": 0, "modifiers": 0 },
      "alternate": { "device": "Gamepad", "source": "Axis", "code": 0, "axis": 5, "axisDir": 1, "modifiers": 0 }
    },
    "Dodge": {
      "primary": { "device": "Keyboard", "source": "ButtonOrKey", "code": 32, "axis": 0, "axisDir": 0, "modifiers": 0 },
      "alternate": { "device": "Gamepad", "source": "ButtonOrKey", "code": 6, "axis": 0, "axisDir": 0, "modifiers": 0 }
    }
  }
}
```

