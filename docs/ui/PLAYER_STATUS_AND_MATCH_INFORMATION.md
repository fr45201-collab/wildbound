# Player Status and Match Information HUD Specification

## 1. Overview & Purpose

In a fantasy Battle Royale, player survival depends on situational awareness of both self-status and global match dynamics. The Player Status and Match Information HUD elements provide real-time updates on:
- **Match Clock**: Elapsed time and countdown until the next storm circle contraction.
- **Storm Phase**: Current zone phase number (e.g., `Phase 1`, `Phase 2`, `Final Circle`).
- **Alive Players Count**: Number of remaining combatants in the arena.
- **Elimination Count**: Player kills recorded in the current match session.
- **Player Status Badges**: Tactical debuffs or buffs (`Normal`, `Poisoned`, `Burned`, `Slowed`, `Defeated`).

---

## 2. Match Information Panel (Top-Right)

Positioned at $(X = 490, Y = 10, W = 140, H = 42)$:

```
+-------------------------------------------------------+
|  MATCH TIME: 04:12          [STORM PHASE 2: 01:30]    |
|  [ALIVE ICON] 18 Alive      [SKULL ICON] 4 Kills      |
+-------------------------------------------------------+
```

### Visual Styling
- **Panel Container**: Beveled rounded container (`#181820` with `#2A2A38` border, alpha 85%).
- **Time String**: Formatted as `MM:SS` using standard 60-second rollover.
- **Storm Countdown**:
  - Green (`#2ECC71`) when storm is stationary.
  - Flashing Orange (`#E67E22`) when storm timer $< 30\text{s}$.
  - Purple (`#9B59B6`) with label `"STORM CLOSING"` when the circle is actively shrinking.
- **Alive Counter**: Displayed next to an avatar silhouette icon.
- **Eliminations**: Displayed next to a crossed swords / skull icon with gold typography.

---

## 3. Player Status Badges (Top-Left Sub-Panel)

Rendered below the Armor bar in the Vitals Cluster:

```
[NORMAL]      - Default active gameplay state.
[POISONED]    - Green badge (#27AE60), flashes subtle damage ticks every 1.0s.
[BURNING]     - Fiery orange badge (#D35400), reduces defense rating.
[SLOWED]      - Frost blue badge (#3498DB), indicates reduced movement velocity.
[DEFEATED]    - Dark crimson badge (#C0392B), displayed on player incapacitation.
```

### Badge Transitions
- Badges automatically fade in with an ease-in alpha over $0.2\text{s}$ when a status effect is applied by `CombatSystem`.
- Multiple statuses display side-by-side with $4\text{ px}$ spacing.
- Hovering or inspecting status provides a short tooltip specifying duration and magnitude.

