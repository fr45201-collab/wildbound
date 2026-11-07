# Running Multiplayer & Matchmaking Tests

This guide provides comprehensive instructions for configuring, launching, and verifying Wildbound's multiplayer coordination layer, dedicated server instances, and client matchmaking flows.

---

## 1. Architecture & Port Allocation

The Wildbound multiplayer network topology separates match coordination from match simulation:

| Component | Executable | Default Port | Protocol | Role |
|---|---|---|---|---|
| **Coordination Service** | `WildboundCoordinator` | `7780` | UDP / ENet | Manages sessions, parties, lobbies, invitations, matchmaking queues, server registry, and match reservations. |
| **Dedicated Server** | `WildboundServer` | `7777` (configurable) | UDP / ENet | Authoritative game simulation. Registers with Coordinator, executes matches, validates player tokens. |
| **Game Client** | `WildboundGame` | Ephemeral UDP | UDP / ENet | Connects to Coordinator for party/matchmaking, then redirects to assigned Dedicated Server. |

### Test Port Overrides
To avoid conflicting with running production or development instances, the automated end-to-end integration test (`test_matchmaking_coordinator_e2e`) runs on isolated test ports:
- Test Coordinator: `17780`
- Test Dedicated Server: `17777`

---

## 2. Building the Targets

### Windows (MSVC / Visual Studio)
Build all multiplayer targets and tests using CMake Presets or direct MSVC build:

```powershell
# Configure (if needed)
cmake -B build -S .

# Build WildboundCoordinator, WildboundServer, and test targets in Release mode
cmake --build build --config Release --target WildboundCoordinator WildboundServer test_matchmaking_coordinator_e2e
```

### Linux / WSL2 (GCC 15+ / Clang)
Build on Linux/WSL2:

```bash
# Configure headless Linux build
cmake -B build-linux -S . -DCMAKE_BUILD_TYPE=Release

# Compile coordinator, dedicated server, and matchmaking tests
cmake --build build-linux --target WildboundCoordinator WildboundServer test_matchmaking_coordinator_e2e -j$(nproc)
```

---

## 3. Running Automated Matchmaking Tests

Wildbound provides 7 dedicated CTest suites covering the matchmaking and coordination subsystems:

| Test ID | Target Name | Description |
|---|---|---|
| 40 | `test_matchmaking_sessions` | Player session creation, heartbeat tracking, and 30s timeout eviction. |
| 41 | `test_matchmaking_lobbies_parties` | Party capacity limits, member readiness, leader assignment/transfer/kick, custom lobbies. |
| 42 | `test_matchmaking_invitations` | Invitation dispatch, 60s expiration, accept/decline state transitions. |
| 43 | `test_matchmaking_queues` | Solo (1), Duos (2), and Squads (4) queue aggregation and party preservation. |
| 44 | `test_matchmaking_server_registry` | Dedicated server registration, heartbeat tracking, 15s offline detection, capacity queries. |
| 45 | `test_matchmaking_match_assignment` | Match formation, token generation, 5s reservation confirmation, and match completion release. |
| 46 | `test_matchmaking_coordinator_e2e` | Full multi-process live network test: 1 Coordinator + 1 Server + 4 Clients through party, queue, reservation, redirection, auth token validation, and match completion. |

### Running the Entire Suite via CTest
```powershell
# Windows
ctest --test-dir build -C Release -R test_matchmaking --output-on-failure

# Linux
ctest --test-dir build-linux -R test_matchmaking --output-on-failure
```

### Running Individual Test Executables
```powershell
# Example: Run live network end-to-end test
./build/tests/Release/test_matchmaking_coordinator_e2e.exe
```

---

## 4. Manual Multi-Process Testing Workflow

To manually test the complete lifecycle across independent processes:

### Step 1: Start the Coordinator Service
Run `WildboundCoordinator` in its own terminal:

```powershell
./build/Release/WildboundCoordinator.exe --port 7780 --tick-rate 30 --log-level 1
```

**Expected Log Output:**
```
[INFO] [CoordinatorServer] Starting Wildbound Coordinator on port 7780 (Tick rate: 30 Hz)...
[INFO] [CoordinatorServer] Listening for dedicated server registrations and client connections.
```

### Step 2: Start One or More Dedicated Servers
In a second terminal, launch `WildboundServer` configured to register with the coordinator:

