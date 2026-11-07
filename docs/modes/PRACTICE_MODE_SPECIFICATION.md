# WILDBOUND — PRACTICE ARENA SPECIFICATION

## 1. Objectives & Philosophy

Practice Mode is a dedicated, pressure-free testing environment for Wildbound:
- Allows players to practice movement, test weapons and abilities, evaluate combat timing, inspect inventory, and experiment with game systems.
- Free of competitive match timers, storm boundaries, and sudden death.
- Competitive statistics and match history persistence are strictly disabled to prevent contamination of Battle Royale leaderboards.

---

## 2. Interactive Practice Controls

The `PracticeMode` subsystem provides immediate sandbox capabilities:
- **Restore Health**: Instantly restores player health to 100% via `RestorePlayerHealth()`.
- **Reset Position**: Teleports the player back to the practice spawn point via `ResetPlayerPosition()`.
- **Spawn Combat Dummies**: Dynamically creates dummy targets using `SpawnCombatDummy()` for DPS and combo testing.
- **Clear Combat Dummies**: Safely deallocates spawned practice targets via `ClearCombatDummies()`.

---

## 3. Data Isolation

- `GameModeDefinition::competitiveResults = false`.
- `GameModeDefinition::victoryCondition = "practice_freeplay"`.
- When completing or exiting a practice session, no JSON match files are written to `user_data/match_history/`.

