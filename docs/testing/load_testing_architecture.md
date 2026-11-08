# Wildbound — Load Testing Architecture

## 1. Overview & Objective

The Wildbound Load Testing Architecture provides an automated, programmatic framework for subjecting `WildboundServer` (dedicated server) and `WildboundCoordinator` (matchmaking service) to extreme concurrent player swarms, network degradation, match cycling, and node failure scenarios.

The architecture was engineered for Phase 48 to satisfy strict empirical validation requirements without requiring thousands of physical client machines. It enables in-process and cross-process headless execution capable of driving realistic high-frequency UDP game traffic through ENet sockets.

---

## 2. Architectural Components

```
+-------------------------------------------------------------------------------+
|                            LoadTestController                                 |
|  - Spawns / Manages In-Process or External DedicatedServer                     |
|  - Manages Swarm of LoadTestClient Instances                                  |
|  - Pumps Simulation Loop & Network Sockets Synchronously                      |
|  - Samples Telemetry & Aggregates Metric Percentiles                          |
+-------------------------------------------------------------------------------+
               |                                               |
     Controls / Steps                                Sends UDP Packets / Receives
               v                                               v
+-------------------------------+             +---------------------------------+
|        DedicatedServer        | <========= |       LoadTestClient Swarm      |
|  - 30 Hz Fixed Timestep Loop  |   ENet UDP  |  - High-frequency Movement      |
|  - Spatial Index & Collision  |   Protocol  |  - Rate-Limited Combat Actions  |
|  - State Snapshot Broadcast   |  (Port 17777)|  - Configurable Latency / Loss  |
|  - Diagnostics & Telemetry    |             |  - ENet Peer Metrics Tracking   |
+-------------------------------+             +---------------------------------+
```

### 2.1 LoadTestMetrics (`LoadTestMetrics.hpp`)
Defines the standard measurement structures across the entire test suite:
- `ClientConnectionMetric`: Tracks per-client connection duration, round-trip time (RTT), packets transmitted/received, snapshots ingested, and error counters.
- `LoadTestLevelResult`: Aggregates configured targets, actual connected clients, admitted participants, connection timings (average and maximum), tick durations, tick overruns, RTT averages, and server stability classification.
- `LatencyScenarioResult`: Evaluates RTT percentiles (P50, P90, P95, P99) under synthetic jitter/delay.
- `PacketLossScenarioResult`: Measures reliable packet recovery, state synchronization stability, and loss resilience under 0% to 20% dropped packets.
- `MatchStabilityResult`: Measures lifecycle completion across consecutive matches, memory stability, and leak detection.
- `ServerRecoveryResult`: Records graceful shutdown, process restart, client survivor reconnection, and match failover behavior.

### 2.2 Simulated Client (`LoadTestClient.hpp` / `LoadTestClient.cpp`)
- Encapsulates `network::ENetClient`.
- Implements the complete protocol handshake: ENet connect -> `MsgClientHello` -> `MsgConnectionAccepted`.
- Generates 30 Hz movement packets with circular wandering pathing.
- Issues periodic combat attack events (every 2.5s) and pickup interaction queries.
- Injects synthetic packet drop and simulated latency to evaluate adversarial network conditions.

### 2.3 Load Test Controller (`LoadTestController.hpp` / `LoadTestController.cpp`)
- Spawns and configures `server::DedicatedServer` headlessly.
- Manages staggered swarm connections with configurable delay intervals (default 15–25 ms) to prevent artificial connection thundering.
- Pumps the fixed timestep simulation loop (`m_loop.SingleStep(dt)`) and polls network sockets for both server and clients in lockstep.
- Enforces timeout budgets, collects diagnostics metrics, and produces structured results.

### 2.4 Test Suite Runner (`test_multiplayer_load.cpp`)
Executes 7 distinct stress suites:
1. Incremental Concurrency (Tiers 1–8: 2, 4, 8, 16, 24, 32, 48, 64 clients).
2. Latency Profiles (0, 20, 50, 100, 150, 250, 400 ms).
3. Packet Loss Conditions (0%, 0.5%, 1%, 2%, 5%, 10%, 20%).
4. Match Stability & Lifecycle Cycling (Repeated 8-player matches).
5. Disconnect & Reconnect Resilience (Abrupt 50% client drop).
6. Server Failure & Recovery (Graceful shutdown, process crash, restart, client failover).
7. Extended Duration Stability (Sustained multi-client simulation with zero memory leaks).

---

## 3. Configuration Management (`assets/config/load_test.json`)

The load testing framework is driven by a schema-validated configuration file:
- **Server Binding**: Configures server host IP, UDP port (default 17777), and game mode.
- **Client Swarms**: Specifies default bot count, connect interval (ms), and duration.
- **Adverse Condition Toggles**: Enables or disables simulated latency, jitter, and packet loss.
- **Sampling & Thresholds**: Configures diagnostics sampling frequency and overload criteria.

---

## 4. Execution Pipeline & Sandbox Compatibility

Because Windows sandbox containers isolate local UDP loopback sockets across separate process boundaries, the multiplayer load testing executable must be executed with socket loopback permissions (`BypassSandbox: true` or native elevated console).

Build target:
```powershell
cmake --build build --config Debug --target test_multiplayer_load
```

Execution target:
```powershell
.\build\tests\Debug\test_multiplayer_load.exe
ctest --test-dir build -C Debug -R MultiplayerLoadTest --output-on-failure
```

