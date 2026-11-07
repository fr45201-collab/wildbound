# WILDBOUND — NETWORK UPDATE LOOP

## 1. Overview
The Wildbound network update loop integrates seamlessly with the fixed and variable engine frames. It guarantees non-blocking execution, bounded event processing, smooth interpolation data flow, and minimal latency between packet reception and game state updates.

---

## 2. Tick Phasing and Architecture

```
Engine Frame Tick (60 Hz / Variable)
  |
  +--> 1. NetworkManager::Update(dt)
  |      +--> Poll ENet Transport Events (enet_host_service with timeout=0)
  |      +--> Validate Incoming Packets (PacketValidator)
  |      +--> Dispatch Handshake & Data Callbacks
  |      +--> Update Rolling Bandwidth & RTT Statistics
  |
  +--> 2. Simulation & Gameplay Systems
  |      +--> Process Client Inputs / Local Simulation
  |      +--> Update AI, Storm, Combat, and Match Lifecycle
  |
  +--> 3. Replication & Packet Dispatch
  |      +--> Serialize Authoritative Updates / Snapshots
  |      +--> Send across respective Channels (1, 2, 3, 4)
  |      +--> Transport Flush (enet_host_flush)
  |
  +--> 4. Render & Developer UI
         +--> Render World & Minimap
         +--> Render DevUI Tab 21 (Network Diagnostics)
```

---

## 3. Non-Blocking Event Service

To prevent stalling the main render thread, `ENetTransport::PollEvent` operates strictly non-blocking:

```cpp
bool ENetTransport::PollEvent(TransportEvent& outEvent) {
    outEvent.type = TransportEvent::Type::None;
    if (!m_host) return false;

    ENetEvent event;
    int res = enet_host_service(m_host, &event, 0); // Non-blocking service
    if (res <= 0) return false;
    ...
}
```

Events are processed in a `while (m_transport.PollEvent(evt))` loop until all queued UDP frames for the current tick are consumed.

