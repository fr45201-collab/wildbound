# WILDBOUND — PHASE 39: DEDICATED SERVER FOUNDATION

## Technical Specification & Architecture Manual

---

## 1. Executive Summary

Phase 39 delivers the standalone, authoritative **Dedicated Server Foundation** for Wildbound. The dedicated server executable (`WildboundServer.exe`) runs completely decoupled from graphical, windowing, GPU, and audio subsystems, operating as a true headless authoritative process. It hosts multiplayer Battle Royale simulations on dedicated hardware or cloud virtual machines, executing fixed-timestep physics, combat, storm progression, match lifecycle, AI navigation, and network entity synchronization across connected clients.

---

## 2. Core Architectural Principles

1. **Headless Execution Decoupling**:
   - Zero invocation of raylib graphics or audio device initializers (`InitWindow`, `CloseWindow`, `BeginDrawing`, `EndDrawing`, `InitAudioDevice`, `CloseAudioDevice`).
   - Zero graphical UI overlays (`DevUI`, `Dear ImGui`, `rlImGui`).
   - Maps, world geometry, collision volumes, and entity spatial indices load into memory without texture allocations or GPU driver contexts (`AssetManager* = nullptr`).
2. **Authoritative Simulation Loop**:
   - High-precision fixed-timestep simulation loop (`ServerLoop`) operating by default at 30 Hz (33.33ms fixed $\Delta t$).
   - High-resolution frame-time accumulation (`std::chrono::steady_clock`).
   - Spiral-of-death defense: maximum allowable frame time clamped to 250ms with bounded catch-up limited to 5 simulation steps per iteration.
   - Low-latency CPU yielding: enters 1ms micro-sleeps when ahead of simulation schedule to prevent CPU spin burning while maintaining millisecond tick precision.
3. **Formal Lifecycle State Machine**:
   - `Uninitialized` $\rightarrow$ `Initializing` $\rightarrow$ `Starting` $\rightarrow$ `Running` $\rightarrow$ `Stopping` $\rightarrow$ `Stopped`.
   - Thread-safe and OS-signal-safe shutdown handling (`SetConsoleCtrlHandler` on Windows, intercepting `CTRL_C_EVENT`, `CTRL_CLOSE_EVENT`, `CTRL_SHUTDOWN_EVENT`).
   - Idempotent termination: guarantees that duplicate calls to `Shutdown()` safely no-op without crashes or dangling resources.
4. **Reused Core Simulation**:
   - Reuses existing gameplay systems rather than maintaining parallel codebases:
     - `physics::CollisionManager`: Spatial partition hashing and AABB collision resolution.
     - `world::World`: Headless Tiled map loading, spatial queries, and loot containers.
     - `combat::CombatSystem`: Authoritative weapon damage, attack validation, and projectile simulation.
     - `items::ItemManager`: Item definitions, catalog parsing, and inventory rules.
     - `storm::StormManager`: Safe zone shrinking, boundary calculations, and periodic corruption damage.
     - `match::MatchManager`: Battle Royale countdown, participant tracking, elimination processing, and results finalization.
     - `gamemodes::GameModeManager`: Game mode rulesets (`Solo`, `Duos`, `Squads`, `Practice`).
     - `network::NetworkManager` & `replication::ReplicationManager`: Authoritative network replication, snapshot delta compression, and client connection management.
     - `scripting::LuaManager`: Headless Lua 5.4 state machine for match scripting.

---

## 3. Subsystem Breakdown

### 3.1 Headless Decoupling Architecture
In prior phases, `MapLoader` and `World` required a valid `AssetManager` reference, which preloaded GPU textures for tilesets. To achieve headless compatibility:
- `MapLoader::LoadMap` was refactored to take an optional `resources::AssetManager* assetMgr = nullptr`.
- When `assetMgr` is null, texture generation is bypassed while geometry, tile layers, object groups, collision rectangles, and spawn points are fully parsed and cached in RAM.
- `World::LoadMap` exposes an overloaded reference wrapper for existing client callers (`assetMgr`) while allowing pointer invocations (`nullptr`) for server runners.
- `CombatSystem`, `Player`, `WorldItem`, and `LootContainer` all support `audio::AudioSystem* audioSys = nullptr`, safely bypassing audio event emissions.

