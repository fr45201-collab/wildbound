# Weapon Animation Integration — Wildbound Phase 24

## 1. Animation State Synchronization

Character animation in Wildbound is driven by `AnimationController` and `AnimationStateMachine` in coordination with the player's `WeaponController`.

When an attack or draw cycle is initiated, the animation state transitions to reflect the active armament:

```
[ Movement State ] ──( Attack Action )──> [ Attack State (Prep -> Active -> Recovery) ]
        ▲                                                      │
        └──────────────────( State == Cooldown )───────────────┘
```

---

## 2. Directional Facing and Weapon Anchoring

The character's body sprite and equipped weapon sprite are rendered in layers according to facing direction:
* **Facing Down / Left / Right**: Weapon renders in the foreground (`Layer::WeaponForeground`).
* **Facing Up**: Weapon renders behind the torso (`Layer::WeaponBackground`).

Anchors are specified relative to the 64x64 modular character canvas:
* Right hand anchor: `(42, 34)`
* Left hand anchor (shield / offhand): `(22, 34)`
* Quiver anchor (Back slot): `(32, 28)`

---

## 3. Weapon Swing Rendering & Trail Effects

During `WeaponAttackState::Active`:
1. The melee sword rotates through an angular interpolation:
   $$\theta(t) = \theta_{\text{start}} + (\theta_{\text{end}} - \theta_{\text{start}}) \times \frac{t - T_{\text{prep}}}{T_{\text{active}}}$$
2. An arc slash trail is rendered using raylib semi-transparent triangles (`Color{ 255, 255, 255, 140 }`), visually communicating the dangerous hit volume to both the attacker and defenders.
3. For bows during `m_isDrawing`, the bow sprite scales along the draw axis or swaps to drawn-string animation frames based on `GetDrawProgress()`.

