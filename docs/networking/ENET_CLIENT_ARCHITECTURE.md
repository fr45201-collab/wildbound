# WILDBOUND — ENET CLIENT ARCHITECTURE

## 1. Overview
The Wildbound client network subsystem is driven by `ENetClient`. It initiates outgoing UDP connections to remote or localhost authoritative servers, maintains connection states across handshakes, tracks connection timeouts, samples round-trip latency, and sends client inputs and requests across channels.

---

## 2. Client Class Architecture

```
+-------------------------------------------------------------+
|                         ENetClient                          |
+-------------------------------------------------------------+
| - m_transport: ENetTransport                                |
| - m_stats: NetworkStatistics                                |
| - m_eventCallback: EventCallback                            |
| - m_serverAddress: std::string                              |
| - m_serverPort: uint16_t                                    |
| - m_state: ConnectionState                                  |
| - m_connectionTimer: float                                  |
| - m_timeoutThreshold: float                                 |
+-------------------------------------------------------------+
| + Connect(address, port, timeoutMs): bool                   |
| + Disconnect(reason): void                                  |
| + Update(dt): void                                          |
| + Send(channel, mode, data, size): bool                     |
| + GetState(): ConnectionState                               |
| + IsConnected(): bool                                       |
| + GetRoundTripTimeMs(): uint32_t                            |
| + GetStats(): const NetworkStatistics&                      |
+-------------------------------------------------------------+
```

---

## 3. Client State Machine and Lifecycle

```
    [Disconnected]
          |
     Connect()
          v
    [Connecting] -------- Timeout / Error ------> [TimedOut / Failed]
          |                                              |
     Connect Event                                  Disconnect()
          v                                              |
     [Connected]                                         v
          |                                        [Disconnected]
   Disconnect() / Timeout
          v
   [Disconnecting]
          |
    [Disconnected]
```

### 1. Connection Initiation
```cpp
bool ENetClient::Connect(const std::string& address, uint16_t port, uint32_t timeoutMs) {
    if (m_state != ConnectionState::Disconnected) Disconnect();
    if (!m_transport.Initialize(0, 1)) return false;
    if (!m_transport.Connect(address, port)) {
        m_transport.Shutdown();
        m_state = ConnectionState::Failed;
        return false;
    }

    m_serverAddress = address;
    m_serverPort = port;
    m_state = ConnectionState::Connecting;
    m_connectionTimer = 0.0f;
    m_timeoutThreshold = static_cast<float>(timeoutMs) / 1000.0f;
    m_stats.Reset();
    return true;
}
```

### 2. Handshake and Timeout Detection
If the remote server does not respond within `m_timeoutThreshold` (default 5000 ms), the client terminates the socket, transitions to `ConnectionState::TimedOut`, and emits `ConnectionEventType::ConnectionTimedOut`.

### 3. Latency and Loss Sampling
When in the `Connected` state, `ENetClient::Update` queries `ENetPeer::roundTripTime` and `ENetPeer::packetLoss` every frame, keeping rolling averages visible in the DevUI Network Diagnostics tab.

