# Match Completion & Results Presentation Specification

## 1. Completion Pipeline

When `MatchManager::CompleteMatch` triggers (upon victory determination, mutual draw, or manual end):
1. `MatchState` advances `Active -> Finishing -> Completed`.
2. Active storm contraction stops (`StormManager::PauseStorm()`).
3. Competitor movement and combat controls are inhibited (`IsGameplayAllowed() == false`).
4. Authoritative `MatchResult` data structure is compiled from live participant records.
5. If `m_screenCallback` is registered and `showResultsScreen == true`, the callback signals the UI to display the `MatchResults` screen (`UIScreen::MatchResults`).
6. Acoustic fanfare is played: `"Music.PlayVictory"` for victory (local placement = 1), or `"Music.PlayDefeat"` for defeat (local placement $> 1$).

---

## 2. Match Results Screen Structure

The results screen rendered by `UIManager::RenderMatchResults` presents:
- **Title**: `"BATTLE ROYALE RESULTS"`
- **Outcome Status**:
  - `VICTORY ROYALE` (Green `#50F078` gradient) if `isVictory == true`.
  - `RANK #X` (Crimson `#F04646` badge) if eliminated.
- **Winner Information**: `"Winner: <Champion Name>"`
- **Local Performance Metrics Panel**:
  - Eliminations count ($K$)
  - Damage Dealt ($D_{\text{out}}$)
  - Damage Taken ($D_{\text{in}}$)
  - Survival Duration (`MM:SS`)
  - Elimination Cause & Killer Name (if eliminated)
- **Interactive Action Buttons**:
  - **`PLAY AGAIN`**:
    Resets the previous match state, reinitializes a fresh session, registers the local player entity, starts pre-match countdown, and hides the results overlay (`UIScreen::None`).
  - **`RETURN TO MENU`**:
    Resets match state and returns to the game main menu (`UIScreen::MainMenu`).

---

## 3. Idempotent Completion Guarantee

If multiple end-game signals arrive simultaneously, `m_winnerDetermined` flag and state machine guards prevent `CompleteMatch()` from executing more than once per match session.
Leaderboard and statistical records are frozen at the exact frame of completion.

