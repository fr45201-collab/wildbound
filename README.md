# Wildbound

> **Wildbound** is a 2D top-down fantasy survival adventure game built with C++20, raylib, Lua 5.4, and sol2.

## Current Status: Phase 3 — Tilemap System & World Environment Foundation

### Key Features
- **2D Tilemap Subsystem**: Robust grid architecture (32×32 pixels per tile) with layer management (`Ground`, `Terrain`, `Water`, `Ground Decorations`, `Environmental Objects`, `Collision`).
- **Tiled Map Editor Integration**: Native loader supporting both Tiled JSON (`.json`/`.tmj`) and TMX XML (`.tmx`) with CSV tile decompression.
- **Camera Visibility Culling**: Frustum bounds calculation and visible coordinate clamping reducing per-frame tile draw calls by >94%.
- **Environmental Object Depth (Y-Sorting)**: Dynamic entity z-ordering anchored at bottom-center entity baselines for visual depth.
- **Spatial Collision Cache**: Pre-indexed collision boundaries for solid terrain, water hazards, and map limits with intersection query APIs.
- **Automated Map Validation (`MapValidator`)**: Diagnostics auditing map dimensions, tile sizes, missing tilesets, out-of-range GIDs, and object collision metadata.
- **Playable Forest Environment (`forest_01 — The Whispering Woods`)**: 80×60 tile world (2560×1920 pixels) featuring dirt pathways, cobblestone courtyards, winding river with a wooden crossing bridge, woodland pond, pine/oak tree groves, mossy granite boulders, berry bushes, and an abandoned forest cabin.
- **Interactive DevUI World Debugger (`F1`)**: Real-time toggles for 32×32 tile grid, collision bounds, world boundaries, object origins, per-layer visibility/opacity sliders, culling performance stats, and a live cursor tile inspector.
- **Automated Verification**: Comprehensive 6-target CTest suite covering core config, Lua runtime, viewport/camera math, asset metadata, world/tilemap architecture, and executable smoke testing.

## Technology Stack
- **Language**: C++20 (MSVC `/std:c++20`)
- **Graphics & Audio**: raylib 5.5
- **Scripting**: Lua 5.4.7 + sol2 v3.3.1
- **Developer UI**: Dear ImGui v1.93.0 + rlImGui
- **Map Editor**: Tiled Map Editor 1.10+
- **JSON Parsing**: nlohmann/json v3.11.3
- **Build System**: CMake 3.20+ with FetchContent
- **Testing**: CTest

## Quick Start
```powershell
# Configure & fetch dependencies
cmake -B build -S .

# Compile (Debug)
cmake --build build --config Debug

# Run automated tests (100% pass)
ctest --test-dir build -C Debug --output-on-failure

# Launch Wildbound
.\build\Debug\wildbound.exe
```

## Documentation
- [World Architecture](docs/WORLD_ARCHITECTURE.md)
- [Tilemap System](docs/TILEMAP_SYSTEM.md)
- [Tiled Map Editor Integration](docs/TILED_INTEGRATION.md)
- [Environmental Objects](docs/ENVIRONMENTAL_OBJECTS.md)
- [Collision Metadata](docs/COLLISION_METADATA.md)
- [Map Format Specification](docs/MAP_FORMAT.md)
- [Architecture Overview](docs/ARCHITECTURE.md)
- [Rendering Architecture](docs/RENDERING_ARCHITECTURE.md)
- [Camera System](docs/CAMERA_SYSTEM.md)
- [Coordinate Systems](docs/COORDINATE_SYSTEMS.md)
- [Asset Management](docs/ASSET_MANAGEMENT.md)
- [Spritesheet Format](docs/SPRITESHEET_FORMAT.md)
- [Asset Licensing](docs/ASSET_LICENSING.md)
- [Tech Stack](docs/TECH_STACK.md)
- [Coding Standards](docs/CODING_STANDARDS.md)
- [Build Guide](docs/BUILD_GUIDE.md)
- [Performance Guidelines](docs/PERFORMANCE_GUIDELINES.md)
