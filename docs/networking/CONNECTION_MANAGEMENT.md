# WILDBOUND — CONNECTION MANAGEMENT

## 1. Overview
On the authoritative server, every connected client is tracked through `ConnectionManager`. This subsystem assigns stable connection identifiers (`ConnectionId`), maintains mappings to raw `ENetPeer*` pointers, stores per-connection metadata (`NetworkConnection`), tracks connection states, and maintains per-connection packet/bandwidth statistics.

---

## 2. Connection Identifiers & Handle Integrity

Wildbound uses a 16-bit identifier (`ConnectionId` / `ClientId`) where:
* `0`: Reserved (Server / Invalid)
* `1` to `65535`: Allocated sequentially to connected clients

Connection lookup uses dual index tables:
1. `std::unordered_map<ConnectionId, std::shared_ptr<NetworkConnection>> m_connections`
2. `std::unordered_map<const void*, ConnectionId> m_peerToConnectionId`

This provides:
- $O(1)$ lookup from connection ID to connection metadata.
- $O(1)$ reverse lookup from raw ENet peer events to connection ID without scanning.

---

## 3. NetworkConnection Structure

Each active connection encapsulates:
* **Connection ID**: Unique numeric handle.
* **ENet Peer**: Opaque `ENetPeer*` pointer.
* **Remote Address & Port**: Stringified IP and UDP port.
* **Lifecycle State**: Current `ConnectionState` (Connecting, Connected, Disconnecting, etc.).
* **Connection Statistics**: Dedicated `NetworkStatistics` instance tracking RTT, packet loss, packets sent/received, bytes sent/received, and rolling bandwidth rates.

---

## 4. Connection State Machine

```cpp
enum class ConnectionState : uint8_t {
    Disconnected = 0,
    Connecting,
    Connected,
    Disconnecting,
    TimedOut,
    Failed
};
```

### Safety and Cleanup
When a peer disconnects or times out:
1. State is immediately transitioned to `Disconnected`.
2. Peer pointer mapping is erased to prevent dangling pointer dereferences.
3. The `NetworkConnection` slot is safely pruned from `ConnectionManager`.

