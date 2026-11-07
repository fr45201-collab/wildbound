# Wildbound — Phase 42 Initial Repository Audit

**Document:** `docs/backend/PHASE_42_INITIAL_AUDIT.md`  
**Date:** November 2026  
**Auditor:** Wildbound Engineering Team  
**Scope:** Pre-implementation audit for Phase 42 (Go Backend: API, Authentication, Player Profiles, Match Records, Cosmetic Ownership & Statistics)

---

## 1. Executive Summary

This audit assesses the technical state of the Wildbound codebase across Phases 32 through 41, evaluates existing infrastructure (Coordinator, Dedicated Server, Raylib client, Cosmetics, Match Results), and verifies toolchain prerequisites for introducing a dedicated Go backend with PostgreSQL persistence.

**Audit Verdict**:
- **Core Engine & Gameplay (Phases 32–34)**: Fully operational and verified. Match lifecycle, results calculation, and game mode systems produce structured telemetry ready for backend consumption.
- **Networking & Server (Phases 35–40B)**: Fully operational and verified. Dedicated server runs headlessly, simulates gameplay authoritatively, and reports match outcomes.
- **Coordination & Matchmaking (Phase 41)**: Fully operational and verified. Standalone `WildboundCoordinator` manages ephemeral sessions, parties, lobbies, queues, and match reservations.
- **Backend & Persistence Prerequisites**:
  - **Go**: Installed (`go version go1.26.0 windows/amd64`).
  - **PostgreSQL**: PostgreSQL 16.11 binaries (`initdb.exe`, `pg_ctl.exe`, `psql.exe`) are present in `C:\Program Files\PostgreSQL\16\bin`.
  - **C++ Client HTTP Infrastructure**: Currently absent in C++ client. A non-blocking HTTP client abstraction is required for API communication.

---

## 2. Phase-by-Phase Verification Matrix

| Phase | System / Subsystem | Verification Status | Source Files Inspected | Findings & Readiness |
|---|---|---|---|---|
| **32** | Match Lifecycle | Verified (100% test pass) | `src/match/MatchManager.hpp`, `MatchState.hpp`, `MatchParticipant.hpp` | Clean states (`WaitingForPlayers` $\to$ `Active` $\to$ `FinalSurvivor` $\to$ `MatchComplete`). Spawning and elimination tracking operational. |
| **33** | Victory & Results | Verified (100% test pass) | `src/results/MatchResultModel.hpp`, `PlayerPerformance.hpp`, `MatchResultManager.hpp` | Produces `MatchResultModel` and `PlayerPerformance` with kills, damage, placement, survival duration, and elimination logs. Exactly matches backend schema requirements. |
| **34** | Game Modes | Verified (100% test pass) | `src/gamemodes/GameModeManager.hpp`, `data/gamemodes/*.json` | Five validated modes (`Solo`, `Duos`, `Squads`, `Practice`, `Offline Bots`). Team friendly-fire filters and mode rules operational. |
| **35** | Multiplayer Arch | Verified (100% test pass) | `docs/networking/PHASE_35_ARCHITECTURE.md`, `src/network/NetworkProtocol.hpp` | Established client-server authority, packet envelopes, replication rules. |
| **36** | ENet Transport | Verified (100% test pass) | `src/network/ENetTransport.hpp`, `ENetServer.hpp`, `ENetClient.hpp`, `PacketValidator.hpp` | 5 virtual channels (`Control`, `ReliableEvents`, `PlayerInput`, `Replication`, `MatchState`). Reliable and unreliable messaging verified. |
| **37** | Entity Synchronization | Verified (100% test pass) | `src/network/replication/ReplicationManager.hpp`, `Snapshot.hpp` | Generational identity, delta snapshots, interpolation, and teleport clamping verified. |
| **38** | Server-Authoritative Gameplay | Verified (100% test pass) | `src/combat/CombatSystem.cpp`, `ProjectileSystem.cpp` | Server validates speed limits, damage calculations, and elimination events. |
| **39** | Dedicated Server Foundation | Verified (100% test pass) | `src/server/DedicatedServer.hpp`, `ServerLoop.hpp`, `ServerCommandLine.cpp` | Headless execution without raylib window/GPU/audio. 30 Hz fixed loop with bounded catch-up. |
| **40A** | Cross-Platform Server | Verified (100% test pass) | `src/platform/WindowsPlatform.hpp`, `LinuxPlatform.hpp`, `CMakeLists.txt` | Clean abstraction of memory metrics, process uptime, and OS signal handling. |
| **40B** | Linux Dedicated Server | Verified (100% test pass) | `build/linux/gcc-release/WildboundServer` | 46/46 CTest tests pass in 12.02s on Linux (GCC 15.2.0 via WSL2). Headless simulation verified. |
| **41** | Lobby & Matchmaking | Verified (100% test pass) | `src/coordinator/CoordinatorServer.hpp`, `SessionManager.hpp`, `PartyManager.hpp`, `MatchmakingManager.hpp`, `DedicatedServerRegistry.hpp`, `MatchAssignmentManager.hpp`, `MultiplayerMenu.hpp` | Complete pre-match coordination service `WildboundCoordinator` (UDP 7780). Server auto-registration, 2-way reservation handshake (`MsgMatchReserve`/`Ack`), token validation, and capacity release verified in multi-process E2E test. |

