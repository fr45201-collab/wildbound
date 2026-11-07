# WILDBOUND — PHASE 37 TECHNICAL MANUAL
## Authoritative Entity Synchronization & Replication Architecture

---

### 1. Replication Architecture Overview

Wildbound's entity synchronization and replication system is designed as a **dedicated server-authoritative architecture** built directly atop the Phase 36 ENet transport layer.

The central design principle is:
> **The server owns gameplay truth. Clients receive authoritative state and render it locally.**

```
+--------------------------------------------------------------------------------+
|                               AUTHORITATIVE SERVER                             |
|                                                                                |
|  +---------------------+    +-----------------------+    +------------------+  |
|  | ReplicationRegistry |    | Authoritative States  |    | Baseline States  |  |
|  +----------+----------+    +-----------+-----------+    +--------+---------+  |
|             |                           |                         |            |
|             +---------------------------+-------------------------+            |
|                                         |                                      |
|                                         v                                      |
|                             ReplicationManager::BuildSnapshot()                |
|                             - Compares against baseline                        |
|                             - Flags EntityDirtyMask                            |
|                             - Generates Delta / Full Snapshots                 |
|                                         |                                      |
|                                         v                                      |
|                             ENetServer::Broadcast()                            |
|                             - Channel 3 (Replication, Unreliable Sequenced)    |
|                             - Channel 1 (Reliable Events, Reliable Ordered)    |
+-----------------------------------------+--------------------------------------+
                                          |
                        UDP Transport (ENet v1.3.18)
                                          |
+-----------------------------------------v--------------------------------------+
|                                 REMOTE CLIENTS                                 |
|                                                                                |
|  +---------------------+    +-----------------------+    +------------------+  |
|  | ENetClient          |--->| SnapshotBuffer (Ring) |--->| Interpolator     |  |
|  | (PacketValidator)   |    | (Stale Rejection)     |    | (Render Delay)   |  |
|  +---------------------+    +-----------------------+    +--------+---------+  |
|                                                                   |            |
|                                                                   v            |
|                                                      Render World & HUD        |
+--------------------------------------------------------------------------------+
```

---

### 2. Server Authority Model

Every gameplay-affecting calculation is performed and validated exclusively on the game server:
1. **Movement Authority**: Clients never submit arbitrary world positions. Instead, clients submit `MsgPlayerInput` containing direction vectors, facing angles, and action flags. The server normalizes vectors, calculates speed with stamina/sprint modifiers, steps physics, checks collision geometry, and updates authoritative coordinates.
2. **Combat Authority**: When a player attacks, the client sends an `AttackRequest`. The server validates that the player is alive, checks weapon cooldowns and range, executes hit detection, computes damage against armor, modifies the target's `HealthComponent`, and emits an authoritative `CombatEvent`.
3. **Loot Authority**: Ground loot pickups are processed atomically on the server. The first player within range claiming an item receives confirmation; any subsequent or simultaneous requests are rejected.
4. **Environment Authority**: Doors, mechanisms, traps, and corruption storm shrink phases are simulated by the server and replicated reliably to all connected clients.

---

### 3. Network Entity Identity (`NetworkEntityIdentity`)

Every replicated entity is assigned a 64-bit combined handle:
- **Lower 32 bits**: Unique slot index / Entity ID.
- **Upper 32 bits**: Generation counter.

```cpp
struct NetworkEntityIdentity {
    NetworkEntityId id{ 0 };
    uint32_t generation{ 0 };

    [[nodiscard]] constexpr uint64_t ToHandle() const noexcept {
        return (static_cast<uint64_t>(generation) << 32) | static_cast<uint64_t>(id.value);
    }
};
```

#### Generational Slot Invalidation
When an entity is destroyed:
1. The slot in `ReplicationRegistry` is marked inactive.
2. The slot's generation counter is incremented ($G \to G + 1$).
3. The slot index is added to `m_freeIndices`.
4. When a new entity reuses this slot, it receives generation $G + 1$. Any packets in flight referencing the old generation are rejected by `IsEntityValid()`.

---

### 4. Player Movement Synchronization

