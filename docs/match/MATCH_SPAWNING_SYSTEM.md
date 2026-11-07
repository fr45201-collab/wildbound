# Match Spawning System Specification

## 1. Spawn Point Discovery & Ingestion

The `MatchSpawner` interfaces with `world::WorldManager` to discover candidate spawn points:
- Queries `WorldManager::GetSpawnRegions()`, which enumerates regional clusters defined in `data/world/spawn_regions.json`.
- Enforces strict boundary verification against `playableBounds` ($[X_{\min}, Y_{\min}, W, H]$):
  $$X_p \in [X_{\min}, X_{\min} + W]$$
  $$Y_p \in [Y_{\min}, Y_{\min} + H]$$
- If any candidate falls outside or within ocean/unplayable terrain, it is rejected.

---

## 2. Selection Strategies

`MatchSpawner::AssignSpawnPoints` supports three configurable distribution strategies:
1. **RegionalDistributed** (Default):
   - Evenly distributes competitors across disparate geographical biome regions (e.g. Forest, Mountain, Desert, Swamp, Snow) using deterministic pseudo-random shuffling ($RNG(\text{seed})$).
2. **MinimumDistance**:
   - Greedily selects coordinates that maximize the minimum Euclidean distance between all assigned competitors:
     $$\max_{i} \left( \min_{j \ne i} \|P_i - P_j\| \right)$$
   - Prevents early spawn camping or immediate melee collisions on tick 0.
3. **Random**:
   - Uniform distribution across all available unoccupied points.

---

## 3. Spawn Lifecycle & Entity Synchronization

When `MatchManager::SpawnParticipants(localPlayer)` executes:
1. `AssignSpawnPoints` marks chosen spawn points as occupied (`isOccupied = true`).
2. Participants' `status` is transitioned to `ParticipantStatus::Alive`.
3. For local human player entities, `Player::Spawn(Vector2)` is called, which repositions the character, resets velocity and stamina, and resets `HealthComponent`.
4. If candidate spawn points are fewer than the registered participant count, `AssignSpawnPoints` fails safely without moving players into uninitialized coordinates.

