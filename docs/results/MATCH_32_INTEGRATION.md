# WILDBOUND — MATCH LIFECYCLE (PHASE 32) INTEGRATION ARCHITECTURE

## 1. Architectural Scope & Boundary Enforcement

Phase 32 established the core Match Lifecycle infrastructure for Wildbound:
- Match state transitions (`Pending`, `Countdown`, `Active`, `SuddenDeath`, `Completed`, `Aborted`).
- Participant registration, spawn point allocation, and participant lifecycle.
- In-match countdown timers, storm phase progression, and alive competitor counting.
- Raw elimination detection when a participant's health reaches zero.

Phase 33 deliberately **does not** replace or duplicate these systems. Instead, Phase 33 extends the lifecycle engine by acting as the analytical, telemetry, placement calculation, and post-match presentation layer.

```
       +-------------------------------------------------------+
       |             Phase 32: MatchManager                    |
       |  - Match Lifecycle FSM (Pending -> Active -> Complete)|
       |  - Participant Registry & Spawning                   |
       |  - Real-Time Tick & Storm Update                     |
       +---------------------------+---------------------------+
                                   |
                         Events & Life Cycle
                                   |
                                   v
       +-------------------------------------------------------+
       |             Phase 33: MatchResultManager              |
       |  - Telemetry & Damage Separation (Storm vs Combat)    |
       |  - Chronological Elimination Log Audit               |
       |  - Tournament Placement Calculation (Olympic Ties)    |
       |  - Victory / Defeat Formal Evaluation                 |
       |  - Immutable MatchResultModel Generation              |
       |  - Local JSON Persistence to Disk                     |
       +---------------------------+---------------------------+
                                   |
                                   v
       +-------------------------------------------------------+
       |                 UIManager / DevUI                     |
       |  - High-Fidelity Results Screen (Victory/Defeat/Draw) |
       |  - Tabbed Overview & Leaderboard Browsing             |
       |  - ImGui Tab 20 Diagnostics & Log Inspector           |
       +-------------------------------------------------------+
```

---

## 2. Integration Touchpoints in `MatchManager`

### 2.1 Initialization & Reset
Inside `MatchManager::Initialize(const MatchConfiguration& config)` and `MatchManager::Reset()`:
- `m_resultManager.Reset()` is invoked, purging all previous match logs, individual player statistics, and cached result models.
- If a match is started, `m_resultManager.Initialize(config, localPlayerId)` initializes the tracker, assigning target IDs and establishing the local player's perspective.

### 2.2 Damage Attribution & Tracking
In `MatchManager::RecordDamage(uint32_t victimId, uint32_t attackerId, float damage, bool isStorm)`:
- Phase 32 tracked aggregated damage per participant.
- Phase 33 enhanced `MatchParticipant` with separated telemetry:
  - `combatDamageReceived`
  - `stormDamageReceived`
- The `isStorm` flag is propagated through `MatchStatistics::RecordDamage` directly into `m_resultManager.RecordDamage()`, guaranteeing strict isolation between combat encounters and environmental storm attrition.

### 2.3 Elimination Processing
When a participant's health reaches zero, `MatchManager::RecordElimination(uint32_t victimId, uint32_t killerId, EliminationCause cause)`:
- Determines whether the elimination was inflicted by another competitor, by the Corruption Storm, or by suicide/hazard.
- Resolves weapons and equipment used by querying participant inventories.
- Invokes `m_resultManager.RecordElimination(...)`, adding a timestamped `EliminationRecord` to the chronological audit trail.
- Checks victory conditions in real time using `wildbound::results::VictoryEvaluator::Evaluate(...)`.

### 2.4 Match Completion & Finalization
In `MatchManager::CompleteMatch()`:
- `VictoryEvaluator` calculates the authoritative `MatchOutcome` (`Victory`, `Defeat`, `Draw`, `Cancelled`, `Failed`).
- `ResultCalculator::CalculatePlacements()` processes all registered participants:
  - Ranks competitors by survival duration (last to die = 1st place).
  - Handles simultaneous eliminations cleanly via Olympic tie logic (shared rank, subsequent rank skipped).
- `m_resultManager.FinalizeMatch(outcome, winningParticipantId)` builds the immutable `MatchResultModel`.
- `ResultSerializer::SaveResultToFile()` writes the resulting JSON payload asynchronously/cleanly into `user_data/match_history/<match_id>.json`.
- The game state transitions to `GameState::MatchResults`, passing the final model to `UIManager` for presentation.

---

## 3. Guarantees & Invariants

1. **Single Source of Truth**: `MatchManager` remains the authoritative owner of match state; `MatchResultManager` queries state from `MatchManager` and never mutates match timers or participant health directly.
2. **Deterministic Placement Calculation**: Placements are calculated strictly from recorded survival duration and elimination timestamps, ensuring replayability and reproducible ranking.
3. **No Dynamic Memory Leaks**: All result models use fixed or value-type STL structures (`std::vector`, `std::string`, `std::chrono`) with clear RAII semantics and standard move operations.
4. **Resilience to Early Abort**: If a match is terminated prematurely via pause menu or console (`AbortMatch()`), `VictoryEvaluator` assigns `MatchOutcome::Cancelled`, preventing crashes, incomplete JSON schemas, or corrupted leaderboard files.

