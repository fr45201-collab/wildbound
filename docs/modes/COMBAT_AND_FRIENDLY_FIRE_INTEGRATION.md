# WILDBOUND — COMBAT & FRIENDLY FIRE INTEGRATION

## 1. Architectural Integration

Friendly-fire handling is integrated directly into Wildbound's `CombatSystem` without modifying core attack definitions or damage formulas.

```
       Attacker Performs Attack (Sword / Bow / Projectile)
                                |
                                v
                HitDetection Intersects Hurtbox
                                |
                                v
               CombatRules::CanTarget(Attacker, Target)
                                |
                                v
            Does friendlyFireFilter Callback Exist?
               |                                |
               | YES                            | NO
               v                                v
    Query GameModeManager                 Standard Faction Check
   CanDamage(Attacker, Target)             (Player vs Monster, etc.)
               |
         +-----+-----+
         |           |
      IsTeammate   Not Teammate
         |           |
         v           v
    Check Mode    Allow Damage
  Friendly-Fire
```

---

## 2. Invariants & Guarantees

1. **Teammate Immunity**: When friendly fire is disabled (the default for Duos and Squads), both melee slashes and ranged projectiles pass through or cause zero damage to allied participants.
2. **Accurate Attribution**: When damage is allowed, full attribution is credited to the attacking participant in `MatchStatistics` and `MatchResultManager`.
3. **No Combatant Class Pollution**: The `Combatant` base class remains unaware of match concepts. Friendly fire is evaluated via participant lookup through `CombatantId`.

