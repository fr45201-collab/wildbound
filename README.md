# Wildbound

> **Wildbound** is a 2D top-down fantasy survival adventure game built with C++20, raylib, and Lua.

## Current Status: Phase 1 — Rendering Engine, Game Loop & Camera Foundation

### Key Features
- **Modern C++20 Core**: Conforming MSVC architecture with strict modularity.
- **Controlled Timestep Game Loop**: Fixed simulation timestep with smooth delta-time interpolation.
- **Pixel-Art Virtual Canvas**: Native 640x360 internal canvas scaled (2x) to 1280x720 with letterboxing and point texture filtering.
- **2D Camera System**: Raylib `Camera2D` abstraction featuring target following, smoothing, zoom clamping, and world boundary enforcement.
- **Coordinate Space Conversion**: Bi-directional conversions between World, Screen, and Virtual coordinate systems.
- **Scripting Integration**: Embedded Lua 5.4 engine via `sol2` for data-driven configuration.
- **Developer Overlay**: Integrated Dear ImGui via `rlImGui` for real-time diagnostics (`F1`).
- **Reproducible Build System**: CMake FetchContent with pinned dependencies and CTest automated testing.

## Technology Stack
- **Language**: C++20
- **Graphics**: raylib 5.5
- **Scripting**: Lua 5.4.7 + sol2 v3.3.1
- **Dev UI**: Dear ImGui v1.93.0 + rlImGui
- **Build System**: CMake 3.20+ / MSVC
- **Testing**: CTest

## Quick Start
```powershell
# Configure & fetch dependencies
cmake -B build -S .

# Compile (Debug)
cmake --build build --config Debug

# Run tests
ctest --test-dir build -C Debug --output-on-failure

# Launch Wildbound
.\build\Debug\wildbound.exe
```

## Documentation
- [Architecture](docs/ARCHITECTURE.md)
- [Tech Stack](docs/TECH_STACK.md)
- [Coding Standards](docs/CODING_STANDARDS.md)
- [Build Guide](docs/BUILD_GUIDE.md)
- [Asset Pipeline](docs/ASSET_PIPELINE.md)
- [Rendering Architecture](docs/RENDERING_ARCHITECTURE.md)
- [Camera System](docs/CAMERA_SYSTEM.md)
- [Coordinate Systems](docs/COORDINATE_SYSTEMS.md)
- [Performance Guidelines](docs/PERFORMANCE_GUIDELINES.md)
