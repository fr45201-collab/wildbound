#pragma once

#include "NetworkTypes.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace wildbound::network {

/// Protocol Magic constant 'WBND' (0x57424E44)
inline constexpr uint32_t PROTOCOL_MAGIC = 0x57424E44;

/// Incremental protocol revision
inline constexpr uint16_t PROTOCOL_VERSION = 1;

/// Maximum allowable UDP payload size before IP fragmentation
inline constexpr size_t MAX_PACKET_PAYLOAD_SIZE = 1200;

/// Standard fixed-size 16-byte envelope header on every wire packet
struct PacketHeader {
    uint32_t magic{ PROTOCOL_MAGIC };
    uint16_t version{ PROTOCOL_VERSION };
    uint16_t messageType{ 0 };
    uint32_t sequenceNumber{ 0 };
    uint16_t senderClientId{ 0 };
    uint16_t payloadLength{ 0 };

    [[nodiscard]] bool IsValid() const noexcept {
        return magic == PROTOCOL_MAGIC && version == PROTOCOL_VERSION && payloadLength <= MAX_PACKET_PAYLOAD_SIZE;
    }
};

inline constexpr size_t PACKET_HEADER_SIZE = 16;

/**
 * Endian-safe, bounds-checked binary stream writer (Little-Endian).
 */
class ByteStreamWriter {
public:
    explicit ByteStreamWriter(size_t initialCapacity = 256);

    void WriteU8(uint8_t val);
    void WriteU16(uint16_t val);
    void WriteU32(uint32_t val);
    void WriteU64(uint64_t val);
    void WriteI16(int16_t val);
    void WriteFloat(float val);
    void WriteString(const std::string& str);
    void WriteBytes(const void* data, size_t length);

    [[nodiscard]] const uint8_t* GetData() const noexcept { return m_buffer.data(); }
    [[nodiscard]] size_t GetSize() const noexcept { return m_buffer.size(); }
    [[nodiscard]] const std::vector<uint8_t>& GetBuffer() const noexcept { return m_buffer; }
    void Clear() noexcept { m_buffer.clear(); }

private:
    std::vector<uint8_t> m_buffer;
};

/**
 * Endian-safe, bounds-checked binary stream reader (Little-Endian).
 */
class ByteStreamReader {
public:
    ByteStreamReader(const uint8_t* data, size_t size);

    [[nodiscard]] bool HasRemaining(size_t bytes) const noexcept;
    [[nodiscard]] size_t GetRemaining() const noexcept { return (m_offset <= m_size) ? (m_size - m_offset) : 0; }
    [[nodiscard]] size_t GetPosition() const noexcept { return m_offset; }

    uint8_t ReadU8();
    uint16_t ReadU16();
    uint32_t ReadU32();
    uint64_t ReadU64();
    int16_t ReadI16();
    float ReadFloat();
    std::string ReadString();
    bool ReadBytes(void* dest, size_t length);

private:
    const uint8_t* m_data{ nullptr };
    size_t m_size{ 0 };
    size_t m_offset{ 0 };
};

// Envelope serialization helpers
bool SerializePacketHeader(ByteStreamWriter& writer, const PacketHeader& header);
bool DeserializePacketHeader(ByteStreamReader& reader, PacketHeader& outHeader);

} // namespace wildbound::network

