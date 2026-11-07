# Active Match State & Gameplay Invariants

## 1. Active State Responsibilities

When the match transitions to `MatchState::Active`:
1. `m_matchTime` begins advancing monotonically ($t = \int dt$).
2. The Corruption Storm enters Phase 1 via `StormManager::StartStorm()`.
3. Player movement, weapon switching, melee/ranged combat, consumable usage, and world item collection are enabled.
4. Alive survival time accumulates on every tick for all active competitors:
   $$T_{\text{survival}} = T_{\text{match}}$$
5. Authoritative death events are intercepted and evaluated for eliminations.

---

## 2. Invariants During Active Play

1. **Authoritative Survivor Count**:
   `GetAliveCount()` accurately reflects living competitors registered in `MatchParticipantRegistry`.
2. **Combat Disablement for Eliminated Entities**:
   Any participant whose status becomes `Eliminated` is barred from dealing further damage, taking loot, or participating in combat actions.
3. **No Premature Winner Declaration**:
   A winner cannot be declared while $\text{AliveCount} > 1$, nor can a winner be declared before the match has officially transitioned to `Active`.
4. **Respawning Invariant**:
   `respawnEnabled` is `false` by default. Once eliminated, competitors remain in the eliminated state for the remainder of the match session.

