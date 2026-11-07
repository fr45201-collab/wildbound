# MATCH RESET AND CLEANUP ARCHITECTURE

## 1. Overview
The Match Reset and Cleanup subsystem in Wildbound Phase 32 guarantees complete determinism, state isolation, and zero memory/entity leakages across sequential matches. Battle Royale matches must be able to end, present results, and either start a new match ("Play Again") or return to the main menu without requiring an engine restart.

## 2. Formal Transition Flow

```
+-----------+       +-----------+       +---------------+
| Completed | ----> |  Cleanup  | ----> | Uninitialized |
+-----------+       +-----------+       +---------------+
                          |                     |
                          v                     v
                  Purge ephemeral        Ready for new
                  participant state      configuration
```

## 3. Subsystem Cleanup Responsibilities

During `MatchManager::ResetMatch(Player* player, StormManager* stormMgr, UIManager* uiMgr)`:

1. **State Machine Reset**:
   - Explicit transition from `Completed` (or any active state) -> `Cleanup` -> `Uninitialized`.
   - Prevents race conditions where incoming game events during unloading trigger match state mutations.

2. **Participant Registry Purging**:
   - Clears `m_participants`, `m_idToIndexMap`, and `m_entityIdToIndexMap`.
   - Resets total participant counts, alive counters, and winner designations.
   - Preserves no stale IDs across match instances.

3. **Spawner De-allocation**:
   - Calls `MatchSpawner::ResetSpawns()`.
   - Resets all candidate spawn points to `isOccupied = false` and `assignedParticipantId = 0`.
   - Clears any dynamically registered spawn points.

4. **Countdown Re-arming**:
   - Calls `MatchCountdown::Reset()`.
   - Halts timer progression and resets remaining seconds to configured duration.

5. **Statistics & Results Cleared**:
   - Calls `MatchStatistics::Reset()` and `MatchResult::Reset()`.
   - Clears damage tallies, kill attributions, survival timers, and leaderboard snapshots.

6. **Storm Subsystem Synchronization**:
   - When `stormMgr` is provided, calls `stormMgr->Reset()`.
   - Returns safe zones to Phase 0 (full map / 4000 unit radius).
   - Pauses/halts damage ticks and resets storm state to `Idle`.

7. **Local Player Reset**:
   - When `player` entity is provided, resets `HealthComponent` (100 HP) and clears active inventory/loot.
   - Resets death state flag to allow immediate respawning.

8. **UI Navigation**:
   - Invokes `ScreenChangeCallback("MainMenu")` unless "Play Again" requests an immediate transition into `Initializing`.

## 4. Leak & Stale State Prevention
Unit test suite 10 (`TestCompleteLifecycleRepeated`) validates 3 consecutive match lifecycles sequentially (`test_cycle_1`, `test_cycle_2`, `test_cycle_3`). The test proves:
- Zero participant ID collisions across repeated cycles.
- Exact spawn point reutilization without false occupancy blocks.
- Exact reset of survival duration and match timers to `0.0s`.

