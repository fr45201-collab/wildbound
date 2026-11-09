# Phase 50E — Test Strategy & Verification Matrix

**Project:** Wildbound  
**Phase:** 50E — Gameplay Systems, Combat & AI Verification  
**Stage:** Stage 1 — Test Strategy Planning (PENDING APPROVAL)  
**Date:** 2026-11-08  
**Verification Scope:** Unit, Integration, Regression, and Runtime Validation

---

## 1. Test Verification Strategy

### 1.1. DEF-004: Specialized Creature Lua AI Verification
- **Target Executable**: `test_lua.exe` (`tests/scripting/test_lua.cpp`, CTest: `LuaScriptingTest`).
- **Test Scenarios to Implement**:
  1. **Script Loading**: Verify `slime.lua`, `snake.lua`, and `cyclops.lua` load cleanly via `LuaManager::LoadScript`.
  2. **Binding Verification**: Verify `wildbound.ai.slime_update`, `snake_update`, and `cyclops_update` functions are bound in the global Lua state.
  3. **Marsh Slime Behavioral Tests**:
     - At spawn without target $\to$ `IDLE`.
     - Target detected in range ($< 110.0$) $\to$ `CHASE` with move vector toward target.
     - Target within attack radius ($\le 26.0$) $\to$ `ATTACK` requesting `"slime_slam"`.
     - Low HP ($\le 25\%$) $\to$ `RETREAT` disengagement.
     - Spawn leash exceeded ($> 90.0$) $\to$ `RETURN` to spawn origin.
  4. **Marsh Viper Behavioral Tests**:
     - Ambush posture without target in range $\to$ `IDLE`.
     - Target detected in range ($< 150.0$) $\to$ `CHASE`.
     - Target in strike range ($\le 34.0$) $\to$ `ATTACK` requesting `"viper_strike"`.
     - Low HP ($\le 35\%$) or post-attack $\to$ rapid `RETREAT` repositioning.
  5. **Mountain Cyclops Behavioral Tests**:
     - Long-range detection ($< 220.0$) $\to$ `CHASE`.
     - Heavy club strike range ($\le 48.0$) $\to$ `ATTACK` requesting `"cyclops_club"`.
     - Low HP ($10\%$ HP) with target in range $\to$ **MUST NEVER RETREAT**; asserts state is `ATTACK`, confirming fearless brute trait.
  6. **Multi-Creature Independence**:
     - Simultaneous evaluation of 5 Slimes, 5 Vipers, and 5 Cyclops in mixed HP and positional states. Verify zero table leakage or cross-contamination.
  7. **Instruction Budget & Sandbox Safety**:
     - Execute 1,000 updates of all archetypes under the 100,000 instruction budget to verify deterministic performance without timeout violations.

---

### 1.2. DEF-008: Match Lifecycle Placement Determinism Verification
- **Target Executable**: `test_match_lifecycle.exe` (`tests/match/test_match_lifecycle.cpp`, CTest: `MatchLifecycleTest`).
- **Test Scenarios to Implement**:
  1. **Sequential Eliminations with Match Clock Advancement**:
     - P3 eliminated at $t = 0.1\text{s}$, update by $10.0\text{s}$, P2 eliminated at $t = 10.1\text{s}$.
     - Assert: P1 Rank 1, P2 Rank 2, P3 Rank 3 in both Debug and Release.
  2. **Coincident Same-Tick Eliminations**:
     - P2 and P3 eliminated at the exact same timestamp ($t = 0.1\text{s}$).
     - Verify deterministic tie-breaking (e.g. secondary kills/damage criteria) assigns stable, non-corrupted placements.
  3. **Zero-Survivor Simultaneous Elimination (Draw)**:
     - All active competitors eliminated in the same tick. Verify draw result without crash or undefined winner.
  4. **Multi-Cycle Clean Teardown**:
     - Run 3 consecutive match cycles to verify placement fields and participant registries reset completely.

---

### 1.3. DEF-009: Network Transport Handshake Verification
- **Target Executable**: `test_network_transport.exe` (`tests/network/test_network_transport.cpp`, CTest: `NetworkTransportTest`).
- **Test Scenarios to Implement**:
  1. **Deadline-Based Localhost Handshake**:
     - Replace 50-iteration cutoff with a 2,000 ms `std::chrono::steady_clock` deadline.
     - Verify client connect, server accept, and clean disconnect in Debug (`/Od /RTC1`) and Release (`/O2`).
  2. **Bidirectional Reliable Messaging Ping-Pong**:
     - Verify packet delivery over established client-server connection using deadline loops.
  3. **Sequenced Player Input Channel**:
     - Verify packet ordering on Channel 2 (PlayerInput) without iteration exhaustion.
  4. **Multi-Channel Packet Dispatch**:
     - Verify all 4 channels (System, GameState, PlayerInput, Chat) deliver packets under Debug execution.
  5. **Connection Timeout Detection**:
     - Attempt connection to an unbound port; verify `ConnectionTimedOut` event triggers within the declared timeout.
  6. **Rapid Connection Churn**:
     - 5 sequential connect/disconnect cycles without socket leak.

---

## 2. Global CTest Regression Suite

Following implementation of the three defect fixes, the full CTest suite will be executed in both configurations:
- `ctest --test-dir build -C Release --output-on-failure`
- `ctest --test-dir build -C Debug --output-on-failure`

### Expected Pass Rate Matrix Post-Phase 50E

| Configuration | Total Targets | Pre-50E Passed | Expected Post-50E Passed | Remaining Failures Post-50E |
|---|---:|---:|---:|---|
| **Release** | 48 | 44 | **44** | 4 sandbox-blocked socket loopback tests (DEF-002 / DEF-006) |
| **Debug** | 48 | 42 | **44** | 4 sandbox-blocked socket loopback tests (DEF-002 / DEF-006) |

*Key Milestone: Debug and Release will achieve 100% parity across all 44 test targets.*
The only remaining non-passing tests will be the 4 environment-blocked socket loopback tests (Tests 38, 39, 46, 48) isolated in Phase 50B.

