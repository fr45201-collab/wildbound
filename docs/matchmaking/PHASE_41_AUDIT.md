# WILDBOUND — PHASE 41 ARCHITECTURAL AUDIT
## Multiplayer Lobby, Party, Matchmaking & Dedicated Server Coordination Infrastructure

**Audit Date:** 2026-11-07  
**Engine Version:** Wildbound 1.0.0 (C++20, Headless Platform Abstraction, ENet Networking)  
**Author:** Wildbound Engine Architecture Team  

---

### Executive Summary

Phase 41 introduces the pre-match coordination service (`WildboundCoordinator`) responsible for organizing connected players into temporary sessions, parties, and lobbies, facilitating player-to-player invitations, managing mode-specific matchmaking queues (Solo, Duos, Squads) with strict party preservation, tracking registered dedicated servers (`WildboundServer`), orchestrating match assignment with two-way reservation handshakes, and redirecting clients with cryptographic match auth tokens to authoritative servers.

This audit evaluates the codebase's existing multiplayer foundation across Phases 34 through 40B to verify prerequisite readiness, identify integration boundaries, and establish the technical design for Phase 41.

---

### 1. Mandatory Audit of Preceding Phases

| Phase | Subsystem | Verification Status | Architectural Alignment |
|---|---|---|---|
| **Phase 34** | Game Modes & Team Architecture | **VERIFIED** | `GameModeRegistry`, `GameModeDefinition`, `TeamManager`, and `TeamVictoryEvaluator` support Solo (team size 1), Duos (team size 2), and Squads (team size 4). Modes define participant capacity bounds and rule configurations reusable directly by matchmaking queues. |
| **Phase 35** | Multiplayer Architecture & Server Authority | **VERIFIED** | Established authoritative client-server relationship, `NetworkRole`, 16-byte envelope header with `PROTOCOL_MAGIC = 0x57424E44` ('WBND'), `PROTOCOL_VERSION = 1`, and standard network identifiers. |
| **Phase 36** | ENet Networking & Transport Layer | **VERIFIED** | `ENetTransport`, `ENetServer`, `ENetClient`, and `PacketValidator` provide 5 virtual channels (Control, ReliableEvents, PlayerInput, Replication, MatchState) with reliable ordered and unreliable sequenced delivery guarantees. |
| **Phase 37** | Entity Synchronization & Replication | **VERIFIED** | Authoritative 30 Hz entity snapshot generation, delta compression, and client interpolation verified under simulated latency and jitter. |
| **Phase 39** | Dedicated Server Foundation | **VERIFIED** | Standalone headless `WildboundServer` executable runs without GPU/audio/window dependencies. Simulates physics, combat, storm, and match lifecycle at 30 Hz. |
| **Phase 40A** | Cross-Platform Server Compatibility | **VERIFIED** | Clean platform abstraction layer (`IPlatform`, `WindowsPlatform`, `LinuxPlatform`, `LinuxSignals`) decoupled server codebase from OS-specific APIs. |
| **Phase 40B** | Linux Dedicated Server Validation | **VERIFIED** | Headless stub decoupling (`raylib_stub.c`, `rlimgui_stub.cpp`), 100% CTest pass rate (39 suites) on both Windows MSVC and Linux GCC 15.2.0 in WSL2. |

---

### 2. Coordination Service Architecture

The Wildbound architecture separates **Coordination** (pre-match organization and assignment) from **Simulation** (authoritative in-match gameplay):

```
+------------------------------------------------------------------------+
|                      WILDBOUND COORDINATOR                            |
|             (Pre-Match Coordination Service - Port 7780)               |
|                                                                        |
|  +--------------------+  +--------------------+  +------------------+  |
|  |   SessionManager   |  |    PartyManager    |  |   LobbyManager   |  |
|  +--------------------+  +--------------------+  +------------------+  |
|  +--------------------+  +--------------------+  +------------------+  |
|  | InvitationManager  |  | MatchmakingManager |  |  ServerRegistry  |  |
|  +--------------------+  +--------------------+  +------------------+  |
|                        +-----------------------+                       |
|                        | MatchAssignmentManager|                       |
|                        +-----------------------+                       |
+------------------------------------------------------------------------+
       ^                         ^                          ^
       | Player Sessions         | Party Invites/Queues     | Server Registration
       | & Redirection           |                          | & Heartbeats
+---------------+         +---------------+          +-------------------+
| Game Client 1 |         | Game Client 2 |          | Dedicated Server  |
|  (wildbound)  |         |  (wildbound)  |          | (WildboundServer) |
+---------------+         +---------------+          +-------------------+
       |                                                    ^
       +------------------- Connect to Game ---------------+
                            (Port 7777, auth token)
```

#### Key Architectural Separation:
1. **Coordination Service (`WildboundCoordinator`)**:
   - Standalone headless executable target listening on UDP port 7780 (default).
   - In-memory data store for temporary player sessions, parties, lobbies, and registered servers.
   - Handles player matchmaking queues and reservation handshakes with available game servers.
2. **Dedicated Game Server (`WildboundServer`)**:
   - Executes authoritative gameplay simulation on port 7777 (or CLI `--port`).
   - Registers with the coordinator on launch via `--coordinator <host:port>`.
   - Sends periodic heartbeats, accepts match reservations (`MsgMatchReserve`), validates incoming client connection tokens, and reports match completion (`MsgMatchCompleted`).
3. **Game Client (`wildbound`)**:
   - Connects to coordinator for matchmaking and party formation.
   - Upon match formation, receives `MsgMatchRedirect` containing target server host, port, match ID, and secret player auth token.
   - Connects directly to `WildboundServer` to join the match.

---

### 3. Detailed Subsystem Specifications