- **Client Input (`MsgPlayerInput`)**:
  - `SequenceNumber inputSequence`: Monotonically increasing sequence for ACK tracking.
  - `float dirX, dirY`: Normalized directional intent.
  - `float facingAngle`: Aim direction in radians.
  - `uint8_t actionFlags`: Bit 0 = Sprint, Bit 1 = Attack, Bit 2 = Interact.
- **Server Movement Validation**:
  - Validates $\text{len}(dirX, dirY) \le 1.0$.
  - Enforces speed caps: walking = 160 units/s, sprinting = 240 units/s.
  - Moves authoritative position: $P_{t+1} = P_t + V \cdot \Delta t$.
  - Flags dirty bits: `DirtyPosition | DirtyVelocity | DirtyFacing | DirtyStatusFlags`.

---

### 5. Player Animation Synchronization

- The server owns authoritative locomotion and status flags (`isMoving`, `isRunning`, `isAlive`).
- Continuous movement animations (idle, walk, run) are derived locally on clients from replicated velocity and status flags.
- Action-triggered animations (melee attacks, spell casts, hurt reactions, death sequences) are driven by explicit server events (`CombatEvent`).
- Eliminates unnecessary network overhead by avoiding raw sprite frames or texture coordinates over the wire.

---

### 6. Enemy State Synchronization

- Server AI (`AIManager`, `AIController`) executes behavior trees and pathfinding.
- Replicated enemy state:
  - `posX, posY`: Interpolated world position.
  - `velX, velY`: Movement velocity.
  - `facingAngle`: Facing orientation.
  - `health, maxHealth`: Authoritative vitals.
  - `animState`: AI state (`Idle`, `Patrol`, `Chase`, `Attack`, `Hurt`, `Dead`).
  - `statusFlags`: Alive/dead bit.
- Clients render interpolated enemy positions and mirror the AI state for animation.

---

### 7. Health & Damage Synchronization

Health replication follows a strict unidirectional pipeline:
```
Client Attack Request ──> Server Hit Detection ──> HealthComponent::ApplyDamage()
                                                            │
                                                            ▼
                                                 ReplicationManager
                                                 - Updates Entity State
                                                 - Generates CombatEvent
                                                            │
                                                            ▼
                                                 Reliable Broadcast
                                                 - Target updates HUD
                                                 - Target plays hurt SFX
```
Clients never send health values. Any unauthorized attempt to set health is ignored.

---

### 8. Combat Event Synchronization & Deduplication

- Combat events carry a 64-bit monotonic `eventId`.
- Delivered reliably over **Channel 1 (`ReliableEvents`)**.
- **Event Deduplicator (`EventDeduplicator`)**:
  - Maintained on each client using a circular history ring buffer of 512 entries.
  - If a packet is retransmitted or delayed, `IsDuplicateAndRecord(eventId)` identifies the duplicate and discards it without applying repeated damage or playing duplicated hit sounds.

---

### 9. Loot & World Item Synchronization

- **Loot Lifecycle**:
  1. `RegisterLoot(localId, itemDefId, x, y, quantity)`: Server creates world loot entity.
  2. Broadcast via snapshot or `EntitySpawnEvent`.
  3. Client sends `PickupRequestEvent` with target loot ID and player ID.
  4. Server performs distance check ($\le 120$ units), inventory capacity check, and availability check.
  5. **Atomic Resolution**: Server marks `isAvailable = false`, unregisters loot entity, and broadcasts `PickupResultEvent(success = true)`.
  6. Concurrent requests from other clients for the same loot find the entity absent or unavailable and fail with `success = false`.

---

### 10. Environmental Gameplay State Synchronization

- **Supported Entities**: Doors, chests, levers, pressure plates, traps, destructible containers, and the Corruption Storm.
- **Environmental State**:
  - `stateId`: Closed (0), Open (1), Locked (2).
  - `active`: Boolean activation status.
  - `destroyed`: Boolean destruction marker.
  - `stateVersion`: Monotonic modification counter.
- **Corruption Storm State**:
  - Center coordinates $(C_x, C_y)$.
  - Current radius and target radius.
  - Phase index and DPS damage tick.
  - Broadcast over Channel 4 (`MatchState`).

---

### 11. Snapshot Architecture & Delta Replication

