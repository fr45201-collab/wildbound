# Wildbound

> **Wildbound** is a 2D top-down fantasy survival adventure game built with C++20, raylib, Lua 5.4, and sol2.

## Current Status: Phase 4 — Player Character System & Animation Engine

### Key Features
- **Player & Character Entity Architecture**: Clean C++20 entity hierarchy (`Entity` -> `Character` -> `Player`) decoupling stats, collision configuration, animation controllers, and input polling.
- **2D Animation Subsystem**: Framerate-independent animation engine supporting 4-directional spritesheets (`Down`, `Up`, `Left`, `Right`) across `Idle`, `Walk`, and `Run` states (12 animation states) with directional tie-breaker bias and automatic run-to-walk fallback at 1.65× speed.
- **Locomotion Mechanics**: Responsive 4-directional movement with WASD and arrow keys, Shift sprint modifier (100 px/s walk to 165 px/s run), and strict diagonal velocity normalization eliminating diagonal speedup.
- **Swept AABB Collision & Wall Sliding**: Axis-decoupled collision detection and pushout resolution against static tilemap obstacles, water bodies, and environmental objects (trees, rocks, fences, buildings) with feet-anchored hitboxes enabling visual depth overlap.
- **Map Spawn Integration**: Automatic player positioning based on Tiled map `player_spawn` markers with safe world boundary fallbacks.
- **Camera Follow Tracking**: Smooth exponential spring follow camera (`CameraMode::Follow`) keeping the player centered with map boundary clamping and integer sub-pixel snapping to avoid pixel-art shimmering.
- **Y-Sorted World Interleaving**: Dynamic rendering pass interleaving player sprites into the environmental object render queue based on feet Y baseline, enabling natural front/behind depth sorting against trees and buildings.
- **Interactive DevUI & Diagnostics (`F1`-`F5`, `R`)**: Real-time controls for player speed, manual input toggles, collision bounds overlay (`F3`), player ground anchor marker (`F4`), live animation HUD overlay (`F5`), and quick teleport reset (`R`).
- **Comprehensive Automated Test Suite**: 7 CTest suites (including 15 dedicated Player, Character, and Animation unit tests in `test_player`) achieving 100% pass rates across both Debug and Release configurations.

## Technology Stack
- **Language**: C++20 (MSVC `/std:c++20`)
- **Graphics & Audio**: raylib 5.5
- **Scripting**: Lua 5.4.7 + sol2 v3.3.1
- **Developer UI**: Dear ImGui v1.93.0 + rlImGui
- **Map Editor**: Tiled Map Editor 1.10+
- **JSON Parsing**: nlohmann/json v3.11.3
- **Build System**: CMake 3.20+ with FetchContent
- **Testing**: CTest (7 test suites, 100% pass)

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
- [Player Character System](docs/PLAYER_SYSTEM.md)
- [Animation Subsystem](docs/ANIMATION_SYSTEM.md)
- [Character Controller](docs/CHARACTER_CONTROLLER.md)
- [Collision & Wall Sliding](docs/COLLISION_SYSTEM.md)
- [Player Input & Key Bindings](docs/PLAYER_INPUT.md)
- [Camera Follow Tracking](docs/CAMERA_FOLLOW.md)
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
