# Phase 50B: Network Test Environment & Isolation Analysis

**Project:** Wildbound  
**Phase:** 50B — Build System Harmonization & Test Infrastructure Validation  
**Date:** 2026-11-08  
**Defects Investigated:** DEF-002, DEF-006  

---

## 1. Executive Summary

During Phase 50A and Phase 50B automated CTest runs, 4 networking-related tests consistently fail or time out:
1. `EntityReplicationTest` (Test 38)
2. `DedicatedServerTest` (Test 39)
3. `MatchmakingCoordinatorE2ETest` (Test 46)
4. `MultiplayerLoadTest` (Test 48)

A comprehensive investigation was conducted to determine whether these failures stem from code-level bugs in the networking subsystems or from host operating system environment constraints (specifically sandboxed loopback UDP socket restrictions).

---

## 2. Test-by-Test Technical Failure Analysis

### 2.1 `EntityReplicationTest` (`tests/network/test_entity_replication.cpp`)
- **Unit Sub-Tests 1 through 9**: **ALL PASS**
  - Generational entity slots and stale invalidation verified.
  - Snapshot binary serialization (full and delta) verified with bandwidth savings.
  - Snapshot buffer chronological ordering verified.
  - Interpolation math and teleport boundary clamping verified.
  - Event deduplication and idempotency verified.
  - Authoritative movement speed limits verified.
  - Combat elimination flow verified.
  - Atomic loot pickup concurrency protection verified.
  - Environmental and storm replication verified.
- **Sub-Test 10 Failure**: `Assertion FAILED: Both clients must be fully connected at line 413`
- **Root Cause**: Test 10 attempts to bind a localhost ENet host and connect two simulated client peers over loopback UDP. In the sandboxed process environment, local UDP loopback packets are blocked by network sandbox filtering, preventing the 3-way ENet handshake from completing.

### 2.2 `DedicatedServerTest` (`tests/server/test_dedicated_server.cpp`)
- **Unit Sub-Tests 1 through 6**: **ALL PASS**
  - Server configuration parsing and validation passed.
  - Command line argument overrides passed.
  - Server lifecycle state machine and diagnostics passed.
  - Fixed-timestep simulation loop and catch-up passed.
  - Headless world loading and 41,503 spatial collision bounds passed.
  - Headless startup, shutdown sequence, and idempotency passed.
- **Sub-Test 7 Failure**: `[TEST FAILURE] Client 1 must be connected at line 299`
- **Root Cause**: Sub-test 7 binds the dedicated server to port `19992` and attempts an in-process client connect over loopback. The handshake packets are dropped by the host sandbox network policy, causing the assertion to abort.

### 2.3 `MatchmakingCoordinatorE2ETest` (`tests/matchmaking/test_coordinator_e2e.cpp`)
- **Diagnostic**: `[TEST FAILURE] Coordinator must have 1 registered server at line 170`
- **Root Cause**: Test attempts to run `CoordinatorServer` on port `17780` and `DedicatedServer` on port `17777`. The dedicated server initiates an ENet peer connection to `127.0.0.1:17780`. Because local UDP inter-socket loopback is non-routable inside the sandboxed test runner, the server registration packet is never received by the coordinator.

### 2.4 `MultiplayerLoadTest` (`tests/load/test_multiplayer_load.cpp`)
- **Diagnostic**: `[TEST FAILED] At least some clients must connect (res.connectedClients > 0) at line 50`
- **Root Cause**: Test attempts to spawn 2 to 32 synthetic ENet clients connecting simultaneously to `0.0.0.0:17777`. As with the tests above, loopback UDP packets fail to traverse the sandboxed network boundary.

---

## 3. Environment Boundary Matrix

| Test Executable | In-Memory Logic Tests | Localhost Socket Tests | Logic Pass Rate | Environment Pass Rate | Defect Classification |
|---|:---:|:---:|:---:|:---:|---|
| `test_entity_replication` | 9/9 Passed (100%) | 0/1 Passed (0%) | **100%** | 0% | DEF-002 (Host Sandbox Boundary) |
| `test_dedicated_server` | 6/6 Passed (100%) | 0/1 Passed (0%) | **100%** | 0% | DEF-002 (Host Sandbox Boundary) |
| `test_matchmaking_coordinator_e2e` | 0/0 (E2E only) | 0/1 Passed (0%) | N/A | 0% | DEF-006 (Multi-Process Loopback Boundary) |
| `test_multiplayer_load` | 0/0 (Load only) | 0/1 Passed (0%) | N/A | 0% | DEF-006 (Multi-Process Loopback Boundary) |

---

## 4. Engineering Recommendations for Subsequent Phases

1. **Test Infrastructure Distinction**:
   - Classify tests into **Tier 1 (Hermetic In-Memory / Subsystem Tests)** and **Tier 2 (Environment-Dependent End-to-End Tests)**.
   - Tier 1 tests (34 unit test suites) run under standard automated CI / sandbox without network permissions.
   - Tier 2 network tests (Tests 38, 39, 46, 48) require explicit sandbox bypass (`BypassSandbox: true`) or an unsandboxed CI agent with network loopback capabilities enabled.
2. **Mock Transport Layer**:
   - In a future phase, introduce an in-memory loopback transport adapter (`MemoryTransport` / `MockPacketBus`) that passes packets between simulated ENet peers directly via shared memory queues without invoking OS Winsock UDP calls.

