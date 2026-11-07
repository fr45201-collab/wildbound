# WILDBOUND — DUOS BATTLE ROYALE SPECIFICATION

## 1. Overview & Ruleset

Duos Battle Royale introduces cooperative pairs to Wildbound:
- Participants are grouped into 2-player teams sharing a unique `TeamId`.
- Friendly fire is disabled by default: teammates cannot harm each other.
- Partial elimination: If one partner is eliminated, the surviving partner remains active and can win the match.
- Team elimination: A duo is considered eliminated only when **both** members have been eliminated.
- Victory is awarded to the last surviving duo team.

---

## 2. Team Architecture & Flow

### 2.1 Team Assignment
- `TeamManager::SetMaxTeamSize(2)`.
- During match setup, participants are paired sequentially into squads:
  - Duo 1: Player 1 (Local) + Bot 1 (Teammate)
  - Duo 2: Bot 2 + Bot 3
  - Duo 3: Bot 4 + Bot 5, etc.

### 2.2 Spatial Clustering at Spawning
- `MatchSpawner::AssignTeamSpawnPoints` assigns partners to proximate spawn points (within 50–150 units).
- Prevents duos from being separated across opposite corners of the island.

### 2.3 Elimination & Victory Evaluation
- When an individual teammate dies, individual telemetry records their death, but the team's `eliminated` flag remains `false`.
- `TeamManager::UpdateTeamEliminationStatus` checks `GetAliveMemberCount() == 0`.
- Once all other teams are eliminated, `TeamVictoryEvaluator::Evaluate` declares the surviving team victorious.
- Both members of the winning duo share #1 Team Placement in the post-match summary.

