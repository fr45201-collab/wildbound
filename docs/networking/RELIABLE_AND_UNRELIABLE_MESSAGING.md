# WILDBOUND — RELIABLE AND UNRELIABLE MESSAGING

## 1. Overview
Networked games require distinct delivery guarantees depending on the semantics of the data. Wildbound implements two fundamental transmission modes mapped to ENet packet flags:
1. **Reliable Ordered (`DeliveryMode::ReliableOrdered`)**: Guarantees delivery and strict in-order arrival via acknowledgments and automatic retransmission.
2. **Unreliable Sequenced (`DeliveryMode::UnreliableSequenced`)**: High-throughput, low-latency transmission where packets may be dropped; late or out-of-order packets are discarded in favor of newer updates.

---

## 2. Mapping to ENet Mechanics

```cpp
bool ENetTransport::Send(uint16_t peerId, ChannelType channel,
                         DeliveryMode mode, const uint8_t* data, size_t size) {
    auto* peer = GetPeer(peerId);
    if (!peer) return false;

    enet_uint32 flags = 0;
    if (mode == DeliveryMode::ReliableOrdered) {
        flags |= ENET_PACKET_FLAG_RELIABLE;
    }
    // Default in ENet on any channel is unreliable sequenced

    ENetPacket* packet = enet_packet_create(data, size, flags);
    return enet_peer_send(peer, static_cast<enet_uint8>(channel), packet) == 0;
}
```

---

## 3. Gameplay Use Cases

| Data Category | Delivery Mode | Channel | Rationale |
|---|---|---|---|
| **Player Input (Move/Aim)** | Unreliable Sequenced | 2 (`PlayerInput`) | Inputs are sampled at 60 Hz. Dropping an old input is harmless because newer input states immediately supersede it. Reliable queuing would cause head-of-line blocking and input lag. |
| **Transform Replication** | Unreliable Sequenced | 3 (`Replication`) | Positional snapshots are updated continuously. Interpolation smooths lost packets; retransmitting stale positions causes teleportation artifacts. |
| **Match State Transitions** | Reliable Ordered | 4 (`MatchState`) | Game phase changes (Waiting -> Countdown -> Active -> MatchOver) must be guaranteed to reach all participants without omission. |
| **Combat / Damage Events** | Reliable Ordered | 1 (`ReliableEvents`) | Weapon firing events, damage calculations, item drops, and death events represent authoritative state changes that must never be dropped. |
| **Heartbeats / Diagnostics** | Reliable Ordered | 1 (`ReliableEvents`) | Ping-pong latency measurements and keep-alive packets verify peer presence. |

