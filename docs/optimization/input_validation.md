# Wildbound — Input & Packet Validation Specification

**Phase:** 47  
**Status:** Completed  
**Subsystems:** Network Serialization, Packet Validator, Input Manager  

---

## 1. Input Threat Vectors

Client-side input simulation is the primary attack surface in online action games:
- **Vector Overspeed**: Transmitting direction vectors with length $> 1.0$ (e.g. `(10.0, 0.0)` instead of `(1.0, 0.0)`), multiplying speed tenfold.
- **NaN / Infinity Injection**: Passing IEEE 754 float `NaN` or `$\pm\infty$` values that propagate through physics calculations, causing collisions to fail, entities to vanish, or servers to crash.
- **Input Flooding**: Transmitting hundreds of input packets per second to exhaust server processing time or induce snapshot deserialization thrashing.
- **Malformed Payloads**: Sending truncated packets or corrupted byte headers to trigger heap corruption or out-of-bounds buffer reads.

---

## 2. Implemented Validation Guardrails

### 2.1 Deserialization Boundary (`src/network/NetworkMessage.cpp`)
All packet deserializers enforce non-negotiable mathematical validity before returning to higher-level server logic:
- `std::isfinite(val)` checks on all floats.
- Automatic vector length normalization if $\|\vec{v}\| > 1.0$.
- Bounds-checked string deserialization with `MAX_STRING_LENGTH` guards.

### 2.2 Wire Deserialization Code Reference
```cpp
void MsgPlayerInput::Serialize(ByteStreamWriter& writer) const {
    writer.WriteUInt32(inputSequence);
    writer.WriteFloat(dirX);
    writer.WriteFloat(dirY);
    writer.WriteFloat(facingAngle);
    writer.WriteUInt8(actionFlags);
}

bool MsgPlayerInput::Deserialize(ByteStreamReader& reader) {
    if (!reader.ReadUInt32(inputSequence) ||
        !reader.ReadFloat(dirX) ||
        !reader.ReadFloat(dirY) ||
        !reader.ReadFloat(facingAngle) ||
        !reader.ReadUInt8(actionFlags)) {
        return false;
    }

    if (!std::isfinite(dirX) || !std::isfinite(dirY) || !std::isfinite(facingAngle)) {
        return false;
    }

    float lenSq = dirX * dirX + dirY * dirY;
    if (lenSq > 1.0f) {
        float invLen = 1.0f / std::sqrt(lenSq);
        dirX *= invLen;
        dirY *= invLen;
    }
    return true;
}
```

---

## 3. Dedicated Server Ingress Routing

In `DedicatedServer::OnClientPacketReceived()`:
1. Every packet is categorized (`GeneralPacket`, `PlayerInput`, `CombatAction`, `Interaction`).
2. Passed to `m_securityManager.CheckRateLimit(clientId, category, messageType)`.
3. If rate-limited, dropped immediately without decoding payload.
4. Deserialized via `msg.Deserialize(reader)`.
5. If deserialization fails, recorded as `SecurityEventType::MalformedPacket` and discarded.

