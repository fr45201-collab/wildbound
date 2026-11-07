#pragma once

#include "NetworkTypes.hpp"
#include <cstdint>
#include <string>

namespace wildbound::network {

/// Component Dirty Bitmask for delta snapshot serialization
enum class EntityDirtyMask : uint16_t {
    Clean             = 0x0000,
    DirtyPosition     = 1 << 0,  // x, y coordinates
    DirtyVelocity     = 1 << 1,  // vx, vy coordinates
    DirtyFacing       = 1 << 2,  // aim/look angle
    DirtyAnimState    = 1 << 3,  // idle, walk, run, attack
    DirtyHealthArmor  = 1 << 4,  // health, armor
    DirtyWeapon       = 1 << 5,  // active weapon slot & weapon id
    DirtyAmmo         = 1 << 6,  // ammo count (owner only)
    DirtyInventory    = 1 << 7,  // items in backpack (owner only)
    DirtyCosmetics    = 1 << 8,  // hair, skin, costume overrides
    DirtyStatusFlags  = 1 << 9,  // alive, spectating, knocked
    All               = 0xFFFF
};

constexpr EntityDirtyMask operator|(EntityDirtyMask a, EntityDirtyMask b) noexcept {
    return static_cast<EntityDirtyMask>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

constexpr EntityDirtyMask operator&(EntityDirtyMask a, EntityDirtyMask b) noexcept {
    return static_cast<EntityDirtyMask>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

constexpr EntityDirtyMask operator~(EntityDirtyMask a) noexcept {
    return static_cast<EntityDirtyMask>(~static_cast<uint16_t>(a));
}

constexpr bool HasFlag(EntityDirtyMask mask, EntityDirtyMask flag) noexcept {
    return (static_cast<uint16_t>(mask) & static_cast<uint16_t>(flag)) != 0;
}

/// Replication Priority Tiers
enum class ReplicationPriority : uint8_t {
    Critical = 0, // Local player avatar (60 Hz)
    High     = 1, // Nearby enemies / bots (30 Hz)
    Medium   = 2, // Ground loot, interactive chests (10 Hz)
    Low      = 3, // Distant players, teammates (5 Hz)
    Culled   = 4  // Outside interest radius (0 Hz)
};

/// Compact replicated entity snapshot state
struct EntityReplicationState {
    NetworkEntityIdentity identity;
    EntityDirtyMask dirtyMask{ EntityDirtyMask::Clean };

    // Transform
    float posX{ 0.0f };
    float posY{ 0.0f };
    float velX{ 0.0f };
    float velY{ 0.0f };
    float facingAngle{ 0.0f };

    // Combat & Vitals
    float health{ 100.0f };
    float armor{ 0.0f };
    uint16_t activeWeaponId{ 0 };
    uint8_t animState{ 0 };
    bool isAlive{ true };

    [[nodiscard]] bool IsDirty() const noexcept {
        return dirtyMask != EntityDirtyMask::Clean;
    }
};

} // namespace wildbound::network

