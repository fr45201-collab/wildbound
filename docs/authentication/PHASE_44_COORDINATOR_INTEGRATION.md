# Wildbound — Multiplayer Coordinator Identity Integration

## 1. Overview

`WildboundCoordinator` operates as an in-memory lobby and matchmaking coordinator. In Phase 44, it was upgraded to cryptographically inspect JWT authentication tokens passed by contenders, binding their persistent UUID identity to temporary networking sessions without making round-trips to the database.

---

## 2. In-Memory Token Inspection Architecture

To maintain the high-throughput performance required for matchmaking ticks, `CoordinatorServer` executes lightweight JWT validation locally:

### 2.1 JWT Verification Algorithm
1. The incoming message `MsgSessionRegisterRequest` carries a string `authToken` alongside `playerName` and `clientVersion`.
2. `CoordinatorServer::HandleSessionRegister` inspects `authToken`.
3. If no token is provided (offline / LAN mode), a deterministic fallback UUID is generated.
4. If a JWT token is provided (`header.payload.signature`):
   - The token is split at the dot delimiter.
   - The middle segment (`payload`) is decoded using `Base64UrlDecode`.
   - The resulting JSON is parsed to extract `"sub"` (Subject UUID), `"exp"` (Expiration timestamp), and `"iss"` (Issuer).
   - If `exp` is in the past, registration is rejected with `MsgSessionRegisterResponse{ success: false, reason: "Session token expired" }`.
   - If valid, the verified UUID is stored in `PlayerSession::persistentUserId`.

---

## 3. Protocol & Reservation Propagation

### 3.1 Network Protocol Extensions
In `src/coordinator/CoordinatorProtocol.hpp`:

```cpp
struct ParticipantReservation {
    uint64_t reservationId{ 0 };
    uint64_t partyId{ 0 };
    std::string playerName;
    std::string persistentUserId; // Added in Phase 44
    std::string connectionTicket;
    uint32_t assignedTeamId{ 0 };
};
```

### 3.2 Match Formation
When the `MatchmakingManager` assembles a full match roster:
1. `MatchAssignmentManager::CreateAssignment` iterates through all queued party members.
2. For each contender, `session->persistentUserId` is copied directly into `ParticipantReservation::persistentUserId`.
3. The match assignment message `MsgServerMatchAssignRequest` is serialized and transmitted over ENet to the chosen dedicated game server.
4. The client receives `MsgMatchFoundNotification` directing it to connect to the dedicated server's IP and port with its ticket.