---

## 3. Subsystem Investigation & Integration Requirements

### 3.1 WildboundCoordinator
- **Session Representation**: `PlayerSession` in `src/coordinator/PlayerSession.hpp` tracks `SessionId` (64-bit uint), `connectionId` (ENet peer), `displayName`, `partyId`, `lobbyId`, `state`, and `lastHeartbeatTime`.
- **Identity Nature**: Currently **temporary**. `SessionManager::CreateSession()` generates ephemeral IDs per network connection.
- **Phase 42 Integration Requirement**:
  - The client must authenticate with `WildboundBackend` first via REST API (`/api/v1/auth/login`), obtaining a signed JWT or access token.
  - When connecting to `WildboundCoordinator` via `MsgSessionRegisterRequest`, the client will supply its persistent `userId` and `accessToken`.
  - The Coordinator must validate the token (either via JWT public key verification or an internal backend token-verification check) to map `SessionId` to the persistent `userId`.
- **Message Serialization**: Binary `ByteStreamWriter`/`ByteStreamReader` over ENet Channel 0 with 16-byte `PacketHeader`.

### 3.2 WildboundServer (Dedicated Server)
- **Participant Representation**: Represented by `match::MatchParticipant` and `coordinator::ParticipantReservation` containing `sessionId`, `teamId`, and `matchAuthToken` (`token_<matchId>_<sessionId>`).
- **Match Results Generation**: On match completion, `MatchResultManager` produces an immutable `MatchResultModel` containing:
  - `matchId`, `matchDuration`, `matchOutcome`, `winnerParticipantId`, `winnerDisplayName`
  - Per-participant `PlayerPerformance`: `participantId`, `displayName`, `placement`, `kills`, `deaths`, `damageDealt`, `totalDamageReceived`, `survivalDuration`, `isWinner`.
- **Completion Reporting**: Currently, `DedicatedServer::ReportMatchCompletedToCoordinator` dispatches `MsgMatchCompletedNotification` back to the Coordinator over ENet.
- **Phase 42 Integration Requirement**:
  - In addition to notifying Coordinator to release capacity, `WildboundServer` must submit the full authoritative match telemetry to `WildboundBackend` via `POST /api/v1/internal/matches/results`.
  - The submission must be authenticated using a dedicated server API key/secret (`X-Server-Token`).
  - To prevent result loss during transient database/backend outages, `WildboundServer` must implement an outbox / retry queue that persists results locally and retries until HTTP 200/201 is confirmed.

### 3.3 Client UI & Infrastructure
- **Existing Menus**: `MainMenu`, `ModeSelectMenu`, `MultiplayerMenu`, `Wardrobe`, `Store`, `MatchResults`, `GameOver`.
- **Player Identity References**:
  - `user_data/cosmetics/cosmetic_ownership.json`: Local file storing `cosmeticId`, `state` ("owned"), `acquisitionSource`, `acquisitionDate`.
  - `user_data/character/appearance_loadouts.json`: Local file storing active appearance slots (`Back`, `Chest`, `Feet`, `Hair`, `Headwear`, `Legs`, `Skin`).
