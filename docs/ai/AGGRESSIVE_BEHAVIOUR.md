# Wildbound — Aggressive Behaviour

## 1. Overview & Combat Engagement

Aggressive behaviour coordinates target tracking, pursuit navigation, closing distance, and executing combat strikes against perceived enemies. It governs the `AIState::Chase` and `AIState::Attack` states.

---

## 2. Pursuit Mechanics (`AIState::Chase`)

### 2.1 Dynamic Re-pathing Cadence
Target players and moving entities constantly shift their world coordinates. Re-pathing on every single frame ($60 \text{ Hz}$) is computationally wasteful. The controller implements an adaptive throttling timer:
- **Baseline Re-path Interval**: $0.25\text{s}$ (4 updates per second).
- **Proximity Acceleration**: When target distance $< 64.0\text{px}$, interval scales down to $0.10\text{s}$ for sharp cornering.
- **LOS Straight Line**: If line of sight is completely clear and unobstructed, pathfinding is bypassed entirely, setting velocity directly towards $\mathbf{p}_{\text{target}}$ with dynamic avoidance enabled.

### 2.2 Pursuit Leashing
To prevent enemies from being dragged across the entire map, each entity archetype enforces a maximum pursuit distance from its spawn anchor ($D_{\text{leash}} \in [250.0\text{px}, 500.0\text{px}]$):
- When $\|\mathbf{p}_{\text{entity}} - \mathbf{p}_{\text{spawn}}\| > D_{\text{leash}}$, the entity drops aggression, clears target memory, and transitions to `AIState::Return`.

---

## 3. Attack Mechanics (`AIState::Attack`)

### 3.1 Attack Range & Conditions
Attack execution triggers when:
1. Target is within weapon reach: $d_{\text{target}} \le R_{\text{attack}}$ (Melee: $24.0\text{px} - 32.0\text{px}$, Ranged: $120.0\text{px} - 180.0\text{px}$).
2. Line of sight between attacker and target is completely open.
3. Attack cooldown timer has satisfied $t_{\text{cooldown}} \ge \frac{1.0}{\text{AttacksPerSecond}}$.

### 3.2 Combat Integration
The `AIController` interfaces directly with Phase 5/24 combat systems:
- Calls `CombatSystem::ProcessAttack` or triggers the entity's weapon attack animation.
- Applies attacker facing direction directly to the weapon swing hitbox.
- Audio cues (e.g. `sound_melee_swing_01`, `sound_arrow_shoot_01`) trigger through `AudioManager`.
- Visual effects (blood splatters, slash arcs) spawn through the particle system.

