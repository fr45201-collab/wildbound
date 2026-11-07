# WILDBOUND — LOCALHOST INTEGRATION TESTS

## 1. Overview
The transport layer automated test suite is implemented in `tests/network/test_network_transport.cpp` and registered in CTest as Test 37 (`NetworkTransportTest`). It validates the low-level ENet networking layer through automated localhost loopback execution.

---

## 2. Test Suite Architecture

All 10 test suites run against distinct loopback ports (`BASE_TEST_PORT + N`):

| Test # | Test Name | Purpose & Verification |
|---|---|---|
| **1** | `TestENetLifecycle` | Atomic reference counting of global ENet initialization, duplicate initialization safety, decrementation, and safe re-initialization. |
| **2** | `TestHostCreationAndDestruction` | Creation of server host on port `17777` with 16 peer slots, creation of client host with 1 peer slot, address binding validation, and clean teardown. |
| **3** | `TestPacketValidationGauntlet` | 8-point security defense checking 0-byte packets, truncated headers (<16 bytes), wrong magic numbers, unsupported protocol versions, payload length mismatches, out-of-bounds channels, oversized packets (>64 KB), and valid envelopes. |
| **4** | `TestConnectionManagerOperations` | Registration of client connections, $O(1)$ ID lookup, peer-pointer mapping, state updates (`Connecting` -> `Connected`), and clean slot removal. |
| **5** | `TestLocalhostConnectionLifecycle` | End-to-end loopback connection handshake: client initiates connect to `127.0.0.1:17778`, server receives and accepts, client confirms connection, and graceful disconnect. |
| **6** | `TestReliableMessagingPingPong` | Bidirectional ping-pong over Channel 1 (`ReliableEvents`) with `DeliveryMode::ReliableOrdered`: client sends `ClientPing`, server receives and replies with `ServerPong`, client verifies receipt. |
| **7** | `TestUnreliableSequencedMessaging` | High-frequency simulated movement inputs streamed across Channel 2 (`PlayerInput`) with `DeliveryMode::UnreliableSequenced`: verifies sequencing and data delivery without drops or stalls. |
| **8** | `TestMultiChannelRouting` | Simultaneous multi-channel transmission across Channels 1, 2, 3, and 4: verifies channel isolation and independent sequencing. |
| **9** | `TestConnectionTimeout` | Attempting to connect to an unassigned local port (e.g., `59999`) with a 300 ms timeout: verifies timeout detection, transition to `ConnectionState::TimedOut`, and event emission. |
| **10** | `TestRapidConnectDisconnectStress` | 5 consecutive rapid connect / disconnect cycles: verifies zero resource leaks, clean peer table recycling, and zero memory degradation. |

---

## 3. Test Execution Results

```
==========================================================
WILDBOUND PHASE 36: ENET TRANSPORT LAYER TEST SUITE
==========================================================
[TEST] 1. ENet Global Lifecycle & Ref-Counting...
  -> Initialization ref-counting and safe re-initialization verified.
[TEST] 2. Server & Client Host Creation & Teardown...
  -> Host creation, bind verification, and clean shutdown verified.
[TEST] 3. Packet Validation Gauntlet & Boundary Defenses...
  -> Packet bounds, magic, version, payload, and channel validation verified.
[TEST] 4. Connection Manager Registry & Tracking...
  -> Connection lookup, peer mapping, state updates, and removal verified.
[TEST] 5. Localhost Client-Server Connection Lifecycle...
  -> Localhost client connect, server accept, and clean disconnect verified.
[TEST] 6. Reliable Messaging Bidirectional Ping-Pong...
  -> Client-to-server reliable send and server-to-client reply verified.
[TEST] 7. Unreliable Sequenced Movement Messaging...
  -> High-frequency unreliable sequenced input streaming verified.
[TEST] 8. Multi-Channel Virtual Mesh Routing (Channels 0 - 4)...
  -> Channel isolation and independent multi-channel routing verified.
[TEST] 9. Connection Timeout Handling...
  -> Inactive host timeout detection and graceful state transition verified.
[TEST] 10. Rapid Connect / Disconnect Cycle Stress...
  -> 5 sequential rapid connect/disconnect cycles executed with zero leaks.
==========================================================
ALL 10 PHASE 36 TRANSPORT TEST SUITES PASSED (100% OK)
==========================================================
```

