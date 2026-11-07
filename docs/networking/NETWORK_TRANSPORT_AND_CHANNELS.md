# WILDBOUND — NETWORK TRANSPORT & CHANNELS SPECIFICATION
## Abstract Transport Interface, Channel Semantics, and Future ENet Integration

**Document ID:** `WB-NET-012`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Transport Abstraction Design

To prevent high-level gameplay systems from coupling directly to third-party networking libraries, Wildbound defines a pure C++20 interface: `INetworkTransport`.

```cpp
namespace wildbound::network {

enum class ChannelType : uint8_t {
    Control        = 0,  // Reliable In-Order: Handshake, Connect, Disconnect
    ReliableEvents = 1,  // Reliable In-Order: Attacks, Pickups, Damage, Eliminations
    PlayerInput    = 2,  // Unreliable Sequenced: Client 60 Hz input packets
    Replication    = 3,  // Unreliable Sequenced: Server 30 Hz world snapshots
    MatchState     = 4,  // Reliable In-Order: State changes, storm transitions
    Count          = 5
};

enum class DeliveryMode : uint8_t {
    ReliableOrdered,     // Guarantees delivery and strict sequence order
    UnreliableSequenced  // Discards out-of-order stale packets, no resend
};

struct TransportEvent {
    enum class Type : uint8_t { None, Connect, Disconnect, Data } type{ Type::None };
    uint16_t peerId{ 0 };
    ChannelType channel{ ChannelType::Control };
    std::vector<uint8_t> data;
};

class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    virtual bool Initialize(uint16_t port = 0, size_t maxPeers = 32) = 0;
    virtual void Shutdown() = 0;

    virtual bool Connect(const std::string& address, uint16_t port) = 0;
    virtual void Disconnect(uint16_t peerId, uint32_t reason = 0) = 0;

    virtual bool Send(uint16_t peerId, ChannelType channel,
                      DeliveryMode mode, const uint8_t* data, size_t size) = 0;

    virtual bool PollEvent(TransportEvent& outEvent) = 0;
    virtual void Flush() = 0;
};

} // namespace wildbound::network
```

---

### 2. Channel Responsibilities & Semantics

Using a single network stream for all traffic causes **Head-of-Line (HoL) Blocking**: if one packet is lost, all subsequent packets (including time-critical position updates) are stalled until the lost packet is retransmitted. Wildbound divides UDP traffic across 5 distinct virtual channels:

| Channel | Purpose | Delivery Mode | Why this configuration? |
|---|---|---|---|
| **Ch 0: Control** | Handshake, Auth, Disconnect | **Reliable Ordered** | Connection negotiation must not be lost. If lost, retransmission occurs. |
| **Ch 1: Reliable Events** | Attacks, Interactions, Eliminations | **Reliable Ordered** | A weapon fire or item pickup event must never be silently dropped. |
| **Ch 2: Player Input** | Client $\to$ Server movement inputs (60 Hz) | **Unreliable Sequenced** | If input packet #102 is lost and #103 has already arrived, resending #102 is useless because #103 is newer. Retransmissions would only cause lag spikes. |
| **Ch 3: Snapshots** | Server $\to$ Client state snapshots (30 Hz) | **Unreliable Sequenced** | Never retransmit old positions. Stale snapshots are discarded immediately. |
| **Ch 4: Match State** | Countdown, Storm phases, Results | **Reliable Ordered** | Match transitions occur rarely and are critical for all participants. |

---

### 3. Future ENet Adapter Integration Plan

When ENet is integrated in Phase 36:
1. `ENetTransport` will inherit from `INetworkTransport`.
2. ENet's `enet_host_service` will drive `PollEvent`.
3. ENet flags `ENET_PACKET_FLAG_RELIABLE` and `ENET_PACKET_FLAG_UNSEQUENCED` will map directly to `DeliveryMode`:
   - `DeliveryMode::ReliableOrdered` $\to$ `ENET_PACKET_FLAG_RELIABLE` on channel $0, 1, 4$.
   - `DeliveryMode::UnreliableSequenced` $\to$ default unreliable on channel $2, 3$.
4. ENet memory allocation hooks will route into Wildbound's core logger for memory leak detection.

