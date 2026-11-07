# WILDBOUND — HUD & MODE PRESENTATION SPECIFICATION

## 1. Dynamic Mode Presentation

The gameplay HUD (`wildbound::ui::HUDManager`) dynamically adapts its top status header to reflect the active game mode:

### 1.1 Solo Battle Royale
- Header format:
  `Alive: 24   Kills: 3   Time: 04:32`
- Focuses on individual survival and personal elimination score.

### 1.2 Duos and Squads Battle Royale
- Header format:
  `Alive: 16 (Squads: 4)   Kills: 2   06:15`
- Displays both total remaining alive individuals and total remaining intact squads.

### 1.3 Practice Arena
- Header format:
  `[ Practice Arena ] Freeplay Mode`
- Replaces competitive countdown timers and storm warnings with freeplay indicators.

---

## 2. In-Match HUD Synchronization

In `Game::Update`:
- Queries `GameModeManager::GetActiveMode()` to update `MatchHUDInfo::gameModeName`, `gameModeId`, `isPracticeMode`.
- Queries `TeamManager::GetAliveTeamCount()` to report active squads in real time.
- Guarantees that all metrics displayed to the player derive from live match simulation data.

