# Match Statistics Specification

## 1. Scope & Lifecycle

Match statistics in Wildbound are strictly scoped to the active match instance.
They aggregate real-time metrics across the match lifecycle and are finalized when transitioning to `MatchState::Completed`.
When `MatchManager::ResetMatch()` is called, all metrics are zeroed out, preventing data leakage between consecutive games.

---

## 2. Aggregated Metrics

| Metric Field | Type | Description |
|---|---|---|
| `matchDuration` | float | Elapsed seconds from match start until completion. |
| `totalParticipants` | uint32 | Total number of registered competitors in match session. |
| `eliminatedParticipants`| uint32 | Count of competitors who perished before match completion. |
| `totalKills` | uint32 | Global tally of eliminations scored by competitors. |
| `totalDamageDealt` | float | Total raw damage inflicted across all combatants. |
| `totalDamageReceived` | float | Total damage sustained across all combatants. |
| `winnerParticipantId` | uint32 | Stable ID of the winning participant (0 if draw). |
| `winnerDisplayName` | string | Display name of the winner or `"No Winner (Draw)"`. |

---

## 3. Per-Participant Metrics

Each `MatchParticipant` records:
- `kills`: Authoritative count of other competitors eliminated.
- `damageDealt`: Accumulated combat damage dealt to targets.
- `damageReceived`: Accumulated damage sustained (combat or storm).
- `survivalDuration`: Exact duration survived in seconds.
- `placement`: Final standing ($1 = \text{Winner}$, $2 = \text{Runner-up}$, etc.).
- `eliminationInfo`: Detailed cause, killer participant ID, killer name, and timestamp.

