# Wildbound — Identity Architecture & System Integration

## 1. Architectural Topology

Wildbound's identity architecture establishes clear separation of responsibilities between persistent cloud services, coordination infrastructure, and authoritative match servers:

```
+-------------------------------------------------------------+
|                      PostgreSQL                             |
|       (wildbound_db - Users, Sessions, Progression)         |
+------------------------------+------------------------------+
                               ^
                               | (SQL Connection Pool)
+------------------------------v------------------------------+
|                     WildboundBackend                        |
|       (Go REST API - Auth, Settings, Persistence)           |
+---------------+------------------------------+--------------+
                ^                              ^
                | (REST API / Bearer JWT)      | (Internal Server Secret)
+---------------+--------------+  +------------+--------------+
|       WildboundClient        |  |      WildboundServer      |
|    (C++20 / raylib Client)   |  |   (Dedicated Match Host)  |
+---------------+--------------+  +------------+--------------+
                |                              ^
                | (ENet / Session Ticket)      | (ENet Authoritative Sim)
                +--------------+---------------+
                               |
                +--------------v---------------+
                |     WildboundCoordinator     |
                |   (Lobbies & Matchmaking)    |
                +------------------------------+
```

---

## 2. Responsibilities and Trust Boundaries

| Component | Technology | Identity Responsibility | State Duration |
| :--- | :--- | :--- | :--- |
| **WildboundBackend** | Go, PostgreSQL | Source of persistent truth. Credentials, JWT issuing, settings, XP curve, achievements catalog. | Permanent |
| **WildboundCoordinator** | C++20, ENet | In-memory multiplayer session coordination. Verifies client JWTs, links contender sessions to persistent UUIDs, allocates match reservations. | Ephemeral (Session / Lobby) |
| **WildboundServer** | C++20, ENet | Authoritative physical simulation. Validates reservation tokens, tracks verified contenders in-game, compiles match results with persistent UUIDs. | Ephemeral (Match runtime) |
| **WildboundClient** | C++20, raylib | Renders authentication UI, stores local session token securely, passes JWT tickets during matchmaking. | Local user session |

---

## 3. The Identity Flow

1. **Authentication**: Player opens `AuthMenu` modal in the game client, enters credentials. The client contacts `WildboundBackend`, receiving a JWT access token and refresh token.
2. **Matchmaking Ticket**: When queuing for a match, the client includes the JWT in `MsgSessionRegisterRequest` sent to `WildboundCoordinator`.
3. **Coordinator Validation**: `CoordinatorServer` parses the JWT payload, checks expiration, extracts the subject UUID (`persistentUserId`), and validates against identity spoofing.
4. **Match Reservation**: Upon match formation, `MatchAssignmentManager` creates a `ParticipantReservation` embedding `persistentUserId` alongside the ephemeral `ticket`.
5. **Dedicated Server Verification**: The dedicated server receives the match assignment and waits for client connections. When the client connects with their ticket, the server binds the connection to the reservation's `persistentUserId`.
6. **Authoritative Result Ingestion**: When the match concludes, `DedicatedServer` compiles the match result JSON with all participants' persistent UUIDs and posts it to `/api/v1/internal/matches/results` using `X-Server-Token`.
7. **Progression Application**: The backend updates statistics, checks against duplicate grants, transactionally calculates level and XP increases, increments achievement counters, and records audit trail events.

