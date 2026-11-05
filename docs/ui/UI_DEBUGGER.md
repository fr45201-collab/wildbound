# UI Debugger Specification (DevUI Tab 18)

## 1. Overview & Purpose

The UI Debugger is integrated into `wildbound::ui::DevUI` under **Tab 18: "UI & HUD"**. It exposes comprehensive developer diagnostics, live state inspection, manual simulation controls, and visual override toggles for rapid iteration without needing to trigger live gameplay scenarios.

---

## 2. Controls & Diagnostic Panels

```
+-----------------------------------------------------------------------------------+
| ImGui: Wildbound Developer Console - Tab 18: [UI & HUD]                           |
+-----------------------------------------------------------------------------------+
|  [Screen State Selector]                                                          |
|  Active Screen: [ None (Gameplay) v ]                                             |
|  ( ) None  ( ) MainMenu  ( ) PauseMenu  ( ) SettingsMenu  ( ) Inventory           |
|                                                                                   |
|  --- VITALS & HUD TELEMETRY SIMULATOR ------------------------------------------  |
|  Simulate Player Health: [====                   ] 20 / 100 HP                    |
|  Trigger Test Damage:    [ -15 HP ]  [ -35 HP ]  [ -50 HP ]                       |
|  Trigger Test Heal:      [ +20 HP ]  [ +50 HP ]  [ Full HP ]                      |
|  Lag Health Value:       55.0 HP (Timer: 0.22s remaining)                         |
|  Simulate Armor Rating:  [=======                ] 45 Armor (Mitigation: 31.0%)   |
|                                                                                   |
|  --- WEAPONS & COMBAT TELEMETRY ------------------------------------------------  |
|  MainHand Cooldown:      [=====                  ] 0.35s / 1.0s                   |
|  Bow Draw Charge:        [=============          ] 0.65                           |
|  Equipped Arrows:        [ 18 ]  Reserve Arrows: [ 64 ]                           |
|                                                                                   |
|  --- MINIMAP RADAR DIAGNOSTICS -------------------------------------------------  |
|  [x] Show Minimap Radar                                                           |
|  Radar Zoom Mode:        ( ) 1.0x  (*) 1.5x  ( ) 2.0x                             |
|  Visible Ground Items:   12 detected within radar bounds                          |
|                                                                                   |
|  --- MATCH STATE SIMULATOR -----------------------------------------------------  |
|  Match Time:             [ 05:24 ]                                                |
|  Storm Phase:            [ Phase 3 ]                                              |
|  Alive Combatants:       [ 14 ]                                                   |
|  Player Eliminations:    [ 6 ]                                                    |
|                                                                                   |
|  --- NOTIFICATION QUEUE DISPATCHER ---------------------------------------------  |
|  Message Text:           [ Inventory full! Cannot pick up Iron Greatsword. ]      |
|  Notification Type:      ( ) Info  (*) Warning  ( ) Error  ( ) Success            |
|  [ Dispatch Test Toast Notification ]                                             |
+-----------------------------------------------------------------------------------+
```

---

## 3. Capabilities & Use Cases

### 3.1 Live Screen Switching
Developers can instantly switch between `None`, `MainMenu`, `PauseMenu`, `SettingsMenu`, and `Inventory` via radio buttons, immediately validating modal backdrop rendering, focus transitions, and layout alignment.

### 3.2 Vitals & Lag Bar Tuning
- Buttons for `[-15 HP]`, `[-35 HP]`, and `[-50 HP]` allow immediate verification of the $0.5\text{s}$ pause timer and $65\text{ HP/s}$ catch-down velocity.
- Sliding health below $25\text{ HP}$ immediately triggers and verifies the pulsing crimson screen vignette without entering combat.

### 3.3 Minimap Blip Verification
- Visualizes the number of detected `WorldItem` ground loot drops.
- Verifies world-to-minimap coordinate projection and clipping circles.

### 3.4 Toast Notification Tester
- Allows sending arbitrary string toasts across all four severity levels (`Info`, `Warning`, `Error`, `Success`).
- Tests entry slide animation, duration clamping, queue overflow, and alpha fadeout.

