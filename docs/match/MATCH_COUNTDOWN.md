# Match Countdown & Action Inhabitation Specification

## 1. Pre-Match Countdown Sequence

The pre-match countdown phase (`MatchState::Countdown`) provides a synchronous staging period (default 10 seconds, configured via `match_countdown_seconds`) where participants are spawned in the world while active combat and unauthorized interactions remain restricted.

```
+-------------------------------------------------------------------------------+
| HUD COUNTDOWN OVERLAY                                                         |
|                                                                               |
|                            MATCH STARTING IN:                                 |
|                                  [ 05 ]                                       |
|                                                                               |
|               [ ======================================= ]                     |
|                                                                               |
|          * Weapons Holstered  * Movement Locked  * Storm Inactive             |
+-------------------------------------------------------------------------------+
```

---

## 2. Gameplay & Input Invariant Controls

During `MatchState::Countdown`, `MatchState::Starting`, `MatchState::Finishing`, and `MatchState::Completed`:
- `MatchManager::IsGameplayAllowed()` returns `false`.
- The game input context suppresses attacks, interactions, projectile firing, and consumable usage.
- Menu, settings, pause, and developer debug UI input contexts remain fully operational.

---

## 3. Transition to Active

When `m_timeRemaining` decreases to $0.0\text{s}$:
1. `MatchCountdown::IsFinished()` becomes `true`.
2. `MatchManager` transitions `Countdown -> Starting -> Active`.
3. `IsGameplayAllowed()` becomes `true`, enabling full character input and combat capabilities.
4. `StormManager::StartStorm()` triggers phase 1 safe zone warnings.
5. Audio event `"Combat.Start"` is dispatched.

