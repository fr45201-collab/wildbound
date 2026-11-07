#include "ReplicationRegistry.hpp"
#include <algorithm>

namespace wildbound::network {

NetworkEntityIdentity ReplicationRegistry::RegisterEntity(NetworkEntityType type, uint32_t localEntityId, uint32_t currentTick) {
    uint32_t index = 0;
    uint32_t generation = 1;

    if (!m_freeIndices.empty()) {
        index = m_freeIndices.back();
        m_freeIndices.pop_back();
        generation = m_slots[index].generation;
    } else {
        index = static_cast<uint32_t>(m_slots.size());
        m_slots.emplace_back();
        m_slots[index].generation = 1;
        generation = 1;
    }

    auto& slot = m_slots[index];
    slot.inUse = true;

    NetworkEntityIdentity identity{
        .id = NetworkEntityId{ index + 1 },
        .generation = generation
    };

    slot.record = ReplicatedEntityRecord{
        .identity = identity,
        .type = type,
        .localEntityId = localEntityId,
        .isActive = true,
        .creationTick = currentTick
    };

    return identity;
}

bool ReplicationRegistry::RegisterRemoteEntity(NetworkEntityIdentity identity, NetworkEntityType type, uint32_t localEntityId, uint32_t currentTick) {
    if (!identity.IsValid()) {
        return false;
    }

    const uint32_t index = identity.id.value - 1;
    if (index >= m_slots.size()) {
        m_slots.resize(index + 1);
    }

    auto& slot = m_slots[index];
    slot.inUse = true;
    slot.generation = identity.generation;
    slot.record = ReplicatedEntityRecord{
        .identity = identity,
        .type = type,
        .localEntityId = localEntityId,
        .isActive = true,
        .creationTick = currentTick
    };

    return true;
}

bool ReplicationRegistry::UnregisterEntity(NetworkEntityIdentity identity) {
    if (!identity.IsValid()) {
        return false;
    }

    const uint32_t index = identity.id.value - 1;
    if (index >= m_slots.size()) {
        return false;
    }

    auto& slot = m_slots[index];
    if (!slot.inUse || slot.generation != identity.generation) {
        return false;
    }

    slot.inUse = false;
    slot.generation++; // Invalidate existing handles
    if (slot.generation == 0) {
        slot.generation = 1; // Wrap around safely avoiding 0
    }
    slot.record = ReplicatedEntityRecord{};

    m_freeIndices.push_back(index);
    return true;
}

bool ReplicationRegistry::IsEntityValid(NetworkEntityIdentity identity) const noexcept {
    if (!identity.IsValid()) {
        return false;
    }

    const uint32_t index = identity.id.value - 1;
    if (index >= m_slots.size()) {
        return false;
    }

    const auto& slot = m_slots[index];
    return slot.inUse && slot.generation == identity.generation;
}

std::optional<NetworkEntityType> ReplicationRegistry::GetEntityType(NetworkEntityIdentity identity) const noexcept {
    if (!IsEntityValid(identity)) {
        return std::nullopt;
    }
    return m_slots[identity.id.value - 1].record.type;
}

uint32_t ReplicationRegistry::GetLocalEntityId(NetworkEntityIdentity identity) const noexcept {
    if (!IsEntityValid(identity)) {
        return 0;
    }
    return m_slots[identity.id.value - 1].record.localEntityId;
}

bool ReplicationRegistry::SetLocalEntityId(NetworkEntityIdentity identity, uint32_t localEntityId) {
    if (!IsEntityValid(identity)) {
        return false;
    }
    m_slots[identity.id.value - 1].record.localEntityId = localEntityId;
    return true;
}

NetworkEntityIdentity ReplicationRegistry::FindIdentityByLocalId(uint32_t localEntityId, NetworkEntityType type) const noexcept {
    if (localEntityId == 0) {
        return NetworkEntityIdentity{};
    }

    for (const auto& slot : m_slots) {
        if (slot.inUse && slot.record.localEntityId == localEntityId && slot.record.type == type) {
            return slot.record.identity;
        }
    }
    return NetworkEntityIdentity{};
}

size_t ReplicationRegistry::GetActiveEntityCount() const noexcept {
    size_t count = 0;
    for (const auto& slot : m_slots) {
        if (slot.inUse) {
            count++;
        }
    }
    return count;
}

size_t ReplicationRegistry::GetEntityCountByType(NetworkEntityType type) const noexcept {
    size_t count = 0;
    for (const auto& slot : m_slots) {
        if (slot.inUse && slot.record.type == type) {
            count++;
        }
    }
    return count;
}

std::vector<NetworkEntityIdentity> ReplicationRegistry::GetAllEntities() const {
    std::vector<NetworkEntityIdentity> result;
    result.reserve(m_slots.size());
    for (const auto& slot : m_slots) {
        if (slot.inUse) {
            result.push_back(slot.record.identity);
        }
    }
    return result;
}

std::vector<NetworkEntityIdentity> ReplicationRegistry::GetEntitiesByType(NetworkEntityType type) const {
    std::vector<NetworkEntityIdentity> result;
    for (const auto& slot : m_slots) {
        if (slot.inUse && slot.record.type == type) {
            result.push_back(slot.record.identity);
        }
    }
    return result;
}

void ReplicationRegistry::Clear() {
    m_slots.clear();
    m_freeIndices.clear();
}

} // namespace wildbound::network

