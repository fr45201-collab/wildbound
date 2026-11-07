# Wildbound — Authentication Architecture & API Specification

## 1. Executive Summary

Phase 44 introduces an authoritative, persistent authentication system for **Wildbound**, centralizing player identity across the Go Backend (`wildbound-backend`), Multiplayer Coordinator (`WildboundCoordinator`), Authoritative Dedicated Game Servers (`WildboundServer`), and the C++20 raylib Game Client (`wildbound`).

The authentication architecture enforces secure password storage via `bcrypt`, short-lived JSON Web Tokens (JWT) for stateless service-to-service validation, and cryptographically secure database-backed refresh tokens enabling granular session management and instant revocation.

---

## 2. Authentication Lifecycle

### 2.1 Player Registration (`POST /api/v1/auth/register`)
- **Inputs**: `username` (3-32 chars, alphanumerics + underscores), `email` (RFC 5322 compliant), `password` (min 8 chars, mixed case, numbers, symbols), optional `display_name`, `device_identifier`, `device_name`.
- **Database Processing**:
  1. Input sanitization and format validation.
  2. Uniqueness check across `users` table for both username and email.
  3. Bcrypt password hashing at cost factor 12.
  4. Insertion into `users` table returning primary UUID.
  5. Automatic atomic provisioning of default records:
     - `player_profiles` (level 1, default avatar, default title).
     - `player_settings` (default language 'en', region 'auto', preferred mode 'battle_royale').
     - `player_progression` (level 1, total_xp 0, reward_points 0).
  6. Generation of initial session with dual-token issuance.
- **Outputs**: HTTP 201 Created containing User entity and Token Pair (`access_token`, `refresh_token`).

### 2.2 Player Login (`POST /api/v1/auth/login`)
- **Inputs**: `email_or_username`, `password`, `device_identifier`, `device_name`, `client_version`.
- **Verification**:
  1. Identity lookup in `users` table by case-insensitive email or username.
  2. Verification that account status is `active`.
  3. Constant-time bcrypt comparison via `bcrypt.CompareHashAndPassword`.
  4. Extraction of client IP and `User-Agent`.
  5. Creation of a session record in `user_sessions`.
  6. Issuance of JWT access token and opaque 256-bit cryptographically secure refresh token.
- **Outputs**: HTTP 200 OK with User entity and Token Pair.

---

## 3. Cryptographic Standards & Token Architecture

### 3.1 Password Hashing
- **Algorithm**: `bcrypt` (`golang.org/x/crypto/bcrypt`).
- **Cost Factor**: `12` (~150ms calculation time, resilient against GPU-based offline dictionary attacks).
- **Salt Generation**: Automatically managed per-hash by bcrypt's CSPRNG salt generator.

### 3.2 Access Tokens (JWT)
- **Algorithm**: HMAC-SHA256 (`HS256`).
- **Secret**: `JWT_SECRET` from environment configuration (min 32 bytes).
- **Expiration**: 15 minutes (`900` seconds).
- **Claims Schema**:
  ```json
  {
    "sub": "b2c6d708-3012-4217-a068-085eec567f1b",
    "username": "StormStriker",
    "session_id": "7f8b9190-6da4-4f2b-866d-1bf9ec92ab80",
    "role": "player",
    "iss": "wildbound-backend",
    "exp": 1794285600,
    "iat": 1794284700
  }
  ```
- **Validation**:
  - Statelessly parsed and verified by Go backend middleware.
  - Parsed by `WildboundCoordinator` to bind `persistentUserId` without requiring database queries.

### 3.3 Refresh Tokens
- **Format**: 64-character hexadecimal representation of 32 CSPRNG bytes (`crypto/rand`).
- **Expiration**: 30 days (`2592000` seconds).
- **Storage**: Stored in `user_sessions` alongside client metadata, hash of the active refresh token, device identifiers, and revocation timestamps.
- **Rotation**: On every token refresh call (`POST /api/v1/auth/refresh`), the old refresh token is marked used and superseded by a new token.

---

## 4. REST Endpoints Specification

| Endpoint | Method | Auth Required | Description |
| :--- | :--- | :--- | :--- |
| `/api/v1/auth/register` | `POST` | No | Creates a new account with settings & progression |
| `/api/v1/auth/login` | `POST` | No | Authenticates credentials and creates device session |
| `/api/v1/auth/refresh` | `POST` | No | Rotates refresh token and issues new access token |
| `/api/v1/auth/logout` | `POST` | Yes (Bearer) | Revokes the current session immediately |
| `/api/v1/auth/logout-all`| `POST` | Yes (Bearer) | Revokes all active sessions across all devices |
| `/api/v1/auth/sessions` | `GET` | Yes (Bearer) | Lists all active and recent sessions for user |
| `/api/v1/auth/sessions/{id}`| `DELETE` | Yes (Bearer) | Revokes a specific remote session |

---

## 5. Security & Error Handling

- **Uniform Invalid Credentials**: Returns generic `401 Unauthorized` (`INVALID_CREDENTIALS`) on both unknown user and incorrect password to prevent enumeration attacks.
- **Rate Limiting Readiness**: Standard HTTP 429 status code reserved for brute-force protection.
- **Context Identity Propagation**: Access token subject UUID is placed in standard Go request context (`r.Context()`) via `AuthMiddleware`.

