# WILDBOUND — GAME MODE ARCHITECTURE

## 1. System Overview

The Game Mode Subsystem in Wildbound provides a centralized, data-driven framework for defining, configuring, and executing varied Battle Royale, Practice, and Bot modes. Rather than creating disparate game loops or duplicating match lifecycle logic, every game mode operates through shared engine systems:

```
                  +-----------------------------------+
                  |        GameModeRegistry           |
                  |  - loads assets/data/gamemodes/   |
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------------------------+
                  |         GameModeManager           |
                  |  - Active GameModeDefinition      |
                  |  - GameModeRules                  |
                  |  - TeamManager                    |
                  |  - PracticeMode                   |
                  |  - BotMatchConfiguration          |
                  +--------+-----------------+--------+
                           |                 |
             Mode Config   |                 | Friendly-Fire
                           v                 v
                 +-------------------+     +---------------------+
                 |   MatchManager    |     |    CombatSystem     |
                 | - Match FSM       |     | - Damage Mitigation |
                 | - Spawner         |     | - Projectiles       |
                 | - Results/Placem. |     | - Hit Detection     |
                 +-------------------+     +---------------------+
```

---

## 2. Core Responsibilities

- **`GameModeDefinition`**: Immutable configuration schema defining participant capacities, team limits, bot availability, storm configurations, and victory conditions.
- **`GameModeRegistry`**: Discovers, parses, validates, and stores mode templates (`solo`, `duos`, `squads`, `practice`, `offline_bots`).
- **`GameModeRules`**: Lightweight runtime query facade answering friendly fire, respawn, and storm questions for gameplay systems.
- **`TeamManager`**: Manages team creation, membership validation, and dynamic team elimination tracking.
- **`TeamVictoryEvaluator`**: Decoupled evaluator computing team-based victories (e.g. last surviving team standing).
- **`PracticeMode`**: Encapsulates sandbox controls (health restore, position reset, dummy spawning) while completely isolating practice play from competitive match history.
- **`BotMatchConfiguration`**: Provides data-driven difficulty tuning (`Easy`, `Normal`, `Hard`) that modulates AI perception, reaction latency, attack aggression, and storm avoidance.

---

## 3. Subsystem Invariants

1. **No System Duplication**: `MatchManager` retains sole ownership over the match state machine, participant registration, and timers.
2. **Single Source of Truth**: Team associations and friendly-fire relationships are queried through `GameModeManager` without mutating combatant health directly.
3. **Deterministic Placement Calculation**: Results and rankings are derived from authoritative timestamps and survivor counts.
4. **Offline Local Execution**: All mode abstractions are architected with clean interfaces ready for future networking, but operate 100% offline without remote server requirements.

