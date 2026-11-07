# Final Survivor Detection & Placements Specification

## 1. Victory Evaluation Rules

The primary objective of a Wildbound Battle Royale match is to determine the last competitor standing.
`MatchManager::EvaluateFinalSurvivor()` executes authoritative evaluation after every elimination:

| Alive Count | Determination | Action |
|---|---|---|
| $> 1$ | Match In Progress | Continue match execution. |
| $== 1$ | Sole Survivor Victory | Mark remaining participant as Winner (`isWinner = true`, `placement = 1`). Transition to `Finishing`. |
| $== 0$ | Draw / Mutual Elimination | No survivor declared. Set result to Draw. Transition to `Finishing`. |

---

## 2. Dynamic Placement Assignment

When a match reaches the completion threshold, placements ($1 \dots N$) are computed for the entire field of competitors:
1. **1st Place**: Assigned to the sole remaining alive participant.
2. **2nd Place through $N$-th Place**:
   - All eliminated participants are sorted in descending order of their elimination timestamp ($t_{\text{elimination}}$):
     $$t_A > t_B \implies \text{Placement}_A < \text{Placement}_B$$
   - A player who survives until late endgame receives a higher placement (e.g. 2nd or 3rd) than a player eliminated in the early minutes.
   - Leaderboard is compiled and sorted ascending by placement.

---

## 3. Simultaneous Final Elimination (Draw Resolution)

If the final two competitors perish in the same frame tick (e.g. mutual combat death or simultaneous tick of outside storm corruption):
- Neither participant is arbitrarily declared the winner based on memory order or registration index.
- The outcome is authoritatively recorded as a Draw (`winnerParticipantId = 0`, `winnerDisplayName = "No Winner (Draw)"`, `isVictory = false`).
- Both competitors receive equal placement weighting.

