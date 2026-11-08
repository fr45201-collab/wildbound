# Wildbound — Phase 47: Performance Baseline & Profiling Report

**Project:** Wildbound  
**Phase:** 47 (Optimization, Performance Engineering & Security Foundations)  
**Platform:** Windows 11 (MSVC, C++20, x64, Debug build configuration)  
**Target Hardware:** x86_64, 8 cores, 8 GB Physical RAM  
**Resolution:** Canvas 640x360, Window 1280x720  
**Date:** November 2026

---

## 1. Executive Summary

This report captures the empirical pre-optimization performance baselines across the graphical client (`wildbound.exe`), the headless dedicated server (`WildboundServer.exe`), the Lua scripting subsystem (`LuaManager`), and network packet processing (`PacketValidator` / `ENetServer`).

---

## 2. Client Rendering & Simulation Baselines

### 2.1 Baseline Metrics (Pre-Optimization)

| Metric | Measured Baseline Value | Target Requirement | Evaluation / Notes |
| :--- | :---: | :---: | :--- |
| **Target Frame Rate** | 60.0 FPS | 60.0 FPS | Capped via raylib `SetTargetFPS(60)` |
| **Average Frame Time** | 16.66 ms | $\le 16.66$ ms | Stable at 60 FPS in empty/light scenes |
| **Tile Rendering Pass** | ~2.80 ms | $< 2.00$ ms | Redundant per-tile texture hash map lookups |
| **Entity Sorting Pass** | ~0.95 ms | $< 0.40$ ms | Heap allocation of `std::vector<RenderItem>` per frame |
| **Frustum Culling** | Partial (Tiles only) | Complete | Offscreen entities are sorted & submitted to GPU |
| **Per-Frame Heap Allocations**| 4–6 dynamic vectors | 0 (reusable buffers) | `renderEntities`, `items`, `visibleObjs` reallocated every frame |
| **Client Virtual Memory** | ~185 MB | $< 256$ MB | Within acceptable budget |
| **Client Resident (RSS)** | ~112 MB | $< 180$ MB | Normal raylib texture footprint |

### 2.2 Workload Scaling (Entity Count Impact)

| Entity Count | Pre-Optimization Frame Time | Estimated Sorting & Culling Time | Observed Bottleneck |
| :---: | :---: | :---: | :--- |
| **1 (Player only)** | 16.66 ms (60 FPS) | 0.08 ms | None |
| **10 Entities** | 16.66 ms (60 FPS) | 0.22 ms | Minor vector resizing |
| **50 Entities** | 17.20 ms (58 FPS) | 1.15 ms | Offscreen entities Y-sorted & rendered |
| **100 Entities** | 20.40 ms (49 FPS) | 2.85 ms | Significant draw-call and sorting overhead |
| **250 Entities** | 29.80 ms (33 FPS) | 7.40 ms | Severe CPU submission bottleneck |

---

## 3. Dedicated Server Simulation Baselines

### 3.1 Baseline Metrics (Headless 30 Hz)

| Metric | Measured Baseline Value | Target Requirement | Evaluation / Notes |
| :--- | :---: | :---: | :--- |
| **Target Server Tick** | 30.0 Hz (33.33 ms) | 30.0 Hz | Fixed timestep simulation accumulator |
| **Average Tick Duration** | 0.45 ms (idle) | $< 10.0$ ms | Excellent idle headroom |
| **Peak Tick Duration** | 2.18 ms | $< 15.0$ ms | Map initialization & collision broadphase |
| **Tick Overruns** | 0 | 0 | Deterministic execution without drift |
| **Server RSS Memory** | ~68 MB | $< 128$ MB | Clean headless execution (zero GPU/audio memory) |
| **Spatial Index Size** | 41,503 bounds | N/A | `battle_royale_island` collision bounds |

### 3.2 AI Scripting Throughput (sol2 / Lua 5.4)

| Population | Total Time (100 ticks) | Average Decision Duration | Headroom vs Budget ($500\,\mu\text{s}$) |
| :---: | :---: | :---: | :---: |
| **1 Enemy** | 1.078 ms | 10.78 $\mu\text{s}$ | 46x |
| **10 Enemies** | 9.932 ms | 9.93 $\mu\text{s}$ | 50x |
| **25 Enemies** | 25.096 ms | 10.04 $\mu\text{s}$ | 50x |
| **50 Enemies** | 48.854 ms | 9.77 $\mu\text{s}$ | 51x |

---

## 4. Network & Protocol Baselines

| Metric | Baseline Value | Target | Notes |
| :--- | :---: | :---: | :--- |
| **Packet Envelope Size** | 16 bytes | 16 bytes | Fixed header (`magic`, `version`, `seq`, etc.) |
| **Max Payload Size** | 1200 bytes | 1200 bytes | Safely below MTU (1500) |
| **Header Validation** | Pre-validated | Strict | Validates magic, version, payload length |
| **Snapshot Generation** | Full list | Delta / Relevant | Transmits all replicated entities unconditionally |
| **Rate Limiting** | None | Enforced | Vulnerable to packet flooding attacks |

---

## 5. Identified Optimization Targets

1. **Rendering**:
   - Frustum culling of all active entities and environment objects against camera viewport + padding.
   - Elimination of per-frame heap allocations (`items`, `renderEntities`, `visibleObjs`) using persistent scratch buffers.
   - Caching tileset texture IDs and handles outside the tile rendering nested loops.
2. **Dedicated Server**:
   - Structured packet dispatcher with client-to-entity mapping.
   - Strict rate-limiting bucket per client connection.
   - Server-authoritative movement speed and teleportation clamping.
   - Attack range, cooldown, and weapon validation pipeline.
   - Atomic loot pickup resolution to prevent race conditions.
3. **Diagnostics**:
   - DevUI performance & security monitoring overlays.

