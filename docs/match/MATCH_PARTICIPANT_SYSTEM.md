# Match Participant System Specification

## 1. Competitor Abstraction

The Battle Royale participant abstraction in Wildbound decouples competitor lifetime and scoring from game engine physics and rendering objects:

```
+-------------------------------------------------------------------------------+
|                            MatchParticipant                                   |
|                                                                               |
|  - participantId: uint32_t (Stable session-unique identifier)                 |
|  - type:          ParticipantType (LocalHuman / RemoteHuman / AI)             |
|  - displayName:   std::string (Competitor gamer tag / bot moniker)            |
|  - entityId:      uint32_t (Entity ID for player / combatant queries)         |
|  - status:        ParticipantStatus (Registered / Alive / Eliminated / Disc)  |
|  - spawnPosition: Vector2 (World coordinates assigned by Spawner)             |
|  - kills:         uint32_t (Authoritative elimination tally)                  |
|  - damageDealt:   float (Aggregated damage inflicted on other competitors)    |
|  - damageReceived:float (Aggregated damage sustained from all sources)        |
|  - survivalTime:  float (Duration survived in active match in seconds)        |
|  - placement:     uint32_t (Final match placement: 1 = Winner, 2 = Runner-up) |
+-------------------------------------------------------------------------------+
```

---

## 2. Participant Registry Operations

The `MatchParticipantRegistry` coordinates all participants in $O(1)$ lookup complexity using two internal index maps:
1. `m_idToIndexMap`: Primary key index by `participantId`.
2. `m_entityIdToIndexMap`: Secondary lookup index by `entityId`.

### Key Operations:
- `RegisterParticipant(participant)`:
  - Validates `participantId != 0`.
  - Rejects duplicate participant IDs.
  - Automatically records entity ID cross-reference if non-zero.
- `UnregisterParticipant(participantId)`:
  - Uses swap-and-pop with the last array element for $O(1)$ removal without memory fragmentation.
  - Updates relocated index entries.
- `GetAliveCount()` & `GetAliveParticipantIds()`:
  - Evaluates current alive pool for victory determination.
- `UpdateParticipantStatus(participantId, newStatus)`:
  - Idempotent status modification (`Alive -> Eliminated`).

---

## 3. PvE Enemy Isolation Guarantee

Ordinary world enemies (e.g. Goblins, Skeletons, Wolves, Bats) are **not** registered as Battle Royale competitors. They remain ambient world hazards unless an approved game mode explicitly registers an AI participant.
This prevents AI wildlife or dungeon spawns from interfering with the winner-determination logic.

