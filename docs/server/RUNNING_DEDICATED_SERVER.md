# WILDBOUND — RUNNING THE DEDICATED SERVER

## Operator & Administrator Guide

---

## 1. Quick Start

### 1.1 Running the Default Server
To launch the dedicated server with default settings (bind `0.0.0.0:7777`, max 32 clients, 30 Hz tick rate, hosting the Battle Royale Island):

```cmd
.\build\Release\WildboundServer.exe
```

### 1.2 Running Headless Verification (`--test-run`)
To verify that all subsystems (world geometry, collision spatial partition, storm manager, item catalog, match lifecycle, Lua scripting, ENet sockets) initialize, step 5 ticks headlessly, and cleanly shut down without error:

```cmd
.\build\Release\WildboundServer.exe --test-run
```

Expected output:
```text
[INFO] [SERVER] === Wildbound Dedicated Server Initializing ===
[INFO] [SERVER] Configured bind: 0.0.0.0:7777 | Max clients: 32 | Tick rate: 30 Hz
[INFO] [SCRIPTING] Lua runtime initialized.
[INFO] [ITEM] Initializing ItemManager from catalog: data/items/item_catalog.json
[INFO] [ITEM] Successfully loaded 40 item definitions from catalog.
[INFO] [ATTACK_SYS] Loaded 3 attack definitions.
[INFO] [COMBAT_SYS] CombatSystem initialized with audio subscribers. Shared attacks loaded: 3
[INFO] [WORLD] Loading map: maps/world/battle_royale_island.json
[INFO] [MAP_LOADER] Loaded Tiled JSON map: battle_royale_island (320x320 tiles, 6 layers, 448 objects)
[INFO] [WORLD] Built collision spatial index with 41503 collision bounds.
[INFO] [WORLD] Map 'battle_royale_island' successfully initialized into World.
[INFO] [STORM] Initialized StormManager: [Wildbound Battle Royale Standard Storm] with 6 phases. Initial radius: 4000.000000
[INFO] [MATCH_CONFIG] Successfully loaded match configuration from: data/match/default_match.json
[INFO] [MATCH] State transition: [Uninitialized] -> [Initializing].
[INFO] [MATCH] Initialized match [wildbound_default] on world [battle_royale_island].
[INFO] [MATCH] State transition: [Initializing] -> [WaitingForPlayers].
[INFO] [GAMEMODE] Loaded 5 game modes from data/gamemodes
[INFO] [GAMEMODE] Selected game mode: [Solo Battle Royale] (Teams: Disabled, TeamSize: 1)
[INFO] [GAMEMODE] GameModeManager initialized successfully. Active mode: [Solo Battle Royale]
[INFO] [SERVER] Starting authoritative ENet server host on port 7777...
[INFO] [SERVER] Dedicated Server is now RUNNING and accepting client connections.
[INFO] [SERVER] Test-run mode enabled. Running 5 simulation ticks headlessly...
[INFO] [SERVER] Headless verification ticks completed successfully.
[INFO] [SERVER] Executing orderly dedicated server shutdown sequence...
[INFO] [MATCH] Resetting match resources and state.
[INFO] [MATCH] State transition: [WaitingForPlayers] -> [Cleanup].
[INFO] [MATCH] State transition: [Cleanup] -> [Uninitialized].
[INFO] [WORLD] Unloading map: battle_royale_island
[INFO] [SCRIPTING] Shutting down Lua runtime.
[INFO] [SERVER] Dedicated Server shutdown completed cleanly.
```

---

## 2. Command-Line Options

The dedicated server supports the following CLI arguments. Command-line parameters take precedence over configuration file fields.

| Option | Argument | Description | Default |
|---|---|---|---|
| `--config` | `<path>` | Path to custom JSON configuration file | `assets/config/server.json` |
| `--host` | `<ip>` | IP address to bind server host socket | `0.0.0.0` |
| `--port` | `<port>` | UDP port to listen for client connections | `7777` |
| `--max-clients` | `<1-64>` | Maximum simultaneous connected clients | `32` |
| `--tick-rate` | `<10-120>`| Authoritative simulation tick rate in Hz | `30` |
| `--map` | `<path>` | Path to world map file to load | `maps/world/battle_royale_island.json` |
| `--mode` | `<id>` | Game mode to host (`solo`, `duos`, `squads`, `practice`) | `solo` |
| `--log-level` | `<level>`| Logging verbosity (`debug`, `info`, `warn`, `error`) | `info` |
| `--test-run` | *(none)* | Initializes server, runs 5 ticks, and exits cleanly | *Disabled* |
| `--help`, `-h` | *(none)* | Displays usage manual and flags | *N/A* |
| `--version`, `-v`| *(none)* | Displays version and build architecture | *N/A* |

### Example CLI Invocations

**Host a high-tick tournament server on custom port:**
```cmd
WildboundServer.exe --port 8888 --tick-rate 60 --max-clients 64 --log-level debug
```

**Host Practice Mode on Forest map:**
```cmd
WildboundServer.exe --port 7778 --mode practice --map maps/forest/forest_01.json
```

---

## 3. Configuration File Reference (`assets/config/server.json`)

The server configuration file uses standard JSON:

```json
{
  "server": {
    "name": "Wildbound Dedicated Server",
    "host": "0.0.0.0",
    "port": 7777,
    "max_clients": 32,
    "tick_rate": 30,
    "max_frame_time_ms": 250,
    "enable_console": true,
    "enable_file_logging": true,
    "log_level": "info"
  },
  "world": {
    "map": "maps/world/battle_royale_island.json",
    "game_mode": "solo",
    "auto_start": false
  },
  "network": {
    "connection_timeout_seconds": 30,
    "max_packet_size_bytes": 65535,
    "channel_count": 5
  }
}
```

---

## 4. Signal Handling & Orderly Teardown

`WildboundServer.exe` installs an authoritative OS signal handler (`SetConsoleCtrlHandler` on Windows):
- Pressing `Ctrl+C` or `Ctrl+Break` initiates graceful termination.
- Closing the console window sends an OS close event to the server.
- The server disconnects all connected clients with reason code 0, flushes pending reliable packets, unloads map geometry, shuts down Lua and ENet, prints diagnostic summaries, and exits cleanly with exit code 0.