### 3.2 Fixed-Timestep Simulation Loop (`ServerLoop`)
```
[Frame Start] -> Accumulate FrameTime (clamped to max 250ms)
                     |
                     v
             [Network Poll Hook] (Drain incoming ENet UDP sockets)
                     |
                     v
             [Accumulator >= FixedDeltaTime?]
              |                         |
             (Yes)                     (No)
              |                         |
      [Steps < MaxCatchUp(5)]           v
              |                  [Replication Hook] (Broadcast Snapshot)
       [FixedUpdate Hook]               |
      (Collision, Combat,               v
       Storm, Match, AI)       [Diagnostics Hook] (Record TPS, Latency)
              |                         |
      [Accumulator -= Dt]               v
              |                  [Yield CPU Sleep (1ms)]
              +-------------------------+
```

### 3.3 Server Diagnostics & Telemetry (`ServerDiagnostics`)
The server continuously samples tick metrics and performance counters:
- **Tick Rates & Latency**: Real-time TPS tracking, average tick duration in milliseconds, and peak tick duration.
- **Simulation Overloads**: Increments counter whenever frame backlog exceeds bounded catch-up capacity (5 ticks).
- **Client & Participant Tracking**: Active ENet transport connections vs match-registered participants.
- **Entity Population**: Total replicated entities, enemy actors, loot items, and interactive world objects.
- **Network I/O**: Aggregated packet counts, bytes sent/received, bandwidth rates in KB/s, and validation drops.

---

## 4. Verification & Testing Gauntlet

The server foundation is backed by an automated 10-suite CTest integration test (`test_dedicated_server.cpp`):
1. **Config Parsing & Validation**: Verifies default values, JSON serialization roundtrip, and out-of-bounds parameter clamping.
2. **Command Line Parsing & Overrides**: Tests CLI argument flags (`--port`, `--max-clients`, `--tick-rate`, `--map`, `--mode`, `--log-level`, `--test-run`, `--help`, `--version`).
3. **Lifecycle State Transitions**: Validates state progression, enum conversions, and string serializations.
4. **Simulation Loop & Catch-Up**: Tests tick frequency accuracy, accumulator progression, and 5-step catch-up clamping during simulated 1000ms lag spikes.
5. **Headless World Loading**: Loads the complete 320x320 Battle Royale island map (`battle_royale_island.json`) headlessly with `assetMgr = nullptr`, confirming > 41,000 collision bounds.
6. **Headless Startup & Shutdown Idempotency**: Starts and stops dedicated server instances cleanly, testing repeated `Shutdown()` calls.
7. **Client Connection Acceptance & Capacity**: Connects concurrent `ENetClient` instances over localhost, verifying handshake handling and capacity limits.
8. **Multi-Client Simulation & Replication**: Registers authoritative entities and broadcasts compressed snapshot packets to multiple clients.
9. **Graceful Shutdown with Connected Clients**: Triggers orderly shutdown while clients are connected, asserting host teardown and socket closure.
10. **Diagnostics & Telemetry**: Evaluates average and peak duration tracking, overload flags, and telemetry summary generation.

---

## 5. Build Artifacts

| Binary / Target | Output Location | Description |
|---|---|---|
| `WildboundServer.exe` | `build/Release/WildboundServer.exe` | Standalone Headless Dedicated Server Executable |
| `test_dedicated_server.exe` | `build/tests/Release/test_dedicated_server.exe` | Dedicated Server Automated Integration Test |
| `assets/config/server.json` | `assets/config/server.json` | Dedicated Server Runtime Configuration File |
| `data/server/server.json` | `data/server/server.json` | Fallback Server Configuration File |

