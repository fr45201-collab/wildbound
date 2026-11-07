# WILDBOUND — CONNECTION LIFECYCLE

## 1. Overview
The connection lifecycle in Wildbound governs the sequence of transitions that occur when a client initiates communication with an authoritative server, exchanges handshake packets, streams game packets, and terminates either gracefully or via timeout.

---

## 2. Full Handshake Sequence

```
Client                                                  Server
  |                                                       |
  |--- ENet Connect Command (SYN) ----------------------->|
  |                                                       | (Accepts, Allocates
  |                                                       |  ConnectionId,
  |<-- ENet Connect Acknowledge (SYN-ACK) ----------------|  Fires ClientConnected)
  |                                                       |
(Transitions to Connected,                                |
 Fires ClientConnected)                                   |
  |                                                       |
  |=== BIDIRECTIONAL CHANNEL PACKET STREAMING ============|
  |                                                       |
```

---

## 3. Disconnection Sequences

### Graceful Client Disconnect
1. Client invokes `ENetClient::Disconnect(reason)`.
2. ENet dispatches an explicit `ENET_PROTOCOL_COMMAND_DISCONNECT` packet.
3. Server receives `ENET_EVENT_TYPE_DISCONNECT`.
4. Server transitions the client state to `Disconnected`, cleans up peer mappings in `ConnectionManager`, and fires `ConnectionEventType::ClientDisconnected`.

### Server-Initiated Kick / Disconnect
1. Server invokes `ENetServer::DisconnectClient(clientId, reason)`.
2. ENet sends disconnect command to peer.
3. Server transitions connection to `Disconnecting` and immediately invalidates lookup tables.
4. Client receives `ENET_EVENT_TYPE_DISCONNECT`, transitions to `Disconnected`, and emits event.

### Abnormal Timeout
1. UDP packets stop arriving (network loss or process crash).
2. Client/Server timeout timer reaches threshold without incoming packets or ping acknowledgments.
3. ENet triggers automatic disconnect or Wildbound timer fires `ConnectionState::TimedOut`.
4. Resources are freed with zero lingering zombie connections.

