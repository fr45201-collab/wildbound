# WILDBOUND — TEAM VICTORY & ELIMINATION SPECIFICATION

## 1. Team Elimination Semantics

In team-based modes (Duos and Squads), elimination semantics operate on two distinct tiers:
1. **Individual Elimination**:
   - Triggered when a participant's health reaches 0 via combat damage, corruption storm damage, or hazard.
   - The participant's status transitions to `ParticipantStatus::Eliminated`.
   - The participant is logged in chronological elimination audits.
   - The participant can no longer move, attack, or trigger hitboxes.
2. **Team Elimination**:
   - `TeamManager::UpdateTeamEliminationStatus` inspects team membership.
   - A team remains **ALIVE** as long as $\ge 1$ registered member has status `ParticipantStatus::Alive`.
   - When the alive member count drops to 0, `team.eliminated` is set to `true`, and `team.eliminationTimestamp` records the moment of squad collapse.

---

## 2. Team Victory Evaluation

The `TeamVictoryEvaluator` computes match conclusion states deterministically:
- **`MatchOutcome::InProgress`**: More than one team has active, alive members.
- **`MatchOutcome::Victory`**: Exactly one team has active, alive members, while all rival teams have been eliminated. The surviving team is declared champion.
- **`MatchOutcome::Draw`**: Zero teams have surviving members (e.g. mutual elimination in the corruption storm).
- **`MatchOutcome::Failed`**: Match active with zero registered teams.

---

## 3. Placement Resolution

- Placements are assigned in reverse chronological order of team elimination:
  - Last surviving team = #1 Team Placement (all squad members share #1).
  - Second-to-last eliminated team = #2 Team Placement.
  - Earliest eliminated team = lowest rank.
- Simultaneous squad eliminations share identical placement with Olympic rank skipping for subsequent positions.

