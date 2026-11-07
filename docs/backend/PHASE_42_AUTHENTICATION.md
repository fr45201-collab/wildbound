# Wildbound — Phase 42: Authentication & Identity

## 1. Authentication Architecture

Wildbound uses a dual-token authentication model combining **short-lived JSON Web Tokens (JWT)** for stateless request authorization and **opaque rotating refresh tokens** for long-lived session persistence.

```mermaid
sequenceDiagram
    autonumber
    actor Player as Contender (Client)
    participant Backend as Wildbound Backend
    participant DB as PostgreSQL 16
    participant Coord as Coordinator / GameServer

    Player->>Backend: POST /api/v1/auth/login (username/email, password)
    Backend->>Backend: Argon2id Verify Password Hash
    Backend->>Backend: Generate Access JWT (15 min) & Random Refresh Token (7 days)
    Backend->>DB: Store SHA-256(RefreshToken)
    Backend-->>Player: 200 OK (AccessToken, RefreshToken, UserProfile)

    Note over Player,Coord: Matchmaking & Gameplay Authorization
    Player->>Coord: MsgSessionRegisterRequest (displayName, authToken=AccessToken)
    Coord->>Coord: Validate Token & Bind Session to persistent UserID

    Note over Player,Backend: Refresh Token Rotation
    Player->>Backend: POST /api/v1/auth/refresh (RefreshToken)
    Backend->>DB: Check Token Hash, Revocation, Expiration
    Backend->>DB: Revoke Old Token, Store New Rotated Token Hash
    Backend-->>Player: 200 OK (New AccessToken, New RefreshToken)

    Note over Player,Backend: Graceful Logout
    Player->>Backend: POST /api/v1/auth/logout (RefreshToken)
    Backend->>DB: UPDATE refresh_tokens SET revoked_at = NOW()
    Backend-->>Player: 200 OK (Session terminated)
```

## 2. Password Hashing Specification

- **Algorithm:** Argon2id (RFC 9106 recommended configuration).
- **Time Cost ($t$):** 3 iterations.
- **Memory Cost ($m$):** 64 MiB (65,536 KiB).
- **Parallelism ($p$):** 4 threads.
- **Salt:** 16 cryptographically random bytes via `crypto/rand`.
- **Key Length:** 32 bytes digest.
- **Verification:** Constant-time comparison via `subtle.ConstantTimeCompare` prevents timing side-channel attacks.

## 3. JWT Access Token Specification

- **Signing Algorithm:** HMAC-SHA256 (`HS256`).
- **Validity Window:** 15 minutes (`AccessTTLMinutes: 15`).
- **Standard Claims:**
  - `iss`: `"wildbound"`
  - `aud`: `"wildbound-client"`
  - `sub`: Player UUID
  - `iat`: Issuance timestamp (Unix seconds)
  - `exp`: Expiration timestamp (Unix seconds)
  - `jti`: Unique token UUID
- **Custom Claims:**
  - `user_id`: UUID string
  - `username`: Contender handle string

## 4. Refresh Token Security & Rotation

- **Entropy:** 32 cryptographically secure random bytes generated with OS CSPRNG (`crypto/rand`).
- **Encoding:** URL-safe Base64 without padding.
- **Database Storage:** SHA-256 hash digest (`token_hash VARCHAR(64) UNIQUE`). The raw refresh token is returned exclusively to the client upon generation and is never retained in plaintext in logs, caches, or database tables.
- **Automatic Rotation:** Every invocation of `/api/v1/auth/refresh` atomically revokes the presented refresh token and issues a fresh token pair.
- **Replay Detection:** If an already-revoked refresh token is re-submitted, the system rejects the request (`401 Unauthorized`), mitigating stolen token replay.

