# WILDBOUND — NETWORK CHANNELS

## 1. Overview
ENet provides multi-channel virtual multiplexing over a single underlying UDP socket. Channels isolate message streams so that packet drops or retransmissions on one stream do not block or delay independent packets on another stream (preventing head-of-line blocking).

---

## 2. Channel Allocation Specification

Wildbound configures 5 distinct channels:

```cpp
enum class ChannelType : uint8_t {
    Default = 0,         // General system control & handshake
    ReliableEvents = 1,  // Authoritative gameplay events (combat, interactions, chat)
    PlayerInput = 2,     // High-frequency client inputs (movement, aiming, triggers)
    Replication = 3,     // World state & transform replication snapshots
    MatchState = 4,      // Match phase, storm updates, circle timings
    Count = 5
};
```

---

## 3. Channel Isolation Guarantees

1. **Independent Sequencing**: Each channel maintains its own incoming and outgoing sequence counters in ENet.
2. **Zero Cross-Channel Head-of-Line Blocking**: If a reliable event packet on Channel 1 is dropped and awaits retransmission, incoming player inputs on Channel 2 or entity replication packets on Channel 3 continue to be processed with zero stall.
3. **Bandwidth Throttling Distribution**: ENet balances channel throughput according to total peer window limits, preventing high-frequency player inputs from starvation during large world state synchronization.

