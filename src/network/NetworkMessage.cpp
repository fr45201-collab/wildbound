#include "NetworkMessage.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::network {

// MsgClientHello
void MsgClientHello::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU16(protocolVersion);
    writer.WriteString(playerName);
    writer.WriteString(authToken);
}

bool MsgClientHello::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(2)) return false;
    protocolVersion = reader.ReadU16();
    playerName = reader.ReadString();
    authToken = reader.ReadString();
    // Security bounds check on untrusted string lengths
    if (playerName.size() > 64 || authToken.size() > 512) {
        return false;
    }
    return true;
}

// MsgServerHello
void MsgServerHello::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(challengeNonce);
    writer.WriteU16(serverTickRate);
}

bool MsgServerHello::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(6)) return false;
    challengeNonce = reader.ReadU32();
    serverTickRate = reader.ReadU16();
    return true;
}

// MsgConnectionAccepted
void MsgConnectionAccepted::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU16(assignedClientId);
    writer.WriteU32(assignedPlayerId.id.value);
    writer.WriteU32(assignedPlayerId.generation);
    writer.WriteString(matchId);
    writer.WriteString(mapName);
}

bool MsgConnectionAccepted::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(10)) return false;
    assignedClientId = reader.ReadU16();
    assignedPlayerId.id.value = reader.ReadU32();
    assignedPlayerId.generation = reader.ReadU32();
    matchId = reader.ReadString();
    mapName = reader.ReadString();
    return true;
}

// MsgConnectionRejected
void MsgConnectionRejected::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU8(rejectionCode);
    writer.WriteString(reason);
}

bool MsgConnectionRejected::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(1)) return false;
    rejectionCode = reader.ReadU8();
    reason = reader.ReadString();
    return true;
}

// MsgPlayerInput
void MsgPlayerInput::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(inputSequence);
    writer.WriteFloat(dirX);
    writer.WriteFloat(dirY);
    writer.WriteFloat(facingAngle);
    writer.WriteU8(actionFlags);
}

bool MsgPlayerInput::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(17)) return false;
    inputSequence = reader.ReadU32();
    dirX = reader.ReadFloat();
    dirY = reader.ReadFloat();
    facingAngle = reader.ReadFloat();
    actionFlags = reader.ReadU8();

    // Numerical sanity and bounds checks (reject NaN, Inf, or unnormalized directions)
    if (!std::isfinite(dirX) || !std::isfinite(dirY) || !std::isfinite(facingAngle)) {
        return false;
    }
    if (std::abs(dirX) > 1.001f || std::abs(dirY) > 1.001f) {
        float len = std::sqrt(dirX * dirX + dirY * dirY);
        if (len > 0.0001f) {
            dirX /= len;
            dirY /= len;
        } else {
            dirX = 0.0f;
            dirY = 0.0f;
        }
    }
    return true;
}

// MsgAttackRequest
void MsgAttackRequest::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU8(weaponSlot);
    writer.WriteFloat(aimAngle);
    writer.WriteU32(clientTick);
}

bool MsgAttackRequest::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(9)) return false;
    weaponSlot = reader.ReadU8();
    aimAngle = reader.ReadFloat();
    clientTick = reader.ReadU32();

    if (!std::isfinite(aimAngle)) {
        return false;
    }
    return true;
}

// MsgPickupRequest
void MsgPickupRequest::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU32(targetItemId.id.value);
    writer.WriteU32(targetItemId.generation);
}

bool MsgPickupRequest::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(8)) return false;
    targetItemId.id.value = reader.ReadU32();
    targetItemId.generation = reader.ReadU32();
    return true;
}

// MsgStateSnapshot
void MsgStateSnapshot::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU64(serverTick);
    writer.WriteU32(acknowledgedInputSeq);
    uint16_t count = static_cast<uint16_t>(entities.size());
    writer.WriteU16(count);

    for (const auto& ent : entities) {
        writer.WriteU32(ent.identity.id.value);
        writer.WriteU32(ent.identity.generation);
        writer.WriteU16(static_cast<uint16_t>(ent.dirtyMask));

        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyPosition)) {
            writer.WriteFloat(ent.posX);
            writer.WriteFloat(ent.posY);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyVelocity)) {
            writer.WriteFloat(ent.velX);
            writer.WriteFloat(ent.velY);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyFacing)) {
            writer.WriteFloat(ent.facingAngle);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyHealthArmor)) {
            writer.WriteFloat(ent.health);
            writer.WriteFloat(ent.armor);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyWeapon)) {
            writer.WriteU16(ent.activeWeaponId);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyAnimState)) {
            writer.WriteU8(ent.animState);
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyStatusFlags)) {
            writer.WriteU8(ent.isAlive ? 1 : 0);
        }
    }
}

