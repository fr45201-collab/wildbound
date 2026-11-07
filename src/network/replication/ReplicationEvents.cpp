#include "ReplicationEvents.hpp"

namespace wildbound::network {

// 1. EntitySpawnEvent
void EntitySpawnEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(identity.id.value);
    writer.WriteU32(identity.generation);
    writer.WriteU8(static_cast<uint8_t>(type));
    writer.WriteFloat(posX);
    writer.WriteFloat(posY);
    writer.WriteFloat(health);
    writer.WriteU32(customData1);
    writer.WriteU32(customData2);
}

bool EntitySpawnEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(29)) {
        return false;
    }
    const uint32_t rawId = reader.ReadU32();
    const uint32_t gen = reader.ReadU32();
    const uint8_t rawType = reader.ReadU8();
    posX = reader.ReadFloat();
    posY = reader.ReadFloat();
    health = reader.ReadFloat();
    customData1 = reader.ReadU32();
    customData2 = reader.ReadU32();

    identity = NetworkEntityIdentity{ .id = NetworkEntityId{ rawId }, .generation = gen };
    type = static_cast<NetworkEntityType>(rawType);
    return true;
}

// 2. EntityDestroyEvent
void EntityDestroyEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(identity.id.value);
    writer.WriteU32(identity.generation);
    writer.WriteU8(reasonCode);
}

bool EntityDestroyEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) {
        return false;
    }
    const uint32_t rawId = reader.ReadU32();
    const uint32_t gen = reader.ReadU32();
    reasonCode = reader.ReadU8();

    identity = NetworkEntityIdentity{ .id = NetworkEntityId{ rawId }, .generation = gen };
    return true;
}

// 3. CombatEvent
void CombatEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU64(eventId);
    writer.WriteU32(sourceEntity.id.value);
    writer.WriteU32(sourceEntity.generation);
    writer.WriteU32(targetEntity.id.value);
    writer.WriteU32(targetEntity.generation);
    writer.WriteU16(combatAction);
    writer.WriteFloat(damageAmount);
    writer.WriteFloat(remainingHealth);
    writer.WriteU8(isCritical ? 1 : 0);
    writer.WriteU8(isElimination ? 1 : 0);
    writer.WriteU32(serverTick);
}

bool CombatEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(40)) {
        return false;
    }

    eventId = reader.ReadU64();
    const uint32_t sId = reader.ReadU32();
    const uint32_t sGen = reader.ReadU32();
    const uint32_t tId = reader.ReadU32();
    const uint32_t tGen = reader.ReadU32();
    combatAction = reader.ReadU16();
    damageAmount = reader.ReadFloat();
    remainingHealth = reader.ReadFloat();
    isCritical = (reader.ReadU8() != 0);
    isElimination = (reader.ReadU8() != 0);
    serverTick = reader.ReadU32();

    sourceEntity = NetworkEntityIdentity{ .id = NetworkEntityId{ sId }, .generation = sGen };
    targetEntity = NetworkEntityIdentity{ .id = NetworkEntityId{ tId }, .generation = tGen };
    return true;
}

// 4. PickupRequestEvent
void PickupRequestEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(targetLootEntity.id.value);
    writer.WriteU32(targetLootEntity.generation);
    writer.WriteU32(requestingPlayer.id.value);
    writer.WriteU32(requestingPlayer.generation);
    writer.WriteU32(clientTick);
}

bool PickupRequestEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(20)) {
        return false;
    }
    const uint32_t lId = reader.ReadU32();
    const uint32_t lGen = reader.ReadU32();
    const uint32_t pId = reader.ReadU32();
    const uint32_t pGen = reader.ReadU32();
    clientTick = reader.ReadU32();

    targetLootEntity = NetworkEntityIdentity{ .id = NetworkEntityId{ lId }, .generation = lGen };
    requestingPlayer = NetworkEntityIdentity{ .id = NetworkEntityId{ pId }, .generation = pGen };
    return true;
}

