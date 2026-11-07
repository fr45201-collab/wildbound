# WILDBOUND — PHASE 34 REPOSITORY AUDIT

## 1. Executive Summary

This audit assesses the Wildbound codebase to verify prerequisite systems, identify missing dependencies, and establish the integration strategy for **Phase 34: Game Modes Architecture and Implementation**.

Wildbound currently features a complete offline Battle Royale island (Phase 30), Corruption Storm lifecycle (Phase 31), Match Lifecycle state machine (Phase 32), and Victory/Results system (Phase 33). However, all match mechanics prior to Phase 34 operate under a strictly individualistic, Solo Battle Royale paradigm.

---

## 2. Detailed Subsystem Audit Matrix

| Subsystem / Requirement | Status | Current Repository Implementation | Gap / Phase 34 Requirement |
|---|---|---|---|
| **Match Configuration** | **PARTIALLY IMPLEMENTED** | `MatchConfiguration` supports `matchId`, `countdownSeconds`, `minimumParticipants`, `maximumParticipants`, `allowAiParticipants`, `worldId`, `stormConfigId`, `respawnEnabled`, `victoryCondition`. | Lacks `gameModeId`, `teamSize`, `friendlyFireEnabled`, `botCount`, `botDifficulty`. Must integrate with `GameModeDefinition`. |
| **Participant Registry** | **COMPLETE** | `MatchParticipantRegistry` manages `MatchParticipant` structs with unique IDs, types (`LocalHuman`, `RemoteHuman`, `AI`), status (`Registered`, `Alive`, `Eliminated`), kills, damage, and stats. | Lacks `teamId` on `MatchParticipant` to track team association. |
| **Participant Identity & Types** | **COMPLETE** | `ParticipantType` defines `LocalHuman`, `RemoteHuman`, `AI`. | None. Ready for bot competitors and future remote players. |
| **Team Identifiers & Membership** | **MISSING** | No team abstraction exists in `src/match/` or `src/entities/`. | Must implement `TeamId`, `struct Team`, and `TeamManager` in `src/gamemodes/`. |
| **Friendly-Fire Rules** | **PARTIALLY IMPLEMENTED** | `CombatRules` defines `friendlyFire`, `allowPvE`, `allowPvP`. `CanTarget(Faction, Faction)` handles faction-level targeting. | Lacks team-aware query: teammates must not be attacked or damaged when friendly fire is disabled. |
| **Match State Machine** | **COMPLETE** | `MatchManager` FSM cleanly transitions through `Uninitialized` -> `Initializing` -> `WaitingForPlayers` -> `Countdown` -> `Starting` -> `Active` -> `SuddenDeath` -> `Finishing` -> `Completed` -> `Cleanup`. | Must support practice mode lifecycle (no competitive countdown or sudden death required) and mode reset. |
| **Spawn Selection & Placement** | **PARTIALLY IMPLEMENTED** | `MatchSpawner` handles 48 island spawn points with collision checks and distance spread. | Works for individuals; needs team-proximity clustering for Duos and Squads so teammates spawn near one another. |
| **Elimination Processing** | **COMPLETE (INDIVIDUAL)** / **MISSING (TEAMS)** | `MatchEliminationSystem` and `MatchManager::ProcessElimination` log causalities, killers, timestamps, and forward to `MatchResultManager`. | Missing team elimination logic: a team remains active until all members are eliminated. |
| **Victory Evaluation** | **PARTIALLY IMPLEMENTED** | `VictoryEvaluator` in `src/results/` evaluates Last Survivor Standing and mutual draw for individual competitors. | Missing team victory evaluation (last surviving team wins) and practice mode handling (no competitive victory). |
| **Match Results & Statistics** | **COMPLETE** | `MatchResultManager`, `MatchResultModel`, `ResultCalculator`, and `ResultSerializer` record and serialize match history to JSON. | Needs `gameModeId`, team placement records, and practice mode isolation (no competitive JSON history). |
| **AI Navigation** | **COMPLETE** | `NavigationGrid`, `Pathfinder` (A* algorithm), `AIMovementController`, `ObstacleAvoidance`, `StuckDetector` in `src/ai/`. | Ready for bot pathfinding and obstacle avoidance. |
| **AI Perception & Behavior** | **PARTIALLY IMPLEMENTED** | `PerceptionSystem`, `TargetSelector`, `TargetMemory`, `PatrolBehaviour`, `InvestigateBehaviour`, `AIController` in `src/ai/`. | AI is tailored for PvE monsters; requires BR competitor bot logic: targeting players and other bots, attacking, storm evasion, and difficulty profiles. |
| **Player Input Contexts** | **COMPLETE** | `InputManager` supports keyboard, mouse, gamepad, actions, and menu navigation. | Mode selection UI can leverage existing menu navigation. |
| **HUD Mode Information** | **PARTIALLY IMPLEMENTED** | `HUDManager` renders health, armor, weapons, ammo, minimap, alive count, and storm indicators. | Missing active mode indicator, team member status panel (for Duos/Squads), and practice controls overlay. |
| **Match Reset & Cleanup** | **COMPLETE** | `MatchManager::ResetMatch()` clears state, participants, spawner, and results without leaks. | Must be extended to reset `TeamManager` and active mode state. |
| **Bot Spawning & Registration** | **PARTIALLY IMPLEMENTED** | `EnemySpawner` spawns PvE monsters. `MatchParticipant` has `ParticipantType::AI`. | Missing automated generation of competitor bots registered into `MatchParticipantRegistry` with combatant entities. |
| **Bot Difficulty Profiles** | **MISSING** | No difficulty tuning presets exist for AI competitors. | Must implement `BotDifficulty` (`Easy`, `Normal`, `Hard`) affecting perception, reaction delay, attack frequency, and storm reaction. |

