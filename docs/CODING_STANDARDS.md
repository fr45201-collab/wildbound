# Wildbound Coding Standards

## 1. C++ Guidelines (C++20)

### Naming Conventions
- **Classes & Structs**: `PascalCase` (e.g., `RenderManager`, `EngineConfig`)
- **Functions & Methods**: `PascalCase` (e.g., `Initialize()`, `Update()`, `Render()`)
- **Member Variables**: `m_camelCase` (e.g., `m_windowWidth`, `m_initialized`)
- **Static Member Variables**: `s_camelCase` (e.g., `s_logMutex`, `s_minLevel`)
- **Local Variables & Parameters**: `camelCase` (e.g., `targetFps`, `fixedDeltaTime`)
- **Constants & Enums**: `PascalCase` or `UPPER_SNAKE_CASE` (e.g., `LogLevel::Debug`, `MAX_BUFFER_SIZE`)
- **Namespaces**: `lower_snake_case` (e.g., `wildbound::core`, `wildbound::scripting`)

### File Organization
- One class per header/source pair where practical.
- Headers use `#pragma once`.
- Include order:
  1. Associated header (e.g. `Game.hpp` in `Game.cpp`)
  2. Local project headers
  3. Third-party library headers (e.g. `<raylib.h>`, `<sol/sol.hpp>`)
  4. Standard Library headers (e.g. `<memory>`, `<vector>`)

### Error Handling & Safety
- Use RAII for all resource lifecycles (`std::unique_ptr`, `std::shared_ptr`).
- Handle exceptions at system boundaries (e.g., Lua scripts, file I/O).
- Never ignore return values from initialization functions.
- Log failures using `wildbound::core::Logger`.

## 2. Lua Guidelines (Lua 5.4)
- **Tables & Keys**: `snake_case` (e.g., `window_width`, `target_fps`).
- **Global Variables**: Avoid globals; encapsulate in modules or tables.
- Return explicit tables from configuration files.

