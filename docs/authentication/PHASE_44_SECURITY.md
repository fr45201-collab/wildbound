# Wildbound — Authentication Security & Threat Analysis

## 1. Threat Modeling & Defensive Controls

Phase 44 implements defensive engineering across all common attack vectors targeting online multiplayer gaming authentication:

| Threat Vector | Potential Impact | Defensive Mitigation in Wildbound |
| :--- | :--- | :--- |
| **Credential Stuffing / Brute Force** | Account takeover via dictionary attack | Bcrypt cost factor 12 enforces computational cost (~150ms per attempt). Uniform error responses (`INVALID_CREDENTIALS`) prevent username enumeration. |
| **Token Theft / Man-in-the-Middle** | Impersonation using intercepted bearer tokens | Short 15-minute JWT lifetime minimizes window of vulnerability. HTTPS transport required in production. |
| **Token Replay / Refresh Reuse** | Persistent rogue access using stolen refresh token | Active token rotation with automated replay detection: submitting an already-rotated or revoked token immediately invalidates ALL sessions for that user account. |
| **SQL Injection** | Data exfiltration / database tampering | 100% parameterized queries using `pgx` prepared statements. Zero string concatenation in SQL queries. |
| **Privilege Escalation** | Contender claiming administrative or server rights | Role claims embedded in signed JWT tokens. Dedicated server endpoints require private `X-Server-Token` matching environment config. |
| **Identity Spoofing in Matches** | Falsifying winner or stats during match reporting | Coordinator binds verified JWT UUIDs to match reservations. Dedicated server reports results directly to backend using trusted server token. |

---

## 2. Password Architecture & Storage

- **Bcrypt Hashing**: `bcrypt.GenerateFromPassword(pwd, 12)`
  - Includes a random 128-bit salt per user.
  - Cost factor 12 requires $2^{12} = 4096$ iterations of the Blowfish key scheduling algorithm.
- **Input Constraints**:
  - Minimum 8 characters, maximum 72 characters (Bcrypt limitation safeguard).
  - Enforced entropy: Requires at least one uppercase letter, one lowercase letter, and one digit.
- **Password Modification**:
  - Requires verification of current password before allowing new password assignment.
  - Automatically revokes all existing sessions upon successful change, forcing immediate re-authentication.

---

## 3. Secrets Management & Environment Hygiene

- **Configuration File**: All sensitive secrets (`JWT_SECRET`, `SERVER_SECRET`, `DATABASE_URL`) are loaded strictly from environment variables or `backend/.env`.
- **Git Hygiene**: `backend/.env` is explicitly documented in `.gitignore` and verified uncommitted.
- **Client Sanitization**: The game client executable never bundles backend database credentials, private keys, or server secrets.

