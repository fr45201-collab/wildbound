# Wildbound Engine Architecture

## 1. Architectural Philosophy
Wildbound is designed with a modular, data-driven, and decoupled architecture. The engine separates core system orchestration, rendering, input handling, audio, physics, scripting, and developer debugging interfaces into distinct modules.

```
+-------------------------------------------------------------+
|                         main.cpp                            |
+-------------------------------------------------------------+
                              |
+-------------------------------------------------------------+
|                      Game (Core Engine)                     |
|  - Lifecycle (Initialize, Run, Shutdown)                    |
|  - Controlled Game Loop (Fixed Timestep + Variable Render)  |
|  - Engine Configuration (EngineConfig)                      |
+-------------------------------------------------------------+
          |                    |                      |
          v                    v                      v
+------------------+  +------------------+  +------------------+
|  Scripting       |  |  Graphics        |  |  UI (Developer)  |
|  (LuaManager)    |  |  (RenderManager) |  |  (DevUI)         |
|  - Lua 5.4       |  |  - raylib        |  |  - Dear ImGui    |
|  - sol2 bindings |  |  - Virtual Canvas|  |  - rlImGui       |
|  - config/scripts|  |  - Pixel Filtering| |  - Metrics & F1  |
+------------------+  +------------------+  +------------------+
```

## 2. Core Modules

### A. Core Engine (`src/core/`)
- **`Game`**: Owns engine subsystem lifecycles, manages the main loop, controls delta timing, processes top-level input, and handles graceful shutdown.
- **`EngineConfig`**: Centralized configuration struct holding resolutions, target FPS, v-sync, debug flags, and game metadata.
- **`Logger`**: Thread-safe centralized logger providing subsystem tags and standard severity levels (`DEBUG`, `INFO`, `WARNING`, `ERROR`).

### B. Graphics & Rendering (`src/graphics/`)
- Encapsulates raylib initialization and shutdown.
- Implements the virtual canvas render texture strategy: renders at internal resolution (640x360) and scales to the target window resolution (1280x720, 2x) with point texture filtering.
- Provides aspect-ratio preservation and letterboxing.

### C. Scripting Runtime (`src/scripting/`)
- Wraps Lua 5.4 state through `sol2`.
- Loads configuration scripts (`scripts/config/engine.lua`).
- Exposes safe C++ functions and variables to Lua under the `wildbound` table.
- Employs strict exception handling (`sol::error`) and detailed logging.

### D. Developer Tools & UI (`src/ui/`)
- Employs Dear ImGui integrated via `rlImGui` exclusively for development tooling and real-time engine telemetry.
- Distinct from in-game UI. Toggleable via `F1`.

## 3. Dependency Decoupling & Extension Points
All subsystems communicate through explicit C++ interfaces rather than global state. Future modules (Input, Audio, Physics, Entities, World) plug directly into the `Game` update and render cycles.
