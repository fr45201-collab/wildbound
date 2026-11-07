# Wildbound — Phase 42: Coordinator Integration & Identity Verification

## 1. Overview
`WildboundCoordinator` is the pre-match session, party, and matchmaking coordinator implemented in Phase 41. It organizes contenders into groups and matches them with available dedicated game servers.

In Phase 42, the Coordinator integrates with the persistent identity layer by associating client connections with verified backend user accounts.

```mermaid
sequenceDiagram
    autonumber
    actor Contender as Game Client
    participant Backend as Wildbound Backend
    participant Coord as Wildbound Coordinator
    participant Server as Dedicated Game Server

    Contender->>Backend: Authenticate (Login)
    Backend-->>Contender: Access Token (JWT with UserID & Username)

    Note over Contender,Coord: Pre-Match Registration
    Contender->>Coord: MsgSessionRegisterRequest (displayName, authToken)
    Coord->>Coord: Validates Token & Binds SessionId to UserID
    Coord-->>Contender: MsgSessionRegisterResponse (sessionId)

    Note over Contender,Coord: Party & Matchmaking Queue
    Contender->>Coord: MsgMatchmakingQueueRequest(solo_br)
    Coord->>Coord: Forms match with assigned DedicatedServer

    Note over Coord,Server: Match Reservation
    Coord->>Server: MsgMatchReserveRequest (matchId, participants=[{userId, matchAuthToken}])
    Server-->>Coord: MsgMatchReserveResponse (success)
    Coord-->>Contender: MsgMatchRedirectNotification (serverIp, port, matchAuthToken)

    Note over Contender,Server: Connect to Game Server
    Contender->>Server: MsgConnect (matchAuthToken)
    Server->>Server: Validates token against reserved list
    Server-->>Contender: Welcome (Authoritative gameplay commences)
```

## 2. Session Protocol Enhancement
- `MsgSessionRegisterRequest`:
  - `displayName`: Contender visual name.
  - `clientVersion`: Client semantic version (e.g. `"1.0.0"`).
  - `authToken`: JWT access token issued by the backend.
- `PlayerSession`:
  - `SessionId sessionId`: Ephemeral uint64 coordinator session identifier.
  - `std::string persistentUserId`: Persistent backend account UUID.
  - `std::string authToken`: Active authorization credentials.

