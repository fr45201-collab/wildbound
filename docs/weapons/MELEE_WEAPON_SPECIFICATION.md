# Melee Weapon Specification — Wildbound Phase 24

## 1. Overview & Mechanics

Melee combat in Wildbound simulates precise, tactical weapon swings with distinct timing windows, directional cones, stamina consumption, and multi-target cleave capabilities.

### Key Principles
1. **Directional Arc Sweeps**: Attacks cleave in a fan/cone oriented along the player's facing angle ($\theta$).
2. **Deterministic State Lifecycle**: Every melee attack progresses through 4 explicit temporal phases:
   - **Windup / Prep** ($T_{prep}$): Anticipation frames. Player committed, hitbox inactive.
   - **Active Swing** ($T_{active}$): Hitbox active in world. Overlaps checked. Hits dealt.
   - **Recovery** ($T_{recov}$): Follow-through. Hitbox inactive. Cannot attack or switch weapons.
   - **Cooldown** ($T_{cd}$): Neutral animation resumed, awaiting cadence reset before next attack.
3. **Multi-Target Cleave with Single-Hit Guarantee**: A single swing can strike multiple enemies in range, but each enemy entity receives damage at most once per swing.

```
[ Attack Trigger ]
       │
       ▼
┌──────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  WINDUP      │ ──> │  ACTIVE      │ ──> │  RECOVERY    │ ──> │  COOLDOWN    │ ──> [ IDLE ]
│  (0.12s)     │     │  (0.15s)     │     │  (0.18s)     │     │  (variable)  │
└──────────────┘     └──────────────┘     └──────────────┘     └──────────────┘
Hitbox: OFF          Hitbox: ON           Hitbox: OFF          Hitbox: OFF
Can Switch: NO       Can Switch: NO       Can Switch: NO       Can Switch: YES
```

---

## 2. Attack Timing & Lifecycle Equations

For a weapon with attack cooldown duration $T_{total} = \text{stats.attackCooldown}$:
* $T_{prep} = \min(0.12, T_{total} \times 0.20)$
* $T_{active} = \min(0.15, T_{total} \times 0.25)$
* $T_{recov} = \min(0.18, T_{total} \times 0.25)$
* $T_{cd} = \max(0.0, T_{total} - (T_{prep} + T_{active} + T_{recov}))$

During $T_{active}$, the hit area is evaluated as a circular sector:
$$\text{Sector}(P, R, \theta, \alpha) = \{ x \in \mathbb{R}^2 \mid \|x - P\| \le R \land |\text{angle}(x - P) - \theta| \le \frac{\alpha}{2} \}$$
where:
* $P$ = Attacker center position $(x, y)$
* $R$ = Weapon reach radius (`stats.reach`)
* $\theta$ = Attacker aim / facing angle
* $\alpha$ = Weapon swing arc angle (`stats.swingArcAngle`)

---

## 3. Hit Deduplication Mechanism

To prevent multi-frame damage ticking while an enemy remains inside the active swing sector:
1. `WeaponController` maintains `m_targetsHitThisSwing` (`std::unordered_set<uint64_t>`).
2. Upon transitioning into `WeaponAttackState::Windup` or starting a new attack, `m_targetsHitThisSwing.clear()` is called.
3. When `CombatSystem::ProcessAttack` queries the spatial index or combatants:
   ```cpp
   if (attacker->HasHitTargetThisAttack(target->GetEntityId())) {
       continue; // Skip: target already received damage this swing
   }
   // Apply damage...
   attacker->RecordHitTargetThisAttack(target->GetEntityId());
   ```
4. This ensures clean, deterministic hit counts regardless of simulation tick rate (60 Hz, 120 Hz, or variable).

---

## 4. Stamina Integration

* Every melee attack requires stamina: `stats.staminaCost`.
* If current stamina $< \text{staminaCost}$, attack cannot initiate (`CanAttack()` returns `false`).
* On successful swing initiation, stamina is immediately deducted from the combatant's stamina pool.

