#include "NetworkProtocol.hpp"
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace wildbound::network {

ByteStreamWriter::ByteStreamWriter(size_t initialCapacity) {
    m_buffer.reserve(initialCapacity);
}

void ByteStreamWriter::WriteU8(uint8_t val) {
    m_buffer.push_back(val);
}

void ByteStreamWriter::WriteU16(uint16_t val) {
    m_buffer.push_back(static_cast<uint8_t>(val & 0xFF));
    m_buffer.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
}

void ByteStreamWriter::WriteU32(uint32_t val) {
    m_buffer.push_back(static_cast<uint8_t>(val & 0xFF));
    m_buffer.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    m_buffer.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    m_buffer.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
}

void ByteStreamWriter::WriteU64(uint64_t val) {
    for (int i = 0; i < 8; ++i) {
        m_buffer.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
    }
}

void ByteStreamWriter::WriteI16(int16_t val) {
    WriteU16(static_cast<uint16_t>(val));
}

void ByteStreamWriter::WriteFloat(float val) {
    uint32_t bits = 0;
    std::memcpy(&bits, &val, sizeof(float));
    WriteU32(bits);
}

void ByteStreamWriter::WriteString(const std::string& str) {
    uint16_t len = static_cast<uint16_t>(std::min<size_t>(str.size(), 65535));
    WriteU16(len);
    if (len > 0) {
        WriteBytes(str.data(), len);
    }
}

void ByteStreamWriter::WriteBytes(const void* data, size_t length) {
    if (!data || length == 0) return;
    const auto* bytePtr = static_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), bytePtr, bytePtr + length);
}

// ----------------------------------------------------------------------------
// ByteStreamReader
// ----------------------------------------------------------------------------

ByteStreamReader::ByteStreamReader(const uint8_t* data, size_t size)
    : m_data(data)
    , m_size(size)
    , m_offset(0)
{
}

bool ByteStreamReader::HasRemaining(size_t bytes) const noexcept {
    return (m_offset + bytes) <= m_size;
}

uint8_t ByteStreamReader::ReadU8() {
    if (!HasRemaining(1)) return 0;
    return m_data[m_offset++];
}

uint16_t ByteStreamReader::ReadU16() {
    if (!HasRemaining(2)) return 0;
    uint16_t val = static_cast<uint16_t>(m_data[m_offset]) |
                  (static_cast<uint16_t>(m_data[m_offset + 1]) << 8);
    m_offset += 2;
    return val;
}

uint32_t ByteStreamReader::ReadU32() {
    if (!HasRemaining(4)) return 0;
    uint32_t val = static_cast<uint32_t>(m_data[m_offset]) |
                  (static_cast<uint32_t>(m_data[m_offset + 1]) << 8) |
                  (static_cast<uint32_t>(m_data[m_offset + 2]) << 16) |
                  (static_cast<uint32_t>(m_data[m_offset + 3]) << 24);
    m_offset += 4;
    return val;
}

uint64_t ByteStreamReader::ReadU64() {
    if (!HasRemaining(8)) return 0;
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val |= (static_cast<uint64_t>(m_data[m_offset + i]) << (i * 8));
    }
    m_offset += 8;
    return val;
}

int16_t ByteStreamReader::ReadI16() {
    return static_cast<int16_t>(ReadU16());
}

float ByteStreamReader::ReadFloat() {
    uint32_t bits = ReadU32();
    float val = 0.0f;
    std::memcpy(&val, &bits, sizeof(float));
    return val;
}

std::string ByteStreamReader::ReadString() {
    uint16_t len = ReadU16();
    if (len == 0 || !HasRemaining(len)) return "";
    std::string str(reinterpret_cast<const char*>(m_data + m_offset), len);
    m_offset += len;
    return str;
}

bool ByteStreamReader::ReadBytes(void* dest, size_t length) {
    if (!dest || !HasRemaining(length)) return false;
    std::memcpy(dest, m_data + m_offset, length);
    m_offset += length;
    return true;
}

// ----------------------------------------------------------------------------
// PacketHeader Serializers
// ----------------------------------------------------------------------------

bool SerializePacketHeader(ByteStreamWriter& writer, const PacketHeader& header) {
    writer.WriteU32(header.magic);
    writer.WriteU16(header.version);
    writer.WriteU16(header.messageType);
    writer.WriteU32(header.sequenceNumber);
    writer.WriteU16(header.senderClientId);
    writer.WriteU16(header.payloadLength);
    return true;
}

bool DeserializePacketHeader(ByteStreamReader& reader, PacketHeader& outHeader) {
    if (!reader.HasRemaining(PACKET_HEADER_SIZE)) return false;
    outHeader.magic = reader.ReadU32();
    outHeader.version = reader.ReadU16();
    outHeader.messageType = reader.ReadU16();
    outHeader.sequenceNumber = reader.ReadU32();
    outHeader.senderClientId = reader.ReadU16();
    outHeader.payloadLength = reader.ReadU16();
    return outHeader.IsValid();
}

} // namespace wildbound::network

