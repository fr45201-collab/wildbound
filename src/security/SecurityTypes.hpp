#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace wildbound::security {

enum class SecurityEventType : uint16_t {
    None = 0,
    InvalidMovementSpeed,
    TeleportAttempt,
    NonFiniteCoordinates,
    AttackCooldownViolation,
    AttackRangeViolation,
    InvalidWeaponReference,
    DuplicateLootClaim,
    LootRangeViolation,
    InvalidInventoryAction,
    MalformedPacket,
    PacketSizeViolation,
    RateLimitExceeded,
    UnauthorizedCosmetic,
    MatchStateTampering,
    ProtocolViolation
};

enum class SecuritySeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Violation = 2,
    Critical = 3
};

enum class SecurityAction : uint8_t {
    Allow = 0,
    Reject = 1,
    RateLimit = 2,
    Disconnect = 3
};

struct SecurityEvent {
    SecurityEventType type{ SecurityEventType::None };
    SecuritySeverity severity{ SecuritySeverity::Warning };
    uint16_t clientId{ 0 };
    uint32_t participantId{ 0 };
    std::string matchId;
    uint64_t timestampMs{ 0 };
    std::string details;
    bool actionRejected{ true };
};

[[nodiscard]] constexpr std::string_view ToString(SecurityEventType type) noexcept {
    switch (type) {
        case SecurityEventType::InvalidMovementSpeed:    return "InvalidMovementSpeed";
        case SecurityEventType::TeleportAttempt:         return "TeleportAttempt";
        case SecurityEventType::NonFiniteCoordinates:    return "NonFiniteCoordinates";
        case SecurityEventType::AttackCooldownViolation: return "AttackCooldownViolation";
        case SecurityEventType::AttackRangeViolation:    return "AttackRangeViolation";
        case SecurityEventType::InvalidWeaponReference:  return "InvalidWeaponReference";
        case SecurityEventType::DuplicateLootClaim:      return "DuplicateLootClaim";
        case SecurityEventType::LootRangeViolation:      return "LootRangeViolation";
        case SecurityEventType::InvalidInventoryAction:  return "InvalidInventoryAction";
        case SecurityEventType::MalformedPacket:         return "MalformedPacket";
        case SecurityEventType::PacketSizeViolation:     return "PacketSizeViolation";
        case SecurityEventType::RateLimitExceeded:       return "RateLimitExceeded";
        case SecurityEventType::UnauthorizedCosmetic:    return "UnauthorizedCosmetic";
        case SecurityEventType::MatchStateTampering:     return "MatchStateTampering";
        case SecurityEventType::ProtocolViolation:       return "ProtocolViolation";
        default:                                         return "None";
    }
}

[[nodiscard]] constexpr std::string_view ToString(SecuritySeverity severity) noexcept {
    switch (severity) {
        case SecuritySeverity::Info:      return "Info";
        case SecuritySeverity::Warning:   return "Warning";
        case SecuritySeverity::Violation: return "Violation";
        case SecuritySeverity::Critical:  return "Critical";
        default:                          return "Unknown";
    }
}

} // namespace wildbound::security

