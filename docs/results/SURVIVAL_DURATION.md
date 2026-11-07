# SURVIVAL DURATION SPECIFICATION

## 1. Overview
The Survival Duration system in Wildbound Phase 33 measures elapsed competitive time per participant using monotonic match-clock delta accumulation.

---

## 2. Calculation Principles
1. **Clock Source**:
   - Survival duration is driven strictly by frame delta time (`dt`) accumulating during `MatchState::Active`.
   - Never uses system wall-clock differences, which are vulnerable to system clock shifts, menu freezes, and OS sleeps.
2. **Exclusions**:
   - Pre-match initialization (`Initializing`, `WaitingForPlayers`).
   - Action-locked countdown (`Countdown`).
   - Post-match screens and menus (`Finishing`, `Completed`, `Cleanup`, `MainMenu`).
   - Pause states: when the game is paused via `PauseMenu`, delta updates cease.
3. **Formulas**:
   - For eliminated competitors:
     $$\text{SurvivalDuration} = \text{EliminationTimestamp} - \text{ActiveMatchStartTimestamp}$$
   - For match winners:
     $$\text{SurvivalDuration} = \text{MatchEndTimestamp} - \text{ActiveMatchStartTimestamp} = \text{TotalMatchDuration}$$

---

## 3. Formatting Rules
The static utility `ResultCalculator::FormatDuration(float seconds)` formats durations as follows:
- Negative values clamp to `00:00`.
- Durations $< 3600\text{s}$ (less than 1 hour): `MM:SS` (e.g. `00:45`, `12:48`, `59:59`).
- Durations $\ge 3600\text{s}$ (1 hour or greater): `HH:MM:SS` (e.g. `01:00:00`, `02:15:30`).