- **Phase 42 Integration Requirement**:
  - Transition cosmetic ownership and equipped loadouts from local mock JSON files to authoritative backend API queries (`/api/v1/players/me/cosmetics`, `/api/v1/players/me/cosmetics/equip`).
  - Introduce an asynchronous HTTP client in C++ using Windows WinHTTP / standard sockets (or libcurl/cross-platform HTTP client) so network calls never stall the 60 FPS Raylib render loop.

---

## 4. Technology & Dependency Findings

### 4.1 Go Toolchain
- **Detected Version**: `go version go1.26.0 windows/amd64`
- **Location**: In system PATH.
- **Module Structure**: `backend/go.mod` will be initialized targeting standard Go idioms with pinned dependencies (`pgx/v5`, `golang-jwt/jwt/v5`, `golang.org/x/crypto/argon2`, `chi/v5` or standard library `net/http` router).

### 4.2 PostgreSQL Database
- **Installed Version**: PostgreSQL 16.11 (`C:\Program Files\PostgreSQL\16\bin`).
- **Cluster Deployment Strategy**:
  - Standard Windows service `postgresql-x64-16` is installed.
  - To avoid permission issues and admin rights requirements, a dedicated project PostgreSQL cluster will be initialized under `user_data/postgres/data` using `initdb.exe` on port `5433` (or default `5432`), with user `wildbound_user`, password `wildbound_password`, and database `wildbound`.
  - This ensures 100% reproducible test and development environments without requiring root/elevation or altering host system services.

---

## 5. Proposed Backend Boundaries & Service Decoupling

```
+-----------------------------------------------------------------------------------------+
|                                    WILDBOUND SYSTEM                                     |
|                                                                                         |
|  +-----------------------+     +-------------------------+     +---------------------+  |
|  |     WildboundGame     |     |  WildboundCoordinator   |     |   WildboundServer   |  |
|  |       (Client)        |     |  (Matchmaking/Parties)  |     | (Authoritative Game)|  |
|  +-----------+-----------+     +------------+------------+     +----------+----------+  |
|              |                              |                             |             |
|              | REST (Auth/Profiles/Cosmetics) | Token Verification        | Internal    |
|              |                              |                             | Match Report|
|              v                              v                             v             |
|  +-----------------------------------------------------------------------------------+  |
|  |                                 WildboundBackend                                  |  |
|  |                                  (Go REST API)                                    |  |
|  |                                                                                   |  |
|  |   [Auth & JWT]  [Player Profiles]  [Cosmetics & Inv]  [Match Records]  [Stats]    |  |
|  +------------------------------------------+----------------------------------------+  |
|                                             |                                           |
|                                             v                                           |
|                               +---------------------------+                             |
|                               |    PostgreSQL Database    |                             |
|                               |   (Relational Storage)    |                             |
|                               +---------------------------+                             |
+-----------------------------------------------------------------------------------------+
```

### Boundary Rules:
1. **Gameplay Authority**: `WildboundServer` retains 100% authority over game simulation, combat, collision, and storm damage. `WildboundBackend` NEVER simulates physics or combat.
2. **Coordination Authority**: `WildboundCoordinator` retains authority over matchmaking queues, parties, custom lobbies, and dedicated server reservations. `WildboundBackend` provides persistent account verification and profile data.
3. **Result Submission**: Only authorized `WildboundServer` instances (authenticated via dedicated server API token) may submit match results to `WildboundBackend`. Ordinary clients cannot submit match results or directly modify their statistics.
4. **Cosmetic Authority**: The backend authoritatively stores and validates cosmetic ownership. Clients cannot equip items they do not own in PostgreSQL.

---

## 6. Audit Conclusion & Next Steps

All prerequisites for Phase 42 are verified and ready.
- **Go toolchain**: Ready (Go 1.26.0).
- **PostgreSQL**: Ready (v16.11 available for cluster creation and migration).
- **Core game, dedicated server, coordinator**: Fully tested and 100% passing.

Proceed to **Step 2 (Backend Architecture & Foundation)**.

