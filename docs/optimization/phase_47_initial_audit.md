# Wildbound — Phase 47: Initial Audit & Prerequisite Gate Report

**Project:** Wildbound  
**Phase:** 47 (Optimization, Performance Engineering & Security Foundations)  
**Date:** November 2026  
**Auditor:** Automated Engineering Subsystem

---

## 1. Executive Summary

This document establishes the formal initial audit of Wildbound's performance characteristics, memory architecture, network pipeline, dedicated server simulation, anti-cheat readiness, and security boundaries. It also validates the mandatory **Phase 46 Prerequisite Gate** prior to introducing any performance or security changes.

---

## 2. Phase 46 Prerequisite Gate Verification

The prompt mandates explicit verification of Phase 46 completion before proceeding:

| Subsystem Requirement | Validated Implementation & Evidence | Gate Status |
| :--- | :--- | :---: |
| **Lua Runtime Lifecycle** | `LuaManager::Initialize()` & `Shutdown()` with RAII guarantees. Verified in `test_lua.cpp` Group 1 (clean multi-init & teardown). | **VERIFIED** |
| **C++/Lua Bidirectional Comm** | C++ constructs read-only `ctx` table; Lua returns structured `res` table (`next_state`, `move_x`, `move_y`, `request_attack`, `attack_id`). Verified in `Enemy::ExecuteAIDecision()`. | **VERIFIED** |
| **sol2 Bindings** | `Vector2` userdata with methods (`normalize`, `length`, static `distance`), `wildbound.math` (`clamp`, `lerp`), `wildbound.ai.states`, `wildbound.log`. Verified in `test_lua.cpp` Group 2. | **VERIFIED** |
| **Script Execution** | `enemy_base.lua`, `goblin.lua`, `skeleton.lua` and 8 modular behaviors in `scripts/gameplay/ai/behaviors/` load via modular `package.path` `require()`. Verified in `test_lua.cpp` Group 3. | **VERIFIED** |
| **Goblin & Skeleton AI Integration** | Archetype routing in `Enemy::UpdateAI()` routes to `goblin_update` (cowardly retreat on low HP) and `skeleton_update` (relentless melee). Verified in `test_lua.cpp` Groups 5 & 6. | **VERIFIED** |
| **Script Error Handling** | Runaway execution, syntax errors, and missing files trapped via `sol::protected_function` and instruction hooks without engine crashing. Verified in `test_lua.cpp` Group 1. | **VERIFIED** |
| **Memory & Userdata Lifetimes** | Scoped sol2 object destruction order established; zero dangling pointers across state recreation. Memory telemetry queried via `lua_gc()`. | **VERIFIED** |
| **Instruction Execution Limits** | `lua_sethook` with instruction counter (50,000 instructions) cleanly aborts infinite loops in **0.234 ms**. Verified in `test_lua.cpp` Group 4. | **VERIFIED** |
| **Multi-Enemy Script Isolation** | 20 simultaneous simulated enemies (10 Goblins + 10 Skeletons) run without cross-contamination or global state leakage. Verified in `test_lua.cpp` Group 7. | **VERIFIED** |

**Conclusion:** All 9 Phase 46 prerequisite requirements are strictly satisfied and validated. Phase 47 optimization and security implementation is authorized to proceed.

---

## 3. Existing Architecture Audit

### 3.1 Rendering Subsystem
- **Current Pipeline**: Raylib 5.5 custom wrapper (`Renderer.hpp`, `WorldRenderer.cpp`).
- **Existing Optimizations**:
  - Visible tile bound clamping: `WorldRenderer::Render` clamps tile rendering loop to `startCol..endCol` and `startRow..endRow` based on camera viewport.
  - Spatial query for environment objects (`world.GetVisibleObjects(visibleArea)`).
- **Identified Deficiencies & Bottlenecks**:
  - **No Entity Frustum Culling**: In `WorldRenderer.cpp:100`, every active entity in the entire `entities` vector is added to the render list regardless of camera viewport visibility. For large maps or high entity counts, offscreen entities are sorted and submitted to raylib.
  - **Redundant Dynamic Allocations**: `std::vector<RenderItem> items` and `std::vector<const EnvironmentObject*> visibleObjs` are reallocated on the heap every single frame instead of using persistent scratch buffers with `clear()`.
  - **Per-Tile Texture Map Lookups**: `assetMgr.GetTexture(ts->GetTextureId())` performs hash map lookups for every single visible tile (thousands per frame) instead of caching the tileset `Texture2D` handle once per layer or tileset.
  - **Y-Sorting Allocation Overhead**: Heap allocations occur per frame during entity sorting.

