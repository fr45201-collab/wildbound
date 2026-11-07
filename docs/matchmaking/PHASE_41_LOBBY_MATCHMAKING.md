# WILDBOUND — PHASE 41 TECHNICAL MANUAL
## Multiplayer Lobby, Party, Matchmaking & Dedicated Server Coordination Infrastructure

**Specification Date:** 2026-11-07  
**Engine Architecture:** Wildbound 1.0.0 (C++20, Cross-Platform Windows/Linux, ENet Transport)  
**Document Classification:** Authoritative Architectural Guide  

---

### 1. Executive Summary

Phase 41 implements the pre-match multiplayer coordination infrastructure for Wildbound. It establishes a complete separation of concerns between:
1. **Coordination Service (`WildboundCoordinator`)**: Responsible for temporary player sessions, party and lobby formation, invitations, matchmaking queues, dedicated server capacity tracking, two-way match reservation handshakes, and client redirection.
2. **Dedicated Game Server (`WildboundServer`)**: Headless authoritative simulation authority executing physics, combat, storm progression, and victory evaluation.
3. **Game Client (`wildbound`)**: Graphical client connecting to the coordinator for party formation and matchmaking, and subsequently redirecting to the assigned dedicated server for match play.

---

### 2. Architectural Blueprint

```
 +-------------------------------------------------------------------------+
 |                       WILDBOUND COORDINATOR                             |
 |                    (UDP Port 7780 / Default)                            |
 |                                                                         |
 |  +-----------------------+ +--------------------+ +------------------+  |
 |  |    SessionManager     | |    PartyManager    | |   LobbyManager   |  |
 |  | (Inactivity: 30s TTL) | | (Cap: 1/2/4 modes) | | (Custom scrims)  |  |
 |  +-----------------------+ +--------------------+ +------------------+  |
 |  +-----------------------+ +--------------------+ +------------------+  |
 |  |   InvitationManager   | |MatchmakingManager  | |  ServerRegistry  |  |
 |  | (Expiration: 60s TTL) | |(Party Preservation)| |(Heartbeat: 15s)  |  |
 |  +-----------------------+ +--------------------+ +------------------+  |
 |                          +----------------------+                       |
 |                          |MatchAssignmentManager|                       |
 |                          |(2-Way Handshake / 5s)|                       |
 |                          +----------------------+                       |
 +-------------------------------------------------------------------------+
       ^                         ^                          ^
       | Player Sessions         | Party Invites/Queues     | Server Registration
       | & Redirection           |                          | & Heartbeats
 +---------------+         +---------------+          +-------------------+
 | Game Client 1 |         | Game Client 2 |          | Dedicated Server  |
 |  (wildbound)  |         |  (wildbound)  |          | (WildboundServer) |
 +---------------+         +---------------+          +-------------------+
       |                                                    ^
       +------------------- Connect to Game ---------------+
                            (UDP Port 7777, auth token)
```

---

### 3. Protocol Specification (`CoordinatorProtocol.hpp`)

All coordinator packets are encapsulated in standard 16-byte fixed-size envelopes (`magic = 0x57424E44`, `version = 1`, Little-Endian):

| Message Type | Value | Direction | Description |
|---|---|---|---|
| `SessionRegisterRequest` | `0x0101` | Client -> Coord | Register temporary session with display name and client version. |
| `SessionRegisterResponse` | `0x0102` | Coord -> Client | Confirms registration and returns 64-bit `SessionId`. |
| `SessionHeartbeat` | `0x0103` | Client -> Coord | Periodic keepalive to refresh session activity. |
| `PartyCreateRequest` | `0x0110` | Client -> Coord | Requests party creation for specified game mode. |
| `PartyCreateResponse` | `0x0111` | Coord -> Client | Confirms party creation and returns 64-bit `PartyId`. |
| `PartyJoinRequest` | `0x0112` | Client -> Coord | Requests joining an existing party. |
| `PartyJoinResponse` | `0x0113` | Coord -> Client | Confirms join or returns rejection reason (e.g. party full/queued). |
| `PartyLeaveRequest` | `0x0114` | Client -> Coord | Member leaves party; triggers leader reassignment or disband. |
| `PartySetReadyRequest` | `0x0116` | Client -> Coord | Toggles member readiness state. |
| `PartySetModeRequest` | `0x0117` | Client -> Coord | Leader changes party game mode ("solo", "duos", "squads"). |
| `PartyKickRequest` | `0x0118` | Client -> Coord | Leader kicks specified party member. |
| `PartyTransferLeaderRequest`| `0x0119` | Client -> Coord | Leader transfers ownership to another member. |
| `PartyStateNotification` | `0x011A` | Coord -> Client | Broadcasts updated member roster, readiness, and game mode. |
| `InviteSendRequest` | `0x0120` | Client -> Coord | Sends party/lobby invitation to recipient `SessionId`. |
| `InviteReceivedNotification`| `0x0122` | Coord -> Client | Notifies recipient of pending invitation (60s TTL). |
| `InviteRespondRequest` | `0x0123` | Client -> Coord | Recipient accepts or declines invitation. |
| `MatchmakingQueueRequest` | `0x0130` | Client -> Coord | Leader enqueues party into mode-specific queue. |
| `MatchmakingCancelRequest`| `0x0132` | Client -> Coord | Cancels queue entry for party. |
| `MatchmakingStatusUpdate` | `0x0134` | Coord -> Client | Updates queue position, elapsed time, and search status. |
| `ServerRegisterRequest` | `0x0140` | Server -> Coord | Dedicated server registers host, port, capacity, and mode. |
| `ServerRegisterResponse` | `0x0141` | Coord -> Server | Confirms registration and returns `ServerInstanceId`. |
| `ServerHeartbeat` | `0x0142` | Server -> Coord | Server reports current players and state (Available/Reserved/InMatch). |
| `MatchReserveRequest` | `0x0150` | Coord -> Server | Coordinator reserves server capacity for formed match with participant tokens. |
| `MatchReserveResponse` | `0x0151` | Server -> Coord | Server confirms reservation acceptance or rejects. |
| `MatchRedirectNotification`| `0x0152` | Coord -> Client | Redirects client to server host:port with personalized auth token. |
| `MatchCompletedNotification`| `0x0153`| Server -> Coord | Server notifies match finish, releasing capacity back to Available. |

