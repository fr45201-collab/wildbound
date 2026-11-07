# MATCH RESULT ARCHITECTURE

## 1. Overview
The Wildbound Phase 33 Results subsystem (`src/results/`) provides a clean, decoupled layer that consumes authoritative state and telemetry from the Phase 32 Match Lifecycle, processes decisive outcomes, calculates tournament placements, and generates immutable result models.

---

## 2. Component Architecture

```
               [ MatchManager ] (Phase 32)
                       |
                       v
         [ results::MatchResultManager ]
         /           |           \        \
        v            v            v        v
[ VictoryEvaluator ][ DefeatEvaluator ][ ResultCalculator ][ ResultSerializer ]
        \            |            /        /
         v           v           v        v
              [ MatchResultModel ] (Immutable Snapshot)
                       |
          +------------+------------+
          |                         |
          v                         v
 [ UIManager (Results UI) ]   [ DevUI (Tab 20) ]
```

---

## 3. Class Roles & Responsibilities

1. **`MatchOutcome` & `PlayerOutcome` (`MatchOutcome.hpp`)**:
   - Discrete enumeration distinguishing overall match state (`Victory`, `Draw`, `Cancelled`, `Failed`) from individual participant perspective (`Victory`, `Defeat`, `Draw`).
2. **`EliminationRecord` (`EliminationRecord.hpp`)**:
   - Audit trail capturing incremental event ID, victim ID/name, killer ID/name, cause, match time, and placement.
3. **`PlayerPerformance` (`PlayerPerformance.hpp`)**:
   - Match-scoped individual statistics: placement, survival duration, kills, deaths, damage dealt, combat damage taken, storm damage taken, and total damage taken.
4. **`MatchResultModel` (`MatchResultModel.hpp`)**:
   - The authoritative immutable post-match container.
5. **`VictoryEvaluator` (`VictoryEvaluator.hpp` / `.cpp`)**:
   - Evaluates remaining alive participants and assigns decisive outcomes.
6. **`DefeatEvaluator` (`DefeatEvaluator.hpp` / `.cpp`)**:
   - Evaluates individual eliminations, status changes, and human-readable defeat reasons.
7. **`ResultCalculator` (`ResultCalculator.hpp` / `.cpp`)**:
   - Standard duration formatting (`MM:SS` / `HH:MM:SS`), placement ranking with tournament tie handling, and snapshot aggregation.
8. **`ResultSerializer` (`ResultSerializer.hpp` / `.cpp`)**:
   - Local JSON persistence to `user_data/match_history/<match_id>.json`.
9. **`MatchResultManager` (`MatchResultManager.hpp` / `.cpp`)**:
   - High-level orchestrator interfacing directly with `MatchManager`.

