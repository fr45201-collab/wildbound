# POST-MATCH SCREEN DESIGN SPECIFICATION

## 1. Overview
The Post-Match Results Screen is rendered within Wildbound's fixed $640 \times 360$ virtual pixel canvas via `UIManager::RenderMatchResults()`. It provides a clean, visual presentation of the match outcome, player performance, and full standings.

---

## 2. Visual Layout & Dimensions

```
+------------------------------------------------------------------+
|                     [ HEADER / BANNER ]                          |
|             "VICTORY ROYALE" / "DEFEAT — RANK #X"                |
|                    "LAST SURVIVOR STANDING"                      |
|                                                                  |
|   [TAB 0: PERFORMANCE & SUMMARY]    [TAB 1: LEADERBOARD]         |
|  +---------------------------+  +------------------------------+ |
|  |     YOUR PERFORMANCE      |  |        MATCH SUMMARY         | |
|  | Final Placement: #1 / 10  |  | Match ID: match_001          | |
|  | Survival: 12:48           |  | Duration: 12:48              | |
|  | Eliminations: 4           |  | Champion: HeroPlayer         | |
|  | Damage Inflicted: 620     |  | Competitors: 10              | |
|  | Combat Damage Taken: 80   |  | Outcome: Victory             | |
|  | Storm Damage Taken: 15    |  | Status: Sole survivor won    | |
|  +---------------------------+  +------------------------------+ |
|                                                                  |
|             [ PLAY AGAIN ]        [ MAIN MENU ]                  |
+------------------------------------------------------------------+
```

---

## 3. Thematic Visual Styling
- **Victory State**:
  - Gradient: Emerald/Dark Forest (`Color{ 18, 36, 26, 255 }` $\rightarrow$ `Color{ 8, 16, 12, 255 }`).
  - Title: Bright Green (`Color{ 80, 245, 120, 255 }`).
  - Subtitle: Gold (`Color{ 255, 215, 80, 255 }`).
- **Defeat State**:
  - Gradient: Crimson/Obsidian (`Color{ 36, 16, 20, 255 }` $\rightarrow$ `Color{ 14, 8, 10, 255 }`).
  - Title: Blood Red (`Color{ 245, 75, 75, 255 }`).
  - Subtitle: Rose Gray (`Color{ 210, 180, 180, 255 }`).
- **Draw State**:
  - Gradient: Deep Amber/Bronze (`Color{ 32, 26, 16, 255 }` $\rightarrow$ `Color{ 14, 12, 8, 255 }`).
  - Title: Warm Amber (`Color{ 255, 195, 70, 255 }`).
  - Subtitle: Muted Khaki (`Color{ 220, 200, 160, 255 }`).