---

### 4. Core Subsystem Implementations

#### 4.1 Player Sessions (`SessionManager`)
- In-memory table indexing sessions by 64-bit `SessionId` and ENet `ClientId`.
- Inactivity cleanup: sessions with no heartbeat for >30 seconds are pruned.
- Stale session cleanup automatically withdraws player from matchmaking queues, leaves parties, and disbands empty groups.

#### 4.2 Parties (`PartyManager`)
- Capacity enforcement derived from Phase 34 definitions:
  - Solo: `maxCapacity = 1`
  - Duos: `maxCapacity = 2`
  - Squads: `maxCapacity = 4`
- Leader authority: only party leader can select mode, queue for matches, kick members, or transfer leadership.
- Departure handling: if the leader departs, seniority transfer reassigns leadership to the next remaining member. Disbands when the last member leaves.

#### 4.3 Matchmaking Engine (`MatchmakingManager`)
- **Strict Party Preservation Invariant**: **Parties are never split across matches or teams.** A 4-player party in Squads remains together as Team 1. A 2-player party in Duos remains together as a duo.
- Configurable thresholds (`soloMinParticipants`, `duosMinParticipants`, `squadsMinParticipants`) allow flexible production scaling (up to 100 players) and rapid integration verification (e.g. 2–4 players).

#### 4.4 Dedicated Server Registry (`DedicatedServerRegistry`)
- Servers register on launch with host, port, max capacity, world map, and game mode.
- Heartbeats sent every 5s; servers missing heartbeats for >15s are flagged `Offline`.
- Capacity allocation matches formed match participant count to `Available` servers.

#### 4.5 Two-Way Reservation Handshake (`MatchAssignmentManager`)
1. **Candidate Selection**: When match forms, coordinator selects an `Available` server and creates a `PendingReservation` assignment with generated participant authentication tokens (`token_<matchId>_<sessionId>`).
2. **Reservation Delivery**: Coordinator transmits `MsgMatchReserveRequest` to dedicated server.
3. **Server Confirmation**: Dedicated server verifies running status, stores participant tokens, marks match ID, and replies with `MsgMatchReserveResponse(accepted = true)`.
4. **Client Redirection**: Coordinator transmits `MsgMatchRedirectNotification` to all clients in the match.
5. **Credential Validation**: When client connects to dedicated server (`MsgClientHello`), the server authenticates the token against its reserved participant table and verifies team assignments.
6. **Completion & Release**: When match finishes, dedicated server sends `MsgMatchCompletedNotification` to coordinator, releasing server status back to `Available`.

---

### 5. Automated Verification Results

| Test Target | Category | Windows MSVC Result | Linux GCC 15.2.0 Result | Status |
|---|---|---|---|---|
| `test_matchmaking_sessions` | Unit | Passed (0.02s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_lobbies_parties` | Unit | Passed (0.03s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_invitations` | Unit | Passed (0.02s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_queues` | Unit | Passed (0.02s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_server_registry` | Unit | Passed (0.02s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_assignment` | Unit | Passed (0.02s) | Passed (0.01s) | **VERIFIED** |
| `test_matchmaking_coordinator_e2e` | End-to-End | Passed (0.29s) | Passed (0.29s) | **VERIFIED** |
| **Full CTest Suite (46 Tests)** | Regression | **100% Passed (15.72s)** | **100% Passed (12.06s)** | **ZERO REGRESSIONS** |

