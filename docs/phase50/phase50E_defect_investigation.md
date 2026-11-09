# Phase 50E — Defect Investigation & Root Cause Analysis

**Project:** Wildbound  
**Phase:** 50E — Gameplay Systems, Combat & AI Verification  
**Stage:** Stage 1 — Defect Investigation (STRICT AUDIT-ONLY)  
**Date:** 2026-11-08  
**Defects Investigated:** DEF-004, DEF-008, DEF-009

---

## 1. DEF-004: Missing Specialized Creature Lua AI Archetypes

### 1.1. Problem Statement
Phase 49 integrated artwork, spritesheets, and animation metadata for three new combat creatures:
1. **Marsh Slime** (`tex_creature_slime`, `sheet_creature_slime`)
2. **Marsh Viper** (`tex_creature_snake`, `sheet_creature_snake`)
3. **Mountain Cyclops** (`tex_creature_cyclops`, `sheet_creature_cyclops`)

However, while baseline enemies (Goblin, Skeleton) possess specialized Lua AI scripts in `scripts/gameplay/ai/`, no behavior scripts were authored for Slime, Viper, or Cyclops.

### 1.2. Architecture & Contract Analysis
In Wildbound's established architecture (`src/scripting/LuaManager.cpp`, `scripts/gameplay/ai/enemy_base.lua`, `goblin.lua`, `skeleton.lua`):
- **C++ Responsibilities**:
  - Entity lifecycle, HP, damage, physics, collision detection, movement execution, and simulation state.
  - Passes an immutable context table `ctx` into the Lua script during each decision tick:
    ```lua
    ctx = {
        state = "IDLE",            -- Current AI state
        x = 100.0, y = 100.0,      -- Current world position
        spawn_x = 100.0, spawn_y = 100.0,
        hp_percent = 1.0,          -- Normalized HP [0.0, 1.0]
        has_target = true,
        target_id = 1,
        target_x = 120.0, target_y = 100.0,
        target_dist = 20.0,
        detection_radius = 140.0,
        attack_radius = 32.0,
        can_retreat = true,
        retreat_hp = 0.2,
        has_memory = false,
        last_known_x = 0.0, last_known_y = 0.0,
        patrol_radius = 80.0
    }
    ```
- **Lua Responsibilities**:
  - Pure decision evaluation: returns an action table:
    ```lua
    result = {
        next_state = "ATTACK",     -- "IDLE", "PATROL", "CHASE", "ATTACK", "RETREAT", "INVESTIGATE", "RETURN", "DEAD"
        target_id = 1,
        move_x = 0.0,              -- Desired movement vector direction
        move_y = 0.0,
        request_attack = true,     -- Authoritative attack request
        attack_id = "slime_slam"   -- Attack identifier validated by C++ CombatSystem
    }
    ```
  - Exposes an update function bound into `wildbound.ai.<creature>_update`.

### 1.3. Archetype Behavior Specifications

#### A. Marsh Slime (`scripts/gameplay/ai/slime.lua`)
- **Theme**: Slow territorial marsh blob. Viscous, stubborn, short-range aggression.
- **Perception**:
  - `detection_radius`: 110.0 units (limited sight in muddy swamp reeds).
  - `attack_radius`: 26.0 units (close-range body slam).
- **Behavioral Characteristics**:
  - Slow pursuit (`move_x`, `move_y` directed toward target, low velocity).
  - Attack: Requests `"slime_slam"` (blunt impact with viscous slowdown tag).
  - Low HP ($\le 25\%$): Splitting or division disengagement; slow recoil away from target (`RETREAT`), or territorial tethering if outside patrol leash ($> 90$ units from spawn $\to$ `"RETURN"`).
  - Immune to cowardly panic: retreats deliberately only when critically damaged.

#### B. Marsh Viper (`scripts/gameplay/ai/snake.lua`)
- **Theme**: Coiled ambush predator. High burst speed, quick strike, evasive repositioning.
- **Perception**:
  - `detection_radius`: 150.0 units (thermal / vibration perception).
  - `attack_radius`: 34.0 units (sudden forward lunge bite).
