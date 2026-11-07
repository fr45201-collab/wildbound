# WILDBOUND — DISCONNECT AND TIMEOUT HANDLING

## 1. Overview
Network connections are inherently volatile. Clients may disconnect gracefully, lose Wi-Fi connectivity, suffer route flaps, or crash abruptly. Wildbound provides deterministic detection and resource reclamation for all disconnection scenarios.

---

## 2. Disconnection Categories

### 1. Graceful Peer Disconnect
- Triggered by `ENetClient::Disconnect()` or `ENetServer::DisconnectClient()`.
- The peer transmits an `ENET_PROTOCOL_COMMAND_DISCONNECT` packet.
- Upon receiving `ENET_EVENT_TYPE_DISCONNECT`, the remote end updates the connection state to `Disconnected`, removes the peer handle, and dispatches `ConnectionEventType::ClientDisconnected`.

### 2. Client-Side Connection Attempt Timeout
- When calling `Connect(address, port, timeoutMs)`, a timer accumulates elapsed time.
- If no server acknowledgment arrives before `timeoutThreshold` (default 5.0 seconds), the client cancels the pending connection, shuts down the host, sets `ConnectionState::TimedOut`, and dispatches `ConnectionEventType::ConnectionTimedOut`.

### 3. Established Session Inactivity Timeout
- Handled automatically by ENet's internal ping/heartbeat mechanism.
- ENet periodically transmits `ENET_PROTOCOL_COMMAND_PING` packets. If no response or data arrives after `ENET_PEER_TIMEOUT_LIMIT` missed acknowledgments, ENet automatically generates `ENET_EVENT_TYPE_DISCONNECT`.
- The server cleans up the participant without leaking connection slots or memory.

---

## 3. Post-Disconnect State Invariants
- `GetConnectionCount()` accurately reflects active connections.
- Stale `ENetPeer*` handles are deleted from reverse lookup dictionaries.
- Bandwidth rate metrics decay to 0 B/s.

