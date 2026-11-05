# Wildbound — Input Buffering Specification

## 1. Motivation & Combat Responsiveness

In fast-paced 2D combat, frame rate fluctuations or brief animation locks (e.g. recovery frames from an attack swing or dodge roll) can cause immediate button presses to be dropped if evaluated strictly frame-by-frame. 

The `InputBuffer` (`src/input/InputBuffer.hpp`) guarantees that player inputs intended for an upcoming action window are captured, timestamped, and stored for consumption when the character system becomes receptive.

---

## 2. Buffer Architecture & Expiration

```
Player Pressed [Attack]
           |
           v
+---------------------------------------------------------+
| BufferedEntry                                           |
|  - action:       InputAction::Attack                    |
|  - timestamp:    t = 12.450 s                           |
|  - duration:     0.120 s (120 ms)                       |
|  - consumed:     false                                  |
+---------------------------------------------------------+
           |
           | Frame Updates (t += dt)
           v
CharacterController ready to execute attack:
  -> HasBufferedAction(Attack) == true
  -> ConsumeBufferedAction(Attack) -> sets consumed = true
  -> Executes attack animation & hitbox
```

### 2.1 Configurable Action Windows
Different action archetypes utilize tailored expiration durations:

| Action Archetype | Actions | Window Duration | Rationale |
|:---|:---|:---|:---|
| Fast Combat | `Attack`, `HeavyAttack` | 120 ms | Tight enough to avoid accidental stale swings; forgiving on input cadence. |
| Tactical Evasion | `Dodge` | 150 ms | Allows buffering roll out of damage stun or attack recovery. |
| World Interaction | `Interact` | 150 ms | Smooth loot pickup / door opening while moving towards target. |
| Consumables | `QuickHeal` | 200 ms | Reliable activation during tense battle royale skirmishes. |

### 2.2 Deduplication & Single Consumption
* `ConsumeBufferedAction(action)` sets an internal flag (`consumed = true`) and removes the active record. An action is **never executed twice** from a single button press.
* Adding an action that already exists in the buffer refreshes its timestamp and remaining duration rather than creating redundant queue entries.
* Transitioning between input contexts (e.g., opening a menu or dying) triggers `ClearCategory(ActionCategory::Combat)`, purging combat buffers to avoid post-menu action firing.

