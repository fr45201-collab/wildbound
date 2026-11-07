# Match Configuration Reference

## 1. Schema & Default File

Match configuration is stored at:
```
data/match/default_match.json
```
(and mirrored at `assets/data/match/default_match.json`).

```json
{
  "match_id": "wildbound_default",
  "match_countdown_seconds": 10.0,
  "minimum_participants": 2,
  "maximum_participants": 100,
  "allow_ai_participants": false,
  "world_id": "battle_royale_island",
  "storm_config_id": "default_corruption_storm",
  "respawn_enabled": false,
  "victory_condition": "last_survivor",
  "show_results_screen": true,
  "allow_dev_single_participant": false
}
```

---

## 2. Field Descriptions

| Field | Type | Default | Description |
|---|---|---|---|
| `match_id` | string | `"wildbound_default"` | Unique match identifier for logging and reporting. |
| `match_countdown_seconds` | float | `10.0` | Pre-match countdown duration in seconds. |
| `minimum_participants` | uint32 | `2` | Minimum competitors required before a match can begin. |
| `maximum_participants` | uint32 | `100` | Upper bound ceiling on registered competitors. |
| `allow_ai_participants` | bool | `false` | Whether AI participant competitors are permitted in match. |
| `world_id` | string | `"battle_royale_island"` | Identifier of the Battle Royale world definition. |
| `storm_config_id` | string | `"default_corruption_storm"` | Profile ID for the Corruption Storm preset. |
| `respawn_enabled` | bool | `false` | Battle Royale respawn toggle (false by default). |
| `victory_condition` | string | `"last_survivor"` | End-game evaluation rule (`last_survivor`). |
| `show_results_screen` | bool | `true` | Automatically toggle UI results screen on completion. |
| `allow_dev_single_participant` | bool | `false` | Developer testing bypass flag for single-player evaluation. |

---

## 3. Validation Logic

Before entering `WaitingForPlayers`, `MatchConfiguration::Validate` verifies:
- `matchId` is non-empty.
- `countdownSeconds` $\ge 0.0$.
- `minimumParticipants` $\ge 1$.
- `maximumParticipants` $\ge \text{minimumParticipants}$.
- `worldId` is non-empty.
If any check fails, the initialization pipeline records explicit error diagnostics and safely transitions to `MatchState::Failed`.

