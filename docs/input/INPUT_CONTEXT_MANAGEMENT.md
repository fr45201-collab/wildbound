# Wildbound — Input Context Management

## 1. Context Stack & Modal Exclusivity

Wildbound employs a hierarchical priority stack (`InputContextManager` in `src/input/InputContext.hpp`) to govern which actions are permitted during different game states.

```
+-------------------------------------------------------------------+
|                        CONTEXT PRIORITY                           |
|                                                                   |
|   [Paused]          (Priority 100) -> Blocks all lower contexts   |
|      |                                                            |
|   [Settings]        (Priority 80)  -> Blocks all lower contexts   |
|      |                                                            |
|   [Wardrobe]        (Priority 60)  -> Blocks all lower contexts   |
|      |                                                            |
|   [Inventory]       (Priority 50)  -> Blocks all lower contexts   |
|      |                                                            |
|   [Menu]            (Priority 40)  -> Blocks all lower contexts   |
|      |                                                            |
|   [Gameplay]        (Priority 10)  -> Base simulation state       |
+-------------------------------------------------------------------+
```

---

## 2. Context Definitions & Action Permissibility

| Context Type | Modal | Allowed Categories | Blocked Actions |
|:---|:---|:---|:---|
| `Gameplay` | No | Movement, Combat, Inventory, Character, General, DevUI | UI navigation (unless overlay active) |
| `Menu` | Yes | UI, General (`Pause`, `UICancel`), DevUI | Movement, Combat, In-game interaction |
| `Inventory` | Yes | Inventory, UI, General (`Pause`, `UICancel`), DevUI | Movement, Combat, Attack, Dodge |
| `Wardrobe` | Yes | Character, UI, General (`Pause`, `UICancel`), DevUI | Movement, Combat |
| `Settings` | Yes | UI, General (`Pause`, `UICancel`), DevUI | Movement, Combat, Inventory |
| `Paused` | Yes | UI, General (`Pause`, `UICancel`), DevUI | All gameplay simulation actions |

---

## 3. Preventing Action Bleed

"Action bleed" occurs when a single button press triggers actions across multiple distinct systems (for instance, pressing `Escape` to close an open Inventory screen, but having the input simultaneously trigger the Game Pause menu).

### 3.1 Solution Mechanism
1. The `InputContextManager` evaluates whether the active top-level context has `blocksLower = true`.
2. When `UIManager` closes a modal screen (e.g., Inventory) in response to `InputAction::Pause` or `InputAction::UICancel`, the action is consumed within the `Inventory` context.
3. The context stack pops `Inventory` back to `Gameplay` at the conclusion of the frame, ensuring lower priority contexts do not see the already-consumed press.

