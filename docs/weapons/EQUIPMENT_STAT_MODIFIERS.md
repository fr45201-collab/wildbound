# Equipment Stat Modifiers — Wildbound Phase 24

## 1. Aggregation Model

Equipping gear modifies a combatant's attributes via the `EquipmentStatModifiers` struct in `src/entities/EquipmentSystem.hpp`:

```cpp
struct EquipmentStatModifiers {
    float totalDefense{0.0f};          // Sum of armor values across all worn pieces
    float damageMitigationPercent{0.0f};// Computed % mitigation: totalDefense / (totalDefense + 100.0)
    float maxHealthBonus{0.0f};        // Additional maximum hit points
    float speedModifierPercent{0.0f};   // Speed delta % (e.g. -5.0 for heavy plate, +10.0 for swift boots)
    float attackSpeedModifierPercent{0.0f}; // Attack rate delta %
};
```

---

## 2. Damage Mitigation Calculation

To prevent linear defense scaling from causing total invulnerability, Wildbound implements standard diminishing returns:

$$\text{MitigationPercent} = \frac{\text{totalDefense}}{\text{totalDefense} + 100.0} \times 100\%$$

* At $\text{Defense} = 25$: Mitigation = $\frac{25}{125} = 20.0\%$
* At $\text{Defense} = 50$: Mitigation = $\frac{50}{150} = 33.3\%$
* At $\text{Defense} = 100$: Mitigation = $\frac{100}{200} = 50.0\%$
* At $\text{Defense} = 200$: Mitigation = $\frac{200}{300} = 66.7\%$

When damage is inflicted in `DamageSystem` / `CombatSystem`:
$$\text{DamageDealt} = \text{RawDamage} \times (1.0 - \text{MitigationPercent})$$

---

## 3. Movement Speed Integration

Worn armor items can add or penalize movement speed:
* Heavy plate armor applies a $-3\%$ to $-6\%$ speed penalty per piece.
* Light leather or boots can grant a $+5\%$ to $+10\%$ speed bonus.

In `Player::Update(float dt)`:
$$\text{EffectiveSpeed} = \text{BaseSpeed} \times \left(1.0 + \frac{\text{modifiers.speedModifierPercent}}{100.0}\right)$$

This ensures tactical trade-offs: heavy armor provides immense survivability in skirmishes but reduces traversal agility in the battle royale zone.

