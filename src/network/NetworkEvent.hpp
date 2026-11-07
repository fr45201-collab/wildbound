#pragma once

#include "NetworkTypes.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace wildbound::network {

enum class NetworkEventCategory : uint8_t {
    Connection = 0,
    Match      = 1,
    Entity     = 2,
    Gameplay   = 3,
    World      = 4
};

enum class NetworkEventType : uint16_t {
    None = 0,
    // Connection
    ClientConnected,
    ClientDisconnected,
    AuthChallenge,
    // Match
    MatchCountdown,
    MatchStarted,
    MatchCompleted,
    // Entity
    EntitySpawned,
    EntityDestroyed,
    EntityOwnershipAssigned,
    // Gameplay
    AttackRequest,
    WeaponFired,
    DamageApplied,
    PlayerEliminated,
    PickupRequest,
    ItemCollected,
    ItemDropped,
    // World
    StormPhaseShift,
    DoorToggled,
    ContainerOpened
};

struct NetworkEventEnvelope {
    uint32_t sequenceNumber{ 0 };
    ClientId senderClientId{ 0 };
    NetworkEntityIdentity actorId{};
    NetworkEventType eventType{ NetworkEventType::None };
    NetworkEventCategory category{ NetworkEventCategory::Gameplay };
    uint64_t serverTimestampMs{ 0 };
    std::vector<uint8_t> payload;

    [[nodiscard]] bool IsValid() const noexcept {
        return eventType != NetworkEventType::None;
    }
};

} // namespace wildbound::network

