# WILDBOUND DEDICATED SERVER — LINUX BUILD & DEPLOYMENT GUIDE

This guide provides instructions for building, testing, and running `WildboundServer` on Linux systems (Ubuntu 22.04+, Debian 12+, RHEL/Rocky Linux 9+).

Development of Wildbound continues entirely on Windows (MSVC / Visual Studio), with Linux officially supported as a headless compilation and execution target.

---

## 1. System Requirements & Prerequisites

### Toolchain Requirements
- **C++20 Compliant Compiler**: GCC 11.2+ (`g++`) or LLVM Clang 14+ (`clang++`)
- **CMake**: Version 3.20 or newer
- **Build System**: Ninja (`ninja-build`) or GNU Make
- **Git**: For FetchContent submodule resolution

### Package Installation

#### Ubuntu 22.04 LTS / Debian 12
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config
```

#### RHEL 9 / Rocky Linux 9 / Fedora
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    git
```

*Note: Headless dedicated servers do not require X11, Wayland, Mesa, ALSA, or OpenGL libraries because WildboundServer bypasses all graphical and audio subsystem initialization.*

---

## 2. Configuring the Build Tree

Wildbound includes first-class CMake Presets for Linux environments in `CMakePresets.json`.

### Option A: GCC Toolchain (Recommended for Production)
```bash
# Debug Build
cmake --preset linux-gcc-debug

# Release Build (Optimized)
cmake --preset linux-gcc-release
```

### Option B: Clang Toolchain
```bash
# Debug Build
cmake --preset linux-clang-debug

# Release Build
cmake --preset linux-clang-release
```

### Manual Configuration (Without Presets)
```bash
cmake -B build/release -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DWILDBOUND_BUILD_TESTS=ON
```

---

## 3. Compiling WildboundServer

### Build Dedicated Server
```bash
# Using Presets
cmake --build --preset linux-gcc-release --target WildboundServer

# Using Direct Build Directory
cmake --build build/linux-gcc-release --target WildboundServer -j$(nproc)
```

The build process will automatically:
1. Fetch and compile pinned dependencies:
   - **Lua 5.4.7** (static C library)
   - **sol2 v3.3.1** (header-only C++20 bindings)
   - **nlohmann/json v3.11.3** (JSON schema parser)
   - **ENet v1.3.18** (BSD socket UDP transport)
2. Compile the platform abstraction layer (`src/platform/linux/LinuxPlatform.cpp`, `LinuxSignals.cpp`).
3. Compile all server-authoritative simulation modules (`world`, `combat`, `items`, `storm`, `match`, `gamemodes`, `replication`).
4. Copy runtime data directories (`assets/`, `config/`, `data/`, `maps/`, `scripts/`) into the target output directory.

---

## 4. Running Automated Tests

Run the headless CTest suite to verify simulation, transport, and replication mechanics:

```bash
# Run all tests using CMake Presets
ctest --preset linux-gcc-release

# Run dedicated server test specifically
ctest --preset linux-gcc-release -R DedicatedServerTest --output-on-failure
```

---

## 5. Running the Dedicated Server

### Headless Verification Mode (Smoke Test)
To verify server initialization, map loading, storm progression, combat registration, and graceful shutdown without holding an open socket:
```bash
./build/linux-gcc-release/WildboundServer --test-run
```

### Production Execution
```bash
./build/linux-gcc-release/WildboundServer \
    --config config/server.json \
    --bind 0.0.0.0 \
    --port 7777 \
    --max-clients 32 \
    --tick-rate 60 \
    --log-level info
```

---

## 6. Linux Systemd Service Configuration

For production cloud hosting (e.g. AWS EC2, GCP Compute Engine, Hetzner, DigitalOcean), manage `WildboundServer` via systemd.

### 1. Create Service File: `/etc/systemd/system/wildbound.service`
```ini
[Unit]
Description=Wildbound Headless Dedicated Battle Royale Server
After=network.target

[Service]
Type=simple
User=wildbound
Group=wildbound
WorkingDirectory=/opt/wildbound
ExecStart=/opt/wildbound/WildboundServer --config /opt/wildbound/config/server.json
Restart=on-failure
RestartSec=5s

# Signal handling configuration
KillMode=mixed
KillSignal=SIGTERM
TimeoutStopSec=15s

# Resource limits
LimitNOFILE=65536
LimitNPROC=4096

[Install]
WantedBy=multi-user.target
```

### 2. Enable and Start Service
```bash
sudo systemctl daemon-reload
sudo systemctl enable wildbound
sudo systemctl start wildbound

# Inspect real-time status and telemetry
sudo systemctl status wildbound
journalctl -u wildbound -f
```

---

## 7. OS Architecture & Signal Handling Details

| Feature | Windows (MSVC) | Linux (GCC / Clang) |
|---|---|---|
| **OS Signal Interception** | `SetConsoleCtrlHandler` | POSIX `sigaction` (`SIGINT`, `SIGTERM`, `SIGHUP`) |
| **Broken Socket Protection** | OS Winsock defaults | `signal(SIGPIPE, SIG_IGN)` |
| **System Memory Telemetry** | `GlobalMemoryStatusEx`, `psapi.h` | `/proc/meminfo`, `/proc/self/statm`, `sysconf(_SC_PAGE_SIZE)` |
| **Monotonic Clock** | `QueryPerformanceCounter` / `steady_clock` | `CLOCK_MONOTONIC` / `steady_clock` |
| **Process Thread Sleeping** | `SleepMs` via `std::this_thread` | `nanosleep` via `std::this_thread` |
| **Console Encoding** | UTF-8 via `SetConsoleOutputCP(CP_UTF8)` | UTF-8 default, line-buffered stdout |

