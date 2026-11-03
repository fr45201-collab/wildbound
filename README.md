# Wildbound

> **Wildbound** is a 2D top-down fantasy survival adventure game built with C++20, raylib, Lua 5.4, and sol2.

## Current Status: Phase 2 — Asset Management System & Graphics Pipeline

### Key Features
- **Asset Management System (AMS)**: Centralized resource registry with manifest serialization (`asset_manifest.json`), validation, and provenance tracking.
- **Resource Caching & Deduplication**: RAII `TextureManager`, `FontManager`, and `AudioManager` preventing redundant disk I/O and GPU allocations.
- **Spritesheet & Animation Foundation**: Grid-based and variable-layout frame calculators with sequential frame interpolation and looping.
- **Pixel-Art Graphics Pipeline**: 640x360 virtual canvas, letterboxing viewport, and strict `TEXTURE_FILTER_POINT` nearest-neighbor filtering.
- **Developer Asset Browser & Diagnostics (`F1`)**: Interactive Dear ImGui panel with category filtering, search, metadata inspection, texture previews, and spritesheet frame scrubber.
- **Automated Verification**: Multi-target CTest suite covering core config, Lua runtime, viewport/camera math, and asset metadata.

## Technology Stack
- **Language**: C++20 (MSVC `/std:c++20`)
- **Graphics & Audio**: raylib 5.5
- **Scripting**: Lua 5.4.7 + sol2 v3.3.1
- **Developer UI**: Dear ImGui v1.93.0 + rlImGui
- **JSON Parsing**: nlohmann/json v3.11.3
- **Build System**: CMake 3.20+ with FetchContent
- **Testing**: CTest

## Quick Start
```powershell
# Configure & fetch dependencies
cmake -B build -S .

# Compile (Debug)
cmake --build build --config Debug

# Run automated tests
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
- [Asset Management](docs/ASSET_MANAGEMENT.md)
- [Spritesheet Format](docs/SPRITESHEET_FORMAT.md)
- [Asset Licensing](docs/ASSET_LICENSING.md)
- [Asset Import Pipeline](docs/ASSET_IMPORT_PIPELINE.md)
- [Resource Lifecycle](docs/RESOURCE_LIFECYCLE.md)