---

## 3. Dependency Policy & Implementation Plan

1. **Team Subsystem (`Team.hpp`, `TeamManager.hpp`)**:
   - Create lightweight, decoupled team management.
   - Map `ParticipantId` to `TeamId`.
   - Prevent duplicate memberships, validate team sizes (Solo = 1, Duos = 2, Squads = 4).
   - Track team alive/eliminated status.
2. **Game Mode Subsystem (`src/gamemodes/`)**:
   - `GameModeDefinition`: Data-driven mode specifications loaded from `assets/data/gamemodes/*.json`.
   - `GameModeRegistry`: Mode catalogue offering Solo, Duos, Squads, Practice, and Offline Bot Matches.
   - `GameModeRules`: Encapsulates team rules, friendly fire, respawns, bot settings, and storm parameters.
   - `GameModeManager`: Coordinates active mode selection, configuration injection, and team management.
3. **Combat Friendly-Fire Integration**:
   - Extend `CombatRules` or provide a team friendly-fire query predicate to `CombatSystem` and `ProjectileSystem`.
   - If `friendlyFire` is disabled, attacks and projectiles from teammates cause zero damage.
4. **Team Victory & Elimination**:
   - Implement `TeamVictoryEvaluator` to evaluate the last surviving team for Duos and Squads.
   - Mode-aware victory evaluation dispatched by `MatchManager`.
5. **Practice Mode**:
   - Independent environment without competitive countdown, victory requirements, or persistent statistics.
   - Interactive developer/practice controls (health restore, enemy dummy spawn, position reset).
6. **Offline Bot Competitors**:
   - Data-driven bot profiles (`Easy`, `Normal`, `Hard`).
   - Spawn real `Enemy` / competitor entities registered into `MatchParticipantRegistry`.
   - Integrate with `AIManager` for movement, target selection, combat engagement, and storm evasion.
7. **Player-Facing UI**:
   - Implement `ModeSelect` screen in `UIManager`.
   - Mode cards with descriptions, team size, participant limits, and configuration (bot count, difficulty).
   - Update HUD with mode banner and team status widgets.

---

## 4. Verification Check

All underlying systems from Phases 0–33 are verified, compile with zero errors, and pass all 34 CTest suites. We proceed to Part 2: Game Mode Architecture.

