# WILDBOUND — SQUADS BATTLE ROYALE SPECIFICATION

## 1. Overview & Ruleset

Squads Battle Royale provides 4-player team combat on Wildbound Island:
- Team size is 4 participants per squad (`teamSize = 4`).
- All squad members share a common `TeamId`.
- Teammates are immune to friendly attacks when friendly fire is disabled.
- Tactical survival: A squad remains active as long as **at least one** member is alive.
- Complete team elimination occurs only when all 4 members reach 0 health.
- The last remaining active squad wins the match.

---

## 2. Squad Organization & Flow

1. **Squad Formation**:
   - `TeamManager::SetMaxTeamSize(4)`.
   - Participants are distributed across 4-person teams.
   - Example 16-player match: 4 squads of 4 players (Squad 1, Squad 2, Squad 3, Squad 4).
2. **Team Spawning**:
   - The team leader is allocated a regional spawn location.
   - The remaining 3 teammates are assigned the closest unoccupied candidate spawn points.
   - Preserves unit cohesion while preventing physics overlaps.
3. **Elimination Rules**:
   - Eliminated squad members enter spectator/dead status and cannot attack or move.
   - Surviving squad members carry forward the squad's placement.
   - Full elimination is recorded when `GetAliveMemberCount(registry) == 0`.
4. **Victory Evaluation**:
   - `TeamVictoryEvaluator` monitors alive squad counts.
   - Declares `MatchOutcome::Victory` when alive squad count equals 1.
   - Post-match leaderboard ranks squads by team survival duration.

