# Locked Technology Stack

| Component | Technology | Version | Purpose / Rationale |
|---|---|---|---|
| **Primary Language** | C++20 | ISO C++20 (`/std:c++20`) | Modern C++ features, concepts, structured binding, `std::filesystem`, strong typing. |
| **Compiler** | MSVC | 19.51+ (VS 2026/2022) | Target Windows 10/11 platform with conforming C++20 support. |
| **Build System** | CMake | 3.20+ (Tested on 4.3.2) | Standard cross-platform build orchestration with `CMakePresets.json`. |
| **Graphics Framework** | raylib | 5.5 | Lightweight, OpenGL-based 2D/3D graphics, window management, audio. |
| **Scripting Language** | Lua | 5.4.7 | Embedded scripting for config, gameplay logic, quests, and data definitions. |
| **Lua-C++ Binding** | sol2 | v3.3.1 | High-performance, header-only type-safe C++ bindings for Lua 5.4. |
| **Developer Interface** | Dear ImGui | v1.93.0 | Immediate-mode UI for internal debugging, telemetry, and dev controls. |
| **ImGui Integration** | rlImGui | Raylib_5_5 | Bridge connecting Dear ImGui to raylib texture and input backends. |
| **Testing** | CTest | CMake CTest | Automated headless unit and integration testing. |
| **Version Control** | Git | 2.x | Source tracking and branch management. |
| **Map Editor** | Tiled Map Editor | TMX / JSON | Standard 2D tilemap authoring tool. |

