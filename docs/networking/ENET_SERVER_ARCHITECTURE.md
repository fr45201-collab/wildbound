# WILDBOUND — ENET SERVER ARCHITECTURE

## 1. Overview
The Wildbound authoritative server runs as a headless host or in-engine server host using `ENetServer`. The server is responsible for listening on configured UDP ports, accepting incoming client connections, assigning unique connection identifiers, managing connection states, routing incoming packets to event handlers, and broadcasting state snapshots or reliable events.

---

## 2. Server Class Architecture

`ENetServer` coordinates transport, connection tracking, statistics, and callback dispatching:

```
+-------------------------------------------------------------+
|                         ENetServer                          |
+-------------------------------------------------------------+
| - m_transport: ENetTransport                                |
| - m_connectionManager: ConnectionManager                    |
| - m_globalStats: NetworkStatistics                          |
| - m_eventCallback: EventCallback                            |
| - m_port: uint16_t                                          |
| - m_isRunning: bool                                         |
+-------------------------------------------------------------+
| + Start(port, maxClients): bool                             |
| + Stop(): void                                              |
| + Update(dt): void                                          |
| + Send(clientId, channel, mode, data, size): bool           |
| + Broadcast(channel, mode, data, size, excludeId): void     |
| + DisconnectClient(clientId, reason): void                  |
| + GetConnectionManager(): ConnectionManager&                |
| + GetGlobalStats(): NetworkStatistics&                      |
+-------------------------------------------------------------+
```

---

## 3. Server Hosting Lifecycle

### 1. Initialization and Binding
The server binds to `ENET_HOST_ANY` (0.0.0.0) on the specified port with a maximum client limit (e.g., 32 peers) and 5 distinct network channels:

```cpp
bool ENetServer::Start(uint16_t port, size_t maxClients) {
    if (m_isRunning) Stop();
    if (!m_transport.Initialize(port, maxClients)) return false;

    m_connectionManager.Reset();
    m_globalStats.Reset();
    m_port = port;
    m_isRunning = true;
    return true;
}
```

### 2. Client Connection Acceptance
When an incoming connection packet arrives, ENet triggers `ENET_EVENT_TYPE_CONNECT`. `ENetServer` registers the client in `ConnectionManager`:
1. Looks up or generates a new `ConnectionId`.
2. Creates a `NetworkConnection` instance.
3. Retrieves peer IP address and port via `enet_address_get_host_ip`.
4. Transitions state to `ConnectionState::Connected`.
5. Emits `ConnectionEventType::ClientConnected` to gameplay subsystems.

### 3. Server Tick and Event Polling
During each game tick (`ENetServer::Update(float dt)`):
1. `m_globalStats.Update(dt)` refreshes rolling send/receive bandwidth rates.
2. Per-connection RTT and loss metrics are fetched from `ENetPeer` handles.
3. `m_transport.PollEvent(evt)` polls incoming events in a loop:
   - **Connect**: Register connection, notify subscribers.
   - **Data**: Validate packet via `PacketValidator`, record statistics, forward verified payload to `m_eventCallback`.
   - **Disconnect**: Unregister connection, emit `ConnectionEventType::ClientDisconnected`.

### 4. Controlled Disconnection
The server can forcefully terminate any client:
```cpp
void ENetServer::DisconnectClient(ClientId clientId, uint32_t reason) {
    m_connectionManager.UpdateState(clientId, ConnectionState::Disconnecting);
    m_transport.Disconnect(clientId, reason);
    m_connectionManager.RemoveConnection(clientId);
}
```

