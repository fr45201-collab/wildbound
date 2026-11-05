# Interaction Prompt UI Specification

## 1. Overview

Wildbound's interaction prompt UI delivers contextual feedback to the player when approaching interactable objects. Prompts are rendered within the player-facing HUD by `UIManager::RenderGameplayHUD` using native raylib drawing primitives at $640 \times 360$ virtual canvas resolution.

---

## 2. Visual Layout & Positioning

The prompt banner is positioned above the bottom hotbar / health indicators:

```
+--------------------------------------------------------------+
|                                                              |
|                     [ World Viewport ]                       |
|                                                              |
|                  +------------------------+                  |
|                  |  [E]  Open Wooden Gate |                  |
|                  +------------------------+                  |
|                                                              |
|        [ HP: 100/100 ]   [ 1 ][ 2 ][ 3 ][ 4 ][ 5 ]           |
+--------------------------------------------------------------+
```

### Visual Styling
* **Key Badge**: Rounded rectangle with warm golden border (`#E5C07B`) and dark background (`#1E1E24`).
* **Input Device Glyphs**:
  - **Keyboard**: `[E]` (or remapped `InputAction::Interact` key).
  - **Gamepad**: `[X]` (Xbox) or `[Square]` (PlayStation).
* **Action Text**: Clean sans-serif text showing the current dynamic verb and target name (e.g., `"Open Wooden Gate"`, `"Unlock Ancient Crypt Door"`, `"Collect Health Potion"`).
* **Feedback Banner**: When an interaction fails (e.g., missing key or full inventory), a brief 2-second alert banner appears in warm amber or warning red (e.g., `"Locked - Key Required!"`).

---

## 3. Modality & Context Awareness

In accordance with the Phase 21 Input Management System:
1. **Gameplay Context**: Interaction prompts are fully active and respond to `InputAction::Interact`.
2. **Menu Context / Inventory Context / Pause Screen**: Interaction prompts are automatically hidden, and interact keypresses are suppressed to prevent accidental action bleed into the simulation.

