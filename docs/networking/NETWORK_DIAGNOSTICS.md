# WILDBOUND — NETWORK DIAGNOSTICS & DEVELOPER UI

## 1. Overview
The Developer Suite (F1) integrates a real-time network telemetry and diagnostics dashboard in **Tab 21 ("Network Diagnostics")**. This dashboard enables developers, QA, and designers to inspect connection states, monitor rolling bandwidth, inspect per-client latency/loss, and test bidirectional communication interactively without external packet sniffers.

---

## 2. DevUI Tab 21 Features

### 1. Connection Role & Status Badges
- **Role Display**: Displays whether the current executable is running as `Server Host`, `Client Connection`, or `Standalone / Inactive`.
- **Status Indicator**:
  - Green `[SERVER LISTENING]`: Server host active.
  - Green `[CLIENT CONNECTED]`: Client connection established.
  - Yellow `[CLIENT CONNECTING...]`: Handshake in progress.
  - Grey `[IDLE / DISCONNECTED]`: Inactive / Disconnected.

### 2. Server Management Controls
- **Port Selector**: Configurable port input (defaults to 17777).
- **Start / Stop Server Buttons**: Dynamic toggle to host a dedicated or local server.
- **Active Connections Table**:
  - Columns: `Conn ID`, `Remote Address`, `State`, `RTT (ms)`, `Packets (S/R)`, `Bytes (S/R)`, `Action`.
  - Individual **Disconnect** buttons to kick or drop specific client connections.

### 3. Client Connection Controls
- **Target Address & Port**: Editable IP (e.g. `127.0.0.1`) and UDP port.
- **Connect / Disconnect Buttons**: Direct trigger of `NetworkManager::ConnectClient` and `DisconnectClient`.
- **Diagnostic Heartbeat Ping**:
  - Text input buffer (`TestHeartbeat`).
  - **Send Reliable Ping**: Transmits a reliable ordered packet on Channel 1.
  - **Send Unreliable Ping**: Transmits an unreliable sequenced packet on Channel 2.

### 4. Bandwidth & Metrics Telemetry
- Two-column telemetry panel contrasting Server Aggregates against Client Metrics:
  - Total packets sent & received.
  - Total bytes sent & received.
  - Real-time rolling throughput rate in KB/s.
  - Reliable packets count.
  - Security gauntlet validation drops.

