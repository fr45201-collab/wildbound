# WILDBOUND — PHASE 45: SECURITY & AUTHORIZATION SPECIFICATION

## 1. Security Architecture & Threat Vectors

The cosmetic ownership and equipment system in Wildbound protects game integrity and cosmetic exclusivity through defense-in-depth principles.

```
                      +------------------------------------------+
                      |               Untrusted Client           |
                      +------------------------------------------+
                                           |
                [Threat: Tampered Token / Fabricated Loadout Packet]
                                           v
                      +------------------------------------------+
                      |         Go API Gateway & Middleware      |
                      |  1. Validate JWT Signature & Expiration  |
                      |  2. Enforce Strict Rate Limiting         |
                      |  3. Validate Input Schemas & UUIDs       |
                      +------------------------------------------+
                                           |
                        [Threat: Equipping Unowned Cosmetics]
                                           v
                      +------------------------------------------+
                      |         Cosmetics Service Layer          |
                      |  1. Verify Item Exists in Catalogue      |
                      |  2. Verify User Ownership in DB Table    |
                      |  3. Enforce Concurrency Version Matching |
                      +------------------------------------------+
                                           |
                      +------------------------------------------+
                      |         PostgreSQL Database              |
                      |  1. ACID Transactions with Row Locking   |
                      |  2. Unique Constraint (user_id, slot)    |
                      |  3. Foreign Key Constraints              |
                      +------------------------------------------+
```

---

## 2. Protected Endpoints & Authorization Rules

| Endpoint | Method | Auth Scheme | Security Rule |
|---|---|---|---|
| `/api/v1/cosmetics` | `GET` | Public | Read-only; limits maximum query page size to 100 to prevent DoS. |
| `/api/v1/players/me/cosmetics` | `GET` | Bearer JWT | Returns only cosmetics owned by the authenticated caller (`auth.ContextKeyUserID`). |
| `/api/v1/players/me/loadout` | `GET` | Bearer JWT | Returns loadout belonging strictly to caller. |
| `/api/v1/players/me/loadout/equip` | `POST` | Bearer JWT | Enforces that user owns the item in `player_cosmetics`; returns 403 `COSMETIC_NOT_OWNED` if not owned. |
| `/api/v1/players/me/loadout/unequip` | `POST` | Bearer JWT | Caller can only unequip their own slots. |
| `/api/v1/internal/cosmetics/grant` | `POST` | `X-Server-Token` | Dedicated internal endpoint. Secret must match `ServerSecret`. Rejects unauthorized calls with 401. |
| `/api/v1/internal/players/{id}/loadout` | `GET` | `X-Server-Token` | Dedicated game server query. Secret verified; returns verified slots and asset IDs. |

---

## 3. Defense Against Exploitation Techniques

### 3.1 Memory Injection / Packet Tampering
- **Attack**: A user edits their client memory or network packet to spoof legendary wings or developer armor.
- **Defense**: Dedicated server completely ignores client appearance claims upon match connect. The server fetches appearance exclusively from the backend via `/api/v1/internal/players/{user_id}/loadout` using its internal secret. Any client tampering is wiped out and superseded by server truth.

### 3.2 Race Conditions & Inventory Duplication
- **Attack**: Rapid parallel requests to equip different items into the same slot to corrupt loadout state.
- **Defense**: 
  - Unique database constraint `uq_player_loadout_slot (user_id, equipment_slot)` guarantees only one item per slot at any instant.
  - Optimistic concurrency control via `player_loadouts.loadout_version`. Any out-of-order write returns HTTP 409 `LOADOUT_VERSION_CONFLICT`.

### 3.3 Replay Attacks
- All JWTs have short-lived access tokens (15 minutes).
- Refresh tokens are single-use rotation with token theft detection.
- Internal server communication requires matching server secrets over TLS.

