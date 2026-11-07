#include "Snapshot.hpp"

namespace wildbound::network {

void EntitySnapshotRecord::Serialize(ByteStreamWriter& writer, bool isDelta) const {
    writer.WriteU32(identity.id.value);
    writer.WriteU32(identity.generation);
    writer.WriteU8(static_cast<uint8_t>(type));

    if (isDelta) {
        writer.WriteU16(static_cast<uint16_t>(dirtyMask));

        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyPosition)) {
            writer.WriteFloat(posX);
            writer.WriteFloat(posY);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyVelocity)) {
            writer.WriteFloat(velX);
            writer.WriteFloat(velY);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyFacing)) {
            writer.WriteFloat(facingAngle);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyHealthArmor)) {
            writer.WriteFloat(health);
            writer.WriteFloat(maxHealth);
            writer.WriteFloat(armor);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyAnimState)) {
            writer.WriteU8(animState);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyStatusFlags)) {
            writer.WriteU8(statusFlags);
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyWeapon)) {
            writer.WriteU32(customData1);
            writer.WriteU32(customData2);
        }
    } else {
        // Full snapshot
        writer.WriteFloat(posX);
        writer.WriteFloat(posY);
        writer.WriteFloat(velX);
        writer.WriteFloat(velY);
        writer.WriteFloat(facingAngle);
        writer.WriteFloat(health);
        writer.WriteFloat(maxHealth);
        writer.WriteFloat(armor);
        writer.WriteU8(animState);
        writer.WriteU8(statusFlags);
        writer.WriteU32(customData1);
        writer.WriteU32(customData2);
    }
}

bool EntitySnapshotRecord::Deserialize(ByteStreamReader& reader, bool isDelta) {
    if (!reader.HasRemaining(9)) {
        return false;
    }

    const uint32_t rawId = reader.ReadU32();
    const uint32_t gen = reader.ReadU32();
    const uint8_t rawType = reader.ReadU8();

    identity = NetworkEntityIdentity{
        .id = NetworkEntityId{ rawId },
        .generation = gen
    };
    type = static_cast<NetworkEntityType>(rawType);

    if (isDelta) {
        if (!reader.HasRemaining(2)) return false;
        dirtyMask = static_cast<EntityDirtyMask>(reader.ReadU16());

        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyPosition)) {
            if (!reader.HasRemaining(8)) return false;
            posX = reader.ReadFloat();
            posY = reader.ReadFloat();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyVelocity)) {
            if (!reader.HasRemaining(8)) return false;
            velX = reader.ReadFloat();
            velY = reader.ReadFloat();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyFacing)) {
            if (!reader.HasRemaining(4)) return false;
            facingAngle = reader.ReadFloat();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyHealthArmor)) {
            if (!reader.HasRemaining(12)) return false;
            health = reader.ReadFloat();
            maxHealth = reader.ReadFloat();
            armor = reader.ReadFloat();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyAnimState)) {
            if (!reader.HasRemaining(1)) return false;
            animState = reader.ReadU8();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyStatusFlags)) {
            if (!reader.HasRemaining(1)) return false;
            statusFlags = reader.ReadU8();
        }
        if (HasFlag(dirtyMask, EntityDirtyMask::DirtyWeapon)) {
            if (!reader.HasRemaining(8)) return false;
            customData1 = reader.ReadU32();
            customData2 = reader.ReadU32();
        }
    } else {
        dirtyMask = EntityDirtyMask::All;
        if (!reader.HasRemaining(42)) {
            return false;
        }
        posX = reader.ReadFloat();
        posY = reader.ReadFloat();
        velX = reader.ReadFloat();
        velY = reader.ReadFloat();
        facingAngle = reader.ReadFloat();
        health = reader.ReadFloat();
        maxHealth = reader.ReadFloat();
        armor = reader.ReadFloat();
        animState = reader.ReadU8();
        statusFlags = reader.ReadU8();
        customData1 = reader.ReadU32();
        customData2 = reader.ReadU32();
    }

    return true;
}

void WorldSnapshot::Serialize(ByteStreamWriter& writer) const {
    writer.WriteU64(serverTick);
    writer.WriteU32(acknowledgedInputSeq);
    writer.WriteFloat(timestamp);
    writer.WriteU8(isDelta ? 1 : 0);
    writer.WriteU64(baselineTick);

    writer.WriteU16(static_cast<uint16_t>(entities.size()));
    for (const auto& ent : entities) {
        ent.Serialize(writer, isDelta);
    }
}

bool WorldSnapshot::Deserialize(ByteStreamReader& reader) {
    if (!reader.HasRemaining(23)) {
        return false;
    }

    serverTick = reader.ReadU64();
    acknowledgedInputSeq = reader.ReadU32();
    timestamp = reader.ReadFloat();
    isDelta = (reader.ReadU8() != 0);
    baselineTick = reader.ReadU64();
    const uint16_t count = reader.ReadU16();

    entities.clear();
    entities.resize(count);

    for (uint16_t i = 0; i < count; ++i) {
        if (!entities[i].Deserialize(reader, isDelta)) {
            return false;
        }
    }

    return true;
}

} // namespace wildbound::network
