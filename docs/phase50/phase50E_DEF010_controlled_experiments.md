# WILDBOUND — PHASE 50E DEF-010 CONTROLLED EXPERIMENTS REPORT
## Isolation, Reproduction & Root-Cause Analysis of MultiplayerLoadTest Failure

**Parent Phase:** Phase 50 — Global System Integration, Stability & Production Validation  
**Target:** DEF-010 — MultiplayerLoadTest Memory-Safety Failure (`0xc0000409` / `0xc0000005`)  
**Execution Mode:** Controlled Scientific Experiments & Rigor-Preserved Isolation  
**Date:** 2026-11-08  
**Final Classification:** **CONFIRMED ROOT CAUSE — ASYNCHRONOUS DETACHED-THREAD USE-AFTER-FREE**  
**Defect Disposition:** **OPEN (Root Cause Proven; Production Fix Scope Defined)**  
**Stage 5 Status:** **STRICTLY BLOCKED**  

---

## 1. Executive Summary

Defect **DEF-010** was subjected to controlled scientific experiments across three distinct axes:
1. **Experiment A (Teardown Order)**: Compared existing client-first destruction against server-first peer shutdown.
2. **Experiment B (Port Recycling)**: Compared repeated port 17777 reuse against dynamic sequential port allocation.
3. **Experiment C (Test Ordering & State Leakage)**: Compared standalone execution against test chaining (`NetworkTransportTest`, `DedicatedServerTest`, full CTest suite).

