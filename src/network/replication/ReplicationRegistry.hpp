#pragma once

#include "NetworkEntityTypes.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <optional>

namespace wildbound::network {

struct ReplicatedEntityRecord {
    NetworkEntityIdentity identity{};
    NetworkEntityType type{ NetworkEntityType::Player };
    uint32_t localEntityId{ 0 };
    bool isActive{ true };
    uint32_t creationTick{ 0 };
};

/**
 * Authoritative registry managing network entity identities and mappings.
 * Generates versioned handles (ID + Generation) to prevent stale handle reuse.
 */
class ReplicationRegistry {
public:
    ReplicationRegistry() = default;
    ~ReplicationRegistry() = default;

    ReplicationRegistry(const ReplicationRegistry&) = delete;
    ReplicationRegistry& operator=(const ReplicationRegistry&) = delete;
    ReplicationRegistry(ReplicationRegistry&&) noexcept = default;
    ReplicationRegistry& operator=(ReplicationRegistry&&) noexcept = default;

    /// Server: Allocate and register a new authoritative network entity
    [[nodiscard]] NetworkEntityIdentity RegisterEntity(NetworkEntityType type, uint32_t localEntityId = 0, uint32_t currentTick = 0);

    /// Client: Register a replicated entity assigned by the authoritative server
    bool RegisterRemoteEntity(NetworkEntityIdentity identity, NetworkEntityType type, uint32_t localEntityId = 0, uint32_t currentTick = 0);

    /// Server/Client: Remove an entity and increment generation counter to invalidate existing handles
    bool UnregisterEntity(NetworkEntityIdentity identity);

    /// Query whether a specific identity exists and matches the current generation
    [[nodiscard]] bool IsEntityValid(NetworkEntityIdentity identity) const noexcept;

    /// Retrieve entity type if valid
    [[nodiscard]] std::optional<NetworkEntityType> GetEntityType(NetworkEntityIdentity identity) const noexcept;

    /// Retrieve mapped local entity ID
    [[nodiscard]] uint32_t GetLocalEntityId(NetworkEntityIdentity identity) const noexcept;

    /// Update mapped local entity ID
    bool SetLocalEntityId(NetworkEntityIdentity identity, uint32_t localEntityId);

    /// Find identity by local entity ID and type
    [[nodiscard]] NetworkEntityIdentity FindIdentityByLocalId(uint32_t localEntityId, NetworkEntityType type) const noexcept;

    /// Get total active registered entities
    [[nodiscard]] size_t GetActiveEntityCount() const noexcept;

    /// Get active entities count filtered by type
    [[nodiscard]] size_t GetEntityCountByType(NetworkEntityType type) const noexcept;

    /// Get list of all currently active entity identities
    [[nodiscard]] std::vector<NetworkEntityIdentity> GetAllEntities() const;

    /// Get list of active identities filtered by type
    [[nodiscard]] std::vector<NetworkEntityIdentity> GetEntitiesByType(NetworkEntityType type) const;

    /// Clear all registered entities and reset slot allocations
    void Clear();

private:
    struct Slot {
        uint32_t generation{ 1 };
        bool inUse{ false };
        ReplicatedEntityRecord record{};
    };

    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_freeIndices;
    uint32_t m_nextNewId{ 1 };
};

} // namespace wildbound::network

