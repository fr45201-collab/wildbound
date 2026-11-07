# WILDBOUND — GAME MODE CONFIGURATION SPECIFICATION

## 1. Schema Definition

Game modes in Wildbound are defined via JSON files located in `assets/data/gamemodes/` (mirrored to `data/gamemodes/`). The schema is parsed into `GameModeDefinition`:

```json
{
  "id": "solo",
  "displayName": "Solo Battle Royale",
  "description": "Every participant fights independently. Survive against all competitors to become the last warrior standing.",
  "minimumParticipants": 2,
  "maximumParticipants": 100,
  "teamSize": 1,
  "teamsEnabled": false,
  "friendlyFireEnabled": false,
  "botsAllowed": true,
  "defaultBotCount": 15,
  "botDifficulty": "normal",
  "respawnEnabled": false,
  "stormEnabled": true,
  "stormConfigId": "default_corruption_storm",
  "competitiveResults": true,
  "victoryCondition": "last_survivor",
  "environment": "battle_royale_island",
  "isAvailable": true,
  "requiresNetworking": false
}
```

---

## 2. Field Specifications

| Property | Type | Default | Description |
|---|---|---|---|
| `id` | String | `"solo"` | Unique, lowercase identifier used for programmatic lookup. |
| `displayName` | String | `""` | User-facing display title shown in menus and HUD banners. |
| `description` | String | `""` | Comprehensive gameplay description shown on mode selection cards. |
| `minimumParticipants` | UInt | `2` | Minimum participants required before match countdown begins. |
| `maximumParticipants` | UInt | `100` | Maximum participant capacity supported by the mode. |
| `teamSize` | UInt | `1` | Capacity of each squad. Must be $\ge 2$ if `teamsEnabled` is true. |
| `teamsEnabled` | Bool | `false` | Enables squad affiliation, friendly-fire suppression, and team victory. |
| `friendlyFireEnabled` | Bool | `false` | If true, teammates may damage each other with attacks/projectiles. |
| `botsAllowed` | Bool | `true` | Allows filling participant slots with AI competitors. |
| `defaultBotCount` | UInt | `15` | Default bot count populated in the mode selection screen. |
| `botDifficulty` | String | `"normal"` | Default difficulty profile (`easy`, `normal`, `hard`). |
| `respawnEnabled` | Bool | `false` | Whether eliminated players can respawn (disabled in Battle Royale). |
| `stormEnabled` | Bool | `true` | Enables the Corruption Storm barrier and shrinking safe zones. |
| `stormConfigId` | String | `"default"` | Identifier of the storm phase schedule configuration. |
| `competitiveResults` | Bool | `true` | Saves match history to `user_data/match_history/`. Disabled in Practice. |
| `victoryCondition` | String | `"last_survivor"` | Rule evaluated: `last_survivor`, `last_team`, or `practice_freeplay`. |
| `environment` | String | `"island"` | Target world map loaded for the match. |
| `isAvailable` | Bool | `true` | Whether the mode is playable in the current build. |
| `requiresNetworking` | Bool | `false` | Distinguishes local offline prototypes from future online features. |

---

## 3. Validation Rules

- `id` must not be empty.
- `minimumParticipants` must be $\ge 1$.
- `maximumParticipants` must be $\ge minimumParticipants$.
- If `teamsEnabled` is `true`, `teamSize` must be $\ge 2$.
- If `teamsEnabled` is `false`, `teamSize` must be $1$.
- `victoryCondition` must be one of `last_survivor`, `last_team`, or `practice_freeplay`.

