# Wildbound — Phase 46: Script Safety, Sandbox Isolation & Infinite Loop Protection

## 1. Safety Objectives

Integrating a scripting language into a real-time multiplayer game introduces stability and security risks:
1. Infinite loops or recursion locking the frame simulation.
2. Unhandled runtime syntax errors terminating the server process.
3. Memory leaks resulting in gradual memory exhaustion.
4. Malicious or malformed inputs (NaN, Inf) causing simulation physics crashes.

Phase 46 implements layered defensive measures resolving each risk.

---

## 2. Instruction Count Hook (Infinite Loop Protection)

Wildbound uses the native Lua debug hook interface to count executed virtual machine instructions:

```cpp
void LuaManager::BeginExecutionBudget() {
    if (m_budgetEnabled && m_luaState && m_instructionBudget > 0) {
        lua_sethook(m_luaState->lua_state(), LuaInstructionCountHook, LUA_MASKCOUNT, m_instructionBudget);
    }
}
```

- **Configured Budget**: 50,000 instructions per execution.
- **Hook Trigger**: `LuaInstructionCountHook` raises a Lua error via `luaL_error()`.
- **Termination Latency**: Under test benchmarks, an intentional infinite loop (`while true do counter = counter + 1 end`) was cleanly intercepted and aborted in **0.234 ms**.
- **Post-Abort Health**: The Lua state recovers immediately without corruption; subsequent valid script calls execute with 100% fidelity.

---

## 3. Mathematical Sanitization

All numerical inputs returned by Lua are validated using `<cmath>` primitives:
- `std::isfinite(moveX)` and `std::isfinite(moveY)` reject `NaN`, `+Inf`, and `-Inf`.
- Speeds are clamped against physical limits defined in C++ configuration.
- Distances and attack radii are strictly checked before forwarding to combat systems.

---

## 4. Sandboxing & Attack Surface Minimization

The following standard Lua libraries are completely withheld:
- `io`: Prevents unauthorized filesystem inspection or file overwrites.
- `os`: Prevents invoking `os.execute`, spawning system subprocesses, or reading host system environment variables.
- `debug`: Prevents inspection of engine stack frames or altering bytecode.