bool MsgStateSnapshot::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(14)) return false;
    serverTick = reader.ReadU64();
    acknowledgedInputSeq = reader.ReadU32();
    uint16_t count = reader.ReadU16();

    entities.clear();
    entities.reserve(count);

    for (uint16_t i = 0; i < count; ++i) {
        if (!reader.HasRemaining(10)) return false;
        EntityReplicationState ent;
        ent.identity.id.value = reader.ReadU32();
        ent.identity.generation = reader.ReadU32();
        ent.dirtyMask = static_cast<EntityDirtyMask>(reader.ReadU16());

        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyPosition)) {
            if (!reader.HasRemaining(8)) return false;
            ent.posX = reader.ReadFloat();
            ent.posY = reader.ReadFloat();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyVelocity)) {
            if (!reader.HasRemaining(8)) return false;
            ent.velX = reader.ReadFloat();
            ent.velY = reader.ReadFloat();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyFacing)) {
            if (!reader.HasRemaining(4)) return false;
            ent.facingAngle = reader.ReadFloat();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyHealthArmor)) {
            if (!reader.HasRemaining(8)) return false;
            ent.health = reader.ReadFloat();
            ent.armor = reader.ReadFloat();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyWeapon)) {
            if (!reader.HasRemaining(2)) return false;
            ent.activeWeaponId = reader.ReadU16();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyAnimState)) {
            if (!reader.HasRemaining(1)) return false;
            ent.animState = reader.ReadU8();
        }
        if (HasFlag(ent.dirtyMask, EntityDirtyMask::DirtyStatusFlags)) {
            if (!reader.HasRemaining(1)) return false;
            ent.isAlive = (reader.ReadU8() != 0);
        }

        entities.push_back(ent);
    }
    return true;
}

// MsgMatchStateUpdate
void MsgMatchStateUpdate::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU8(matchState);
    writer.WriteFloat(timeRemaining);
    writer.WriteU16(aliveCount);
}

bool MsgMatchStateUpdate::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(7)) return false;
    matchState = reader.ReadU8();
    timeRemaining = reader.ReadFloat();
    aliveCount = reader.ReadU16();
    return true;
}

// MsgStormStateUpdate
void MsgStormStateUpdate::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU8(currentPhase);
    writer.WriteFloat(centerX);
    writer.WriteFloat(centerY);
    writer.WriteFloat(currentRadius);
    writer.WriteFloat(targetRadius);
}

bool MsgStormStateUpdate::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(17)) return false;
    currentPhase = reader.ReadU8();
    centerX = reader.ReadFloat();
    centerY = reader.ReadFloat();
    currentRadius = reader.ReadFloat();
    targetRadius = reader.ReadFloat();
    return true;
}

// MsgPlayerAppearanceSync
void MsgPlayerAppearanceSync::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU16(targetClientId);
    writer.WriteU32(targetEntityId.id.value);
    writer.WriteU32(targetEntityId.generation);
    writer.WriteU32(loadoutVersion);
    writer.WriteU16(static_cast<uint16_t>(equippedSlots.size()));
    for (const auto& [slot, cosmeticId] : equippedSlots) {
        writer.WriteString(slot);
        writer.WriteString(cosmeticId);
    }
}

bool MsgPlayerAppearanceSync::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(16)) return false;
    targetClientId = reader.ReadU16();
    targetEntityId.id.value = reader.ReadU32();
    targetEntityId.generation = reader.ReadU32();
    loadoutVersion = reader.ReadU32();
    uint16_t count = reader.ReadU16();
    equippedSlots.clear();
    equippedSlots.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        std::string slot = reader.ReadString();
        std::string cosmeticId = reader.ReadString();
        equippedSlots.emplace_back(std::move(slot), std::move(cosmeticId));
    }
    return true;
}

} // namespace wildbound::network

