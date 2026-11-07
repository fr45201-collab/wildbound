# Wildbound — Player Profiles & Public Identity

## 1. Overview

Player profiles represent the public persona and identity of contenders in Wildbound. While sensitive data (such as emails, password hashes, and security logs) is strictly protected within the `users` and `user_sessions` tables, `player_profiles` defines the visible showcase for community interactions, leaderboards, and match lobbies.

---

## 2. Schema and Model

```sql
CREATE TABLE player_profiles (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    display_name VARCHAR(64) NOT NULL,
    avatar_url VARCHAR(255) NOT NULL DEFAULT 'default_avatar.png',
    title VARCHAR(64) NOT NULL DEFAULT 'Contender',
    bio TEXT NOT NULL DEFAULT '',
    banner_url VARCHAR(255) NOT NULL DEFAULT 'default_banner.png',
    reputation_score INT NOT NULL DEFAULT 100,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

---

## 3. Profile Access and Privacy Control

### 3.1 Self Profile Access
- **Endpoint**: `GET /api/v1/players/me/profile`
- **Authentication**: Bearer JWT
- **Behavior**: Returns the full profile of the authenticated player, regardless of privacy settings.

### 3.2 Public Profile Lookup
- **Endpoint**: `GET /api/v1/players/{id}/profile`
- **Authentication**: Optional (Public or Authenticated)
- **Privacy Enforcement**:
  - The backend joins with `player_settings` for the requested user ID.
  - If `profile_visibility` is `'private'`, HTTP 403 Forbidden (`PROFILE_PRIVATE`) is returned unless the requester is the profile owner.
  - If `profile_visibility` is `'friends'`, public access is restricted to mutual party/friend relationships.
  - If `profile_visibility` is `'public'`, the profile payload is rendered along with public match statistics (subject to `match_history_visibility`).

---

## 4. Profile Customization

Players update their profile via `PATCH /api/v1/players/me/profile`:
- **Allowed Fields**:
  - `display_name` (3-32 characters, filtered for profanity).
  - `avatar_url` (must be a valid relative path from the approved cosmetic catalog).
  - `title` (selected from unlocked achievement rewards or battle pass titles).
  - `bio` (up to 500 characters).
  - `banner_url` (selected from owned cosmetic banners).
- **Validation**: Rejects invalid characters and prevents arbitrary external URLs to prevent malicious asset injections.

