# Wildbound — Cross-Platform Multiplayer Compatibility Analysis

## 1. Objective & Platform Scope

Wildbound is engineered for a mixed platform operational environment:
- **Client Application**: Primary development and player target on Windows 10/11 (MSVC C++20, raylib, DirectX/OpenGL).
- **Dedicated Game Server**: Headless Windows 10/11 for local development; Linux (Ubuntu/Debian x86_64 GCC 12+ / Clang 16+, headless, containerized via Docker) for production fleet deployment.
- **Backend Services**: Linux containerized Go binary and PostgreSQL.

This document verifies the technical compatibility guarantees ensuring seamless multiplayer interoperability between Windows clients and Linux dedicated servers.

---

## 2. Binary Wire Protocol Compatibility

### 2.1 Endianness & Byte Order
- All numeric primitives transmitted over the wire (packet headers, entity IDs, positions, ticks, angles) use standard little-endian encoding via `ByteStreamWriter` and `ByteStreamReader`.
- Since both Windows x86_64 (MSVC) and Linux x86_64 (GCC/Clang) share identical little-endian memory architectures, no byte-swapping overhead is incurred during serialization.

### 2.2 Struct Packing & Alignment Independence
- **Zero Raw C-Struct Transmissions**: Wildbound forbids transmitting raw C++ struct memory over sockets (`send(&struct, sizeof(struct))`), which would create padding divergences between MSVC and GCC.
- **Explicit Field-by-Field Serialization**: Every network message (`MsgClientHello`, `MsgConnectionAccepted`, `MsgPlayerInput`, `WorldSnapshot`) defines explicit `.Serialize(writer)` and `.Deserialize(reader)` routines:
  ```cpp
  writer.WriteUInt32(inputSequence);
  writer.WriteFloat(movementDirectionX);
  writer.WriteFloat(movementDirectionY);
  writer.WriteFloat(facingAngle);
  writer.WriteUInt8(actionFlags);
  ```
- This guarantees bit-level identical wire payloads regardless of compiler alignment flags.

---

## 3. Determinism & Floating-Point Behavior

### 3.1 Server-Authoritative Simulation Architecture
- Wildbound does **not** rely on lockstep deterministic physics across client machines.
- The dedicated server is 100% authoritative for all player movement validation, collision bounds, projectile hits, and storm damage.
- Clients merely send intent inputs ($\vec{d}, \theta, \text{actions}$) and receive server-authoritative positional snapshots.
- Sub-epsilon floating-point rounding discrepancies between MSVC CRT and Linux glibc `libm` have zero impact on gameplay synchronization because the server's output is absolute truth.

### 3.2 Protocol Constants Validation
The following protocol constants are enforced identically on both platforms:
- `PROTOCOL_MAGIC`: `0x57424E44` ("WBND")
- `PROTOCOL_VERSION`: `1`
- `PACKET_HEADER_SIZE`: `16` bytes
- `SERVER_CLIENT_ID`: `0`
- `TICK_RATE`: `30` Hz ($33.33\text{ ms}$)

---

## 4. Platform-Specific Socket & Transport Abstractions

| System Feature | Windows (MSVC) | Linux (GCC / Clang) |
|---|---|---|
| Socket API | Winsock 2 (`ws2_32.lib`) | POSIX BSD Sockets (`sys/socket.h`, `netinet/in.h`) |
| Threading & Clocks | Win32 Threads / `std::thread`, `std::chrono::steady_clock` | POSIX pthreads (`-lpthread`), `clock_gettime(CLOCK_MONOTONIC)` |
| Signals & Shutdown | `SetConsoleCtrlHandler` (Ctrl+C, Close, Shutdown) | `sigaction` (`SIGINT`, `SIGTERM`, `SIGHUP`) |
| High-Resolution Timers | `timeBeginPeriod(1)` via `winmm.lib` | Native Linux nanosecond scheduler resolution |
| Dynamic Libraries | `.dll` (sol2/lua-5.4.7) | `.so` shared objects |

All platform differences are encapsulated cleanly within `ENetTransport`, `PlatformSignalHandler`, and CMake platform definitions (`if(WIN32) ... elseif(UNIX) ...`).

