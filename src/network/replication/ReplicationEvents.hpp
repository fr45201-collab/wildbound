#pragma once

#include "NetworkEntityTypes.hpp"
#include "../NetworkProtocol.hpp"
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <deque>

namespace wildbound::network {

enum class ReplicationEventType : uint16_t {
    EntitySpawn              = 0x0100,
    EntityDestroy            = 0x0101,
    CombatAction             = 0x0102,
    CombatDamage             = 0x0103,
    CombatElimination        = 0x0104,
    PickupRequest            = 0x0105,
    PickupResult             = 0x0106,
    EnvironmentalStateChange = 0x0107,
    StormStateChange         = 0x0108
};

/// 1. Entity Spawn Event
struct EntitySpawnEvent {
    NetworkEntityIdentity identity{};
    NetworkEntityType type{ NetworkEntityType::Player };
    float posX{ 0.0f };
    float posY{ 0.0f };
    float health{ 100.0f };
    uint32_t customData1{ 0 };
    uint32_t customData2{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 2. Entity Destroy Event
struct EntityDestroyEvent {
    NetworkEntityIdentity identity{};
    uint8_t reasonCode{ 0 }; // 0=Eliminated, 1=PickedUp, 2=Despawned, 3=OutOfRelevancy

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 3. Combat Event (Attacks, Hits, Damage, Eliminations)
struct CombatEvent {
    uint64_t eventId{ 0 };
    NetworkEntityIdentity sourceEntity{};
    NetworkEntityIdentity targetEntity{};
    uint16_t combatAction{ 0 }; // 1=Melee, 2=Ranged, 3=AreaOfEffect
    float damageAmount{ 0.0f };
    float remainingHealth{ 0.0f };
    bool isCritical{ false };
    bool isElimination{ false };
    uint32_t serverTick{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 4. Loot Pickup Request Event (Client -> Server)
struct PickupRequestEvent {
    NetworkEntityIdentity targetLootEntity{};
    NetworkEntityIdentity requestingPlayer{};
    uint32_t clientTick{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 5. Loot Pickup Result Event (Server -> Clients)
struct PickupResultEvent {
    uint64_t eventId{ 0 };
    NetworkEntityIdentity targetLootEntity{};
    NetworkEntityIdentity recipientPlayer{};
    uint32_t itemDefinitionId{ 0 };
    uint32_t quantity{ 1 };
    bool success{ false };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 6. Environmental State Change Event (Server -> Clients)
struct EnvironmentalStateEvent {
    uint64_t eventId{ 0 };
    NetworkEntityIdentity entityId{};
    uint16_t stateId{ 0 };
    bool active{ false };
    bool destroyed{ false };
    uint32_t stateVersion{ 0 };
    uint32_t serverTick{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/// 7. Storm State Change Event (Server -> Clients)
struct StormStateEvent {
    uint8_t currentPhase{ 0 };
    float centerX{ 0.0f };
    float centerY{ 0.0f };
    float currentRadius{ 0.0f };
    float targetRadius{ 0.0f };
    float dps{ 0.0f };
    uint32_t serverTick{ 0 };

    void Serialize(ByteStreamWriter& writer) const;
    bool Deserialize(ByteStreamReader& reader);
};

/**
 * Deduplicator cache for network events to enforce application-level idempotency.
 * Prevents retransmitted or duplicate reliable packets from causing duplicated damage or item pickups.
 */
class EventDeduplicator {
public:
    explicit EventDeduplicator(size_t maxTrackedEvents = 512);
    ~EventDeduplicator() = default;

    /// Returns true if event has ALREADY been processed (duplicate). Returns false and records event if new.
    bool IsDuplicateAndRecord(uint64_t eventId);

    /// Query without recording
    [[nodiscard]] bool HasSeenEvent(uint64_t eventId) const noexcept;

    /// Clear tracked history
    void Clear();

    [[nodiscard]] size_t GetTrackedCount() const noexcept { return m_eventSet.size(); }
    [[nodiscard]] uint64_t GetDuplicateCount() const noexcept { return m_duplicatesDetected; }

private:
    size_t m_maxTracked{ 512 };
    std::unordered_set<uint64_t> m_eventSet;
    std::deque<uint64_t> m_eventQueue;
    uint64_t m_duplicatesDetected{ 0 };
};

} // namespace wildbound::network

