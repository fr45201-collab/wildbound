# Pause Menu Specification

## 1. Overview & Purpose

The Pause Menu (`wildbound::ui::PauseMenu`) provides a modal, game-suspending interface invoked during active gameplay sessions. When opened, it completely freezes gameplay simulation (physics updates, enemy AI routines, projectile motion, and combat cooldown timers) while maintaining active rendering of the frozen world underneath a dark backdrop.

---

## 2. Modal Layout & Geometry

The Pause Menu is rendered as a clean, beveled dialog centered on the $640 \times 360$ virtual canvas:
- **Dialog Geometry**:
  - Origin: $X = 220, Y = 60$
  - Width: $200\text{ px}$
  - Height: $240\text{ px}$
- **Backdrop Dimming**:
  - Covers the entire screen with `Color{ 0, 0, 0, 160 }` (63% alpha), dimming world entities and HUD elements to focus visual hierarchy on the menu options.
- **Header**:
  - Text: `"GAME PAUSED"`
  - Font Size: $16\text{ px}$, Bold Gold `#D4AF37`.

```
+-----------------------------------------------------------------------------------+
|  [FROZEN GAMEPLAY WORLD - DIMMED 63%]                                             |
|                                                                                   |
|                         +-----------------------------+                           |
|                         |         GAME PAUSED         |                           |
|                         +-----------------------------+                           |
|                         |  [ Resume ]                 |                           |
|                         |  [ Inventory ]              |                           |
|                         |  [ Wardrobe ]               |                           |
|                         |  [ Settings ]               |                           |
|                         |  [ Main Menu ]              |                           |
|                         |  [ Quit Game ]              |                           |
|                         +-----------------------------+                           |
|                                                                                   |
+-----------------------------------------------------------------------------------+
```

---

## 3. Pause Menu Options & Navigation

| Menu Index | Action Name | Destination / Execution |
|---|---|---|
| 0 | **Resume** | Restores `UIScreenState::None`, resumes game clock and physics. |
| 1 | **Inventory** | Transitions to `UIScreenState::Inventory` with pause modal preserved underneath. |
| 2 | **Wardrobe** | Transitions to `UIScreenState::CosmeticsStore` for appearance modification. |
| 3 | **Settings** | Transitions to `UIScreenState::SettingsMenu`, setting previous state to `PauseMenu`. |
| 4 | **Main Menu** | Returns to `UIScreenState::MainMenu`, resetting match state. |
| 5 | **Quit Game** | Requests clean game exit. |

---

## 4. Simulation Freeze Mechanics

In `src/core/Game.cpp`, the update loop verifies pause state before ticking gameplay:
```cpp
bool isPaused = (m_uiManager->GetScreenState() == UIScreenState::PauseMenu ||
                 m_uiManager->GetScreenState() == UIScreenState::SettingsMenu);

if (!isPaused) {
    m_world->Update(deltaTime);
    m_combatSystem->Update(deltaTime);
    m_enemyManager->Update(deltaTime);
}
// UI is always updated with real wall-clock delta time
m_uiManager->Update(unscaledDeltaTime);
```

### Input Context Preservation
- When opening the Pause Menu, active movement vectors and attack actions are flushed (`InputManager::ResetActiveStates()`) to avoid sticky movement or attack buffering upon resumption.
- Directional navigation (`Up`, `Down`, `Select`, `Cancel`) is routed exclusively to `PauseMenu`.
- Pressing `Escape` or Gamepad `Start` / `B` immediately resumes gameplay.

