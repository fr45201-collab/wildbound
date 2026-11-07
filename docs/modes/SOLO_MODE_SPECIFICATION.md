# WILDBOUND — SOLO BATTLE ROYALE SPECIFICATION

## 1. Overview & Ruleset

Solo Battle Royale is the foundational competitive mode for Wildbound:
- Every competitor fights completely independently.
- Team size is strictly 1.
- No teammates, no shared shields, no revive mechanics.
- Elimination is permanent and irrevocable (no respawns).
- The final surviving competitor standing is crowned victor (#1 Placement).

---

## 2. Participant & Spawning Flow

1. **Participant Registration**:
   - The local human player is registered as ID 1.
   - Remaining slots (up to match capacity) may be populated by AI bots or future remote players.
   - Each competitor receives a unique `participantId` and `teamId = 0` (or independent team of 1).
2. **Spawn Point Allocation**:
   - Uses `MatchSpawner::AssignSpawnPoints(RegionalDistributed)`.
   - Spawns competitors across the island, maximizing initial spacing between rivals.
3. **Combat & Damage**:
   - Friendly fire is not applicable; all competitors are hostile enemies.
   - Attack and projectile damage is applied directly to opponent health.
4. **Elimination & Victory Resolution**:
   - When a competitor reaches 0 health, `ProcessElimination` records the event and updates alive counts.
   - When `aliveCount == 1`, `VictoryEvaluator::Evaluate` declares `MatchOutcome::Victory`.
   - When `aliveCount == 0` (e.g. mutual final elimination), declares `MatchOutcome::Draw`.

