# WILDBOUND — ENET DEPENDENCY INTEGRATION MANUAL

## 1. Executive Overview

This document specifies the technical integration of the **ENet v1.3.18** UDP transport library into the Wildbound C++20 engine build system and runtime architecture. ENet serves as the underlying low-level transport provider for all client-server communication in Wildbound, offering sequenced unreliable delivery and reliable ordered messaging across independent channels.

---

## 2. Dependency Specification

* **Library**: ENet (Reliable UDP networking library)
* **Target Version**: `v1.3.18` (Git commit / tag `v1.3.18`)
* **Repository**: `https://github.com/lsalzman/enet.git`
* **License**: MIT License (Permissive open source)
* **Build System**: CMake via `FetchContent`
* **Platform Sockets**:
  * Windows: Winsock 2 (`ws2_32.lib`, `winmm.lib`)
  * POSIX (Linux / macOS): Standard BSD sockets

---

## 3. CMake Integration Details

ENet is integrated directly into the root CMake build (`CMakeLists.txt`) using modern CMake `FetchContent`:

```cmake
# ============================================================================
# ENET DEPENDENCY (FetchContent)
# ============================================================================
include(FetchContent)
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.18
)
FetchContent_MakeAvailable(enet)
```

### Windows Socket Linkage
Under Windows, the standard socket library (`ws2_32.lib`) and high-resolution multimedia timer library (`winmm.lib`) must be linked to both the main game executable and all test runners:

```cmake
if(WIN32)
    target_link_libraries(wildbound PRIVATE ws2_32 winmm)
    target_compile_definitions(wildbound PRIVATE NOGDI WIN32_LEAN_AND_MEAN)
endif()
```

### Header Decoupling & Symbol Collision Mitigation
Windows header files (`windows.h`, `winuser.h`, `wingdi.h`) declare symbols that conflict with Raylib (notably `Rectangle`, `CloseWindow`, and `ShowCursor`). Wildbound eliminates these collisions through two architectural guardrails:
1. `NOGDI` and `WIN32_LEAN_AND_MEAN` are passed as compile definitions across the project.
2. `NetworkManager.hpp` employs a forward-declared pImpl model for `ENetServer` and `ENetClient`, preventing any inclusion of `<enet/enet.h>` or `<winsock2.h>` in game headers or Raylib translation units.

---

## 4. Verification and Binary Artifacts

* Static Library: `build/_deps/enet-build/Debug/enet.lib` (and Release equivalent)
* Header Directory: `build/_deps/enet-src/include`
* Linked Targets:
  * `wildbound.exe`
  * `test_network_transport.exe` (Test 37)
  * `test_network_architecture.exe` (Test 36)

