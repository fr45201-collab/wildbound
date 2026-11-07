# Wildbound — Phase 44: Initial Repository Audit

**Phase:** 44 — Authentication & Player Persistence  
**Date:** 2026-11-08  
**Auditor:** Wildbound Architecture Agent  
**Status:** **AUDIT COMPLETE**

---

## 1. Executive Summary

A comprehensive repository audit was conducted across Phase 41 (Lobby & Matchmaking Coordinator), Phase 42 (Go Backend & C++ HTTP Integration), and Phase 43 (PostgreSQL Database Infrastructure & Repositories).

The repository contains an established, functional authentication and database foundation:
- Password hashing uses `bcrypt` (`golang.org/x/crypto/bcrypt`) with standard cost.
- JWT access tokens (15-minute TTL) and SHA-256 hashed refresh tokens (7-day TTL) are implemented.
- PostgreSQL database infrastructure uses `jackc/pgx/v5` with connection pooling, transactional helpers (`WithTransaction`), and migration versioning (`wildbound-migrate.exe`).
- Migrations `000001` (core schema), `000002` (cosmetics seed), and `000003` (last login & inventory) are verified and applied.

However, significant functionality required for Phase 44 is currently missing or partially implemented:
1. **Multi-Session Management**: Session tracking exists solely as raw refresh token rows without device metadata, listing, individual session revocation, or logout-all.
2. **Account Settings**: No persistent player settings table or endpoints exist (language, region, privacy preferences).
3. **Account Progression**: No persistent account XP, level progression curves, progression events, or reward points exist. (Match results currently only update aggregate kill/death stats in `player_statistics`).
4. **Achievements**: No achievement definitions or player unlock records exist.
5. **Account Management**: No endpoints exist for changing passwords, changing emails, or retrieving account security metadata.
6. **Coordinator Identity Verification**: While `PlayerSession` in `WildboundCoordinator` has `persistentUserId` and `authToken`, the Coordinator accepts them without cryptographically verifying the JWT token or extracting the verified user ID.
7. **Client UI**: The C++ client has `BackendClient` with network calls, but lacks an in-game graphical login, registration, and profile/progression UI in the main menus.

---

## 2. Detailed Component Audit

### 2.1 Authentication & Session Management (`backend/internal/auth`)

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| Registration (`POST /auth/register`) | Implemented | Validates email, username, password complexity, creates user, profile, stats, starter cosmetics. | Wrap with initial progression, settings, and achievement initialization. |
| Login (`POST /auth/login`) | Implemented | Validates credentials via bcrypt, checks status, returns JWT access token + refresh token, updates `last_login_at`. | Bind to session metadata (`user_sessions` or enhanced session model with device info). |
| Refresh (`POST /auth/refresh`) | Implemented | Rotates refresh token, invalidating the old token hash. | Implement explicit token reuse / theft detection (if revoked token is re-submitted, revoke entire session family). |
| Session Listing | **Missing** | No endpoint exists to view active sessions. | Implement `GET /api/v1/auth/sessions`. |
| Individual Session Revocation | **Missing** | No endpoint exists to revoke a specific session. | Implement `DELETE /api/v1/auth/sessions/{session_id}`. |
| Logout-All | **Missing** | Cannot invalidate all devices at once. | Implement `POST /api/v1/auth/logout-all`. |
| Password Change | **Missing** | No endpoint to update password. | Implement `PATCH /api/v1/account/password` with old password verification and session invalidation. |
| Email Change | **Missing** | No endpoint to update email. | Implement `PATCH /api/v1/account/email`. |
| Security Status | **Missing** | No endpoint for account security overview. | Implement `GET /api/v1/account/security`. |

### 2.2 Player Profiles (`backend/internal/profiles`)

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| `GET /players/me` | Implemented | Returns private profile of authenticated contender. | Retain; ensure consistent response model. |
| `PATCH /players/me` | Implemented | Updates display name, biography, avatar cosmetic. | Retain; verify avatar ownership checks. |
| `GET /players/{player_id}` | Implemented | Returns public profile of another contender. | Filter fields based on `player_settings.privacy_profile_visibility`. |

### 2.3 Account Settings (New Domain)

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| `player_settings` Table | **Missing** | No table exists. | Create table in Migration `000004` (`user_id`, `language`, `region`, `preferred_game_mode`, `privacy_profile_visibility`, `privacy_match_history`, `allow_friend_requests`). |
| Settings Service & Repository | **Missing** | No repository or service exists. | Implement `repository.SettingsRepository` and `settings.Service`. |
| Settings Endpoints | **Missing** | No HTTP routes exist. | Implement `GET /api/v1/players/me/settings` and `PATCH /api/v1/players/me/settings`. |

