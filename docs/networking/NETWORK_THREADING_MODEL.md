# WILDBOUND — NETWORK THREADING MODEL

## 1. Overview
This document specifies the current threading design of the Wildbound network transport layer, the concurrency constraints governing ENet, and future expansion paths toward dedicated worker threads.

---

## 2. Current Architecture: Single-Threaded Main Loop Polling

In Phase 36, all ENet host service calls, packet polling, serialization, and dispatching execute on the engine's main thread inside `Game::Update(float dt)`.

### Advantages for Phase 36:
1. **Zero Race Conditions**: No multi-threaded lock contention, race hazards, or synchronization deadlocks between gameplay systems, entity managers, and the network transport.
2. **Deterministic Execution**: Inbound packets are processed in strict lockstep with the frame rate (60 Hz or 120 Hz).
3. **Simplicity and Testability**: Localhost client-server integration tests run synchronously with predictable event ordering.

---

## 3. ENet Threading Invariants

* **ENet is NOT thread-safe on a single host**: A single `ENetHost*` must never be serviced, flushed, or sent through by multiple threads simultaneously without explicit synchronization (mutexes).
* Separate `ENetHost*` instances (e.g. server host and client host) can safely run on distinct threads, as they share only global Winsock/socket libraries, which are inherently thread-safe.

---

## 4. Future Threading Expansion (Dedicated Network Worker)

When scaling to 100-player Battle Royale matches:
1. A dedicated background I/O thread can run `enet_host_service` with a small timeout (e.g., 2–5 ms).
2. Verified packets will be pushed to thread-safe lock-free queues (`ConcurrentQueue<InboundPacket>`).
3. The main thread will drain the inbound queue at the start of each simulation tick and push outgoing replication packets to an outbound dispatch queue.

