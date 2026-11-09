# WILDBOUND — PHASE 50E STAGE 3 INVESTIGATION REPORT
## Root Cause & Determinism Investigation for DEF-008

**Parent Phase:** Phase 50 — Global System Integration, Stability & Production Validation  
**Phase:** 50E Stage 3  
**Target Defect:** DEF-008 — Match Lifecycle Coincident Elimination Placement Race  
**Investigation Date:** 2026-11-08  
**Components Investigated:**
- `tests/match/test_match_lifecycle.cpp`
- `src/results/ResultCalculator.cpp`
- `src/match/MatchManager.cpp`
- `src/match/MatchEliminationSystem.cpp`
- `tests/results/test_match_results.cpp`

---

## 1. Executive Summary & Root Cause Confirmation

The historical hypothesis regarding DEF-008 was confirmed:

1. **Failure Location**: `tests/match/test_match_lifecycle.cpp:345` (`assert(p3Final->placement == 3)`).
2. **Root Cause**: The test fixture in `TestWinnerDetermination()` intended to simulate a chronological sequence of eliminations:
   - Comment: `// Eliminate P3 at t = 10s`
   - Comment: `// Eliminate P2 at t = 20s`
   However, the test author failed to advance the match clock (`m_matchTime`) between the two eliminations. In `MatchManager`, `m_matchTime` increments exclusively during `mgr.Update(dt)`. Because `ProcessElimination` was invoked for both participants back-to-back without an intervening `mgr.Update()`, both events were timestamped at $t = 0.1\text{s}$ ($\Delta t = 0.0\text{s}$).
3. **Production Placement Behavior**:
   In `src/results/ResultCalculator.cpp:72`, standard tournament competition ranking ("1224" ranking) is implemented:
   ```cpp
   while (j < eliminatedList.size() &&
          std::abs(eliminatedList[j]->eliminationInfo.timestamp - eliminatedList[i]->eliminationInfo.timestamp) < 0.001f) {
       ++j;
   }
   for (size_t k = i; k < j; ++k) {
       eliminatedList[k]->placement = currentRank;
   }
   currentRank += static_cast<uint32_t>(j - i);
   ```
   Because $|t_{p2} - t_{p3}| < 0.001\text{s}$, `ResultCalculator` correctly and intentionally treated them as simultaneously eliminated, assigning both participants a tied Rank 2 (`p2Final->placement == 2`, `p3Final->placement == 2`).
4. **Why It Passed in Release**:
   `test_match_lifecycle.cpp` used `<cassert>` `assert(...)`. Under standard Release compilation flags (`-DNDEBUG`), the CRT macro `assert(cond)` is compiled out into `((void)0)`. The failing assertion was completely stripped from the Release binary. In Debug, `assert()` remained active, raising an abort on `p3Final->placement == 3`.

---

## 2. Experimental Reproduction Evidence

Running the uncorrected Debug binary `test_match_lifecycle.exe`:
```text
[INFO] [RESULTS] Elimination #1: [P3] eliminated by [P1] at 00:00
[INFO] [MATCH] Participant 3 eliminated by cause [Combat]. Remaining alive: 2
[INFO] [RESULTS] Elimination #2: [P2] eliminated by [Corruption Storm] at 00:00
[INFO] [MATCH] Participant 2 eliminated by cause [CorruptionStorm]. Remaining alive: 1
[INFO] [MATCH] Final survivor detected! Winner: [P1] (Participant ID: 1)
...
[INFO] [MATCH] Match completed successfully in 0.100000s. Local result: VICTORY (Rank #1)
Assertion failed: p3Final->placement == 3, file C:\dev\Wildbound\tests\match\test_match_lifecycle.cpp, line 345
```
- **Recorded Timestamps**:
  - `p1` (Winner): Alive $\to$ Placement 1
  - `p2`: Timestamp = `0.100000s` $\to$ Placement 2
  - `p3`: Timestamp = `0.100000s` $\to$ Placement 2 (tied)
- **Time Elapsed in Match**: `0.100000s` instead of the documented `20.0s`.

---

## 3. Simultaneous Elimination Semantics & Production Validity

An exhaustive audit of `src/results/ResultCalculator.cpp` and `tests/results/test_match_results.cpp` established that the production logic is correct:

1. **Tournament Tie Semantics**:
   - Standard Competition Ranking ("1224") is explicitly documented and tested in `test_match_results.cpp:201` (`TestPlacementCalculationWithTies`):
     - Winner receives Rank 1.
     - Two simultaneous eliminations receive shared Rank 2.
     - Subsequent eliminations skip to Rank 4 ($1 + 2 + 1 = 4$).
2. **Draw Resolution**:
   - In a zero-survivor draw (all final combatants eliminated simultaneously in the same tick), `winnerId == 0`, and all final participants receive tied Rank 1.
3. **Event Arrival Invariance**:
   - Regardless of whether event P2 is processed before event P3 or vice versa, the $\epsilon = 0.001\text{s}$ cluster groups all tied entities together and assigns the identical rank. Arrival order has zero impact on assigned placements.
4. **Decision**:
   - **No modification to `src/results/ResultCalculator.cpp` is required or permitted.** Changing production placement logic to arbitrarily break ties based on non-tournament criteria (kills, damage, or arrival order) would violate established gameplay specifications.

---

## 4. Release Assertion Infrastructure Finding

The audit revealed that test files across the repository had divergent assertion policies:
- Files using `TEST_ASSERT(cond, msg)` (`test_lua.cpp`, `test_combat.cpp`, `test_ai.cpp`) remained active under `NDEBUG`.
- Files using raw `assert(...)` (`test_match_lifecycle.cpp`, `test_creatures.cpp`, `test_network_transport.cpp`) were completely disabled under `NDEBUG`.

To resolve this defect cleanly:
`test_match_lifecycle.cpp` was updated with the standard `TEST_ASSERT` macro mapped to `assert(...)` placed after all header includes. This ensures that every assertion in `test_match_lifecycle.cpp` is actively validated in both Debug and Release builds.

