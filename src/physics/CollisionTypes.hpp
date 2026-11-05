#pragma once

#include <raylib.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

namespace wildbound::physics {

using ColliderId = uint64_t;
using EntityId = uint32_t;

constexpr ColliderId INVALID_COLLIDER_ID = 0;
constexpr EntityId INVALID_ENTITY_ID = 0;

/**
 * 10 Logical Collision Layers represented as bitflags.
 * Allows fast bitwise mask operations and bilateral filtering.
 */
enum class CollisionLayer : uint32_t {
    None         = 0,
    World        = 1u << 0,  // Static environment geometry, terrain, rocks, trees
    Player       = 1u << 1,  // Human-controlled player character
    Enemy        = 1u << 2,  // Hostile AI creatures (Goblins, Skeletons, etc.)
    Wildlife     = 1u << 3,  // Ambient non-hostile creatures (Birds, Butterflies, etc.)
    Projectile   = 1u << 4,  // Arrows, bolts, spells, throwables
    Pickup       = 1u << 5,  // Collectible world items, ground loot
    Trigger      = 1u << 6,  // Non-physical volume triggers (zones, boundaries)
    Building     = 1u << 7,  // Building walls, structural interiors, roofs
    Hazard       = 1u << 8,  // Environmental hazards, traps, fire, spikes
    Interactable = 1u << 9,  // Chests, doors, levers, quest shrines

    All          = 0xFFFFFFFFu
};

// Bitwise operators for CollisionLayer
inline constexpr CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr CollisionLayer operator&(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr CollisionLayer operator~(CollisionLayer a) {
    return static_cast<CollisionLayer>(~static_cast<uint32_t>(a));
}

inline constexpr CollisionLayer operator^(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

inline constexpr CollisionLayer& operator|=(CollisionLayer& a, CollisionLayer b) {
    a = a | b;
    return a;
}

inline constexpr CollisionLayer& operator&=(CollisionLayer& a, CollisionLayer b) {
    a = a & b;
    return a;
}

inline constexpr CollisionLayer& operator^=(CollisionLayer& a, CollisionLayer b) {
    a = a ^ b;
    return a;
}

/**
 * Helper to convert CollisionLayer enum to readable string name.
 */
inline std::string_view CollisionLayerToString(CollisionLayer layer) {
    switch (layer) {
        case CollisionLayer::World:        return "World";
        case CollisionLayer::Player:       return "Player";
        case CollisionLayer::Enemy:        return "Enemy";
        case CollisionLayer::Wildlife:     return "Wildlife";
        case CollisionLayer::Projectile:   return "Projectile";
        case CollisionLayer::Pickup:       return "Pickup";
        case CollisionLayer::Trigger:      return "Trigger";
        case CollisionLayer::Building:     return "Building";
        case CollisionLayer::Hazard:       return "Hazard";
        case CollisionLayer::Interactable: return "Interactable";
        case CollisionLayer::None:         return "None";
        case CollisionLayer::All:          return "All";
        default:                           return "Custom/Mixed";
    }
}

/**
 * Helper to parse CollisionLayer enum from string name.
 */
inline CollisionLayer StringToCollisionLayer(std::string_view str) {
    if (str == "World")        return CollisionLayer::World;
    if (str == "Player")       return CollisionLayer::Player;
    if (str == "Enemy")        return CollisionLayer::Enemy;
    if (str == "Wildlife")     return CollisionLayer::Wildlife;
    if (str == "Projectile")   return CollisionLayer::Projectile;
    if (str == "Pickup")       return CollisionLayer::Pickup;
    if (str == "Trigger")      return CollisionLayer::Trigger;
    if (str == "Building")     return CollisionLayer::Building;
    if (str == "Hazard")       return CollisionLayer::Hazard;
    if (str == "Interactable") return CollisionLayer::Interactable;
    if (str == "All")          return CollisionLayer::All;
    return CollisionLayer::None;
}

/**
 * Classifier for whether a collider is a physical blocking obstacle or a trigger volume.
 */
enum class ColliderType : uint8_t {
    Physical = 0, // Blocks movement, resolves contact penetration
    Trigger  = 1  // Non-blocking, detects overlap entry/stay/exit
};

/**
 * Returns default bilateral collision mask for a given primary layer.
 */
inline uint32_t GetDefaultCollisionMask(CollisionLayer layer) {
    switch (layer) {
        case CollisionLayer::World:
            // World blocks Player, Enemy, Wildlife, Projectiles
            return static_cast<uint32_t>(CollisionLayer::Player |
                                         CollisionLayer::Enemy |
                                         CollisionLayer::Wildlife |
                                         CollisionLayer::Projectile);

        case CollisionLayer::Player:
            // Player collides with World, Enemy, Wildlife, Pickups, Triggers, Buildings, Hazards, Interactables
            return static_cast<uint32_t>(CollisionLayer::World |
                                         CollisionLayer::Enemy |
                                         CollisionLayer::Wildlife |
                                         CollisionLayer::Pickup |
                                         CollisionLayer::Trigger |
                                         CollisionLayer::Building |
                                         CollisionLayer::Hazard |
                                         CollisionLayer::Interactable);

        case CollisionLayer::Enemy:
            // Enemy collides with World, Player, other Enemies, Projectiles, Buildings, Hazards
            return static_cast<uint32_t>(CollisionLayer::World |
                                         CollisionLayer::Player |
                                         CollisionLayer::Enemy |
                                         CollisionLayer::Projectile |
                                         CollisionLayer::Building |
                                         CollisionLayer::Hazard);

        case CollisionLayer::Wildlife:
            // Wildlife collides with World, Player, Buildings
            return static_cast<uint32_t>(CollisionLayer::World |
                                         CollisionLayer::Player |
                                         CollisionLayer::Building);

        case CollisionLayer::Projectile:
            // Projectiles hit World, Player, Enemy, Buildings
            return static_cast<uint32_t>(CollisionLayer::World |
                                         CollisionLayer::Player |
                                         CollisionLayer::Enemy |
                                         CollisionLayer::Building);

        case CollisionLayer::Pickup:
            // Ground items only interact with Player
            return static_cast<uint32_t>(CollisionLayer::Player);

        case CollisionLayer::Trigger:
            // Triggers detect Player and Enemy presence
            return static_cast<uint32_t>(CollisionLayer::Player |
                                         CollisionLayer::Enemy);

        case CollisionLayer::Building:
            // Buildings block Player, Enemy, Wildlife, Projectiles
            return static_cast<uint32_t>(CollisionLayer::Player |
                                         CollisionLayer::Enemy |
                                         CollisionLayer::Wildlife |
                                         CollisionLayer::Projectile);

        case CollisionLayer::Hazard:
            // Hazards affect Player and Enemy
            return static_cast<uint32_t>(CollisionLayer::Player |
                                         CollisionLayer::Enemy);

        case CollisionLayer::Interactable:
            // Interactable targets (chests, doors) interact with Player
            return static_cast<uint32_t>(CollisionLayer::Player);

        default:
            return 0u;
    }
}

} // namespace wildbound::physics

