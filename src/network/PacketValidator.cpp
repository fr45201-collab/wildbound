#include "PacketValidator.hpp"

namespace wildbound::network {

PacketValidationResult PacketValidator::Validate(const uint8_t* data, size_t size,
                                                 uint8_t channel, PacketHeader& outHeader) {
    if (!data || size < PACKET_HEADER_SIZE) {
        return PacketValidationResult::TooSmall;
    }

    if (size > (PACKET_HEADER_SIZE + MAX_PACKET_PAYLOAD_SIZE)) {
        return PacketValidationResult::TooLarge;
    }

    if (channel >= static_cast<uint8_t>(ChannelType::Count)) {
        return PacketValidationResult::InvalidChannel;
    }

    ByteStreamReader reader(data, size);
    if (!DeserializePacketHeader(reader, outHeader)) {
        // Distinguish why header deserialization failed
        if (outHeader.magic != PROTOCOL_MAGIC) {
            return PacketValidationResult::BadMagic;
        }
        if (outHeader.version != PROTOCOL_VERSION) {
            return PacketValidationResult::UnsupportedVersion;
        }
        return PacketValidationResult::TooSmall;
    }

    // Verify declared payload matches actual remaining bytes in packet
    size_t actualPayload = size - PACKET_HEADER_SIZE;
    if (outHeader.payloadLength != actualPayload) {
        return PacketValidationResult::PayloadLengthMismatch;
    }

    // MessageType sanity check (range check: gameplay 0x0001-0x0051, coordinator 0x0100-0x01FF)
    if (outHeader.messageType == 0 || outHeader.messageType > 0x01FF) {
        return PacketValidationResult::UnknownMessageType;
    }

    return PacketValidationResult::Valid;
}

} // namespace wildbound::network

