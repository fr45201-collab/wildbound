# WILDBOUND — PACKET TRANSMISSION

## 1. Overview
The packet transmission pipeline defines how in-game state updates, inputs, events, and RPCs are serialized into binary payloads, enveloped with standardized packet headers, validated, submitted to ENet channel queues, and flushed across UDP sockets.

---

## 2. Wire Packet Structure

Every transmission over the Wildbound network adheres to the 16-byte standardized binary packet envelope:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Magic (0x444E4257)                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Version            |          MessageType          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Sequence Number                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        SenderClientId         |         PayloadLength         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Payload Data...                        |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### Fields:
* **Magic** (`uint32_t`): `0x444E4257` (`WBND` in Little Endian ASCII). Prevents cross-protocol noise and scanning probes.
* **Version** (`uint16_t`): Protocol version (currently `1`).
* **MessageType** (`uint16_t`): Identifies message schema (e.g., `Heartbeat`, `PlayerInput`, `EntitySpawn`, etc.).
* **SequenceNumber** (`uint32_t`): Monotonically increasing per-channel sequence number.
* **SenderClientId** (`uint16_t`): Client handle originating the packet (`0` = Server).
* **PayloadLength** (`uint16_t`): Exact byte count of following message payload.

---

## 3. Serialization Pipeline

1. **Payload Serialization**: The message payload is written into a `ByteStreamWriter`.
2. **Envelope Construction**: A `PacketHeader` is initialized with the payload length and serialized.
3. **Buffer Assembly**: The payload bytes are appended immediately behind the header.
4. **Transport Submission**: `ENetTransport::Send` submits the contiguous buffer to the chosen channel with the designated `DeliveryMode`.
5. **Immediate or Batched Flush**: Packets can be dispatched on the next `Update()` or pushed immediately using `Flush()`.

