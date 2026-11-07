# RESULTS UI NAVIGATION SPECIFICATION

## 1. Overview
The Post-Match Results screen in Wildbound supports fully unified mouse, keyboard, and gamepad input navigation without letting gameplay commands leak into menu interactions.

---

## 2. Input Mapping & Focus Model

| Input Device | Action | Triggered Behavior |
|---|---|---|
| **Keyboard** | `KEY_LEFT` / `KEY_A` | Selects `[ PLAY AGAIN ]` (Button index `0`). Plays `"UI.ButtonClick"`. |
| **Keyboard** | `KEY_RIGHT` / `KEY_D` | Selects `[ MAIN MENU ]` (Button index `1`). Plays `"UI.ButtonClick"`. |
| **Keyboard** | `KEY_TAB` | Toggles between Tab `0` (Performance & Summary) and Tab `1` (Leaderboard). Plays `"UI.TabSwitch"`. |
| **Keyboard** | `KEY_ENTER` / `KEY_SPACE` | Confirms and activates currently selected button. Plays `"UI.ButtonConfirm"`. |
| **Mouse** | Hover | Real-time hover highlight and focus synchronization. |
| **Mouse** | Left Click | Direct activation of buttons or tabs. |

---

## 3. Action Handlers

### A. "PLAY AGAIN"
1. Plays audio event `"UI.ButtonConfirm"`.
2. Invokes `MatchManager::ResetMatch(player)`.
3. Calls `MatchManager::Initialize()`.
4. Registers local player as competitor #1.
5. Invokes `MatchManager::StartCountdown()`.
6. Switches UI screen to `UIScreen::None` (in-game HUD).

### B. "MAIN MENU"
1. Plays audio event `"UI.ButtonConfirm"`.
2. Invokes `MatchManager::ResetMatch(player)`.
3. Switches UI screen to `UIScreen::MainMenu`.

