# WILDBOUND — PHASE 40A CROSS-PLATFORM SERVER ARCHITECTURE

## Cross-Platform Dedicated Server Compatibility & Abstraction Layer

**Document Version:** 1.0  
**Phase:** 40A  
**Primary Development Workstation:** Windows 11 x64 (MSVC 19.4x / Visual Studio, C++20)  
**Supported Server Deployment Targets:** Windows Server x64, Linux x86_64 (GCC 11+, Clang 14+)  

---

### 1. Executive Summary

Phase 40A prepares `WildboundServer`—Wildbound's standalone headless dedicated server—for deployment in both Windows and Linux production environments.

The architectural guiding principle is **uncompromised dual compatibility**:
- **Windows / MSVC** remains the primary development, debugging, and verification environment. No developer migration to Linux is required.
- **Linux / GCC / Clang** is fully supported as an authoritative server compilation target for cost-effective cloud hosting (AWS EC2, Hetzner, GCP Compute Engine).
- **Zero Third-Party Dependency Drift**: raylib 5.5, Lua 5.4.7, sol2 3.3.1, Dear ImGui 1.93.0, rlImGui, nlohmann/json 3.11.3, and ENet 1.3.18 remain locked at their pinned release versions.
- **Strict Boundary Encapsulation**: Operating system-specific header inclusions (such as `<windows.h>` and `<csignal>`) are isolated inside a dedicated platform layer (`src/platform/`).

```
+-----------------------------------------------------------------------------------+
|                            WILDBOUND DEDICATED SERVER                             |
|       (Headless Simulation: World, Combat, Storm, Items, Replication, Match)      |
+-----------------------------------------------------------------------------------+
                                          |
                                          v
+-----------------------------------------------------------------------------------+
|                        PLATFORM ABSTRACTION LAYER (IPlatform)                     |
|                               src/platform/Platform.hpp                           |
|        - Signal Handling     - Monotonic Clock       - System Memory Telemetry    |
|        - Console Encoding    - Thread Sleep/Yield    - Environment Variables      |
+-----------------------------------------------------------------------------------+
                   |                                                 |
                   v                                                 v
+-------------------------------------+             +-------------------------------------+
|        WINDOWS IMPLEMENTATION       |             |         LINUX IMPLEMENTATION        |
|      (src/platform/windows/)        |             |        (src/platform/linux/)        |
| - WindowsPlatform.cpp               |             | - LinuxPlatform.cpp                 |
| - WindowsConsole.cpp                |             | - LinuxSignals.cpp                  |
| - SetConsoleCtrlHandler             |             | - POSIX sigaction (INT, TERM, HUP)  |
| - GlobalMemoryStatusEx / psapi      |             | - /proc/meminfo, /proc/self/statm   |
| - Windows Sockets (ws2_32, winmm)   |             | - POSIX Sockets (pthread, libm)     |
+-------------------------------------+             +-------------------------------------+
```

---

### 2. Platform Abstraction Architecture (`src/platform/`)

The platform abstraction layer isolates all operating system interactions behind the `IPlatform` interface. Neither `<windows.h>` nor POSIX system headers leak into server gameplay code.

#### 2.1 Interface Definition (`src/platform/Platform.hpp`)
```cpp
namespace wildbound::platform {

enum class PlatformType {
    Windows,
    Linux,
    Unknown
};

struct MemoryMetrics {
    uint64_t totalPhysicalBytes{ 0 };
    uint64_t availablePhysicalBytes{ 0 };
    uint64_t processPhysicalUsedBytes{ 0 };
};

using TerminationSignalHandler = std::function<void(const std::string& reason)>;

class IPlatform {
public:
    virtual ~IPlatform() = default;

    [[nodiscard]] virtual PlatformType GetType() const noexcept = 0;
    [[nodiscard]] virtual const char* GetName() const noexcept = 0;

    virtual bool RegisterTerminationHandler(TerminationSignalHandler handler) = 0;
    virtual void UnregisterTerminationHandler() = 0;
    virtual void ConfigureConsole() = 0;

    [[nodiscard]] virtual MemoryMetrics GetMemoryMetrics() const noexcept = 0;
    [[nodiscard]] virtual double GetMonotonicTimeSeconds() const noexcept = 0;
    virtual void SleepMs(uint32_t ms) = 0;
    [[nodiscard]] virtual std::string GetEnvironmentVariable(const std::string& name) const = 0;
};

std::unique_ptr<IPlatform> CreatePlatform();
IPlatform& GetPlatform();

} // namespace wildbound::platform
```

