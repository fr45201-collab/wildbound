# MATCH LIFECYCLE DEBUGGER & DEVELOPER TOOLS

## 1. Overview
Wildbound integrates real-time Match Lifecycle inspection and manipulation directly into the in-game developer overlay (`DevUI`). The controls reside in **Tab 20: "Match Lifecycle"**, accessible during development and testing by pressing `F1`.

## 2. DevUI Tab 20 Features

### Real-Time Diagnostics
- **Lifecycle State**: Displays current `MatchState` with color-coded badges (`WaitingForPlayers` [Yellow], `Active` [Green], `Completed` [Cyan], `Failed` [Red]).
- **Match Clock**: Tracks authoritative match duration in seconds (`m_matchTime`).
- **Participant Counts**: Total registered vs. active living participants (`m_registry.GetAliveCount()`).
- **Dev Single Participant Flag**: Toggleable flag to allow matches with $N=1$ competitor for rapid gameplay and storm testing.

### Interactive Controls
1. **Initialize Default Match**:
   - Reloads `data/match/default_match.json`.
   - Populates spawn points from active `WorldManager`.
   - Registers local player as competitor #1.
2. **Start Countdown**:
   - Validates participant thresholds and begins the 10-second countdown.
3. **Force Start (Skip Countdown)**:
   - Immediately transitions `WaitingForPlayers -> Starting -> Active`, spawns competitors, and activates the Corruption Storm.
4. **Trigger Local Player Death**:
   - Simulates lethal combat damage to test death event dispatch, elimination callbacks, and placement calculations.
5. **Simulate Storm Elimination**:
   - Directly calls `ProcessElimination(id, EliminationCause::CorruptionStorm, 0)` on competitors.
6. **Force Complete Match**:
   - Immediately transitions active match into `Finishing -> Completed` and constructs final `MatchResult`.
7. **Reset Match**:
   - Executes full cleanup sequence (`Completed -> Cleanup -> Uninitialized`), purging competitor entities and resetting storm to Phase 0.

### Competitors Table
A real-time table displaying:
- **Participant ID**: Unique match identifier.
- **Display Name**: Player or bot callsign.
- **Type**: `LocalHuman`, `RemoteHuman`, or `AI`.
- **Status**: `Registered`, `Alive`, `Eliminated`, or `Disconnected`.
- **Kills**: Authoritative kills credited.
- **Damage Dealt**: Aggregated combat damage output.
- **Survival Time**: Duration survived in seconds.
- **Placement**: Final placement ($1 \dots N$) when eliminated or victorious.

## 3. Keyboard Shortcuts & Fast Iteration
- Pressing `F1` toggles `DevUI`.
- Tab navigation directly accesses the lifecycle status without halting frame updates.
- In `WaitingForPlayers`, clicking "Force Start" allows instant testing of combat and storm mechanics within 1 frame.

