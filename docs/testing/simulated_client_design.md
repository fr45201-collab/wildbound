# Wildbound — Simulated Client Design

## 1. Objective & Behavioral Requirements

To test dedicated servers under authentic production conditions, client bots must execute the exact wire protocol, rate limits, and network channels as real human players. Passive connection sockets or simple ping utilities fail to stress spatial collision indices, replication builders, or security token buckets.

The `LoadTestClient` is designed to:
1. Complete full ENet transport handshakes and Wildbound authentication handshakes.
2. Ingest `MsgConnectionAccepted` and register assigned player entity IDs.
3. Transmit 30 Hz input movement commands (`MsgPlayerInput`).
4. Execute periodic attacks (`MsgAttackRequest`) and item pickups (`MsgPickupRequest`).
5. Ingest and deserialize incoming `StateSnapshot` replication payloads.
6. Support synthetic packet loss injection and artificial latency simulation.

---

## 2. Client Lifecycle State Machine

```
   [Disconnected]
         |
         | ENet Connect() initiated
         v
   [Connecting]
         |
         | ENet Transport Event: Connect
         v
    [Connected] --------------> Sends MsgClientHello (Channel: Control)
         |
         | Server sends MsgConnectionAccepted (Channel: Control)
         v
    [Admitted]  --------------> Begins 30 Hz Simulation Loop
         |                      - Fixed 30 Hz MsgPlayerInput (Channel: PlayerInput)
         |                      - Periodic MsgAttackRequest (Channel: ReliableEvents)
         |                      - Snapshot Ingestion (Channel: Replication)
         v
   [Disconnect] --------------> Graceful ENet teardown
```

---

## 3. Wire Protocol & Channel Mapping

`LoadTestClient` strictly honors the channel layout defined in `NetworkTypes.hpp`:

| Channel | Identifier | Delivery Mode | Content |
|---|---|---|---|
| **Control** | 0 | Reliable Ordered | `MsgClientHello`, `MsgConnectionAccepted`, `MsgConnectionRejected` |
| **ReliableEvents** | 1 | Reliable Ordered | `MsgAttackRequest`, `MsgPickupRequest`, Chat, Combat events |
| **PlayerInput** | 2 | Unreliable Sequenced | 30 Hz `MsgPlayerInput` (direction, facing angle, action flags) |
| **Replication** | 3 | Unreliable Sequenced | Authoritative `StateSnapshot` packets from server |
| **MatchState** | 4 | Reliable Ordered | Storm progression, match phase transitions, winner announcements |

---

## 4. Input & Movement Simulation Model

Each simulated client runs an autonomous circular wandering pathing model:
- **Input Frequency**: Exactly 30 Hz ($T = 33.3\text{ ms}$).
- **Trajectory Generator**:
  $$\theta_{t} = \theta_{t-1} + 0.05\text{ rad}$$
  $$\vec{d} = (\cos\theta_t, \sin\theta_t)$$
- **Initial Spawn Layout**: Distributed grid offset by index:
  $$x_0 = 300.0 + (i \pmod 8) \times 40.0$$
  $$y_0 = 300.0 + \lfloor i / 8 \rfloor \times 40.0$$
- **Combat Events**: Every 2.5 seconds, the bot executes an attack along its current facing angle.
- **Rate-Limiter Compliance**: Transmissions stay strictly within Phase 47 token-bucket thresholds (PlayerInput limit: 60 pkts/s; Combat limit: 10 actions/s).

---

## 5. Adverse Network Condition Simulation

To evaluate client degradation and packet drop resilience, `LoadTestClient` supports:
- **Artificial Loss**: Stochastically drops outbound packets prior to transmission using a Mersenne Twister uniform distribution:
  $$P(\text{drop}) < \text{lossPercent}$$
- **Artificial Latency**: Emulates adverse round-trip delays and measures response deviation.
- **Round-Trip Time Telemetry**: Ingests smoothed RTT directly from ENet peer statistics (`peer->roundTripTime`).