#### 3.1 Player Sessions (`SessionManager`)
- **SessionId**: High-entropy 64-bit non-zero identifier assigned upon initial handshake with coordinator.
- **Fields**: `sessionId`, `displayName`, `connectionId` (ENet ClientId), `partyId`, `lobbyId`, `matchId`, `state` (`Connected`, `InParty`, `InQueue`, `InMatch`, `Disconnected`), `lastHeartbeat`.
- **Inactivity Timeout**: 30 seconds of missing heartbeats triggers session expiration, automatically disbanding/leaving parties and withdrawing from matchmaking queues.

#### 3.2 Parties (`PartyManager`)
- **PartyId**: 64-bit unique identifier.
- **Composition**: Leader `SessionId`, ordered vector of member `SessionId`s, per-member readiness flags (`isReady`).
- **Capacity**: Enforced by game mode (Solo = 1, Duos = 2, Squads = 4).
- **Party Leadership**: The leader selects the game mode, initiates queue entry, invites players, and kicks members. If the leader disconnects or leaves, leadership automatically transfers to the next senior member. If the last member departs, the party disbands.

#### 3.3 Custom Lobbies (`LobbyManager`)
- **LobbyId**: 64-bit unique identifier.
- **Attributes**: Host `SessionId`, human-readable lobby name, player list (up to 32 players), privacy flag, join passcode, map selection, custom game mode rules.

#### 3.4 Player Invitations (`InvitationManager`)
- **InvitationId**: 64-bit unique identifier.
- **Attributes**: `invitationType` (`PartyInvite`, `LobbyInvite`), sender ID & name, recipient ID, target party/lobby ID, creation timestamp, expiration duration (60 seconds).
- **State Machine**: `Pending` -> `Accepted` | `Declined` | `Expired` | `Revoked`. All invitations are single-use.

#### 3.5 Matchmaking Engine (`MatchmakingManager`)
- **Mode Queues**: Dedicated queues for Solo, Duos, and Squads.
- **Queue Entity**: `QueuedParty` holds the party ID, party size, queued timestamp, and member sessions.
- **Party Preservation Invariant**: **Parties are strictly never split across matches or teams.** A 4-player party in Squads remains together in the same squad. A 2-player party in Duos remains together in the same duo.
- **Match Formation**: When queued parties satisfy the participant requirements (configurable threshold for testing, e.g. 2–4 players, up to 100 for production), the matchmaking manager triggers match formation and transitions parties to `FormingMatch`.

#### 3.6 Dedicated Server Registry (`DedicatedServerRegistry`)
- **Server Instance**: `serverId` (64-bit), `name`, `hostAddress`, `port`, `maxCapacity`, `currentPlayers`, `assignedMatchId`, `state` (`Available`, `Reserved`, `InMatch`, `Offline`), `lastHeartbeatTime`.
- **Heartbeat & Health**: Servers send heartbeats every 5 seconds. Servers missing heartbeats for >15 seconds are flagged `Offline`.
- **Capacity Allocation**: Coordinator selects `Available` servers with adequate capacity for the formed match.

#### 3.7 Match Assignment Handshake (`MatchAssignmentManager`)
1. **Reservation**: Coordinator sends `MsgMatchReserve` to selected `Available` server with match ID, game mode, map, participant list, and cryptographic player auth tokens. Server enters `Reserved` state.
2. **Acknowledgement**: Server responds with `MsgMatchReserveAck`. If server fails or times out within 5 seconds, the reservation rolls back and coordinator fails over to another available server.
3. **Redirection**: Coordinator sends `MsgMatchRedirect` to all client sessions in the formed match with server host, port, match ID, and player auth token.
4. **Validation**: When client connects to dedicated server (`MsgClientHello`), the server validates the incoming auth token against its reserved participant table before allowing match entrance.
5. **Release**: When match finishes, dedicated server transmits `MsgMatchCompleted` to coordinator, releasing server capacity back to `Available`.

---

### 4. Verification & Testing Strategy

1. **Unit Test Coverage**:
   - `test_sessions.cpp`: Session creation, heartbeat tracking, timeout cleanup, lookup.
   - `test_lobbies_parties.cpp`: Party creation, joining, capacity limits, member readiness, leader transfer, disbandment, lobby creation.
   - `test_invitations.cpp`: Sending invitations, 60s expiration, accept/decline state transitions, revocation.
   - `test_matchmaking_queues.cpp`: Queue insertion, party preservation, Solo/Duos/Squads match formation, cancellation.
   - `test_server_registry.cpp`: Server registration, heartbeats, stale server timeout, capacity querying.
   - `test_match_assignment.cpp`: Reservation handshake, rollback on timeout, token generation and verification.
2. **Full End-to-End Integration Test**:
   - `test_coordinator_e2e.cpp`: 1 `CoordinatorServer` + 1 `DedicatedServer` + 4 `ENetClient` instances over real ENet network sockets:
     - Coordinator and Dedicated Server startup.
     - Dedicated server registers with coordinator.
     - 4 clients connect, establish sessions, form a squad party.
     - Party leader queues for Squads match.
     - Coordinator forms match, reserves capacity on dedicated server, receives ACK.
     - Coordinator redirects all 4 clients to dedicated server with tokens.
     - Clients connect to dedicated server, which validates tokens.
     - Server completes match and notifies coordinator, verifying capacity release.
3. **Dual-Platform Build & Verification**:
   - Windows MSVC (Visual Studio 2022).
   - Linux GCC 15.2.0 via WSL2 Ubuntu 26.04.
   - CTest integration ensuring zero regressions across all test suites.

---
**Audit Approved for Implementation:** Wildbound Technical Lead  
**Phase Status:** Ready to implement Part 2 Coordinator Data Structures & Architecture.

