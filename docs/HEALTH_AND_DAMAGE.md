# Wildbound Health & Damage Architecture

The Health and Damage Subsystem encapsulates entity vitality, invulnerability frames, damage resistance, and authoritative combat calculation in Wildbound.

---

## 1. HealthComponent

Defined in [`src/combat/HealthComponent.hpp`](file:///c:/dev/Wildbound/src/combat/HealthComponent.hpp) and implemented in [`src/combat/HealthComponent.cpp`](file:///c:/dev/Wildbound/src/combat/HealthComponent.cpp), `HealthComponent` is a self-contained component managing current and maximum health values.

### State & Capabilities
- **Clamped Health**: Current health is strictly clamped between `[0.0f, maxHealth]`.
- **Health Percent**: Exposes `GetHealthPercent()` returning normalized vitality `[0.0f, 1.0f]`.
- **Invulnerability Period**: Optional temporary invulnerability frames (e.g. after receiving a heavy blow) during which further incoming damage is ignored.
- **Passive Health Regeneration**: Configurable regeneration rate (HP per second) with enable/disable flags.
- **Death State**: `IsDead()` evaluates whether health has reached zero or the entity has died.

### Data Contract (`HealthConfig`)
```json
{
  "max_health": 50.0,
  "initial_health": 50.0,
  "regeneration_enabled": false,
  "regeneration_rate": 0.0
}
```

---

## 2. Damage Types

Defined in [`src/combat/CombatTypes.hpp`](file:///c:/dev/Wildbound/src/combat/CombatTypes.hpp) and loaded from `data/combat/damage_types.json`:

```cpp
enum class DamageType {
    Physical,       // Slashing, piercing, and bludgeoning attacks
    Magical,        // Spells, elemental fire, cold, arcana
    Environmental,  // Thorns, acid pools, falling damage, lava
    TrueDamage      // Bypasses armor and damage reduction entirely
};
```

---

## 3. Damage Calculation Pipeline (`DamageSystem`)

All damage is computed authoritatively by `wildbound::combat::DamageSystem`. The calculation uses the following formula:

$$\text{effectiveDamage} = (\text{baseDamage} \times \text{typeMultiplier} \times \text{critMultiplier}) - \text{armorReduction}$$

```mermaid
flowchart LR
    Input[Attack Request] --> Base[Base Damage]
    Base --> TypeMod[Type Multipliers]
    TypeMod --> Crit[Crit Check]
    Crit --> Armor[Armor / Defense Reduction]
    Armor --> Clamp[Clamp Min 1.0]
    Clamp --> Output[Final Damage Event]
```

### Steps:
1. **Invulnerability Verification**: If the target has active invulnerability frames, damage resolution is aborted immediately and returns zero.
2. **Type Multipliers**: Base damage is scaled by attacker damage attributes.
3. **Critical Hit Evaluation**: If `isCritical` is true, the damage is multiplied by the critical strike multiplier (default $1.5\times$).
4. **Defense Reduction**: Flat armor reduces physical damage, while resistance percentages reduce elemental damage. Minimum damage is clamped to $1.0$ (unless completely immune).
5. **Application**: `targetHealth.ApplyDamage(finalDamage)` mutates health and emits a `DamageEvent`.

---

## 4. Death Handling & Deduplication

In high-speed combat with multi-hit attacks, multiple damage instances can land in the same frame. The `DeathHandler` component ensures that:
- An entity can only die once.
- Only a single `DeathEvent` is ever broadcast per entity lifecycle.
- Once dead, subsequent attacks against that entity's hurtbox return false and produce no damage events.

