#pragma once

#include "NetworkProtocol.hpp"
#include <cstdint>
#include <cstddef>
#include <string>

namespace wildbound::network {

enum class PacketValidationResult : uint8_t {
    Valid,
    TooSmall,
    TooLarge,
    BadMagic,
    UnsupportedVersion,
    UnknownMessageType,
    InvalidChannel,
    PayloadLengthMismatch
};

[[nodiscard]] constexpr std::string_view ToString(PacketValidationResult res) noexcept {
    switch (res) {
        case PacketValidationResult::Valid:                 return "Valid";
        case PacketValidationResult::TooSmall:              return "TooSmall";
        case PacketValidationResult::TooLarge:              return "TooLarge";
        case PacketValidationResult::BadMagic:              return "BadMagic";
        case PacketValidationResult::UnsupportedVersion:    return "UnsupportedVersion";
        case PacketValidationResult::UnknownMessageType:    return "UnknownMessageType";
        case PacketValidationResult::InvalidChannel:        return "InvalidChannel";
        case PacketValidationResult::PayloadLengthMismatch: return "PayloadLengthMismatch";
        default:                                            return "Unknown";
    }
}

class PacketValidator {
public:
    static PacketValidationResult Validate(const uint8_t* data, size_t size,
                                           uint8_t channel, PacketHeader& outHeader);
};

} // namespace wildbound::network

