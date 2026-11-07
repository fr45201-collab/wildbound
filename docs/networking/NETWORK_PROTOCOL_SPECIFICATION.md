# WILDBOUND — NETWORK PROTOCOL SPECIFICATION
## Versioned Binary Framing, Endian Safety, and Security Validation

**Document ID:** `WB-NET-010`  
**Phase:** 35 — Multiplayer Architecture & Server Authority Design  
**Author:** Wildbound Engine Architecture Team  
**Date:** 2026-11-07  
**Status:** Approved / Authoritative  

---

### 1. Protocol Design Philosophy

Wildbound avoids text-based formats (JSON, XML) and raw struct casting (`memcpy` of C++ structs) for live network communication:
- **No JSON over UDP**: Text protocols introduce massive string-parsing CPU overhead and 5–10x bandwidth bloat.
- **No Raw Struct Memory Dumps**: Directly casting C++ structs onto network buffers violates portability due to compiler memory padding, field alignment, struct endianness, and uninitialized padding bytes leaking memory contents.
- **Explicit Field-by-Field Binary Serialization**: Every message is packed and unpacked field-by-field in standard **Little-Endian** byte order with strict bounds checking.

---

### 2. Standard Packet Header Envelope

Every UDP packet sent across the wire starts with an identical 16-byte fixed-size packet envelope:

```text
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                 Protocol Magic (0x57424E44 = 'WBND')          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       Protocol Version        |          Message Type         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Sequence Number                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       Sender Client ID        |         Payload Length        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Payload Data...                        |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

#### Envelope Field Specification
1. **`protocol_magic` (4 bytes, uint32)**: Fixed constant `0x57424E44` (ASCII `"WBND"`). Packets lacking this magic number are immediately discarded at the socket level.
2. **`protocol_version` (2 bytes, uint16)**: Monotonically incrementing protocol version integer. Current: `1`. Rejects incompatible clients.
3. **`message_type` (2 bytes, uint16)**: Identifier mapping to the `MessageType` enum.
4. **`sequence_number` (4 bytes, uint32)**: Monotonically increasing packet sequence ID per channel.
5. **`sender_client_id` (2 bytes, uint16)**: Assigned client ID (`0` for server packets).
6. **`payload_length` (2 bytes, uint16)**: Size in bytes of the following message payload. Enforces `payload_length <= MAX_PACKET_PAYLOAD_SIZE (1200 bytes)` to prevent IP fragmentation.

---

### 3. Binary ByteStream Serialization Helper

Field-by-field serialization is governed by two lightweight, bounds-checked classes: `ByteStreamWriter` and `ByteStreamReader`:

```cpp
namespace wildbound::network {

class ByteStreamWriter {
public:
    explicit ByteStreamWriter(size_t capacity = 1400);

    void WriteU8(uint8_t val);
    void WriteU16(uint16_t val);
    void WriteU32(uint32_t val);
    void WriteU64(uint64_t val);
    void WriteI16(int16_t val);
    void WriteFloat(float val);
    void WriteString(const std::string& str); // 2-byte length prefix + chars
    void WriteBytes(const void* data, size_t length);

    [[nodiscard]] const uint8_t* GetData() const noexcept { return m_buffer.data(); }
    [[nodiscard]] size_t GetSize() const noexcept { return m_buffer.size(); }

private:
    std::vector<uint8_t> m_buffer;
};

class ByteStreamReader {
public:
    ByteStreamReader(const uint8_t* data, size_t size);

    [[nodiscard]] bool HasRemaining(size_t bytes) const noexcept;
    uint8_t ReadU8();
    uint16_t ReadU16();
    uint32_t ReadU32();
    uint64_t ReadU64();
    int16_t ReadI16();
    float ReadFloat();
    std::string ReadString();
    bool ReadBytes(void* dest, size_t length);

    [[nodiscard]] size_t GetPosition() const noexcept { return m_offset; }
    [[nodiscard]] size_t GetRemaining() const noexcept { return m_size - m_offset; }

private:
    const uint8_t* m_data{ nullptr };
    size_t m_size{ 0 };
    size_t m_offset{ 0 };
};

} // namespace wildbound::network
```

---

### 4. Protocol Security Limits

To protect against buffer overruns, packet amplification attacks, and resource exhaustion:

| Security Parameter | Enforced Limit | Violation Action |
|---|---|---|
| **Max Packet Size** | 1264 bytes (16B header + 1248B payload) | Packet dropped; connection warned |
| **Max String Length** | 256 bytes (player names, chat) | Truncated or connection rejected |
| **Max Entity Count in Snapshot** | 64 entities per packet | Partitioned into multiple sub-packets |
| **Client Input Packet Rate** | Max 70 packets / second | Excess rate-limited / dropped |
| **Handshake Challenge Timeout** | 5.0 seconds | Connection slot reclaimed |
| **Inactivity Keepalive Timeout** | 15.0 seconds | Connection terminated (`TimedOut`) |

