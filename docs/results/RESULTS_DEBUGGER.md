# WILDBOUND — RESULTS & STATISTICS DEBUGGER (DEVUI TAB 20)

## 1. Overview

To facilitate rapid iteration, playtesting, and automated verification without requiring full 15-minute Battle Royale matches, Phase 33 integrates a dedicated diagnostic suite into Dear ImGui's developer interface (`src/ui/DevUI.cpp`, Tab 20: *Match & Storm* / *Results Diagnostics*).

The debugger provides:
- Real-time match outcome evaluation.
- Live participant performance monitors.
- Full elimination audit log explorer with filtering.
- One-click testing triggers (Force Victory, Force Defeat, Force Mutual Elimination Draw, Force Match Abort).
- Disk persistence inspection and JSON re-export triggers.

---

## 2. Interface Controls & Panels

### 2.1 Live Outcome Monitor
Displays the current status evaluated on every frame by `VictoryEvaluator::Evaluate(...)`:
- **Current Outcome**: `InProgress`, `Victory`, `Defeat`, `Draw`, or `Cancelled`.
- **Participants Registered**: Total count of registered competitors.
- **Competitors Remaining**: Number of alive non-spectator participants.
- **Elapsed Duration**: Live MM:SS counter with millisecond precision.

### 2.2 Local Player Telemetry Panel
Inspects the active player's metrics in real time:
- **Placing**: Estimated live placement based on remaining player count.
- **Eliminations**: Total confirmed kills credited to the local player.
- **Damage Dealt**: Cumulative combat damage inflicted on other competitors.
- **Combat Damage Taken**: Damage received from player attacks.
- **Storm Damage Taken**: Environmental damage suffered while outside the Corruption Storm barrier.
- **Survival Time**: Elapsed time since match start until elimination or victory.

### 2.3 Elimination Audit Table
A scrollable ImGui table listing all recorded elimination events in chronological order:
- **Timestamp**: In-match timestamp formatted as `MM:SS.ms`.
- **Victim**: Participant name and ID.
- **Killer**: Attacker name and ID (or "Corruption Storm" / "Environment").
- **Cause**: Elimination cause enum string (`Combat`, `StormDamage`, `Suicide`, `Hazard`).
- **Weapon / Item**: Weapon or environmental effect responsible for the final blow.

### 2.4 Quick Test & Simulation Triggers
The debugger provides immediate simulation actions for edge-case testing:
1. **Force Victory**: Instantly eliminates all active enemy competitors, leaving the local player as the sole survivor. Verifies Victory celebration UI, golden styling, and #1 placement.
2. **Force Defeat**: Inflicts lethal damage on the local player while opponents remain alive. Verifies Defeat screen styling, killer attribution, and spectator/return options.
3. **Simulate Mutual Draw**: Simultaneously eliminates all remaining competitors (including the local player) at the exact same sub-frame timestamp. Verifies Olympic tie-handling and mutual Draw outcome.
4. **Trigger Result Screen**: Forces an immediate transition to `GameState::MatchResults` using current match data.
5. **Dump JSON to Disk**: Manually invokes `ResultSerializer::SaveResultToFile()` to test disk writing and verify file output in `user_data/match_history/`.

---

## 3. Best Practices for Developers
- Keep Dear ImGui diagnostic code completely decoupled from the production `UIManager`: DevUI queries `MatchManager::GetResultManager()`, while player-facing screens use raylib draw calls.
- When adding new match telemetry (e.g. accuracy, items looted in future phases), bind the property into `PlayerPerformance` and add a column to Tab 20's live inspection table.