### 2.4 Persistent Progression (New Domain)

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| `player_progression` Table | **Missing** | No table exists. | Create table in Migration `000004` (`user_id`, `account_level`, `total_xp`, `available_reward_points`, `matches_completed`, `progression_version`). |
| `progression_events` Table | **Missing** | No audit log exists. | Create table in Migration `000004` (`id`, `user_id`, `source_type`, `source_id`, `xp_delta`, `reward_points_delta`, `reason`, unique constraint on `(user_id, source_type, source_id)`). |
| Progression Curve Engine | **Missing** | No data-driven curve calculation. | Implement `progression.Calculator` with configurable level curve. |
| Match Ingestion Integration | Partial | Dedicated server submits match results, but backend only updates kill/death stats. | Update `matches.Service.SubmitResult` to calculate XP, award progression events, and advance player levels transactionally. |
| Progression Endpoints | **Missing** | No HTTP routes exist. | Implement `GET /api/v1/players/me/progression` and public summary endpoint. |

### 2.5 Achievements Foundation (New Domain)

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| `achievement_definitions` Table | **Missing** | No table exists. | Create table in Migration `000004` and seed foundational achievements (First Contender, First Blood, Arena Victor, Survivor, Master Hunter). |
| `player_achievements` Table | **Missing** | No table exists. | Create table in Migration `000004` (`user_id`, `achievement_id`, `progress`, `target`, `is_unlocked`, `unlocked_at`). |
| Achievement Endpoints | **Missing** | No HTTP routes exist. | Implement `GET /api/v1/achievements` and `GET /api/v1/players/me/achievements`. |

### 2.6 Coordinator & Dedicated Server Integration

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| Coordinator Auth Token Handling | Partial | `PlayerSession` stores `authToken`, but Coordinator does not validate JWT signatures or enforce claims. | Enhance Coordinator to validate token signature or claims and populate `persistentUserId`. |
| Server Participant Identity | Partial | Dedicated server submits display names in match results; `user_id` is set if participant provides it. | Ensure persistent `user_id` is passed through reservation tokens to the game server. |
| Outbox Durable Submission | Implemented | Dedicated server durably spools match results to disk and retries with backoff. | No changes required to transport layer; backend handles progression upon ingestion. |

### 2.7 C++ Client UI & Networking

| Feature | Current Status | Finding | Action Required for Phase 44 |
|---|---|---|---|
| `BackendClient` | Implemented | Asynchronous HTTP client wrapping login, register, refresh, logout, profile, and stats. | Add methods for sessions, settings, progression, achievements, and password changes. |
| Main Menu Authentication UI | **Missing** | Main menu only has buttons ("SOLO", "DUOS", "SQUADS", "PRACTICE", "SETTINGS", "QUIT"). | Add Contender Account status bar, Login / Register modal, Profile / Progression inspection modal. |
| Session Restoration | **Missing** | Does not restore saved sessions on startup. | Provide session loading mechanism from persistent user configuration. |

---

## 3. Database Migration Requirements (Migration `000004`)

To support Phase 44 without altering existing Phase 42/43 tables:
1. Create `user_sessions` (or enhance `refresh_tokens`) with `device_identifier`, `device_name`, `ip_address`, `last_used_at`, `revocation_reason`.
2. Create `player_settings` with defaults for language, region, game mode, and privacy.
3. Create `player_progression` with `account_level`, `total_xp`, `available_reward_points`, `matches_completed`.
4. Create `progression_events` with strict unique constraint on `(user_id, source_type, source_id)` for duplicate reward prevention.
5. Create `achievement_definitions` and `player_achievements`.
6. Seed foundational achievements into `achievement_definitions`.

---

## 4. Implementation Plan & Phased Sequence

1. **Step 1: Database Migration `000004`**: Author `.up.sql` and `.down.sql`, apply via `wildbound-migrate.exe`.
2. **Step 2: Repository Layer Extensions**: Add `sessions_repository`, `settings_repository`, `progression_repository`, and `achievements_repository` in `backend/internal/repository/`.
3. **Step 3: Session Management & Account Security**: Implement enhanced token theft detection, `logout-all`, session listing, session revocation, and password/email modification.
4. **Step 4: Player Settings Service & API**: Implement `GET` and `PATCH /api/v1/players/me/settings`.
5. **Step 5: Progression & Achievement System**: Implement data-driven XP curve, progression event audit log, and tie match result submission directly to XP awards.
6. **Step 6: Coordinator & Server Identity Integration**: Propagate and validate verified `persistentUserId`.
7. **Step 7: C++ Client UI & HTTP Integration**: Add authentication modal, progression display, and session restoration in raylib client.
8. **Step 8: Automated Verification**: Comprehensive unit, integration, concurrency, security, and end-to-end tests.
9. **Step 9: Complete Documentation Suite**: Deliver all 14 markdown documents in `docs/authentication/`.

