# 2D Character Animation Subsystem

## 1. Overview

The Wildbound 2D animation subsystem decouples frame-rate timing, spritesheet slicing, and state machine logic. It supports four-directional spritesheet playback, velocity-driven state transitions, and fallback behaviors.

---

## 2. Directional Modeling & Biasing

Characters support four cardinal orientations:
* `Direction::Down`
* `Direction::Up`
* `Direction::Left`
* `Direction::Right`

When computing direction from a 2D velocity vector `(vx, vy)`:
1. If the magnitude is zero (`vx == 0 && vy == 0`), the character retains their **last facing direction**.
2. If `|vx| > |vy|`, horizontal movement dominates:
   * `vx > 0` -> `Direction::Right`
   * `vx < 0` -> `Direction::Left`
3. If `|vy| > |vx|`, vertical movement dominates:
   * `vy > 0` -> `Direction::Down`
   * `vy < 0` -> `Direction::Up`
4. **Diagonal Tie-Breaker**: When `|vx| == |vy|`, diagonal input can cause visual flutter between orientations. To preserve visual stability, `VectorToDirection` applies a bias towards the current facing direction. If the current facing matches one of the active axes, that axis is retained.

---

## 3. Movement States & State Transitions

Three movement states are tracked:
* `MovementState::Idle`: Velocity magnitude $\approx 0$.
* `MovementState::Walking`: Velocity magnitude $> 0$, shift modifier inactive.
* `MovementState::Running`: Velocity magnitude $> 0$, shift modifier active.

### Clip Mapping Matrix

| Movement State | Down | Up | Left | Right |
| :--- | :--- | :--- | :--- | :--- |
| **Idle** | `player_idle_down` | `player_idle_up` | `player_idle_left` | `player_idle_right` |
| **Walking** | `player_walk_down` | `player_walk_up` | `player_walk_left` | `player_walk_right` |
| **Running** | `player_run_down` | `player_run_up` | `player_run_left` | `player_run_right` |

---

## 4. Run-to-Walk Fallback Mechanism

If dedicated running animation clips are not authored or loaded in the `AssetManager`, the `AnimationStateMachine` automatically falls back to the corresponding walking clip and applies a **1.65× playback speed multiplier**:
$$\text{Playback Speed} = \frac{\text{Run Speed}}{\text{Walk Speed}} = \frac{165.0}{100.0} = 1.65$$

This ensures footstep cadences visually match the increased locomotion speed without causing animation stutter or missing frame errors.

---

## 5. Frame-Rate Independent Playback

`AnimationController` accumulates delta time (`dt`) independently of display refresh rate:
1. `m_elapsedTime += dt * m_playbackSpeedMultiplier;`
2. Frame index is resolved via modulo arithmetic:
   $$\text{frameIndex} = \lfloor \frac{\text{m\_elapsedTime}}{\text{frameDuration}} \rfloor \pmod{\text{totalFrames}}$$
3. Loop flags wrap back to index 0 smoothly.
