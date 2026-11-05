# HUD Architecture Specification

## 1. Architectural Overview

The Wildbound Heads-Up Display (`wildbound::ui::HUDManager`) presents vital, contextual, real-time battle royale information during gameplay without obstructing central sightlines.

The HUD operates under a strict **responsive anchor architecture** defined within the virtual $640 \times 360$ coordinate system:

```
(0, 0) Top-Left                                        (640, 0) Top-Right
  +-------------------------------------------------------------+
  | [VITALS CLUSTER]                        [MATCH INFO CLUSTER] |
  |  - Player Name / Status                  - Alive / Elim Count|
  |  - Health Bar & Lag Bar                  - Phase & Storm Time|
  |  - Armor Indicator & %                                      |
  |                                                             |
  |                                                             |
  |                                             [MINIMAP RADAR] |
  |                                              - 108 x 108 px |
  |                                              - Compass & Blip|
  |                                                             |
  |                                                             |
  | [WEAPON & AMMO CLUSTER]                                     |
  |  - Dual Weapon Slots (Main/Off)                             |
  |  - Cooldown Bar & Bow Charge                                |
  |  - Ammo Reserve Readout                                     |
  +-------------------------------------------------------------+
(0, 360) Bottom-Left                                (640, 360) Bottom-Right
```

---

## 2. HUD Screen Anchors & Metrics

| Anchor Region | Elements Contained | Virtual Bounds $(X, Y, W, H)$ |
|---|---|---|
| **Top-Left** | Player name badge, HP Bar, Lag Bar, Armor Indicator | $(12, 10, 180, 50)$ |
| **Top-Right** | Match Timer, Phase label, Alive count, Kills badge | $(490, 10, 140, 42)$ |
| **Bottom-Right (Upper)**| Minimap Radar, Compass marks, Zoom level tag | $(516, 60, 112, 112)$ |
| **Bottom-Left** | Weapon Slot 1 (Main), Slot 2 (Off), Bow Charge, Ammo Count | $(12, 300, 160, 48)$ |
| **Center-Bottom** | Contextual Interaction Prompt (`[E] Open Chest`) | $(220, 310, 200, 24)$ |
| **Screen Perimeter** | Critical Low-HP Pulse Screen Vignette | $(0, 0, 640, 360)$ |

---

## 3. Core Sub-Element Responsibilities

1. **Vitals Module**:
   - Immediate health drop + delayed catch-down "lag bar" (fighting game damage trace).
   - Pulsing crimson vignette on screen edges when $HP \le 25\%$.
   - Armor mitigation percentage ($1 - \frac{100}{100 + \text{Armor}}$) and defense points badge.
2. **Weapons & Equipment Module**:
   - Dual active/inactive weapon slots with border highlighting.
   - Attack cooldown sweep overlay.
   - Bow charge bar $[0.0..1.0]$ when charging ranged attacks.
   - Ammunition counter (equipped quiver count + backpack reserve).
3. **Minimap Radar Module**:
   - World-to-radar transformed blips for player, ground loot, and storm boundary.
   - Player orientation directional chevron.
   - Dynamic zoom toggle ($1.0\times, 1.5\times, 2.0\times$).
4. **Match Information Module**:
   - Real-time match clock.
   - Storm phase indicator (e.g., `"Phase 1"`, `"Phase 2"`).
   - Players alive and player elimination count.
5. **Interaction Prompt**:
   - Surfaces nearest interactable verb and glyph dynamically from `InteractionManager`.

---

## 4. Frame Update & Rendering Pipeline

`HUDManager::Update(float dt, const PlayerContext& context)` is executed every frame before rendering:

1. **Data Polling**:
   - Reads `HealthComponent` current and maximum HP.
   - Reads `EquipmentSystem` total armor rating.
   - Reads `WeaponManager` active weapon, ammo, and draw charge.
   - Reads `World` ground items within radar radius.
2. **Animation Tweens**:
   - Ticks `m_lagTimer` and decrements `m_lagHealth`.
   - Ticks `m_pulseTimer` for sinusoidal alpha oscillation when HP is critical.
   - Ticks toast notification entry/exit animations.
3. **Rendering Order**:
   - Vignette $\to$ Vitals Cluster $\to$ Weapons Cluster $\to$ Match Info $\to$ Minimap $\to$ Prompts.