// 5. PickupResultEvent
void PickupResultEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU64(eventId);
    writer.WriteU32(targetLootEntity.id.value);
    writer.WriteU32(targetLootEntity.generation);
    writer.WriteU32(recipientPlayer.id.value);
    writer.WriteU32(recipientPlayer.generation);
    writer.WriteU32(itemDefinitionId);
    writer.WriteU32(quantity);
    writer.WriteU8(success ? 1 : 0);
}

bool PickupResultEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(33)) {
        return false;
    }
    eventId = reader.ReadU64();
    const uint32_t lId = reader.ReadU32();
    const uint32_t lGen = reader.ReadU32();
    const uint32_t pId = reader.ReadU32();
    const uint32_t pGen = reader.ReadU32();
    itemDefinitionId = reader.ReadU32();
    quantity = reader.ReadU32();
    success = (reader.ReadU8() != 0);

    targetLootEntity = NetworkEntityIdentity{ .id = NetworkEntityId{ lId }, .generation = lGen };
    recipientPlayer = NetworkEntityIdentity{ .id = NetworkEntityId{ pId }, .generation = pGen };
    return true;
}

// 6. EnvironmentalStateEvent
void EnvironmentalStateEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU64(eventId);
    writer.WriteU32(entityId.id.value);
    writer.WriteU32(entityId.generation);
    writer.WriteU16(stateId);
    writer.WriteU8(active ? 1 : 0);
    writer.WriteU8(destroyed ? 1 : 0);
    writer.WriteU32(stateVersion);
    writer.WriteU32(serverTick);
}

bool EnvironmentalStateEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(26)) {
        return false;
    }
    eventId = reader.ReadU64();
    const uint32_t eId = reader.ReadU32();
    const uint32_t eGen = reader.ReadU32();
    stateId = reader.ReadU16();
    active = (reader.ReadU8() != 0);
    destroyed = (reader.ReadU8() != 0);
    stateVersion = reader.ReadU32();
    serverTick = reader.ReadU32();

    entityId = NetworkEntityIdentity{ .id = NetworkEntityId{ eId }, .generation = eGen };
    return true;
}

// 7. StormStateEvent
void StormStateEvent::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU8(currentPhase);
    writer.WriteFloat(centerX);
    writer.WriteFloat(centerY);
    writer.WriteFloat(currentRadius);
    writer.WriteFloat(targetRadius);
    writer.WriteFloat(dps);
    writer.WriteU32(serverTick);
}

bool StormStateEvent::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(25)) {
        return false;
    }
    currentPhase = reader.ReadU8();
    centerX = reader.ReadFloat();
    centerY = reader.ReadFloat();
    currentRadius = reader.ReadFloat();
    targetRadius = reader.ReadFloat();
    dps = reader.ReadFloat();
    serverTick = reader.ReadU32();
    return true;
}

// EventDeduplicator
EventDeduplicator::EventDeduplicator(size_t maxTrackedEvents)
    : m_maxTracked(std::max<size_t>(16, maxTrackedEvents)) {}

bool EventDeduplicator::IsDuplicateAndRecord(uint64_t eventId) {
    if (eventId == 0) {
        return false; // Zero IDs are untracked
    }

    if (m_eventSet.contains(eventId)) {
        m_duplicatesDetected++;
        return true;
    }

    m_eventSet.insert(eventId);
    m_eventQueue.push_back(eventId);

    while (m_eventQueue.size() > m_maxTracked) {
        const uint64_t oldest = m_eventQueue.front();
        m_eventQueue.pop_front();
        m_eventSet.erase(oldest);
    }

    return false;
}

bool EventDeduplicator::HasSeenEvent(uint64_t eventId) const noexcept {
    if (eventId == 0) return false;
    return m_eventSet.contains(eventId);
}

void EventDeduplicator::Clear() {
    m_eventSet.clear();
    m_eventQueue.clear();
    m_duplicatesDetected = 0;
}

} // namespace wildbound::network
