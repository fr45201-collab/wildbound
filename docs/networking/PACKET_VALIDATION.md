# WILDBOUND — PACKET VALIDATION

## 1. Overview
The packet validation system (`PacketValidator`) serves as the frontline security boundary for the authoritative server and client. It inspects all raw incoming UDP packet buffers before deserialization, rejecting malformed, truncated, oversized, corrupted, or malicious packets with zero impact on engine stability.

---

## 2. Validation Gauntlet Specification

Every packet received by `ENetServer` or `ENetClient` passes through `PacketValidator::Validate`:

```cpp
PacketValidationResult PacketValidator::Validate(
    const uint8_t* data, size_t size, uint8_t channel, PacketHeader& outHeader) noexcept;
```

### Validation Checks:
1. **Minimum Size Defense**:
   - `size < PACKET_HEADER_SIZE` (16 bytes) -> `PacketValidationResult::TooSmall`.
2. **Maximum Size Defense**:
   - `size > MAX_PACKET_SIZE` (65,535 bytes) -> `PacketValidationResult::TooLarge`.
3. **Magic Number Check**:
   - `outHeader.magic != PROTOCOL_MAGIC` (`0x444E4257`) -> `PacketValidationResult::BadMagic`.
4. **Protocol Version Compatibility**:
   - `outHeader.version != PROTOCOL_VERSION` (`1`) -> `PacketValidationResult::UnsupportedVersion`.
5. **Payload Length Cross-Check**:
   - `outHeader.payloadLength != (size - PACKET_HEADER_SIZE)` -> `PacketValidationResult::PayloadLengthMismatch`.
6. **Channel Range Verification**:
   - `channel >= ChannelType::Count` (5) -> `PacketValidationResult::InvalidChannel`.
7. **Message Type Range Verification**:
   - `outHeader.messageType == 0 || outHeader.messageType > 0x00FF` -> `PacketValidationResult::UnknownMessageType`.

---

## 3. Defense and Metric Tracking

When a packet fails validation:
1. It is immediately dropped; no deserializers or gameplay systems are invoked.
2. The failure counter `validationFailures` in `NetworkStatistics` is incremented.
3. If repetitive validation failures occur from a peer, the connection can be flagged for disconnection.

