# MATCH HISTORY PERSISTENCE SPECIFICATION

## 1. Overview
The local Match History persistence subsystem in `ResultSerializer` archives post-match summaries to local disk in standard JSON format without introducing cloud dependencies, database servers, or external services.

---

## 2. Storage Architecture
- **Location**: `user_data/match_history/<match_id>.json`
- **Format**: Validated JSON (via `nlohmann::json`).
- **Sanitization**: All operating system filename-reserved characters (`/`, `\`, `:`, `*`, `?`, `"`, `<`, `>`, `|`) are replaced with `_`.

---

## 3. JSON Schema Specification

```json
{
    "match_id": "phase33_e2e_match",
    "match_outcome": "Victory",
    "local_player_outcome": "Victory",
    "winner_participant_id": 1,
    "winner_display_name": "Player1",
    "match_duration": 185.4,
    "total_participants": 10,
    "total_eliminations": 9,
    "survivors_count": 1,
    "completion_reason": "Sole survivor emerged victorious.",
    "local_player": {
        "participant_id": 1,
        "display_name": "Explorer",
        "type": 0,
        "outcome": "Victory",
        "placement": 1,
        "survival_duration": 185.4,
        "kills": 4,
        "deaths": 0,
        "damage_dealt": 850.0,
        "combat_damage_received": 120.0,
        "storm_damage_received": 0.0,
        "total_damage_received": 120.0,
        "elimination_cause": 0,
        "killer_participant_id": 0,
        "killer_display_name": "",
        "is_winner": true
    },
    "leaderboard": [ ... ],
    "eliminations": [
        {
            "event_id": 1,
            "victim_id": 4,
            "victim_name": "Bot_4",
            "killer_id": 1,
            "killer_name": "Explorer",
            "cause": 0,
            "match_time": 45.2,
            "placement": 10
        }
    ]
}
```

---

## 4. Resilience & Error Handling
1. **Directory Auto-Creation**: `fs::create_directories("user_data/match_history")` creates destination if missing.
2. **Corrupted File Guard**: Missing or malformed JSON files return `false` gracefully without crashing.
3. **No Entity Pointers**: Stores strictly POD scalars and string identifiers; no raw memory addresses or ECS entity handles are persisted.

