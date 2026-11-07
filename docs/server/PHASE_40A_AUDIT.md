# WILDBOUND — PHASE 40A AUDIT REPORT
## Cross-Platform Server Compatibility & Linux Readiness Audit

**Audit Date:** 2026-11-07  
**Primary Platform:** Windows 11 x64 (MSVC 19.4x, Windows SDK 10.0.26100.0)  
**Target Cross-Platform Environment:** Linux x86_64 (GCC 11+, Clang 14+)  
**Status:** COMPLETE — READY FOR PLATFORM ABSTRACTION & DUAL TARGET SUPPORT

---

### 1. Architectural Objectives & Constraints

Phase 40A transitions the `WildboundServer` dedicated server codebase into a cross-platform architecture while preserving Windows as the primary developer workstation environment.

1. **Dual-Target Codebase**: Server source code must compile cleanly under both MSVC on Windows and GCC/Clang on Linux.
2. **Zero Functional Regression on Windows**: The graphical client (`wildbound`), dedicated server (`WildboundServer`), and all automated test suites must continue compiling and passing on Windows.
3. **No External Stack Alterations**: Third-party dependencies (raylib 5.5, Lua 5.4.7, sol2 3.3.1, ImGui 1.93.0, nlohmann/json 3.11.3, ENet 1.3.18) remain pinned. No container orchestration (Docker/K8s) or database layers are introduced.
4. **Isolate OS Invasions**: Win32 headers (`<windows.h>`, `SetConsoleCtrlHandler`) must be eradicated from game server logic and encapsulated behind an abstract platform interface (`IPlatform`).

---

### 2. Comprehensive Codebase Platform Audit

#### 2.1 Operating System Headers and API Calls
An exhaustive search across `src/`, `include/`, and `tests/` was conducted for platform-specific headers and Win32 APIs:

| Search Target | Occurrences Found | Location | Impact / Resolution |
|---|---|---|---|
| `<windows.h>` | 1 file | `src/server/DedicatedServer.cpp` | **High**. Used for `SetConsoleCtrlHandler` and console shutdown events. Must be moved to `src/platform/windows/WindowsConsole.cpp`. |
| `SetConsoleCtrlHandler` | 1 file | `src/server/DedicatedServer.cpp` | Encapsulate within `src/platform/windows/WindowsConsole.cpp`; mirror with POSIX `sigaction` (`SIGINT`, `SIGTERM`, `SIGHUP`) in `src/platform/linux/LinuxSignals.cpp`. |
| `<winsock2.h>` | 0 direct calls | Only via ENet | None. Handled internally by ENet's cross-platform transport layer. |
| `Sleep()` | 0 | None | Codebase universally uses `std::this_thread::sleep_for()` or `std::chrono`. |
| `GetTickCount()` / `QueryPerformanceCounter()` | 0 | None | Codebase universally uses `std::chrono::steady_clock` and `std::chrono::duration`. |
| `localtime_s` vs `localtime_r` | 2 files | `src/core/Logger.cpp`, `src/cosmetics/CosmeticsOwnershipManager.cpp` | **Clean**. Already portably conditionalized with `#if defined(_WIN32) localtime_s ... #else localtime_r ... #endif`. |
| `_WIN32` Preprocessor Directives | 4 locations | `src/core/Logger.cpp`, `src/network/ENetTransport.hpp`, `src/cosmetics/CosmeticsOwnershipManager.cpp`, `src/server/DedicatedServer.cpp` | All are appropriate header guards or safely abstracted. |

#### 2.2 Filesystem, Path Separators & Case Sensitivity Audit
Linux ext4 filesystems enforce strict case-sensitivity and forward slash separators (`/`). Windows NTFS is case-preserving but insensitive:

1. **Path Separators**:
   - All path definitions across `src/server/ServerConfig.cpp`, `src/world/WorldManager.cpp`, `src/scripting/LuaManager.cpp`, `src/items/ItemManager.cpp`, and `src/core/EngineConfig.cpp` use forward slashes (`/`).
   - Example: `"config/server.json"`, `"maps/world/battle_royale_island.json"`, `"scripts/main.lua"`.
   - File manipulation utilizes standard `std::filesystem::path`, which transparently formats path delimiters per operating system.
2. **Directory & File Casing**:
   - Audited all directory paths on disk:
     - `assets/` (lowercase)
     - `config/` (lowercase)
     - `data/` (lowercase)
     - `docs/` (lowercase)
     - `maps/` (lowercase)
     - `scripts/` (lowercase)
     - `src/` (lowercase)
     - `tests/` (lowercase)
   - Header include directives inside C++ sources match the exact case of filenames on disk (`#include "ServerConfig.hpp"`, `#include "../core/Logger.hpp"`).
   - No case mismatches detected.

#### 2.3 Dependency Compatibility Audit

| Dependency | Upstream Linux Support | Linux Build Requirements |
|---|---|---|
| **ENet 1.3.18** | Native Linux support (`unix.c`, BSD sockets) | Links standard libc socket APIs; does not require `ws2_32` or `winmm`. Requires `-pthread`. |
| **Lua 5.4.7** | Native POSIX / Linux C library | Compiles cleanly with GCC/Clang via FetchContent. |
| **sol2 3.3.1** | Header-only C++17/20 library | Requires standard C++20 compiler support (`<concepts>`, `<variant>`). |
| **nlohmann/json 3.11.3** | Header-only C++ | 100% portable across all standard C++ platforms. |
| **raylib 5.5** | Linux DRM/KMS, X11, Wayland | Dedicated server runs headlessly; client uses raylib. Dedicated server does not initialize display or audio devices. |
| **Dear ImGui 1.93.0** | Cross-platform core | Headless server bypasses ImGui rendering contexts. |

---

### 3. CMake & Compiler Flag Strategy

1. **MSVC (Windows)**:
   - Existing flags: `/W4`, `/EHsc`, `/utf-8`, `/permissive-`, `/Zc:__cplusplus`, `/Z7`.
   - Remained strictly scoped to `if(MSVC)`.
2. **GCC / Clang (Linux)**:
   - Required compile options:
     - `-std=c++20`
     - `-Wall -Wextra -Wpedantic`
     - `-pthread`
     - `-fexceptions`
   - Platform definitions: `-D_GNU_SOURCE`.
3. **Platform Source Partitioning**:
   - `src/platform/Platform.hpp` & `src/platform/Platform.cpp`: Common interface and factory.
   - `src/platform/windows/`: Added to server target when `if(WIN32)`.
   - `src/platform/linux/`: Added to server target when `elseif(UNIX AND NOT APPLE)`.

---

### 4. Audit Verdict

Wildbound's dedicated server architecture is in an outstanding state of modularity. Because Phase 39 strictly enforced headless execution (no raylib window or audio initialization in `DedicatedServer`), the only direct Windows platform coupling is the console control signal handler. 

Encapsulating this in a clean `IPlatform` layer completely decouples the server logic, enabling immediate Linux build readiness while keeping Windows development completely intact.