#### 2.2 Windows Implementation (`src/platform/windows/`)
- **`WindowsConsole.cpp`**: Encapsulates `SetConsoleCtrlHandler` to intercept `CTRL_C_EVENT`, `CTRL_CLOSE_EVENT`, and `CTRL_SHUTDOWN_EVENT`. Configures console input/output codepages to UTF-8 (`SetConsoleOutputCP(CP_UTF8)`) and enables virtual terminal processing.
- **`WindowsPlatform.cpp`**: Queries `GlobalMemoryStatusEx` for host physical memory and `GetProcessMemoryInfo` for resident working set memory. Uses `std::chrono::steady_clock` for high-resolution timing.

#### 2.3 Linux Implementation (`src/platform/linux/`)
- **`LinuxSignals.cpp`**: Installs POSIX `sigaction` signal handlers for `SIGINT` (Ctrl+C), `SIGTERM` (systemd / docker stop), and `SIGHUP` (terminal disconnect). Configures `signal(SIGPIPE, SIG_IGN)` to prevent unexpected process termination during client socket resets.
- **`LinuxPlatform.cpp`**: Parses `/proc/meminfo` (`MemTotal`, `MemAvailable`) and `/proc/self/statm` for resident memory calculation multiplied by `sysconf(_SC_PAGE_SIZE)`.

---

### 3. Dedicated Server Refactoring

In Phase 39, `DedicatedServer.cpp` included `<windows.h>` directly with guard macros to register `SetConsoleCtrlHandler`.

In Phase 40A, `DedicatedServer` was completely decoupled from Win32 APIs:
1. `<windows.h>` was purged entirely from `DedicatedServer.cpp`.
2. `DedicatedServer` instantiates `IPlatform` via `platform::CreatePlatform()` during construction.
3. `SetupConsoleSignalHandling()` delegates directly to `m_platform->RegisterTerminationHandler()`.
4. `Initialize()` queries `m_platform->GetName()` and `m_platform->GetMemoryMetrics()` for startup telemetry.
5. `Shutdown()` unregisters signal handlers cleanly via `m_platform->UnregisterTerminationHandler()`.

---

### 4. Cross-Platform Build System Architecture

#### 4.1 Compiler Options in `CMakeLists.txt`
Compiler options are partitioned to ensure no MSVC-specific flags (`/W4`, `/utf-8`, `/permissive-`, `/Z7`) are applied to GCC or Clang:

```cmake
if(MSVC)
    add_compile_options(
        /W4          # High warning level
        /EHsc        # Enable C++ standard exception handling
        /utf-8       # Set source and execution character sets to UTF-8
        /permissive- # Enforce standards conformance
        /Zc:__cplusplus # Correct __cplusplus macro
        /Z7          # Embed debug info in .obj files
    )
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -pthread
    )
    add_compile_definitions(_GNU_SOURCE)
endif()
```

#### 4.2 Platform Source Partitioning
Platform source files are added conditionally based on the target OS:
```cmake
set(PLATFORM_SOURCES
    src/platform/Platform.hpp
    src/platform/Platform.cpp
)
if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/platform/windows/WindowsPlatform.hpp
        src/platform/windows/WindowsPlatform.cpp
        src/platform/windows/WindowsConsole.hpp
        src/platform/windows/WindowsConsole.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND PLATFORM_SOURCES
        src/platform/linux/LinuxPlatform.hpp
        src/platform/linux/LinuxPlatform.cpp
        src/platform/linux/LinuxSignals.hpp
        src/platform/linux/LinuxSignals.cpp
    )
endif()
```

#### 4.3 Link Libraries
- **Windows**: `WildboundServer` and `enet` link `ws2_32` and `winmm`.
- **Linux**: `WildboundServer` and `enet` link standard POSIX threads and math (`pthread`, `m`).

---

### 5. Multi-Platform CMake Presets (`CMakePresets.json`)

CMake Presets define unified configuration, build, and test profiles for both operating systems:
- Windows:
  - `windows-msvc-debug` / `debug`
  - `windows-msvc-release` / `release`
  - `windows-ninja-debug`
- Linux:
  - `linux-gcc-debug`
  - `linux-gcc-release`
  - `linux-clang-debug`
  - `linux-clang-release`

---

### 6. Filesystem and Path Case Sensitivity Guarantee

- All internal asset, configuration, map, and script file references use forward slashes (`/`), conforming to POSIX standards while natively supported by Windows Win32 APIs and `std::filesystem::path`.
- Path casing in code strictly matches directory and filename casing on disk, ensuring zero file-not-found errors on case-sensitive Linux ext4 filesystems.

