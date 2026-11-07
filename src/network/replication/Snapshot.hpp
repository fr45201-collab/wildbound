#pragma once

#include "NetworkEntityTypes.hpp"
#include "../NetworkProtocol.hpp"
#include "../ReplicationTypes.hpp"
#include <vector>
#include <cstdint>

namespace wildbound::network {

/// Comprehensive record representing the state of an individual replicated entity
struct EntitySnapshotRecord {
    NetworkEntityIdentity identity{};
    NetworkEntityType type{ NetworkEntityType::Player };
    EntityDirtyMask dirtyMask{ EntityDirtyMask::All };

    // Transform
    float posX{ 0.0f };
    float posY{ 0.0f };
    float velX{ 0.0f };
    float velY{ 0.0f };
    float facingAngle{ 0.0f };

    // Vitals & Status
    float health{ 100.0f };
    float maxHealth{ 100.0f };
    float armor{ 0.0f };
    uint8_t animState{ 0 };
    uint8_t statusFlags{ 1 }; // bit 0: isAlive, bit 1: isMoving, bit 2: isRunning
    uint32_t customData1{ 0 }; // e.g. itemDefId or stateId
    uint32_t customData2{ 0 }; // e.g. quantity or stateVersion

    [[nodiscard]] bool IsAlive() const noexcept { return (statusFlags & 0x01) != 0; }
    void SetAlive(bool alive) noexcept {
        if (alive) statusFlags |= 0x01;
        else statusFlags &= ~0x01;
    }

    [[nodiscard]] bool IsMoving() const noexcept { return (statusFlags & 0x02) != 0; }
    void SetMoving(bool moving) noexcept {
        if (moving) statusFlags |= 0x02;
        else statusFlags &= ~0x02;
    }

    [[nodiscard]] bool IsRunning() const noexcept { return (statusFlags & 0x04) != 0; }
    void SetRunning(bool running) noexcept {
        if (running) statusFlags |= 0x04;
        else statusFlags &= ~0x04;
    }

    void Serialize(ByteStreamWriter& writer, bool isDelta) const;
    bool Deserialize(ByteStreamReader& reader, bool isDelta);
};

/// World state snapshot containing one or more replicated entity records
struct WorldSnapshot {
    TickType serverTick{ 0 };
    SequenceNumber acknowledgedInputSeq{ 0 };
    float timestamp{ 0.0f };
    bool isDelta{ false };
    TickType baselineTick{ 0 };
    std::vector<EntitySnapshotRecord> entities;

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);

    [[nodiscard]] const EntitySnapshotRecord* FindEntity(NetworkEntityIdentity id) const noexcept {
        for (const auto& ent : entities) {
            if (ent.identity == id) return &ent;
        }
        return nullptr;
    }
};

} // namespace wildbound::network

