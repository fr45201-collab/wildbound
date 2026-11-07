# Wildbound — Session Management & Token Rotation Architecture

## 1. Overview

In Wildbound, player sessions are modeled as stateful persistent records in PostgreSQL (`user_sessions`), providing granular visibility into multi-device logins, active game clients, device metadata, and instant session termination capabilities.

---

## 2. Session Data Model

The `user_sessions` table maintains the active lifecycle of every connected client:

| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | `UUID` | `PRIMARY KEY` | Unique session identifier |
| `user_id` | `UUID` | `REFERENCES users(id) ON DELETE CASCADE` | Associated player account |
| `refresh_token_hash` | `VARCHAR(64)` | `NOT NULL, UNIQUE` | SHA-256 hash of active refresh token |
| `device_identifier` | `VARCHAR(128)` | `NOT NULL` | Client-generated stable hardware ID |
| `device_name` | `VARCHAR(128)` | `NOT NULL` | Human-readable name (e.g. "Main PC") |
| `ip_address` | `VARCHAR(45)` | `NOT NULL` | IPv4 / IPv6 client connection source |
| `user_agent` | `TEXT` | `NOT NULL` | Client HTTP user agent header |
| `client_version` | `VARCHAR(32)` | `NOT NULL` | Executable version identifier |
| `is_active` | `BOOLEAN` | `DEFAULT TRUE` | Active connection flag |
| `expires_at` | `TIMESTAMPTZ` | `NOT NULL` | Absolute session timeout timestamp |
| `revoked_at` | `TIMESTAMPTZ` | `NULL` | Timestamp of explicit revocation |
| `revocation_reason`| `VARCHAR(64)` | `NULL` | e.g. 'user_logout', 'token_theft_detected' |
| `created_at` | `TIMESTAMPTZ` | `DEFAULT NOW()` | Session establishment time |
| `updated_at` | `TIMESTAMPTZ` | `DEFAULT NOW()` | Last activity / refresh time |

---

## 3. Token Rotation and Theft Detection

### 3.1 Refresh Flow
When an access token approaches expiry, the client submits its refresh token to `POST /api/v1/auth/refresh`:
1. The backend hashes the presented raw refresh token using SHA-256.
2. The hash is queried against `user_sessions`.
3. If no session is found, HTTP 401 `INVALID_REFRESH_TOKEN` is returned.

### 3.2 Token Theft / Replay Detection
If the session record exists but `revoked_at` is NOT null or `is_active` is false, this indicates a **replay attack**:
- An attacker or compromised client attempted to use a refresh token that was already consumed during a previous legitimate rotation or revoked by the user.
- **Immediate Mitigation Action**:
  1. The backend detects token theft (`ErrTokenTheftSuspected`).
  2. The system invokes `RevokeAllUserSessions(ctx, session.UserID, "token_theft_detected")`.
  3. All active sessions for the compromised user account across all devices are immediately terminated.
  4. The request returns HTTP 401 with error code `TOKEN_THEFT_DETECTED`:
     ```json
     {
       "success": false,
       "data": null,
       "error": {
         "code": "TOKEN_THEFT_DETECTED",
         "message": "Security alert: suspicious token reuse detected. All sessions revoked."
       }
     }
     ```
  5. The legitimate user is forced to re-authenticate with their password, securing their account.

### 3.3 Normal Rotation
If the session is valid and active:
1. A new CSPRNG 256-bit refresh token is generated.
2. The session record in the database is updated with the new `refresh_token_hash`, and `updated_at` is bumped.
3. A fresh 15-minute JWT access token is minted.
4. The client receives the new token pair and replaces its stored credentials.

---

## 4. Multi-Device and Remote Revocation

Players can inspect and manage their active sessions from any client:
- **`GET /api/v1/auth/sessions`**:
  Returns all active and recently revoked sessions, showing device name, device ID, IP address, creation time, and last active time.
- **`DELETE /api/v1/auth/sessions/{id}`**:
  Allows a player on their desktop PC to revoke a session running on a misplaced laptop or mobile deck.
- **`POST /api/v1/auth/logout-all`**:
  Transactionally revokes all active sessions for the user with reason `user_logout_all`. All devices must re-login.

