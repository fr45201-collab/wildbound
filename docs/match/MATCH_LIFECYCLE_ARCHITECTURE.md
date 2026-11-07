# Match Lifecycle Architecture

## 1. High-Level Architectural Overview

The Battle Royale Match Lifecycle System in Wildbound coordinates the end-to-end progression of a match—from initialization, participant registration, and distributed spawning, through pre-match countdowns, authoritative active combat, multi-cause eliminations, and final survivor resolution, to match completion, results presentation, and memory-safe cleanup.

```
+---------------------------------------------------------------------------------------------------+
|                                        Game Engine Loop                                           |
|                                                                                                   |
|   +-----------------------+   Config    +-----------------------------------------------------+   |
|   |  default_match.json   | ----------> | wildbound::match::MatchManager                      |   |
|   +-----------------------+             |                                                     |   |
|                                         |  +--------------------+   +----------------------+  |   |
|   +-----------------------+             |  | MatchState Machine |   | Participant Registry |  |   |
|   |  WorldManager         |             |  | (10 Valid States)  |   | (Authoritative IDs)  |  |   |
|   |  - PlayableBounds     |             |  +--------------------+   +----------------------+  |   |
|   |  - Spawn Regions      | ----------> |                                                     |   |
|   +-----------------------+             |  +--------------------+   +----------------------+  |   |
|                                         |  | MatchSpawner       |   | MatchCountdown       |  |   |
|   +-----------------------+             |  | (Distribution)     |   | (Action Lock)        |  |   |
|   |  Combat & Storm       |             |  +--------------------+   +----------------------+  |   |
|   |  - DeathEvents        | ----------> |                                                     |   |
|   |  - Outside Damage     |             |  +--------------------+   +----------------------+  |   |
|   +-----------------------+             |  | Elimination System |   | MatchResult & Stats  |  |   |
|                                         |  | (Idempotent Death) |   | (Final Placements)   |  |   |
|                                         |  +--------------------+   +----------------------+  |   |
|                                         +--------------------------+--------------------------+   |
|                                                                    |                              |
|                                      Sync & State Signals          |                              |
|                                      v                             v                              |
|                     +-----------------------------+   +----------------------------+              |
|                     | storm::StormManager         |   | ui::UIManager              |              |
|                     | - StartStorm() on Active    |   | - MatchResults Screen      |              |
|                     | - Pause on Complete         |   | - Play Again / Main Menu   |              |
|                     +-----------------------------+   +----------------------------+              |
+---------------------------------------------------------------------------------------------------+
```

---

## 2. Architectural Principles

1. **Separation of Concerns**:
   - `MatchManager` does not implement physics, rendering, combat calculations, inventory storage, or storm damage algorithms.
   - It acts strictly as an authoritative coordinator, querying state and directing transitions.
2. **Stable Identity without Raw Pointer Coupling**:
   - Participants are identified by immutable numeric `uint32_t participantId`.
   - Entity references (`entityId`) are decoupled from lifetime ownership.
3. **Multiplayer Readiness Without Premature Network Stubs**:
   - The participant abstraction natively accommodates `ParticipantType::LocalHuman`, `ParticipantType::RemoteHuman`, and `ParticipantType::AI`.
   - No mock networking protocols (ENet, sockets, dedicated servers) are introduced into this single-node simulation phase.
4. **Idempotence & State Isolation**:
   - Eliminations, match start, match finish, and cleanup operations are guaranteed to be idempotent.
   - Calling `ResetMatch()` clears match-scoped memory while preserving engine configurations, textures, sound buffers, and user settings.

