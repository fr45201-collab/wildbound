# Wildbound — Network Optimization & Bandwidth Engineering

**Phase:** 47  
**Status:** Completed  
**Subsystems:** Network Protocol, Serialization, ENet Transport, Replication Manager  

---

## 1. Network Architecture & Bandwidth Constraints

Wildbound is a fast-paced multiplayer top-down action game with up to 32 players per match. Real-time gameplay requires minimal round-trip latency, high packet delivery determinism, and minimal per-packet bandwidth overhead.

### Key Transport Design Principles:
1. **Channel Segregation**:
   - `Channel 0 (Control)`: Match negotiation, state handshakes, authentication tokens, connection acceptance (`ReliableOrdered`).
   - `Channel 1 (GameEvents)`: Combat actions, loot acquisition, eliminations, sound events (`ReliableOrdered`).
   - `Channel 2 (Replication)`: World delta snapshots, position/velocity interpolation (`UnreliableUnsequenced` or `UnreliableSequenced`).
2. **Strict Wire Deserialization Guardrails**:
   - No untrusted lengths or string allocations without pre-flight size limits.
   - Immediate rejection of packets with non-finite (`NaN` or `$\pm\infty$`) floating-point fields.
   - Vector magnitude normalization to prevent client-injected super-speed input packets.
3. **Bandwidth Optimization**:
   - Binary serialization via `ByteStreamWriter` and `ByteStreamReader` (little-endian binary layout).
   - Packet header overhead reduced to 12 bytes (`magic`, `version`, `messageType`, `sequenceNumber`, `senderClientId`, `payloadLength`).

---

## 2. Packet Deserialization Hardening & Sanitization

In `src/network/NetworkMessage.cpp`, all incoming packet types are fortified:

### 2.1 `MsgPlayerInput`
- Sanitized in `Deserialize`:
  ```cpp
  if (!std::isfinite(dirX) || !std::isfinite(dirY) || !std::isfinite(facingAngle)) {
      return false; // Poisoned coordinate rejected
  }
  float lenSq = dirX * dirX + dirY * dirY;
  if (lenSq > 1.0f) {
      float invLen = 1.0f / std::sqrt(lenSq);
      dirX *= invLen;
      dirY *= invLen; // Clamped to unit vector
  }
  ```

### 2.2 `MsgAttackRequest`
- Checked for finite `aimAngle` and valid `weaponSlot`:
  ```cpp
  if (!std::isfinite(aimAngle)) {
      return false;
  }
  ```

### 2.3 Buffer Overrun & Truncation Safety
- If the wire payload contains fewer bytes than expected by the struct schema, `ByteStreamReader::ReadBytes()` safely returns `false`, gracefully aborting deserialization without crashing or reading past buffer boundaries.

---

## 3. Bandwidth Profile & Efficiency

| Packet Type | Size (Bytes) | Frequency (Hz) | Bandwidth per Client | Channel | Delivery Mode |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `MsgPlayerInput` | 25 | 30 | ~0.75 KB/s | 1 | UnreliableSequenced |
| `MsgAttackRequest` | 17 | On Action (<5 Hz) | <0.10 KB/s | 1 | ReliableOrdered |
| `MsgPickupRequest` | 18 | On Interaction | Negligible | 1 | ReliableOrdered |
| `MsgStateSnapshot` | ~120 - 450 | 30 | ~3.6 - 13.5 KB/s | 2 | UnreliableUnsequenced |
| `MsgConnectionAccepted` | 74 | 1 (Connection) | One-shot | 0 | ReliableOrdered |

---

## 4. Replication & Interest Management

- **Tick Rate**: Dedicated server simulates gameplay at a fixed $30\text{ Hz}$ ($33.33\text{ ms}$ timestep).
- **Snapshot Accumulator**: Dedicated server accumulates elapsed frame delta and emits replication snapshots every $1/30\text{ s}$.
- **Replication Caching**: State snapshots serialize participant positions, health, combat states, and storm circle radii in single contiguous datagrams.

