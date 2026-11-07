# WILDBOUND — PHASE 46: INITIAL REPOSITORY AUDIT
## Lua Integration & Scripting Validation

**Project:** Wildbound  
**Phase:** 46  
**Primary Language:** C++20  
**Scripting Language:** Lua 5.4.7  
**C++/Lua Binding:** sol2 v3  
**Audit Date:** 2026-11-08  
**Audit Status:** COMPLETE  

---

# 1. Executive Summary

Wildbound combines a high-performance C++20 game engine with Lua 5.4 for configurable gameplay logic and enemy artificial intelligence. This audit investigates the complete scripting pipeline: from Lua state initialization, sol2 bindings, script discovery, and C++-to-Lua execution paths, to enemy AI decision-making, fallback execution, memory safety, and error handling.

### Key Audit Findings:
1. **Lua Runtime Foundation Exists**: `wildbound::scripting::LuaManager` initializes a `sol::state` with standard libraries (`base`, `package`, `math`, `string`, `table`) and successfully loads engine configuration (`scripts/config/engine.lua`).
2. **Enemy AI Invokes Lua**: `Enemy::ExecuteAIDecision` calls `wildbound.ai.update_enemy` in `scripts/gameplay/ai/enemy_base.lua` with an execution context table and parses the resulting movement and attack decisions.
3. **Disconnected Behavior Scripts**: The repository contains 8 behavior scripts in `scripts/gameplay/ai/behaviors/` (`attack.lua`, `chase.lua`, `defend.lua`, `idle.lua`, `investigate.lua`, `patrol.lua`, `retreat.lua`, `return.lua`) that are never loaded or referenced by any C++ system or by `enemy_base.lua`.
4. **Missing Creature-Specific Scripts**: `data/characters/enemies/goblin.json` and `skeleton.json` both point generically to `scripts/gameplay/ai/enemy_base.lua`. No dedicated `goblin.lua` or `skeleton.lua` scripts exist to express their distinct creature personalities (cowardly goblin retreat vs. relentless skeleton fight-to-the-death).
5. **No Execution-Budget or Infinite Loop Protection**: `LuaManager` lacks a `lua_sethook` instruction counter. A `while true do end` loop in any script will lock the main thread indefinitely.
6. **Minimal C++ sol2 Bindings**: Only four logging functions (`log_info`, `log_warn`, `log_error`, `log_debug`) and an engine version string are registered. Lua scripts cannot query game math, vector transformations, navigation grids, or entity attributes directly.
7. **No Script Registry or Caching**: Scripts are loaded by raw string path without version tracking, caching, or development hot reload.
8. **Missing Developer UI Diagnostics**: There is no Lua/Scripting tab in DevUI (`src/ui/DevUI.cpp`) to monitor active scripts, execution durations, memory allocations, or error counts.

---

# 2. Lua Runtime & sol2 Audit

### 2.1 Lua 5.4 Runtime
- **Source**: `build/_deps/lua-src/lua-5.4.7/` compiled as static library `lua_static`.
- **Target**: Linked with `sol2` into `wildbound`, `WildboundServer`, and test executables.
- **State Lifecycle**: Managed via `std::unique_ptr<sol::state> m_luaState` inside `LuaManager`.
  - Initialized on `LuaManager::Initialize()`.
  - Cleanly destructed in `LuaManager::Shutdown()`.
  - Re-initialization is supported (`m_luaState.reset()`).
- **Standard Libraries Loaded**: `base`, `package`, `math`, `string`, `table`.
- **Sandbox Restrictions**: Libraries `io`, `os`, and `debug` are omitted by default, preventing unauthorized filesystem access and OS process execution.

### 2.2 sol2 Binding Architecture
- **Version**: sol2 v3 header-only library (`#define SOL_ALL_SAFETIES_ON 1`).
- **Safety Status**: `SOL_ALL_SAFETIES_ON 1` ensures bounds-checking, type verification on userdata, and safe exceptions on Lua type mismatches.
- **Current Registered Bindings** (`src/scripting/LuaManager.cpp`):
  ```cpp
  wildboundTable["version"] = "0.1.0-phase0";
  wildboundTable["log_info"] = [](const std::string& msg) { Logger::Info("LUA", msg); };
  wildboundTable["log_warn"] = [](const std::string& msg) { Logger::Warn("LUA", msg); };
  wildboundTable["log_error"] = [](const std::string& msg) { Logger::Error("LUA", msg); };
  wildboundTable["log_debug"] = [](const std::string& msg) { Logger::Debug("LUA", msg); };
  ```
- **Binding Deficiencies**:
  - No `Vector2` userdata or binding.
  - No `AIState` enum reflection.
  - No direct distance / angle math helper functions.
  - No read-only entity inspection interface.

---

# 3. Complete Lua Script Inventory

