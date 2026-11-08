# Wildbound — Phase 47 Benchmark & Profiling Report

**Project:** Wildbound  
**Phase:** 47: Optimization, Performance Engineering & Security Foundations  
**Platform:** Windows 11 (MSVC 2022, C++20, x64 Debug build)  
**Date:** November 2026  

---

## 1. Executive Summary

Phase 47 established empirical performance baselines, identified critical bottlenecks across rendering, memory allocation, and dedicated server execution, implemented targeted architectural optimizations, and introduced a complete server-authoritative anti-cheat and rate-limiting foundation.

All optimizations were validated with zero regressions: both `wildbound.exe --test-run` and `WildboundServer.exe --test-run` pass cleanly with exit code 0, and all project test suites (including `SecurityValidationTest`) achieve a 100% pass rate.

---

## 2. Quantitative Performance Comparison

### 2.1 Client Rendering & Frame Time Benchmarks

| Metric | Pre-Optimization Baseline | Post-Optimization (Phase 47) | Improvement / Delta |
| :--- | :--- | :--- | :--- |
| **Average Frame Time** | 2.45 ms (408 FPS uncapped) | **1.82 ms (549 FPS uncapped)** | **+34.6% FPS (+0.63 ms savings)** |
| **Visible Tile Render Count** | 640 tiles / frame | 640 tiles / frame | Exact viewport coverage |
| **Tileset Texture Lookups** | 640 map queries / frame | **1 cached lookup** | **99.8% query reduction** |
| **Render Allocations (Heap)** | ~240 allocs / sec | **0 allocs / sec** | **100% eliminated** |
| **Frustum Culling Savings** | Offscreen entities drawn | **Entities & anims culled outside viewport + 64px** | Reduced GPU draw submissions |
| **Particle Pruning Time** | $O(N)$ vector erase copies | **$O(1)$ swap-and-pop** | Negligible CPU cost |

---

### 2.2 Dedicated Server Runtime Benchmarks

| Metric | Pre-Optimization Baseline | Post-Optimization (Phase 47) | Target Requirement |
| :--- | :--- | :--- | :--- |
| **Dedicated Server Tick Rate** | 30.00 Hz | **30.00 Hz** | 30.00 Hz (Fixed) |
| **Average Tick Duration** | 0.00 ms (headless idle) | **0.00 ms** | < 10.0 ms |
| **Peak Tick Duration** | 0.16 ms | **0.16 ms** | < 15.0 ms |
| **Overload Tick Count** | 0 | **0** | 0 |
| **Spatial Bounds Search Space** | 41,503 bounds | **41,503 bounds indexed** | Spatial Partitioning Active |
| **Ingress Packet Validation** | Unchecked wire deserialization | **Normalized, finite, bounded** | 100% untrusted client boundary |
| **Simulated Ingress Attack Peak** | Crashes possible via NaN | **0 crashes, 100% rejected** | Zero memory corruption |

---

### 2.3 Lua AI & Scripting Performance

| Metric | Benchmark Result | Evaluation |
| :--- | :--- | :--- |
| **Goblin AI Decision Latency** | 9.77 $\mu\text{s}$ / call | Well within $50.0\,\mu\text{s}$ per-enemy target |
| **Skeleton AI Decision Latency** | 9.85 $\mu\text{s}$ / call | Nominal |
| **Instruction Budget Ceiling** | 50,000 instructions | Default guard against infinite loops |
| **Emergency Abort Latency** | 0.234 ms | Infinite loop aborted without engine stutter |

---

## 3. Anti-Cheat & Security Benchmark

| Attack Vector | Simulated Test Invocations | Result | Mitigation |
| :--- | :--- | :--- | :--- |
| **Rate Limit Flooding** | 200 packets / burst | **Rejected at token threshold** | Token-bucket rate limiter |
| **Speed Hack (750 px/s)** | Continuous movement packets | **Clamped to max allowable speed** | Movement distance clamping |
| **Teleport Hack (707 px jump)** | Direct coordinate rewrite | **Snapped back to server truth** | Snapback to last valid pos |
| **NaN Coordinate Poisoning** | Injected IEEE 754 NaN float | **Rejected at network boundary** | `std::isfinite` deserialization |
| **Reach Hack (150 px attack)** | Out-of-range combat request | **Rejected with Violation** | Weapon physical range check |
| **Attack Macro (0 ms delay)** | Rapid-fire attack trigger | **Rejected with Cooldown Violation** | Server timestamp cooldown check |
| **Loot Race Condition** | 2 simultaneous pickup packets | **1st claims, 2nd rejected** | Atomic `m_claimedLootIds` hash set |
| **Vacuum Loot Hack (275 px)** | Distant pickup attempt | **Rejected with Range Violation** | Proximity reach verification |

