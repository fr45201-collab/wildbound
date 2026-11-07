# WILDBOUND — GAME MODE SELECTION UI SPECIFICATION

## 1. User Interface Layout & Visual Hierarchy

The Mode Selection screen (`wildbound::ui::ModeSelectMenu`) provides a dedicated player-facing interface within the 640x360 virtual resolution canvas:

```
+-----------------------------------------------------------------------------+
| SELECT GAME MODE                                                            |
+------------------------------+----------------------------------------------+
| [ Solo Battle Royale ]       | Solo Battle Royale      [ PLAYABLE OFFLINE ] |
| Every warrior for themselves |                                              |
|                              | Every participant fights independently.      |
| [ Duos Battle Royale ]       | Survive against all competitors to become    |
| Cooperate in 2-player squads | the last warrior standing.                   |
|                              | -------------------------------------------- |
| [ Squads Battle Royale ]     | Team Structure:       Disabled (Solo)        |
| Dominate in 4-player squads  | Participant Range:    2 - 100                |
|                              | AI Competitors:       [-] 15 bots [+]        |
| [ Practice Arena ]           | AI Difficulty:        < normal >             |
| Freeplay combat & testing    | Storm System:         Corruption Storm Active|
|                              |                                              |
| [ Offline Bot Matches ]      |                                              |
| Custom offline AI competitors|           [ BACK ]       [ START MATCH ]     |
+------------------------------+----------------------------------------------+
```

---

## 2. Interaction & Navigation Design

- **Mouse Interaction**:
  - Clicking any mode card on the left column highlights the card and refreshes the details pane.
  - Interactive plus/minus buttons dynamically adjust bot count between 1 and 32.
  - Clicking the difficulty button cycles through `< easy >`, `< normal >`, and `< hard >`.
  - Action buttons: `[ BACK ]` returns to Main Menu, while `[ START MATCH ]` dispatches the configured parameters to `Game::StartSelectedGameMode()`.
- **Keyboard & Gamepad Navigation**:
  - `Up` / `Down`: Cycles mode cards sequentially.
  - `Left` / `Right`: Decrements or increments bot count / difficulty.
  - `Enter` / `Space`: Activates the selected mode and starts the match.
  - `Escape`: Returns to Main Menu.
- **Auditory Feedback**:
  - Button hover dispatches `UI.ButtonHover`.
  - Button click dispatches `UI.ButtonClick`.

