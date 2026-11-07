# Wildbound — Phase 46: Lua Runtime Architecture & Lifecycle

## 1. Executive Summary

Wildbound employs **Lua 5.4.7** embedded into the C++20 engine using the **sol2** binding library (single-header v3 release). The runtime is encapsulated inside the `wildbound::scripting::LuaManager` subsystem, responsible for deterministic initialization, sandboxed execution, script metadata caching, execution budget enforcement, and orderly shutdown.

The scripting subsystem is architected according to the following design requirements:
1. **Headless & Client Parity**: The exact same `LuaManager` instance and scripting pipeline operate identically in both the graphical game client (`wildbound.exe`) and the headless dedicated server (`WildboundServer.exe`).
2. **C++ Simulation Authority**: Lua scripts operate strictly as stateless or softly stateful decision agents. Lua never directly mutates world physics, entity health, or inventory tables; it returns decision records that C++ strictly validates and executes.
3. **Protected Execution & Resiliency**: All script executions occur under `sol::protected_function` and instruction count hooks. Any runtime or syntax error in a script is trapped, logged, and isolated without destabilizing the server or client frame loop.

---

## 2. Runtime Initialization Sequence

The initialization sequence occurs via `LuaManager::Initialize()`:

```
+-----------------------------------------------------------+
|              LuaManager::Initialize()                     |
+-----------------------------------------------------------+
                             |
                             v
   [1. Instantiate sol::state (wraps lua_State*)]
                             |
                             v
   [2. Open standard libraries: base, package, math, string, table]
                             |
                             v
   [3. Configure package.path for modular require()]
       - scripts/?.lua
       - scripts/?/init.lua
       - scripts/gameplay/ai/?.lua
       - scripts/gameplay/ai/behaviors/?.lua
                             |
                             v
   [4. Register Core Bindings (wildbound.*, Vector2, math, logger)]
                             |
                             v
   [5. Configure Execution Budget Defaults (50,000 instructions)]
                             |
                             v
   [6. Set m_initialized = true & Log Readiness]
```

### 2.1 Library Sandboxing
Only safe, necessary standard libraries are exposed to Lua scripts:
- `base`: `print`, `type`, `tostring`, `tonumber`, `error`, `pairs`, `ipairs`, `assert`, `pcall`, `xpcall`
- `package`: Required for `require()` to import modular behaviors
- `math`: Standard mathematical operations (`sin`, `cos`, `sqrt`, `min`, `max`, `random`)
- `string`: String manipulation (`format`, `sub`, `len`, `find`)
- `table`: Table utilities (`insert`, `remove`, `sort`, `concat`)

Dangerous libraries (`io`, `os`, `debug`) are deliberately omitted to eliminate file-system access, process execution, or arbitrary memory inspection from user-authored or external scripts.

---

## 3. Script Search Paths (`package.path`)

To allow scripts like `goblin.lua` or `enemy_base.lua` to modularly require sub-modules (e.g. `local chase = require("chase")`), `LuaManager` prepends relative directory roots:

```lua
scripts/?.lua;
scripts/?/init.lua;
scripts/gameplay/ai/?.lua;
scripts/gameplay/ai/behaviors/?.lua;
../scripts/?.lua;
../scripts/gameplay/ai/?.lua;
?.lua
```

This guarantees that both in-development directory structures (running from build directories) and distributed root directory installations find behaviors seamlessly without hardcoded paths.

---

## 4. Protected Execution & Instruction Hook

Each script execution or function call is bracketed by an execution budget:

```cpp
void LuaManager::BeginExecutionBudget() {
    if (m_budgetEnabled && m_luaState && m_instructionBudget > 0) {
        lua_sethook(m_luaState->lua_state(), LuaInstructionCountHook, LUA_MASKCOUNT, m_instructionBudget);
    }
}

void LuaManager::EndExecutionBudget() {
    if (m_luaState) {
        lua_sethook(m_luaState->lua_state(), nullptr, 0, 0);
    }
}
```

If an infinite loop or runaway calculation executes more instructions than `m_instructionBudget` (default: 50,000 instructions), the hook triggers:

```cpp
static void LuaInstructionCountHook(lua_State* L, lua_Debug* /*ar*/) {
    lua_sethook(L, nullptr, 0, 0);
    luaL_error(L, "Script exceeded instruction execution budget (infinite loop protection)");
}
```

The error is caught by sol2's protected function handlers, incrementing `m_budgetViolations`, logging the incident, and returning control to C++ within < 0.3 milliseconds without locking the frame thread.

---

## 5. Memory Management & Tracking

Lua 5.4 tracks all allocations internally through its garbage collector. `LuaManager` surfaces real-time memory telemetry:
- `GetMemoryUsageKb()`: Queries `lua_gc(L, LUA_GCCOUNT, 0)` to inspect live heap consumption.
- Typical baseline memory: ~120 KB after loading all core modules and creature scripts.
- Scaled 50-enemy simulation memory: ~210 KB.

---

## 6. Shutdown Lifecycle & RAII Destruction Order

Orderly shutdown is implemented in `LuaManager::Shutdown()`:
1. `EndExecutionBudget()`: Disables all active debug hooks.
2. `m_scripts.clear()`: Clears the metadata registry.
3. `m_luaState.reset()`: Invokes `lua_close()`, safely releasing all allocated Lua objects.
4. `m_initialized = false`: Resets state flags.

### Critical sol2 Object Lifetime Invariant
Any C++ `sol::table`, `sol::function`, `sol::protected_function`, or `sol::object` stores a raw pointer to `lua_State*`. Therefore, C++ callers must ensure that all sol wrapper variables are destructed or cleared **before** `LuaManager::Shutdown()` or `LuaManager::~LuaManager()` executes. Adherence to this RAII guideline prevents dereferencing freed Lua states.

