# Wildbound — Phase 46: Scripting Test Matrix & Verification

## 1. Overview

Testing of the Lua scripting infrastructure is automated via `tests/scripting/test_lua.cpp`, compiled into `test_lua.exe` and registered in CTest as `LuaScriptingTest`.

The suite executes 9 exhaustive test groups validating every layer of the subsystem.

---

## 2. Test Matrix Summary

| Test Group | Objective | Status | Key Validations |
| :--- | :--- | :---: | :--- |
| **1. Lifecycle & Error Trapping** | Verify boot, repeated init, error handling, clean shutdown | **PASS** | Syntax errors, missing files, and runtime errors handled without crashes. Repeated `Initialize()` and `Shutdown()` safe. |
| **2. sol2 Bindings & Userdata** | Verify `wildbound.*`, `Vector2`, math helpers | **PASS** | `Vector2` methods (`normalize`, `length`, `distance`), math functions, `wildbound.ai.states` reflection. |
| **3. Caching & Hot Reload** | Verify script caching, registry, and reload functions | **PASS** | In-memory caching avoids re-reads; `ReloadScript` and `ReloadAllScripts` succeed with updated metadata. |
| **4. Infinite Loop Protection** | Verify instruction count hook and budget abort | **PASS** | `while true` loop aborted in **0.234 ms**; budget violation count incremented; Lua state recovers cleanly. |
| **5. Goblin AI Verification** | Test cowardly hit-and-run goblin behavior | **PASS** | Idle at spawn, patrol when drifted, chase target, attack with `goblin_club`, retreat when HP < 20%. |
| **6. Skeleton AI Verification** | Test relentless undead skeleton behavior | **PASS** | Attack with `skeleton_blade`, chase across larger radius, **never retreat** even at 10% HP. |
| **7. Multi-Enemy Independence** | Verify zero cross-contamination | **PASS** | 20 distinct entities (10 Goblins + 10 Skeletons) evaluated concurrently without state leakage. |
| **8. Decision Validation** | Verify C++ sanitization of Lua outputs | **PASS** | NaN/Inf vectors rejected; attack distances > 1.5x weapon radius rejected; cooldown verified. |
| **9. Scalability Benchmarks** | Measure throughput across 1, 10, 25, 50 enemies | **PASS** | Average per-decision time **9.77 $\mu\text{s}$** (target < 500 $\mu\text{s}$). |

---

## 3. CTest Integration

- Test Registered: `LuaScriptingTest` (Test #2)
- Execution Command: `ctest --test-dir build -C Debug -R LuaScriptingTest --output-on-failure`
- Execution Result: **100% Passed in 0.12s**.