```powershell
./build/Release/WildboundServer.exe --port 7777 --max-clients 30 --coordinator 127.0.0.1:7780 --region us-east
```

**Expected Server Log Output:**
```
[INFO] [DedicatedServer] Connecting to coordinator at 127.0.0.1:7780...
[INFO] [DedicatedServer] Registered successfully with Coordinator! (ServerId: srv_127.0.0.1_7777)
[INFO] [DedicatedServer] Heartbeat loop active (interval: 5000ms).
```

**Expected Coordinator Log Output:**
```
[INFO] [DedicatedServerRegistry] Registered dedicated server srv_127.0.0.1_7777 (127.0.0.1:7777, capacity: 30)
```

### Step 3: Run Matchmaking Simulation Clients or Game Clients
Clients connect to port `7780` and issue coordinator protocol messages:
1. `MsgSessionAuth`: Authenticates player, receiving `sessionId`.
2. `MsgPartyCreate` / `MsgPartyInvite` / `MsgPartyAccept`: Assembles party members.
3. `MsgPartyReady`: All members toggle ready.
4. `MsgQueueJoin`: Party leader queues for mode (e.g., `Squads`).
5. `MsgMatchReserve` / `MsgMatchReserveAck`: Coordinator reserves capacity on `srv_127.0.0.1_7777`.
6. `MsgMatchRedirect`: Coordinator sends server endpoint and secure `matchAuthToken` (`token_<matchId>_<sessionId>`) to each client.
7. `MsgClientConnect`: Clients disconnect from coordinator and connect directly to `127.0.0.1:7777`.
8. Dedicated server verifies the client's `matchAuthToken` against the reserved assignment before allowing spawn into the match.
9. On match victory/conclusion, `DedicatedServer` dispatches `MsgMatchCompleted` back to the coordinator, releasing server capacity back to `Available`.

---

## 5. Command-Line Reference

### WildboundCoordinator
| Flag | Description | Default |
|---|---|---|
| `--port <port>` | UDP port to bind for incoming traffic. | `7780` |
| `--config <path>` | Path to JSON coordinator configuration file. | `assets/config/coordinator.json` |
| `--tick-rate <hz>` | Main loop tick frequency in Hertz. | `30` |
| `--log-level <0-3>` | Verbosity level: `0` (Debug), `1` (Info), `2` (Warn), `3` (Error). | `1` |
| `--test-run` | Validates configuration, initial subsystem setup, and exits immediately. | Disabled |
| `--help` / `-h` | Prints available command-line arguments. | — |

### WildboundServer (Coordinator Integration Flags)
| Flag | Description | Default |
|---|---|---|
| `--coordinator <ip:port>` | Coordinator service address for auto-registration and reservations. | `127.0.0.1:7780` |
| `--region <name>` | Server deployment region tag (e.g. `us-east`, `eu-central`). | `default` |
| `--heartbeat-interval <ms>` | Heartbeat interval in milliseconds sent to coordinator. | `5000` |

---

## 6. Troubleshooting & Diagnostics

### Connection Refused / Timeout
- **Symptom**: `Failed to connect to coordinator at 127.0.0.1:7780`.
- **Cause**: The coordinator is not running, or firewall is blocking UDP packets on port 7780.
- **Fix**: Verify coordinator is running first. On Windows, ensure Windows Defender Firewall allows incoming UDP on the selected port.

### Matchmaking Timeout / Server Unregistered
- **Symptom**: Parties remain in queue indefinitely with status `Searching...`.
- **Cause**: No available dedicated servers registered with the coordinator have matching capacity.
- **Diagnostics**: Check coordinator logs for `Registered dedicated server`. If no servers are active, start at least one `WildboundServer` instance.

### Reservation Rollback
- **Symptom**: `Match assignment failed: Reservation confirmation timeout`.
- **Cause**: The assigned dedicated server failed to respond with `MsgMatchReserveAck` within 5.0 seconds.
- **Resolution**: Ensure the dedicated server is not hanging or blocked by breakpoint debugging. The coordinator automatically returns unassigned players to the front of the queue and marks the non-responsive server for health checking.

### Invalid Auth Token on Server Connect
- **Symptom**: Dedicated server rejects incoming player during handshake.
- **Cause**: The client supplied an invalid or expired match auth token.
- **Resolution**: Tokens are strictly scoped: `token_<matchId>_<sessionId>`. Ensure the client connects to the exact dedicated server address returned in `MsgMatchRedirect`.
