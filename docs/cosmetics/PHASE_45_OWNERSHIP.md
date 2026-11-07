# Wildbound — Persistent Cosmetic Ownership Architecture

## 1. Overview

In Wildbound, cosmetic ownership is strictly account-scoped and managed authoritatively in PostgreSQL (`player_cosmetics`). Clients can never unilaterally declare or modify ownership records.

---

## 2. Data Model (`player_cosmetics`)

```sql
CREATE TABLE player_cosmetics (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cosmetic_id VARCHAR(64) NOT NULL REFERENCES cosmetic_definitions(id) ON DELETE CASCADE,
    acquired_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    acquisition_source VARCHAR(64) NOT NULL DEFAULT 'starter',
    is_equipped BOOLEAN NOT NULL DEFAULT false,
    CONSTRAINT uq_player_cosmetics_user_item UNIQUE (user_id, cosmetic_id)
);
```

### 2.1 Ownership Rules
- Duplicate ownership is prevented via unique constraint `uq_player_cosmetics_user_item`.
- Ownership grants require trusted operations:
  - Account provisioning grants default starter cosmetics (`starter`).
  - Store rewards or achievements invoke backend transactional grants (`granted`, `achievement_reward`).
  - Dedicated servers and administrative services use internal authenticated endpoints (`POST /api/v1/internal/cosmetics/grant`).

---

## 3. API Endpoints

### 3.1 List Owned Cosmetics (`GET /api/v1/players/me/cosmetics`)
- **Authentication**: Bearer JWT.
- **Response**: Array of owned cosmetics with metadata, acquisition timestamps, and equipped flags.

### 3.2 Internal Grant (`POST /api/v1/internal/cosmetics/grant`)
- **Authentication**: Internal Server Secret (`X-Server-Token`).
- **Body**:
  ```json
  {
    "user_id": "b2c6d708-3012-4217-a068-085eec567f1b",
    "cosmetic_id": "hat_iron_helmet_001",
    "source": "battle_pass_tier_1"
  }
  ```

