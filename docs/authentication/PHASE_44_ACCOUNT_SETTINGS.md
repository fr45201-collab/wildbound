# Wildbound — Persistent Account Settings

## 1. Overview

Player settings in Wildbound persist user preferences across all devices and game clients. Settings are stored authoritatively in PostgreSQL (`player_settings`) and synchronized seamlessly whenever a player logs in or modifies preferences in the client options menu.

---

## 2. Database Schema

```sql
CREATE TABLE player_settings (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    language VARCHAR(10) NOT NULL DEFAULT 'en',
    region VARCHAR(32) NOT NULL DEFAULT 'auto',
    preferred_game_mode VARCHAR(32) NOT NULL DEFAULT 'battle_royale',
    profile_visibility VARCHAR(16) NOT NULL DEFAULT 'public',
    match_history_visibility VARCHAR(16) NOT NULL DEFAULT 'public',
    allow_friend_invites BOOLEAN NOT NULL DEFAULT TRUE,
    crossplay_enabled BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

---

## 3. Allowed Enumerations & Validation Rules

To prevent corrupted settings state, the service validates all incoming patch values against strict enumeration sets:

- **Languages (`language`)**:
  `en` (English), `es` (Spanish), `fr` (French), `de` (German), `ja` (Japanese), `zh` (Chinese), `pt` (Portuguese), `ru` (Russian).
- **Regions (`region`)**:
  `auto` (Automatic lowest-ping detection), `us-east`, `us-west`, `eu-central`, `asia-east`, `sa-east`, `oce-sydney`.
- **Game Modes (`preferred_game_mode`)**:
  `battle_royale`, `solo`, `duos`, `squads`, `deathmatch`, `training`.
- **Visibility Toggles (`profile_visibility`, `match_history_visibility`)**:
  `public` (Visible to everyone), `friends` (Only party members / mutual friends), `private` (Hidden from public queries).

---

## 4. API Endpoints

### 4.1 Retrieve Settings
- **Method & Route**: `GET /api/v1/players/me/settings`
- **Auth**: Bearer JWT
- **Response**:
  ```json
  {
    "success": true,
    "data": {
      "user_id": "8fbb1a90-3498-4b71-b068-3e4a3b1a4570",
      "language": "en",
      "region": "auto",
      "preferred_game_mode": "battle_royale",
      "profile_visibility": "public",
      "match_history_visibility": "public",
      "allow_friend_invites": true,
      "crossplay_enabled": true,
      "updated_at": "2026-11-08T00:30:00Z"
    }
  }
  ```

### 4.2 Patch Settings
- **Method & Route**: `PATCH /api/v1/players/me/settings`
- **Auth**: Bearer JWT
- **Body**: Partial JSON payload containing only the fields being updated.
- **Error Handling**: Rejects invalid language codes or disallowed region strings with HTTP 400 Bad Request (`INVALID_SETTINGS_DATA`).

