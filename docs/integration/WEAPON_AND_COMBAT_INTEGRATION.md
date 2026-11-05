# Weapon and Combat Integration

## 1. Overview

Wildbound's combat system integrates swords (melee) and bows (ranged projectiles) into a unified attack execution pipeline with authoritative hit detection and defense mitigation:

- **Melee Combat**: Executed via [`CombatSystem::ProcessAttack`](file:///c:/dev/Wildbound/src/combat/CombatSystem.hpp) generating oriented sweep hitboxes offset from the player's facing direction.
- **Ranged Combat**: Equipping a Bow in the `EquipmentSlot::Weapon` overrides directional melee swings and activates [`ProjectileSystem`](file:///c:/dev/Wildbound/src/combat/ProjectileSystem.hpp) to launch physical flying arrow entities.
- **Weapon Catalog**: 12 production weapon designs loaded by [`WeaponManager`](file:///c:/dev/Wildbound/src/weapons/WeaponManager.hpp) from `data/weapons/weapon_catalog.json`.

---

## 2. Combat Execution Flow

```
                  Player Input (Attack Key / Left Click)
                                  |
                                  v
                        Player::TryAttack()
                                  |
            +---------------------+---------------------+
            |                                           |
    Equipped Weapon is Sword                  Equipped Weapon is Bow
            |                                           |
            v                                           v
  CombatSystem::ProcessAttack()             ProjectileSystem::SpawnArrow()
  - Oriented AABB Hitbox                    - Velocity: 480 px/s
  - Overlap with Enemy Hurtboxes            - Rotation matching facing
  - Dispatches "Combat.SwordSlash"          - Dispatches "Combat.BowShoot"
            |                                           |
            +---------------------+---------------------+
                                  |
                                  v
                      DamageSystem::CalculateDamage()
                      - Base attack damage
                      - Target armor defense mitigation:
                        defense / (defense + 100)
                      - Critical hit evaluation
                                  |
                                  v
                      HealthComponent::ApplyDamage()
                      - Dispatches "Combat.Hit" / "Combat.BowHit"
                      - Triggers DeathHandler on HP <= 0
```

---

## 3. Projectile Subsystem Architecture

The [`ProjectileSystem`](file:///c:/dev/Wildbound/src/combat/ProjectileSystem.hpp) provides high-performance simulation of physical ranged projectiles:

### 3.1 Projectile State
```cpp
struct Projectile {
    uint64_t id;
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float baseDamage;
    DamageType damageType;
    CombatantId sourceId;
    Faction faction;
    float lifetime;
    float maxLifetime;
    float hitRadius;
    std::string textureId;
    bool active;
};
```

### 3.2 Directional Arrow Spawning
When `Player::TryAttack` executes with a bow:
- Facing `Up`: $\text{velocity} = (0, -480)$, $\text{rotation} = -90^\circ$
- Facing `Down`: $\text{velocity} = (0, 480)$, $\text{rotation} = 90^\circ$
- Facing `Left`: $\text{velocity} = (-480, 0)$, $\text{rotation} = 180^\circ$
- Facing `Right`: $\text{velocity} = (480, 0)$, $\text{rotation} = 0^\circ$

### 3.3 Collision & Damage Resolution
Each simulation frame (`ProjectileSystem::Update(dt, combatants, audioSys)`):
1. **Movement**: $\text{position} \mathrel{+}= \text{velocity} \times dt$; $\text{lifetime} \mathrel{+}= dt$.
2. **Bounds & Lifetime Expiry**: If $\text{lifetime} \ge \text{maxLifetime}$ (default 2.0s), projectile expires.
3. **Combatant Intersection**: Evaluates circular bounding radius ($R = 14\text{px}$) against opposing faction combatant hurtboxes.
4. **Damage Application**: Calculates final damage via `DamageSystem::CalculateDamage` (incorporating target armor defense) and invokes `target->GetHealth().ApplyDamage()`.
5. **Audio Event**: Dispatches `"Combat.BowHit"`.

---

## 4. Weapon Catalog Integration

| Weapon ID | Display Name | Category | Base Damage | Attack Range | Cooldown |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `sword_wooden_training_001` | Wooden Training Sword | Sword | 12 | 28 px | 0.40s |
| `sword_rusty_short_001` | Rusty Short Sword | Sword | 18 | 32 px | 0.45s |
| `sword_iron_broadsword_001` | Iron Broadsword | Sword | 25 | 34 px | 0.50s |
| `sword_curved_001` | Curved Sword | Sword | 22 | 32 px | 0.38s |
| `sword_ancient_rune_001` | Ancient Rune Sword | Sword | 40 | 36 px | 0.45s |
| `bow_wooden_hunting_001` | Wooden Hunting Bow | Bow | 28 | 480 px (Range) | 0.65s |
| `bow_longbow_001` | Longbow | Bow | 38 | 560 px (Range) | 0.85s |
| `bow_recurve_composite_001` | Recurve Composite Bow | Bow | 34 | 520 px (Range) | 0.60s |
| `bow_elven_leaf_001` | Elven Leaf Bow | Bow | 45 | 600 px (Range) | 0.55s |

---

## 5. Verification & Tests

Covered by [`tests/integration/test_gameplay_integration.cpp`](file:///c:/dev/Wildbound/tests/integration/test_gameplay_integration.cpp):
- Equipping sword executes melee slash without spawning arrows.
- Equipping bow intercepts primary attack, spawns active arrow projectile with correct direction, velocity, and damage.
- Arrow flight update, hurtbox collision, and damage calculation verified.

