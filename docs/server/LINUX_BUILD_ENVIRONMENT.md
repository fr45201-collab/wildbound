# WILDBOUND LINUX BUILD ENVIRONMENT SPECIFICATION

This document records the exact Linux build host environment, hardware topology, compiler toolchain, and dependency versions used for compiling and executing `WildboundServer`.

---

## 1. Operating System & Host Environment

| Property | Value | Notes |
|---|---|---|
| **Operating System** | Linux (Ubuntu LTS) | Canonical Ubuntu WSL2 container |
| **Distribution Name** | Ubuntu | Canonical reference environment |
| **Distribution Release** | 26.04 LTS (Resolute Raccoon) | Modern Linux libc & systemd baseline |
| **Kernel Version** | `6.6.114.1-microsoft-standard-WSL2` | Preemptible dynamic SMP Linux kernel |
| **Hardware Architecture** | `x86_64` (AMD64) | 64-bit little-endian |
| **Workspace Mount** | `/mnt/c/dev/Wildbound` | NTFS shared filesystem mount |

---

## 2. Hardware Resource Topology

| Resource | Value | Measurement Command |
|---|---|---|
| **Logical CPU Cores** | `8 cores` | `nproc` |
| **Total Physical RAM** | `3,738 MB` (~3.7 GB) | `free -m` (MemTotal) |
| **Available Physical RAM** | `3,323 MB` (~3.3 GB) | `free -m` (MemAvailable) |
| **Configured Swap** | `1,024 MB` (~1.0 GB) | `free -m` (SwapTotal) |

---

## 3. Compiler & Toolchain Specifications

| Tool | Version / Path | Purpose |
|---|---|---|
| **C++ Compiler (GCC)** | `15.2.0` (`/usr/bin/g++`) | Primary C++20 compiler (`-std=c++20`) |
| **C Compiler (GCC)** | `15.2.0` (`/usr/bin/gcc`) | C dependency compilation (Lua 5.4, ENet) |
| **Clang Compiler** | Not Installed (`NONE`) | Reported as NOT RUN |
| **CMake** | `4.2.3` (`/usr/bin/cmake`) | Build configuration & CTest harness |
| **Build Generator** | GNU Make `4.4.1` (`/usr/bin/make`) | Parallel build orchestration (`Unix Makefiles`) |
| **Version Control** | Git `2.43+` (`/usr/bin/git`) | Submodule and dependency management |

---

## 4. Pinned Dependency Versions

| Component | Pinned Version / Tag | Linux Integration Type |
|---|---|---|
| **C++ Language Standard** | ISO C++20 | Native compiler flag (`-std=c++20`) |
| **ENet** | `v1.3.18` | BSD socket transport via `unix.c`, compiled from source |
| **Lua** | `v5.4.7` | Static ANSI C library compiled from source via CMake |
| **sol2** | `v3.3.1` | Header-only C++20 bindings to Lua runtime |
| **nlohmann/json** | `v3.11.3` | Header-only modern C++ JSON parser |
| **raylib** | `5.5` | Bypassed during headless server execution |
| **Platform Abstraction** | Phase 40A Layer | `src/platform/linux/LinuxPlatform.cpp`, `LinuxSignals.cpp` |