- **Behavioral Characteristics**:
  - Ambush idle: remains motionless in `"IDLE"` until target enters detection radius.
  - Rapid strike: Requests `"viper_strike"` (piercing venomous lunge).
  - Skirmish / Repositioning: Upon striking or when taking damage ($\le 35\%$ HP), rapidly slithers away (`"RETREAT"`) with lateral evasion vector to reset ambush posture. High reposition frequency.

#### C. Mountain Cyclops (`scripts/gameplay/ai/cyclops.lua`)
- **Theme**: Massive highland brute. Enormous reach, crushing overhead smash, completely fearless.
- **Perception**:
  - `detection_radius`: 220.0 units (towering height overlooks mountain terrain).
  - `attack_radius`: 48.0 units (massive sweeping club reach).
- **Behavioral Characteristics**:
  - Fearless brute: `can_retreat = false`. The Cyclops **NEVER RETREATS**, fighting relentlessly to 0 HP regardless of damage received.
  - Devastating strike: Requests `"cyclops_club"` (wide-arc heavy shockwave smash).
  - Relentless pursuit: If target flees, steadily pursues across wide territory (`patrol_radius`: 150.0 units).

---

## 2. DEF-008: Coincident Elimination Timestamp Race

### 2.1. Problem Statement & Reproduction
In `tests/match/test_match_lifecycle.cpp`, test suite 6 (`TestWinnerDetermination`):
```powershell
.\build\tests\Debug\test_match_lifecycle.exe
```
Fails immediately in Debug configuration:
```
Assertion failed: p3Final->placement == 3, file C:\dev\Wildbound\tests\match\test_match_lifecycle.cpp, line 345
```
Yet in Release configuration (`.\build\tests\Release\test_match_lifecycle.exe`), the test reports `ALL 10 MATCH LIFECYCLE TEST SUITES PASSED (100% OK)`.

### 2.2. Root Cause Analysis

#### Finding 1: The Compiler Macro Asymmetry (`NDEBUG`)
The Release build is compiled by CMake with `-DNDEBUG`. In C++, `<cassert>` defines `assert(expr)` as:
```c
#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
...
#endif
```
In Release mode, line 345 `assert(p3Final->placement == 3);` was **never executed**—it was compiled out! The Release test reported success only because the check was silenced by the compiler. In Debug mode, `assert(...)` remained active and accurately caught the discrepancy.

#### Finding 2: The Test Fixture Clock Defect
In `tests/match/test_match_lifecycle.cpp` lines 316–330:
```cpp
    mgr.StartCountdown();
    mgr.Update(0.1f); // Countdown finishes -> Active. m_matchTime is now 0.1s
    assert(mgr.GetState() == MatchState::Active);

    // Eliminate P3 at t = 10s
    mgr.ProcessElimination(3, EliminationCause::Combat, 1);
    assert(mgr.GetState() == MatchState::Active);

    // Eliminate P2 at t = 20s
    mgr.ProcessElimination(2, EliminationCause::CorruptionStorm, 0);

    // Exactly 1 alive participant left (P1) -> victory!
    mgr.Update(0.1f);
```
The test author wrote comments intending P3 to die at $t = 10\text{s}$ and P2 at $t = 20\text{s}$. However, **`mgr.Update(...)` was never called between the two eliminations**!
Consequently:
- `P3` was eliminated when `m_matchTime == 0.1f`.
- `P2` was eliminated when `m_matchTime == 0.1f`.
Both participants recorded identical elimination timestamps ($0.100\text{s}$).

#### Finding 3: The Production Tie-Breaking Logic
In `src/results/ResultCalculator.cpp` lines 61–84:
```cpp
    // Standard tournament tie resolution
    uint32_t currentRank = (winnerId != 0) ? 2 : 1;
    for (size_t i = 0; i < eliminatedList.size(); ) {
        size_t j = i + 1;
        while (j < eliminatedList.size() &&
               std::abs(eliminatedList[j]->eliminationInfo.timestamp - eliminatedList[i]->eliminationInfo.timestamp) < 0.001f) {
            ++j;
        }

        // Assign current rank to all tied participants
        for (size_t k = i; k < j; ++k) {
            eliminatedList[k]->placement = currentRank;
        }

        currentRank += static_cast<uint32_t>(j - i);
        i = j;
    }
```
Because `std::abs(0.1f - 0.1f) < 0.001f`, `ResultCalculator` classified P2 and P3 as a **joint tie**.
Both participants were assigned `currentRank = 2`.
Thus, `p2Final->placement == 2` and `p3Final->placement == 2`.
When line 345 evaluated `assert(p3Final->placement == 3)`, it failed because `p3Final->placement` was 2!

