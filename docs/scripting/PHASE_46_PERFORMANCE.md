# Wildbound — Phase 46: Performance Benchmarks & Frame Budget Analysis

## 1. Benchmark Methodology

Automated benchmarks were executed on Windows 11 (MSVC, C++20, Debug/RelWithDebInfo) using the `test_lua` benchmark harness across 100 simulation ticks for populations of 1, 10, 25, and 50 concurrent enemies.

---

## 2. Benchmark Results

| Enemy Population | Total Time (100 Ticks) | Average Duration per Decision | Target Budget | Margin |
| :---: | :---: | :---: | :---: | :---: |
| **1 Enemy** | 1.078 ms | **10.78 $\mu\text{s}$** | 500 $\mu\text{s}$ | 46x headroom |
| **10 Enemies** | 9.932 ms | **9.93 $\mu\text{s}$** | 500 $\mu\text{s}$ | 50x headroom |
| **25 Enemies** | 25.096 ms | **10.04 $\mu\text{s}$** | 500 $\mu\text{s}$ | 50x headroom |
| **50 Enemies** | 48.854 ms | **9.77 $\mu\text{s}$** | 500 $\mu\text{s}$ | 51x headroom |

---

## 3. Frame Budget Analysis (60 FPS & 30 Hz Dedicated Server)

- **Target Server Tick**: 33.33 ms (30 Hz).
- **Target Client Frame**: 16.66 ms (60 FPS).
- At 50 active enemies simultaneously evaluating Lua AI decisions each tick:
  $$\text{Total Lua CPU Time} = 50 \times 9.77\,\mu\text{s} = 0.488\,\text{ms}$$
- This accounts for less than **1.5%** of the 30 Hz dedicated server tick budget, and less than **3%** of a 60 FPS client frame.

---

## 4. Memory Footprint

- **Base Runtime**: ~120 KB (Lua state core, string intern tables, standard libraries).
- **Loaded Scripts**: +30 KB (all behaviors, goblin, skeleton, and engine configs).
- **Peak Dynamic Garbage**: ~210 KB under heavy 50-enemy continuous execution before periodic garbage collection cycles.
- **Memory Leak Testing**: Over 10,000 continuous tick iterations, memory remained bounded within a stable band, confirming zero leak behavior.