#### Snapshot Layout
```
WorldSnapshot (Envelope)
├── serverTick: uint64
├── acknowledgedInputSeq: uint32
├── timestamp: float (seconds)
├── isDelta: uint8 (0=Full, 1=Delta)
├── baselineTick: uint64
├── entityCount: uint16
└── entities[]: EntitySnapshotRecord
    ├── identity: uint32 id, uint32 gen
    ├── type: uint8
    ├── dirtyMask: uint16 (if isDelta)
    └── fields (conditionally included based on dirtyMask)
```

#### Bandwidth Comparison
- **Full Snapshot**: 42 bytes per entity.
- **Delta Snapshot (Position Only)**: 19 bytes per entity (55% bandwidth savings).
- **Delta Snapshot (Idle Entity)**: 11 bytes per entity (74% bandwidth savings).

---

### 12. Interpolation & Client Smoothing

- **Snapshot Buffer (`SnapshotBuffer`)**:
  - Circular buffer of 32 historical snapshots on client.
  - Stale snapshots ($T_{\text{server}} \le T_{\text{latest}}$) are rejected immediately.
- **Render Time Delay**:
  $$\text{renderTime} = \text{currentTime} - \frac{\text{interpolationDelayMs}}{1000}$$
  Default delay: $100\text{ ms}$ (smooths over two 20 Hz update intervals).
- **Interpolation Factor**:
  $$\alpha = \frac{\text{renderTime} - S_0.\text{timestamp}}{S_1.\text{timestamp} - S_0.\text{timestamp}}$$
- **Shortest-Path Angular Interpolation**:
  Normalizes angular difference to $[-\pi, \pi]$ to prevent full $360^\circ$ wrap spins.
- **Teleport Threshold**:
  Displacements $> 250\text{ units}$ snap instantly without smoothing.

---

### 13. Network Channel Policies

| Channel ID | Channel Name | Transport Delivery Mode | Message Categories |
|---|---|---|---|
| **0** | `Control` | Reliable Ordered | Handshake, Connect, Disconnect, Heartbeat |
| **1** | `ReliableEvents` | Reliable Ordered | Combat events, damage, loot pickup results, environmental transitions |
| **2** | `PlayerInput` | Unreliable Sequenced | 60 Hz client movement and aim input commands |
| **3** | `Replication` | Unreliable Sequenced | 20–30 Hz world state snapshots (full and delta) |
| **4** | `MatchState` | Reliable Ordered | Storm phases, countdowns, match results, victory/defeat |

---

### 14. Testing & Verification Summary

#### Automated Test Suite (`test_entity_replication.cpp`)
All 10 test suites pass with 100% success rate:
1. `Test1_EntityIdentityAndGeneration`: Generational invalidation and slot reuse.
2. `Test2_SnapshotSerialization`: Full and delta binary serialization with bandwidth savings.
3. `Test3_SnapshotBufferOrdering`: Snapshot ordering and stale packet rejection.
4. `Test4_InterpolationMath`: Position smoothing, angular wrapping, and teleport snap.
5. `Test5_EventDeduplication`: Application idempotency and duplicate event filtering.
6. `Test6_AuthoritativeMovement`: Server movement speed limits and speed-hack protection.
7. `Test7_CombatEventFlow`: Hit detection, damage deduction, and elimination.
8. `Test8_AtomicLootPickup`: Race-safe concurrent pickup resolution.
9. `Test9_EnvironmentalAndStormSync`: Door toggling and storm boundary replication.
10. `Test10_TwoClientLocalhostIntegration`: End-to-end 2-client loopback snapshot sync over ENet.

---

### 15. Known Limitations & Deferred Capabilities

- **Client-Side Movement Prediction**: Deferred to Phase 38. Current architecture supports local prediction via configuration (`enable_local_prediction = false`), but prediction replay is disabled until server reconciliation validation is complete.
- **Lag Compensation / Rollback**: Raycast attacks currently evaluate against present server world state; retroactive hit registration rewinding is planned for Phase 39.
- **Spatial Interest Partitioning**: Global snapshot broadcasting is currently used; spatial grid interest filtering is defined in configuration but deferred to high-player-count phases.