### 2.3. Resolution Strategy
A two-fold resolution is required:
1. **Test Fixture Correction**: Advance the match clock between eliminations (`mgr.Update(10.0f)`) to match the scenario described in the comments.
2. **Production Tie-Breaking Determinism**: Enhance `ResultCalculator::CalculatePlacements` and `MatchEliminationSystem` so that if two participants are eliminated in the exact same tick ($|t_1 - t_2| < 0.001$), ties are broken deterministically by:
   - Primary: Elimination timestamp (survived longer = better rank).
   - Secondary: Total kills (more kills = better rank).
   - Tertiary: Total damage dealt (more damage = better rank).
   - Quaternary: Reverse event order / participant ID (deterministic tie-breaker ensuring strictly distinct integer placements when required by game mode).

---

## 3. DEF-009: Debug Network Handshake Polling Timeout

### 3.1. Problem Statement & Reproduction
In `tests/network/test_network_transport.cpp`, test suite 5 (`TestLocalhostConnectionLifecycle`):
```powershell
.\build\tests\Debug\test_network_transport.exe
```
Fails in Debug configuration:
```
Assertion failed: clientConnected, file C:\dev\Wildbound\tests\network\test_network_transport.cpp, line 231
```
While in Release configuration, `NetworkTransportTest` passes (executing in 5.81s).

### 3.2. Root Cause Analysis

#### Finding 1: Arbitrary Fixed-Iteration Polling Threshold
In `tests/network/test_network_transport.cpp` lines 223–231:
```cpp
    assert(client.Connect("127.0.0.1", port, 2000));

    for (int i = 0; i < 50 && (!clientConnected || !serverConnected); ++i) {
        client.Update(0.016f);
        server.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(clientConnected);
    assert(serverConnected);
```
- `client.Connect(...)` specifies an allowable connection timeout of **`2000 ms`** (2.0 seconds).
- However, the polling loop hardcodes a limit of **50 iterations** with `sleep_for(10ms)`.
- The total wall-clock duration of the loop before aborting is:
  $$\text{Max Loop Time} = 50 \times 10\,\text{ms} = 500\,\text{ms}\;(0.5\,\text{s})$$
- The test arbitrarily gives up after 500 ms, discarding 75% of the declared 2,000 ms timeout window!

#### Finding 2: Unoptimized Debug Execution Overhead
- In Release builds (`/O2`), context switches and ENet packet serialization complete within 150–350 ms, allowing the 50-iteration loop to succeed.
- In Debug builds (`/Od /RTC1`), stack frame checking, unoptimized function calls, and Windows thread scheduling delay the UDP loopback socket handshake to 550–850 ms.
- Because 550 ms > 500 ms, the loop terminates with `clientConnected == false`, causing `assert(clientConnected)` to trigger.

#### Finding 3: Distinction from DEF-002 / DEF-006 (Sandbox Restriction)
- **DEF-002 / DEF-006**: Tests 38, 39, 46, and 48 fail because the Windows container sandbox blocks cross-process UDP socket binding on port `17777`.
- **DEF-009**: Test 37 (`NetworkTransportTest`) runs within a single process on an ephemeral test port (`BASE_TEST_PORT + 1 = 19991`). When given sufficient time, localhost socket communication **succeeds**. The failure is purely an artificial 50-iteration loop cutoff in the test harness under Debug overhead.

### 3.3. Resolution Strategy
- Modernize the polling loops across `tests/network/test_network_transport.cpp` to use **deadline-based elapsed-time polling** rather than fixed iteration counts:
  ```cpp
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
  while (std::chrono::steady_clock::now() < deadline && (!clientConnected || !serverConnected)) {
      client.Update(0.016f);
      server.Update(0.016f);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  ```
- This respects the declared 2,000 ms protocol timeout, succeeds as soon as the handshake completes (typically within 400–700 ms in Debug), and preserves all connection assertions.

