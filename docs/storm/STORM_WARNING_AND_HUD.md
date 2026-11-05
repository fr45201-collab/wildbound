# Storm Warning and HUD Integration

## 1. HUD Visual Layout

The HUD incorporates real-time storm state information via `ui::HUDManager` and `storm::StormManager`.

```
+-------------------------------------------------------------------------------+
| TOP-CENTER NOTIFICATION BANNER                                                |
|                                                                               |
|        [ ICON ]  PHASE 2 - ADVANCING STORM  |  CLOSING IN: 01:15              |
|        +----------------------------------------------------------+           |
|        | [============ Progress Bar =========================]    |           |
|        +----------------------------------------------------------+           |
+-------------------------------------------------------------------------------+
| STATUS INDICATORS (When Outside Safe Perimeter)                               |
|                                                                               |
|        (!) YOU ARE OUTSIDE THE SAFE ZONE!                                     |
|        Taking 2.0 HP/sec  |  Distance to safety: 42.5m                        |
|                                                                               |
|                   [ ---> ] DIRECTIONAL COMPASS                                |
+-------------------------------------------------------------------------------+
```

---

## 2. Dynamic Banner States

The top notification banner shifts color, iconography, and pulsing behavior based on storm status:
- **Phase Warning State**:
  - Icon: `icon_storm_approaching`
  - Text: `Phase N - <Title> (Wait)`
  - Color: Electric Gold / Yellow `(255, 215, 0, 255)`
  - Displays remaining countdown seconds until shrinking commences.
- **Phase Shrinking State**:
  - Icon: `icon_zone_shrinking`
  - Text: `Phase N - <Title> (Shrinking)`
  - Color: Urgent Crimson / Orange `(255, 90, 40, 255)`
  - Displays countdown until current boundary reaches the target perimeter.
- **Outside Safe Zone Alert**:
  - Icon: `icon_outside_zone`
  - Text: `WARNING: CORRUPTION STORM DAMAGE`
  - Color: Danger Magenta `(255, 40, 180, 255)` with 4Hz flashing alpha.
  - Accompanied by full-screen boundary vignette and directional pointer.
- **Final Circle Collapse**:
  - Icon: `icon_final_zone`
  - Text: `FINAL PHASE - TOTAL COLLAPSE`
  - Color: Ultra-Violet Flashing `(220, 20, 60, 255)`.

---

## 3. Data Synchronization Pipeline

1. In `Game::Update(float dt)`:
   ```cpp
   m_stormManager->Update(dt, m_player);
   m_stormManager->SyncWithHUD(m_hudManager);
   ```
2. In `HUDManager::Render(dt)`:
   The banner draws using high-resolution font rendering, calibrated screen-space anchors, and layout scaling compatible with 720p, 1080p, 1440p, and 4K viewports.

