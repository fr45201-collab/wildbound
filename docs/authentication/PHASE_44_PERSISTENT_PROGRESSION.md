# Wildbound — Persistent Account Progression & XP Curve

## 1. Overview

Wildbound features a persistent account progression system that rewards contenders for match participation, survival time, eliminations, and achievement completions. Progression is authoritatively managed by `WildboundBackend` with strict idempotency and audit logs.

---

## 2. Mathematical XP Curve & Progression Formula

Account levels scale progressively using a quadratic growth function designed to provide rapid initial progression (levels 1-10) transitioning into meaningful mastery milestones up to level 100:

### 2.1 Formula
$$\text{XP}_{\text{required}}(L) = 500 \cdot L + 150 \cdot (L - 1)^2$$

Where $L$ is the target level being unlocked.

### 2.2 Threshold Table (Sample Levels)

| Level | XP to Next Level | Cumulative Total XP | Milestone Reward |
| :--- | :--- | :--- | :--- |
| **1** | 500 XP | 0 XP | Starter Contender Emblem |
| **2** | 650 XP | 500 XP | 10 Reward Points |
| **3** | 1,100 XP | 1,150 XP | 10 Reward Points |
| **5** | 2,900 XP | 3,850 XP | Rare Banner Unlock + 25 Points |
| **10** | 9,050 XP | 24,100 XP | Veteran Contender Title + 50 Points |
| **25** | 46,100 XP | 243,250 XP | Master Contender Skin + 100 Points |
| **50** | 165,100 XP | 1,605,500 XP | Elite Gladiator Frame + 250 Points |
| **100** | 610,100 XP | 10,750,000 XP | Grand Arena Champion Crown |

---

## 3. Match XP Calculation

When a match concludes, the backend automatically awards XP to every participant based on verified performance metrics:

$$\text{XP}_{\text{match}} = \text{XP}_{\text{placement}} + (E \cdot 100) + \lfloor D \cdot 0.25 \rfloor + \lfloor S \cdot 0.5 \rfloor$$

- **Placement XP ($\text{XP}_{\text{placement}}$)**:
  - 1st Place (Victory): +500 XP
  - Top 3 (Podium): +300 XP
  - Top 5: +200 XP
  - Top 10: +100 XP
  - Participation: +50 XP
- **Elimination XP**: 100 XP per confirmed elimination.
- **Damage XP**: 1 XP per 4 damage dealt ($\approx 0.25$).
- **Survival XP**: 1 XP per 2 seconds survived ($\approx 0.5$).
- **Reward Points**: Granted on level-up and bonus placements (e.g. 1st place grants +25 points).

---

## 4. Idempotency & Duplicate Reward Prevention

To guarantee that replayed network packets or retry attempts from dedicated servers do not result in double-grants of XP or reward points, Wildbound utilizes an auditable event ledger:

```sql
CREATE TABLE progression_events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    source_type VARCHAR(32) NOT NULL, -- e.g. 'match', 'achievement', 'admin_grant'
    source_id VARCHAR(128) NOT NULL,  -- match_id or achievement_id
    xp_delta INT NOT NULL,
    reward_points_delta INT NOT NULL DEFAULT 0,
    reason TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_progression_source UNIQUE (user_id, source_type, source_id)
);
```

- When `AwardMatchProgression` runs inside the match ingestion database transaction, it first checks `HasProgressionEvent(user_id, "match", match_id)`.
- If an event already exists, the function returns `ErrDuplicateProgressionEvent`, skipping the balance update while returning the existing progression state safely.
- If no event exists, it transactionally writes the event ledger record and updates `player_progression`.

---

## 5. API Endpoints

- **`GET /api/v1/players/me/progression`**:
  Returns current account level, total XP, current level progress, XP required for next level, percentage progress, and available reward points.
- **`GET /api/v1/players/me/progression/events`**:
  Returns paginated list of recent progression transactions with timestamps, sources, and reasons.

