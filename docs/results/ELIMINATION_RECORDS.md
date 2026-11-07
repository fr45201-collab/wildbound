# ELIMINATION RECORDS & AUDIT TRAIL

## 1. Overview
The Elimination Records subsystem in Wildbound Phase 33 maintains an append-only, chronologically ordered log of all competitor eliminations occurring within an active match.

## 2. Elimination Record Schema

```cpp
struct EliminationRecord {
    uint32_t eventId = 0;              // Monotonic 1-based event counter
    uint32_t victimParticipantId = 0;  // Eliminated competitor ID
    std::string victimName;            // Competitor display callsign
    uint32_t killerParticipantId = 0;  // Killer competitor ID (0 if environmental/storm)
    std::string killerName;            // Killer display callsign or "Corruption Storm"
    match::EliminationCause cause;     // Combat, CorruptionStorm, Environment, Disconnect
    float matchElapsedTime = 0.0f;     // Monotonic match clock seconds at time of death
    uint32_t placement = 0;            // Final competitor rank (populated upon completion)
};
```

## 3. Guarantees & Constraints
1. **Uniqueness & Idempotency**:
   - Each elimination is recorded exactly once via `MatchResultManager::RecordEliminationEvent`.
   - Repeated death events or redundant damage ticks cannot produce duplicate elimination entries.
2. **Authoritative Killer Attribution**:
   - Attributed killer ID is forwarded directly from the Phase 5/19 `CombatSystem::GetEvents().OnDeath` dispatcher.
   - For Corruption Storm deaths, killer ID is set to `0` and killer name is recorded as `"Corruption Storm"`.
3. **Immutability After Finalization**:
   - Once `MatchResultManager::FinalizeMatchResult` is called, no further elimination events can be appended.

