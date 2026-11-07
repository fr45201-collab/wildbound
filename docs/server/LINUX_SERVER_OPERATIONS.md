# Linux Dedicated Server Operations Manual

This guide describes operational procedures for compiling, deploying, configuring, monitoring, and managing the `WildboundServer` dedicated game server binary on Linux platforms.

---

## 1. Prerequisites & System Requirements

### Minimum Hardware
- **CPU**: 2 cores (x86_64)
- **RAM**: 512 MB available (server consumes ~16.6 MB RSS at runtime)
- **Disk**: 100 MB free space for binary, assets, scripts, and logs
- **Network**: 1 UDP port open for game traffic (default: `7777`)

### Software Environment
- **Operating System**: Ubuntu 22.04 LTS / 24.04 LTS / 26.04 LTS, Debian 12+, RHEL 9+, Arch Linux
- **C/C++ Standard Libraries**: `libc.so.6`, `libm.so.6`, `libstdc++.so.6`, `libgcc_s.so.1`
- **Build Tools (if compiling from source)**:
  - GCC / G++ 12+ (or Clang 15+)
  - CMake 3.20+
  - GNU Make 4.0+ (or Ninja)

*Note: No X11, Wayland, Mesa/OpenGL, Vulkan, ALSA, or audio packages are required. The dedicated server executable is completely headless.*

---

## 2. Compilation from Source

Wildbound provides standardized CMake presets in `CMakePresets.json`:

```bash
# Clone the repository
git clone https://github.com/Wildbound/Wildbound.git
cd Wildbound

# Configure Release build
cmake --preset linux-gcc-release

# Compile dedicated server binary
cmake --build --preset linux-gcc-release --target WildboundServer -j$(nproc)

# Run verification test suite
ctest --preset linux-gcc-release --output-on-failure
```

The resulting binary is located at:
```bash
build/linux/gcc-release/WildboundServer
```

---

## 3. Server Deployment & Directory Structure

To deploy to a production directory (e.g. `/opt/wildbound`):

```bash
# Create application directory
sudo mkdir -p /opt/wildbound

# Copy server binary and deployed runtime directories
sudo cp build/linux/gcc-release/WildboundServer /opt/wildbound/
sudo cp -r config /opt/wildbound/
sudo cp -r data /opt/wildbound/
sudo cp -r maps /opt/wildbound/
sudo cp -r scripts /opt/wildbound/
sudo cp -r assets /opt/wildbound/

# Create a dedicated system user
sudo useradd -r -s /bin/false wildbound
sudo chown -R wildbound:wildbound /opt/wildbound
```

Target directory layout:
```
/opt/wildbound/
├── WildboundServer           # 64-bit ELF executable
├── config/
│   └── server.json           # Server configuration file
├── data/
│   ├── items/                # Item definitions
│   ├── match/                # Match flow and storm parameters
│   └── gamemodes/            # Game mode rules
├── maps/                     # Tiled JSON world maps
├── scripts/                  # Lua game scripts
└── logs/                     # Operational logs
```

---

## 4. Configuration Reference

Configuration can be specified in `config/server.json` or overridden via command-line arguments:

```json
{
  "serverName": "Wildbound Authoritative Dedicated Server",
  "port": 7777,
  "maxClients": 32,
  "tickRate": 30,
  "mapName": "maps/world/battle_royale_island.json",
  "gameMode": "Solo Battle Royale",
  "logLevel": "info"
}
```

### Command-Line Arguments
```bash
./WildboundServer [options]

Options:
  --config <path>       Path to JSON configuration file (default: config/server.json)
  --host <address>      IP address to bind server host (default: 0.0.0.0)
  --port <port>         UDP port to listen on (default: 7777)
  --max-clients <count> Maximum simultaneous connected clients (1-64, default: 32)
  --tick-rate <hz>      Simulation tick rate in Hz (10-120, default: 30)
  --map <path/name>     World map path or identifier to load
  --mode <name>         Game mode to host (e.g., 'Solo Battle Royale')
  --log-level <level>   Log verbosity: trace, debug, info, warn, error
  --test-run            Verify headless initialization and exit cleanly
  --help, -h            Show help manual
  --version, -v         Display server version and target platform
```

---

## 5. Systemd Service Configuration

For production deployment, create a systemd unit file at `/etc/systemd/system/wildbound-server.service`:

```ini
[Unit]
Description=Wildbound Authoritative Dedicated Game Server
After=network.target

[Service]
Type=simple
User=wildbound
Group=wildbound
WorkingDirectory=/opt/wildbound
ExecStart=/opt/wildbound/WildboundServer --config /opt/wildbound/config/server.json
Restart=on-failure
RestartSec=5s

# Standard POSIX graceful termination
KillSignal=SIGTERM
TimeoutStopSec=10s

# Security hardening
NoNewPrivileges=true
ProtectSystem=full
ProtectHome=true

# Resource limits
LimitNOFILE=65535

[Install]
WantedBy=multi-user.target
```

### Managing the Service
```bash
# Reload systemd daemon
sudo systemctl daemon-reload

# Enable service on boot
sudo systemctl enable wildbound-server

# Start server
sudo systemctl start wildbound-server

# Check status
sudo systemctl status wildbound-server

# View live log stream
journalctl -u wildbound-server -f

# Stop server gracefully
sudo systemctl stop wildbound-server
```

---

## 6. Process Monitoring & Diagnostics

### Real-Time Diagnostics
Every 5 seconds, `WildboundServer` logs authoritative simulation metrics:
```
[INFO] [SERVER] [SERVER DIAGNOSTICS] State: Running | Uptime: 300.00s | Tick: 9000 | TPS: 30.00 / 30 Hz | AvgTick: 0.01ms (Peak: 0.15ms) | Overloads: 0
  Clients: 4 connected (4 registered) | Entities: 14 (10 enemies, 4 players) | Net I/O: 12.40 KB in (120 pkts) / 48.20 KB out (240 pkts)
```

### Linux Performance Checks
```bash
# Check CPU & Memory of the server process
ps -C WildboundServer -o pid,user,%cpu,%mem,vsz,rss,stat,start,time,command

# Monitor process I/O and resource usage
top -p $(pidof WildboundServer)

# Check active UDP listening sockets
ss -u -l -p | grep WildboundServer
```

---

## 7. Graceful Termination & Signal Handling

`WildboundServer` implements robust POSIX signal handlers for graceful shutdown:

- **`SIGINT` (Ctrl+C / `kill -2 <pid>`)**: Orderly shutdown: saves state, sends disconnect packets to clients, unloads map, cleans up ENet host.
- **`SIGTERM` (`kill -15 <pid>`)**: Standard systemd shutdown sequence.
- **Exit Code**: Clean shutdowns always exit with code `0`.

---

## 8. Troubleshooting

| Symptom | Cause | Solution |
|---|---|---|
| `Failed to initialize server host on port` | Port already bound or lack of permissions | Check for conflicting process: `sudo lsof -i :7777`. Choose a port `> 1024` if running non-root. |
| `Failed to load map file` | Working directory mismatch | Ensure `WorkingDirectory` points to directory containing `maps/` and `data/`. |
| `Connection timed out` from client | Firewall blocking UDP port | Open UDP port: `sudo ufw allow 7777/udp`. |
| High CPU usage | Tick rate configured too high | Set `--tick-rate 30` or `--tick-rate 60`. Verify `AvgTick` in diagnostics logs. |

