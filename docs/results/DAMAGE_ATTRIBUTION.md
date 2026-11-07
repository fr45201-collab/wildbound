# DAMAGE ATTRIBUTION SPECIFICATION

## 1. Overview
Accurate combat and environmental damage attribution is critical for tournament fairness and player telemetry. The Wildbound Phase 33 Damage Attribution pipeline integrates with the Phase 5/19 `DamageSystem` and Phase 31 `StormDamageSystem`.

---

## 2. Damage Source Taxonomy

```
                   Damage Event
                        |
            +-----------+-----------+
            |                       |
            v                       v
      Direct Combat          Corruption Storm
     (Player/Bot Attack)      (Environmental Tick)
            |                       |
            v                       v
   attacker.damageDealt      victim.stormDamageReceived
   victim.combatDamageRecv   victim.damageReceived
   victim.damageReceived
```

---

## 3. Attribution Invariants
1. **Target Acceptance Bound**:
   - Damage dealt is recorded strictly after target mitigation and capping (damage dealt cannot exceed health lost).
2. **Double-Counting Prevention**:
   - Each damage event is processed once via `MatchStatistics::RecordDamage`.
   - `totalDamageReceived == combatDamageReceived + stormDamageReceived`.
3. **Storm Separation**:
   - Environmental storm ticks explicitly pass `isStorm = true`.
   - Storm damage increments `victim.stormDamageReceived` without fabricating an attacker participant ID.
4. **Killer Attribution Validation**:
   - A killer is only credited if their authoritative participant ID is established by `CombatSystem::GetEvents().OnDeath`.
   - If an entity dies with no killer attributed (e.g. out of bounds), killer ID remains `0`.