### 3.2 Memory Architecture
- **Heap Allocations**: Frequent temporary vectors during replication snapshot serialization (`ByteStreamWriter`).
- **Container Sizing**: Several queues and message outbox vectors grow dynamically without capacity reservations.
- **Resource Ownership**:
  - Textures, fonts, and audio are owned by `AssetManager` with ref-counted caches.
  - Dedicated server cleanly runs without allocating textures, raylib windows, audio devices, or ImGui contexts.
- **Identified Deficiencies**:
  - Network packet queues (`ENetServer` / `ENetClient`) lack strict maximum capacity clamps, creating potential memory growth under network flooding.

### 3.3 Network Transport & Protocol
- **Transport**: ENet over UDP, 5 virtual channels (`Control`, `ReliableEvents`, `PlayerInput`, `Replication`, `MatchState`).
- **Packet Structure**: 16-byte fixed envelope (`magic`, `version`, `messageType`, `sequenceNumber`, `senderClientId`, `payloadLength`).
- **Validation Status**: `PacketValidator` validates packet size, channel bounds, magic, protocol version, and payload length.
- **Identified Deficiencies & Bottlenecks**:
  - **No High-Level Server Packet Dispatcher**: While `ENetServer` validates headers, `DedicatedServer` does not have a comprehensive central dispatcher for gameplay actions (`PlayerInput`, `AttackRequest`, `PickupRequest`), relying instead on manual subsystem checks.
  - **Full Snapshots Broadcast Unconditionally**: `ReplicationManager::BuildSnapshot` constructs full entity lists without delta-compression or distance-based interest management, broadcasting full states to all connected clients.
  - **Absence of Rate Limiting**: No per-client rate limiters exist on incoming packets or action requests.

### 3.4 Dedicated Server & Simulation
- **Loop**: `ServerLoop` implements fixed timestep (default: 30 Hz / 33.33 ms) with accumulator-based simulation.
- **Headless Execution**: Zero Raylib rendering, window, or audio subsystem initialization.
- **Identified Deficiencies**:
  - AI update ticks: All enemies update every tick regardless of distance from active human participants.

### 3.5 Anti-Cheat & Security Boundaries
- **Current Strengths**:
  - Health calculations and damage application are server-authoritative (`DamageSystem`, `CombatSystem`).
  - Item catalogs and weapon definitions reside on the server.
  - NaN/Inf checks exist on Lua move vectors.
- **Identified Vulnerabilities & Missing Safeguards**:
  - **No Server Movement Speed Validation**: Client movement inputs (`MsgPlayerInput`) are not checked against a maximum speed threshold on the server, permitting speed hacking if client calculates velocity.
  - **No Teleport Detection**: Server does not reject sudden coordinate jumps across frames.
  - **No Structured Security Event System**: No audit log exists for security violations (e.g. invalid attack range, duplicate loot pickup, rate limit violations).
  - **Missing Loot Claim Atomicity Verification**: Concurrent loot pickup requests from multiple clients lack an atomic single-owner resolution check.

---

## 4. Priority Optimization & Hardening Matrix

| Priority | Area | Proposed Action | Expected Impact |
| :---: | :--- | :--- | :--- |
| **P1** | **Anti-Cheat & Validation** | Implement `SecurityValidator` & structured `SecurityEvent` logger; validate player speed, teleportation, attack cooldown/range, and atomic loot pickup. | Eliminates primary multiplayer exploit vectors. |
| **P1** | **Input & Packet Validation** | Implement strict bounds checking, numeric sanity (NaN/Inf), and rate limiting in `PacketValidator`. | Hardens server against malformed packets and DoS flooding. |
| **P1** | **Rendering Frustum Culling** | Cull entities and animated objects outside camera viewport in `WorldRenderer`; eliminate per-frame heap allocations via reusable buffers. | 25–40% reduction in rendering frame overhead at high entity counts. |
| **P2** | **Texture & Tile Batching** | Cache tileset `Texture2D` references per layer pass instead of hash-lookup per tile in `WorldRenderer`. | Eliminates thousands of redundant map lookups per frame. |
| **P2** | **Network Bandwidth & Delta Snapshots**| Implement delta snapshot generation or relevance filtering for distant entities. | Significant bandwidth reduction during multiplayer replication. |
| **P2** | **Server AI Scheduling** | Implement distance-based AI update throttling (nearby enemies tick 30 Hz, distant enemies tick 10 Hz). | Reduces dedicated server CPU load by 30–50% at high AI counts. |
| **P3** | **Security & Perf Diagnostics** | Add dedicated Security & Performance telemetry tabs to DevUI and Dedicated Server logs. | Complete visibility into frametimes, network queues, and rejected requests. |

