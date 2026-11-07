# Wildbound — Achievement Catalog & Progress Tracking

## 1. Overview

Phase 44 introduces the foundational data architecture and API services for Wildbound's achievement system. Achievements represent persistent milestones across combat, survival, mastery, and exploration categories that award account XP and reward points upon unlock.

---

## 2. Schema Architecture

### 2.1 Achievement Definitions (`achievement_definitions`)
Static catalog of available achievements seeded via database migrations:

```sql
CREATE TABLE achievement_definitions (
    id VARCHAR(64) PRIMARY KEY,
    achievement_key VARCHAR(64) NOT NULL UNIQUE,
    display_name VARCHAR(128) NOT NULL,
    description TEXT NOT NULL,
    category VARCHAR(32) NOT NULL, -- combat, survival, mastery, progression
    target_value INT NOT NULL,
    reward_xp INT NOT NULL DEFAULT 0,
    reward_points INT NOT NULL DEFAULT 0,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

### 2.2 Player Progress (`player_achievements`)
Tracks individual progress and completion status per player:

```sql
CREATE TABLE player_achievements (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    achievement_id VARCHAR(64) NOT NULL REFERENCES achievement_definitions(id) ON DELETE CASCADE,
    progress INT NOT NULL DEFAULT 0,
    target INT NOT NULL,
    is_unlocked BOOLEAN NOT NULL DEFAULT FALSE,
    unlocked_at TIMESTAMPTZ,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_player_achievement UNIQUE (user_id, achievement_id)
);
```

---

## 3. Seeded Foundational Achievements

The initial catalog establishes five key milestones:

| ID | Name | Category | Target | Rewards |
| :--- | :--- | :--- | :--- | :--- |
| `ach_first_blood` | First Blood | Combat | 1 Elimination | 150 XP, 10 Points |
| `ach_arena_victor` | Arena Victor | Mastery | 1 Match Victory | 500 XP, 50 Points |
| `ach_seasoned_survivor`| Seasoned Survivor | Survival | 1,000 Seconds Survived | 300 XP, 25 Points |
| `ach_sharpshooter` | Sharpshooter | Combat | 2,500 Damage Dealt | 350 XP, 30 Points |
| `ach_veteran_contender`| Veteran Contender | Progression | 10 Matches Completed | 400 XP, 40 Points |

---

## 4. Match Ingestion Integration

During match result ingestion (`matches.Service.SubmitMatchResult`), the backend automatically updates achievement progress inside the database transaction:
- **`ach_veteran_contender`**: Progress incremented by 1 for every match participant.
- **`ach_first_blood`**: Progress incremented by total eliminations if eliminations > 0.
- **`ach_arena_victor`**: Progress incremented by 1 if contender won the match.
- **`ach_sharpshooter`**: Progress incremented by integer damage dealt.
- **`ach_seasoned_survivor`**: Progress incremented by seconds survived in the match.

When progress reaches or exceeds `target_value`, `is_unlocked` flips to `true` and `unlocked_at` is stamped.

---

## 5. API Endpoints

- **`GET /api/v1/achievements`**:
  Returns the complete active achievement catalog.
- **`GET /api/v1/players/me/achievements`**:
  Returns the calling player's achievement list, showing current progress, unlock status, target, and completion date.