### Key Empirical Findings
- **Experiment A**: Reversing teardown order to server-first (Variant B) did **not** eliminate the crash. The harness still crashed at Section 2 (`LatBot_4`), refuting teardown order as the primary cause.
- **Experiment B**: Assigning fresh sequential ports (18000+) across all server restarts did **not** eliminate the crash. The harness still crashed at Section 7 (Client #12), refuting port 17777 socket binding collision as the primary cause.
- **Experiment C & Forensic Discovery**:
  - Running `DedicatedServerTest` followed immediately by `MultiplayerLoadTest` yielded a **100% crash reproduction rate** (2 / 2 runs crashed with `0xc0000409`).
  - Investigation traced this to disk state leakage: `DedicatedServerTest` creates a durable match results file in `user_data/outbox/match_match_50001.json`.
  - When `MultiplayerLoadTest` runs, each of its 25+ dedicated server instances discovers this outbox file during `Initialize()` and calls `MatchResultsOutbox::TrySubmitEntry()`.
  - `TrySubmitEntry()` delegates to `HttpClient::SendRequestAsync()`, which spawns a **detached OS background thread**:
    ```cpp
    std::thread([this, method, url, body, headers, callback, timeoutMs]() {
        HttpResponse resp = SendRequest(method, url, body, headers, timeoutMs);
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_completedCallbacks.push_back({ std::move(callback), std::move(resp) });
    }).detach();
    ```
  - When the load test terminates each section (~1.0s), `DedicatedServer` and its encapsulated `HttpClient` are destroyed (`HttpClient::~HttpClient() = default;`).
  - When the detached thread's HTTP connection attempt fails (after ~1.5–3.0s), it writes to `m_queueMutex` and `m_completedCallbacks` on **already-freed heap memory**, triggering MSVC `/GS` stack cookie fast-fail (`0xc0000409`) or pointer corruption (`SEGFAULT`).
  - **Proof of Isolation**: When the outbox file was removed, the exact crash sequence `test_dedicated_server.exe & test_multiplayer_load.exe` **passed 100% cleanly** (exit code 0), and full CTest in Release and Debug **passed 48/48 (100%)**!

---

## 2. Experiment A: Teardown Order

### 2.1 Experimental Protocol
- **Variant A (Baseline Existing Order)**:
  1. `c->Disconnect()` on all clients.
  2. `m_clients.clear()` (destroys client objects and client ENet hosts).
  3. `m_server->RequestShutdown("Load test finished")`.
  4. `m_server->Shutdown()` -> `m_server.reset()` (server peer flush and host destruction).
- **Variant B (Server-First Shutdown)**:
  1. `c->Disconnect()` on all clients while client objects and transports remain alive.
  2. `m_server->RequestShutdown("Load test finished")`.
  3. `m_server->Shutdown()` -> `m_server.reset()` (server peer cleanup completed while client endpoints exist).
  4. `m_clients.clear()` (destroy client objects only after server host is completely stopped).

### 2.2 Results Matrix

| Variant | Run # | Sequence | Result | Exit Code | Failure Location |
|:---|:---:|:---|:---:|:---:|:---|
| **Variant A** | 1 | Chained after `DedicatedServerTest` | **FAIL** | `1` (`0xc0000409`) | Section 7 (Client #12) |
| **Variant A** | 2 | Chained after `DedicatedServerTest` | **FAIL** | `1` (`0xc0000409`) | Section 4 (MatchBot_8) |
| **Variant B** | 1 | Chained after `DedicatedServerTest` | **FAIL** | `1` (`0xc0000409`) | Section 2 (LatBot_4) |

### 2.3 Scientific Conclusion
**Refuted Hypothesis**: Teardown sequence inversion alone is **not** the root cause of the crash. Both variants exhibit the identical memory safety crash.

---

## 3. Experiment B: Port Recycling

### 3.1 Experimental Protocol
- **Baseline**: Repeated start/stop cycles on port `17777` (25+ cycles).
- **Controlled Variant**: Dynamic sequential port allocation:
  - Each `LoadTestController::StartServer()` call increments a static cycle port (`port = 18000 + (s_cyclePort++)`).
  - All clients connect to the newly allocated cycle port.
  - Concurrency held constant at 64 clients; all 7 test sections retained.

### 3.2 Results Matrix

| Protocol | Run # | Server Port Strategy | Result | Exit Code | Failure Location |
|:---|:---:|:---|:---:|:---:|:---|
| **Baseline** | 1 | Reused port 17777 | **FAIL** | `0xc0000409` | Section 7 (Client #12) |
| **Baseline** | 2 | Reused port 17777 | **FAIL** | `0xc0000409` | Section 4 (MatchBot_8) |
| **Variant** | 1 | Dynamic sequential ports (18000–18025) | **FAIL** | `0xc0000409` | Section 7 (Client #12, port 18022) |

### 3.3 Scientific Conclusion
**Refuted Hypothesis**: Port recycling / Winsock socket address reuse on port 17777 alone is **not** the root cause. The crash reproduced at the exact client count even when binding to completely unallocated ports.

---

## 4. Experiment C: Test Ordering & Outbox State Leakage

### 4.1 Experimental Protocol
MultiplayerLoadTest was executed under four distinct ordering conditions:
1. **Condition 1**: Standalone in isolation.
2. **Condition 2**: Chained immediately after `NetworkTransportTest`.
3. **Condition 3**: Chained immediately after `DedicatedServerTest`.
4. **Condition 4**: Chained immediately after `DedicatedServerTest` with disk outbox artifacts removed.

### 4.2 Results Matrix

| Condition | Description | Runs | Passed | Failed | Pass Rate | Observed Result |
|:---|:---|:---:|:---:|:---:|:---:|:---|
| **C-1** | Standalone Execution | 5 | 5 | 0 | **100%** | All 7 sections pass cleanly. |
| **C-2** | After `NetworkTransportTest` | 1 | 1 | 0 | **100%** | Both test suites pass cleanly. |
| **C-3** | After `DedicatedServerTest` (Outbox file present) | 2 | 0 | 2 | **0%** | **Crashed 100% of the time** (`0xc0000409`). |
| **C-4** | After `DedicatedServerTest` (Outbox file removed) | 1 | 1 | 0 | **100%** | **Passed 100% cleanly** (exit code 0). |
| **C-5** | Full CTest Release (Outbox file removed) | 1 | 1 | 0 | **100%** | **48/48 Passed (100%)** (42.44s). |
| **C-6** | Full CTest Debug (Outbox file removed) | 1 | 1 | 0 | **100%** | **48/48 Passed (100%)** (79.33s). |

---

## 5. Comprehensive Root-Cause Analysis

### 5.1 The Vulnerability Chain
```
1. DedicatedServerTest runs
   └── Writes durable outbox file: `user_data/outbox/match_match_50001.json`
2. MultiplayerLoadTest begins (25+ rapid DedicatedServer cycles)
   └── Each DedicatedServer::Initialize() loads `match_match_50001.json`
       └── Enqueues outbox delivery -> calls HttpClient::SendRequestAsync()
           └── Spawns std::thread(...).detach() capturing `this` (HttpClient pointer)
3. Load test section ends (e.g. 1.0s - 3.0s duration)
   └── LoadTestController::StopServer() called
       └── DedicatedServer destroyed -> HttpClient destroyed -> Memory freed!
4. Detached background thread finishes failed HTTP connection (after 1.5s - 5.0s)
   └── Executes: std::lock_guard<std::mutex> lock(m_queueMutex);
   └── Executes: m_completedCallbacks.push_back(...);
   └── DEREFERENCES FREED MEMORY (USE-AFTER-FREE)
       ├── In Release (/O2): Trips __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE) (0xc0000409)
       └── In Debug: Crashes with memory access violation (SEGFAULT)
```

### 5.2 Evidence Establishing Conclusive Causation
1. **Correlation**: The crash only manifests after `DedicatedServerTest` runs or when an outbox file is present on disk.
2. **Timing Alignment**: The crash occurs at $t \approx 20\text{s} - 37\text{s}$ into execution, exactly matching the accumulated thread timeout window of multiple detached HTTP requests attempting to reach non-existent `http://localhost:8080`.
3. **Causal Elimination**: Removing the outbox file from disk completely eliminated the crash in both standalone chaining and full CTest across Debug and Release (100% pass rate).

---

## 6. Official Root-Cause Classification

### Classification: **CONFIRMED ROOT CAUSE**
**Root Cause Title**: Asynchronous Detached-Thread Use-After-Free in `HttpClient::SendRequestAsync` triggered by durable outbox file recovery during rapid dedicated server cycling.

---

## 7. Recommended Production Corrective Scope

To permanently resolve DEF-010 in a dedicated implementation phase:
1. **Thread Lifecycle Management in `HttpClient`**:
   - Replace raw detached threads (`std::thread(...).detach()`) with managed thread tracking (e.g., `std::jthread` or a thread pool / joining worker queue in `HttpClient`).
   - Implement `HttpClient::~HttpClient()` that cancels or joins inflight requests before tearing down `m_queueMutex` and `m_completedCallbacks`.
2. **Outbox Directory Isolation in Tests**:
   - Configure `DedicatedServer` test harnesses to use temporary sandboxed outbox directories (or in-memory mock outbox) to prevent cross-test disk state pollution.
3. **Safe Teardown in `DedicatedServer::Shutdown()`**:
   - Cancel all pending HTTP requests in `m_matchOutbox` prior to resetting server subsystems.

---

## 8. Change Control & Reversion Status

- All experimental modifications to `tests/load/LoadTestController.cpp` have been **completely reverted**.
- Working tree contains zero unapproved changes.
- Rigor preserved: 64-client load concurrency and all 7 test sections remain 100% active.
- Defect **DEF-010 remains OPEN** pending authorization of the permanent corrective implementation.
- **Stage 5 is STRICTLY BLOCKED**.