| Script Path | Purpose | Owning System | Entry Point | Status |
|---|---|---|---|---|
| `scripts/config/engine.lua` | Primary engine configuration (window, display, FPS, canvas) | `LuaManager::LoadEngineConfig` | Returns root configuration table | **Active & Integrated** |
| `config/engine.lua` | Fallback engine configuration | `LuaManager::ResolveConfigPath` | Returns root configuration table | **Active (Fallback)** |
| `scripts/gameplay/ai/enemy_base.lua` | Generic enemy AI state machine (Idle, Patrol, Chase, Attack, Retreat, Return) | `Enemy::ExecuteAIDecision` | `AI.update_enemy(ctx)` | **Active & Integrated** |
| `scripts/gameplay/ai/behaviors/idle.lua` | Evaluates idle state | None | `Idle.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/patrol.lua` | Evaluates patrol movement | None | `Patrol.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/chase.lua` | Evaluates chase pursuit | None | `Chase.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/attack.lua` | Evaluates combat attack triggers | None | `Attack.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/retreat.lua` | Evaluates retreat vector away from target | None | `Retreat.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/investigate.lua` | Evaluates investigate vector toward last known position | None | `Investigate.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/defend.lua` | Evaluates defensive state | None | `Defend.evaluate(ctx)` | **Disconnected** |
| `scripts/gameplay/ai/behaviors/return.lua` | Evaluates return-to-spawn vector | None | `Return.evaluate(ctx)` | **Disconnected** |

---

# 4. C++ ↔ Lua Execution Flow & AI Integration

### 4.1 Invocation Pipeline
```text
Game::Update(dt)
   │
   ▼
EnemySpawner::Update(dt, world, combatSys, assetMgr, luaMgr)
   │
   ▼
Enemy::UpdateWithWorld(dt, world, combatSys, assetMgr, luaMgr)
   │ (Triggered when m_aiDecisionTimer <= 0)
   ▼
Enemy::ExecuteAIDecision(combatSys, luaMgr)
   │
   ├── 1. Check luaMgr->IsInitialized()
   ├── 2. Extract sol::state& and updateAi = lua["wildbound"]["ai"]["update_enemy"]
   ├── 3. Populate ctx table (enemy_id, position, spawn, hp, detection_radius, etc.)
   ├── 4. Call updateAi(ctx) via sol::protected_function
   ├── 5. On Valid Result:
   │      - Extract next_state, target_id, move_x, move_y, request_attack, attack_id
   │      - Validate and normalize movement vector
   │      - Execute combatSys.ProcessAttack(this, atkId, m_facing) if requested
   └── 6. On Error or Uninitialized:
          - Fall back immediately to C++ native AI State Machine
```

### 4.2 Deficiencies in AI Invocation
1. **Missing Context Attributes**:
   - `ctx["can_retreat"]` is never populated from `m_config.ai.canRetreat`.
   - `ctx["enemy_type"]` (e.g. `"goblin"` vs `"skeleton"`) is missing.
   - `ctx["delta_time"]` is not supplied.
2. **No Archetype Routing**:
   - `EnemyConfig.ai.behaviorScript` is defined in `EnemyConfig.hpp` and in creature JSONs, but `Enemy::ExecuteAIDecision` hardcodes `lua["wildbound"]["ai"]["update_enemy"]` rather than loading or routing to the enemy's assigned script!
3. **Decision Validation Gaps**:
   - C++ normalizes the movement vector, but does not check whether the requested attack ID is valid for the creature's attack cooldown.
   - Target range is not re-checked before delegating to `combatSys.ProcessAttack`.
4. **AIManager Disconnection**:
   - `AIManager::Update` accepts `scripting::LuaManager* luaMgr`, but casts it to `(void)luaMgr` and does not pass it down to `AIController`.

---

# 5. Safety, Performance & Lifecycle Risks

1. **Denial of Service via Runaway Loops**:
   No instruction budget is enforced. A script with an unintentional `while true do` will lock the application loop.
2. **Error Recovery**:
   `sol::protected_function` is used correctly in `Enemy::ExecuteAIDecision` with a try/catch block, properly catching Lua errors and routing to C++ fallback without crashing.
3. **State Contamination**:
   The single shared `sol::state` is accessed sequentially on the main thread during enemy updates. However, context tables are created repeatedly (`lua.create_table()`), causing GC allocations every decision tick.
4. **Memory Leaks**:
   `m_luaState.reset()` cleanly destroys the Lua state and frees all associated Lua memory upon shutdown.

---

# 6. Action Plan & Recommended Corrections

1. **Strengthen `LuaManager`**:
   - Implement script caching and script registry.
   - Implement execution-budget hook (`lua_sethook`) to terminate infinite loops safely.
   - Add performance profiling: execution timer, memory usage reporting, and error tracking.
   - Add development script hot reload.
2. **Expand sol2 C++ Bindings**:
   - Register math helpers, direction conversions, AI state names, and combat action queries.
3. **Modularize Creature Scripts**:
   - Create `scripts/gameplay/ai/goblin.lua` (cowardly, hit-and-run, retreats when low HP).
   - Create `scripts/gameplay/ai/skeleton.lua` (relentless, aggressive, never retreats).
   - Wire the 8 disconnected behavior modules into a clean require structure.
4. **Strengthen C++ Enemy AI Invocation**:
   - Route execution to the creature's assigned `behaviorScript`.
   - Supply full context (`can_retreat`, `enemy_type`, `delta_time`).
   - Validate attack cooldown and range before executing combat actions.
5. **Add Script Diagnostics Tab to DevUI**:
   - Display runtime status, loaded scripts, active enemy script execution times, memory usage, and fallback activation counts.
6. **Implement Comprehensive Test Suite**:
   - Add tests for Lua runtime lifecycle, sol2 bindings, script safety/budget abort, Goblin AI, Skeleton AI, multi-enemy independence, and CTest integration.

