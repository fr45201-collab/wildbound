# Wildbound — Phase 46: Developer UI & Scripting Diagnostics

## 1. Overview

In Phase 46, a dedicated diagnostics and management interface was added to the Developer UI (`src/ui/DevUI.cpp`), accessible via F1/F2 overlay under Tab 22 ("Lua & Scripting").

---

## 2. Telemetry & Metric Displays

The panel provides live readouts for:

1. **Lua Engine Status**:
   - Status indicator (Initialized / Uninitialized).
   - Lua Version: Lua 5.4.7 via sol2.
   - Live Memory Usage: Real-time KB gauge queried via `lua_gc()`.
2. **Instruction Budget Controls**:
   - Instruction limit slider ($1,000 \dots 200,000$ instructions).
   - Enable / Disable toggle for budget enforcement.
   - Budget Violation Counter: Tracks intercepted runaway scripts.
3. **Execution Telemetry**:
   - Total Lua AI Decisions evaluated since boot (`s_luaDecisionCount`).
   - Total Fallback AI executions triggered (`s_fallbackAICount`).
   - Fallback Rate percentage.

---

## 3. Interactive Script Table & Hot Reload Controls

A live table displays all registered scripts:
- **Path**: Relative path in `scripts/`.
- **Status**: Loaded / Error.
- **Execution Count**: Number of times loaded or invoked.
- **Duration**: Milliseconds elapsed during last execution.
- **Actions**:
  - `[Reload]`: Triggers an immediate reload of that individual file from disk.
  - `[Reload All Scripts]`: Refreshes all registered scripts at once.

---

## 4. Console Logging Integration

Lua script errors, syntax failures, and `wildbound.log.*` statements appear directly in the Developer UI console window with color coding:
- **Green / Info**: `[INFO] [SCRIPTING]`
- **Yellow / Warning**: `[WARNING] [SCRIPTING]`
- **Red / Error**: `[ERROR] [SCRIPTING]` including detailed Lua stack tracebacks.

