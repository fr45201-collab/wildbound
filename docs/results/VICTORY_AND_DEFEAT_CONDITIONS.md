# VICTORY AND DEFEAT CONDITIONS SPECIFICATION

## 1. Overview
The Victory and Defeat evaluation system in Wildbound Phase 33 establishes deterministic, authoritative rules for concluding Battle Royale matches and assigning individual participant outcomes.

The standard competitive game mode is **Last Survivor Standing**.

---

## 2. Victory Scenarios Matrix

| Scenario | State Machine Trigger | Match Outcome | Local Player Outcome | Winner ID |
|---|---|---|---|---|
| Sole survivor remains alive ($N=1$) | Active match tick | `MatchOutcome::Victory` | `PlayerOutcome::Victory` (if local) / `Defeat` (if eliminated) | Sole survivor ID |
| All opponents eliminated | Combat or Storm kill | `MatchOutcome::Victory` | `PlayerOutcome::Victory` (if local) / `Defeat` (if eliminated) | Sole survivor ID |
| Final opponent killed by storm | Storm damage tick | `MatchOutcome::Victory` | `PlayerOutcome::Victory` (if local) / `Defeat` (if eliminated) | Sole survivor ID |
| Mutual elimination (0 survivors left) | Simultaneous kills / Storm | `MatchOutcome::Draw` | `PlayerOutcome::Draw` | `0` ("No Winner (Draw)") |
| Single remaining player disconnects | Disconnect event | `MatchOutcome::Draw` / `Failed` | `PlayerOutcome::Defeat` | `0` ("No Winner (Draw)") |
| Match aborted prior to start | Pre-match cancellation | `MatchOutcome::Cancelled` | `PlayerOutcome::Undecided` | `0` |
| Initialization fails ($N < \text{min}$) | Initialization check | `MatchOutcome::Failed` | `PlayerOutcome::Undecided` | `0` |

---

## 3. Evaluation Rules & Invariants

1. **Active Match Boundary**:
   - Victory evaluation only executes when `MatchState == MatchState::Active`.
   - Victory cannot be declared during `Initializing`, `WaitingForPlayers`, or `Countdown`.
2. **Authoritative Competitor Isolation**:
   - Only registered competitors in `MatchParticipantRegistry` are evaluated.
   - PvE enemies (goblins, skeletons, wildlife) are completely excluded from survivor counts.
3. **Simultaneous Draw Invariant**:
   - If all remaining living competitors perish in the same logical frame tick, `Evaluate()` produces `MatchOutcome::Draw` with winner ID `0`.
   - Eliminates event-processing race conditions.
4. **Idempotence & Execution Guard**:
   - `Evaluate()` locks upon the first decisive outcome (`Victory` or `Draw`) and transitions the lifecycle to `Finishing -> Completed`.

