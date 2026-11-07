#pragma once

#include <cstdint>
#include <compare>
#include <string>

namespace wildbound::network {

/// Strong-typed wrapper for 32-bit network entity ID
struct NetworkEntityId {
    uint32_t value = 0;

    [[nodiscard]] constexpr bool IsValid() const noexcept { return value != 0; }
    [[nodiscard]] constexpr auto operator<=>(const NetworkEntityId&) const = default;
};

/// 64-bit Combined Handle: ID (lower 32 bits) + Generation (upper 32 bits)
struct NetworkEntityIdentity {
    NetworkEntityId id{ 0 };
    uint32_t generation{ 0 };

    [[nodiscard]] constexpr bool IsValid() const noexcept {
        return id.IsValid() && generation > 0;
    }

    [[nodiscard]] constexpr uint64_t ToHandle() const noexcept {
        return (static_cast<uint64_t>(generation) << 32) | static_cast<uint64_t>(id.value);
    }

    [[nodiscard]] static constexpr NetworkEntityIdentity FromHandle(uint64_t handle) noexcept {
        return NetworkEntityIdentity{
            .id = NetworkEntityId{ static_cast<uint32_t>(handle & 0xFFFFFFFF) },
            .generation = static_cast<uint32_t>(handle >> 32)
        };
    }

    [[nodiscard]] constexpr auto operator<=>(const NetworkEntityIdentity&) const = default;
};

/// Unique 16-bit client identifier
using ClientId = uint16_t;
inline constexpr ClientId SERVER_CLIENT_ID = 0;
inline constexpr ClientId INVALID_CLIENT_ID = 0xFFFF;

/// Monotonically increasing simulation tick
using TickType = uint64_t;

/// Sequence number for packet sequencing and ACK tracking
using SequenceNumber = uint32_t;

/// High-level role of the current engine instance
enum class NetworkRole : uint8_t {
    None         = 0,
    Client       = 1,
    Server       = 2,
    ListenServer = 3
};

/// 5 Virtual Channels for Transport Separation
enum class ChannelType : uint8_t {
    Control        = 0,  // Reliable in-order (Handshake, Connect, Disconnect)
    ReliableEvents = 1,  // Reliable in-order (Combat actions, damage, pickups)
    PlayerInput    = 2,  // Unreliable sequenced (60 Hz player movement commands)
    Replication    = 3,  // Unreliable sequenced (30 Hz world state snapshots)
    MatchState     = 4,  // Reliable in-order (Storm phases, countdowns, victory)
    Count          = 5
};

/// Delivery guarantees for transport packets
enum class DeliveryMode : uint8_t {
    ReliableOrdered     = 0,
    UnreliableSequenced = 1
};

} // namespace wildbound::network

