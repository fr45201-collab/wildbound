# WILDBOUND — MATCH LIFECYCLE (PHASE 32) INTEGRATION

## 1. Pipeline Unification

Phase 34 integrates game modes directly into Phase 32's `MatchManager` pipeline without creating parallel lifecycle logic or duplicate match states.

```
+-------------------------------------------------------------+
|                      Game Mode Selected                     |
|          (Solo, Duos, Squads, Practice, Offline Bots)       |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|             GameModeManager::GenerateMatchConfig            |
| - Minimum & maximum participants                            |
| - Allow AI participants & bot count                         |
| - Storm config schedule & environment                       |
| - Victory condition ("last_survivor", "last_team", etc.)    |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|               MatchManager::Initialize(config)              |
| - Participant registration (Player + Bots)                  |
| - Team assignment via TeamManager                           |
| - Spawning: Individual or Proximate Team Clusters           |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                    Match State Progression                  |
| WaitingForPlayers -> Countdown -> Starting -> Active        |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                Authoritative Victory Evaluation             |
| MatchManager queries GameModeManager::EvaluateVictory       |
| - Practice: Infinite Freeplay (no victory)                  |
| - Solo/Bots: Last Survivor Standing                         |
| - Duos/Squads: Last Surviving Squad Standing                |
+-------------------------------------------------------------+
```

---

## 2. Clean State Teardown & Reset

When a match concludes or is reset via `MatchManager::ResetMatch`:
- `m_gameModeManager->Reset()` clears all team memberships, reset team IDs, and clears active practice state.
- `m_registry.ResetRegistry()` purges competitor records.
- `m_enemySpawner->ClearAllEnemies(*m_combatSystem)` deallocates bot entities and frees combatant IDs.
- Prevents stale participants, residual bot entities, or cross-match team contamination.

