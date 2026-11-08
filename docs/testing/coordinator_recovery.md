# Wildbound — Coordinator Recovery & Matchmaking High Availability

## 1. Objective

`WildboundCoordinator` serves as the centralized orchestration layer responsible for player lobbies, party formation, matchmaking queues, and dedicated server registration. This document analyzes how the coordinator handles network partitions, process restarts, and client failover.

---

## 2. Coordinator Architecture & Resilience Flow

```
                      +-----------------------+
                      | WildboundCoordinator  |
                      |  - Matchmaking Queues |
                      |  - Server Registry    |
                      |  - Party Management   |
                      +-----------------------+
                                  ^
                 +----------------+----------------+
                 |                                 |
       Heartbeat & Register             Lobby / Queue Packets
                 |                                 |
      +--------------------+             +-------------------+
      |  Dedicated Server  |             |    Game Client    |
      +--------------------+             +-------------------+
```

---

## 3. Failure & Recovery Modes

### 3.1 Coordinator Process Restart
When `WildboundCoordinator` restarts:
1. **Dedicated Server Re-Registration**: All running `DedicatedServer` instances detect socket disconnection to the coordinator. The internal coordinator client enters a reconnect loop (retrying every 3 seconds). Once the coordinator restarts, servers immediately send `MsgServerRegisterRequest`, restoring server capacity in the registry.
2. **Active Matches Unaffected**: Authoritative gameplay runs entirely on `DedicatedServer`. Even if the coordinator goes offline mid-match, active matches proceed with zero interruption because players communicate directly with the game server's UDP socket.
3. **Player Lobby Re-Negotiation**: Clients waiting in matchmaking queues detect coordinator disconnection and reconnect upon service availability. Matchmaking tickets are transient and re-submitted upon reconnection.

### 3.2 Coordinator Network Partition
- If communication between a dedicated server and coordinator is severed, the dedicated server finishes its current match normally.
- Match results are written to `MatchResultsOutbox` on disk and queued until the connection is restored.
- The server will not be assigned new matches until it re-establishes its coordinator heartbeat.

### 3.3 Matchmaking Queue Self-Healing
- Stale queue entries: If a client in queue disconnects or crashes, heartbeat timeouts automatically purge the client from the matchmaking pool within 15 seconds, preventing "ghost" matches from forming.
- Party leadership transfer: If a party leader disconnects, the coordinator automatically reassigns leadership to the next available party member.

