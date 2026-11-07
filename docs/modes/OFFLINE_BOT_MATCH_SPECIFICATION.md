# WILDBOUND — OFFLINE BOT MATCH SPECIFICATION

## 1. Overview & Architectural Integrity

Offline Bot Matches allow players to experience the complete Battle Royale gameplay loop against autonomous AI competitors without requiring an internet connection or matchmaking servers.

### Architectural Anti-Mocking Rule
In Wildbound, bots are **not** virtual numbers or simulated math equations. Every bot in an Offline Bot Match:
1. Is a real `Enemy` / `Character` entity registered in the world.
2. Implements `Combatant`, having real collision hurtboxes, an `AttackSystem`, and a `HealthComponent`.
3. Possesses a dedicated `AIController` with navigation grid awareness, perception tracking, and combat state machines.
4. Is registered as a participant in `MatchParticipantRegistry` with `type = ParticipantType::AI` and linked via `entityId = enemy->GetCombatantId()`.
5. Navigates, engages in melee/ranged combat, takes damage, avoids or suffers the Corruption Storm, and is eliminated through the standard elimination pipeline.

---

## 2. Bot Participant Registration & Spawning Flow

1. **Match Initialization**:
   - `GameModeManager::SelectMode("offline_bots")`.
   - Generates match configuration with `allowAiParticipants = true`.
2. **Entity Creation & Spawning**:
   - Bots are spawned across candidate island spawn locations using `MatchSpawner::AssignSpawnPoints`.
   - Each bot entity is initialized with the selected `BotDifficultyParams`.
3. **Registry Linkage**:
   - `MatchParticipant` records are populated with unique IDs, names (e.g. `Gladiator_Bot_1`), and entity IDs.
   - Elimination events directly map entity combat deaths to match participant eliminations.
4. **Storm Attrition**:
   - Bots outside the shrinking safe zone suffer frame-accurate damage per second via `StormDamageSystem::Update`.
   - Death from the storm triggers `EliminationCause::CorruptionStorm`.

