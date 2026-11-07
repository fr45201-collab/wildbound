#pragma once

#include "../NetworkTypes.hpp"
#include "../ReplicationTypes.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace wildbound::network {

/// Distinct categories of replicated network entities
enum class NetworkEntityType : uint8_t {
    Player              = 0,
    Enemy               = 1,
    Projectile          = 2,
    Loot                = 3,
    Interactable        = 4,
    EnvironmentalObject = 5,
    Count               = 6
};

inline constexpr std::string_view NetworkEntityTypeToString(NetworkEntityType type) noexcept {
    switch (type) {
        case NetworkEntityType::Player:              return "Player";
        case NetworkEntityType::Enemy:               return "Enemy";
        case NetworkEntityType::Projectile:          return "Projectile";
        case NetworkEntityType::Loot:                return "Loot";
        case NetworkEntityType::Interactable:        return "Interactable";
        case NetworkEntityType::EnvironmentalObject: return "EnvironmentalObject";
        default:                                     return "Unknown";
    }
}

/// Authoritative Player movement and locomotion state
struct PlayerNetworkState {
    NetworkEntityIdentity identity{};
    float posX{ 0.0f };
    float posY{ 0.0f };
    float velX{ 0.0f };
    float velY{ 0.0f };
    float rotation{ 0.0f };
    bool isMoving{ false };
    bool isRunning{ false };
    uint32_t inputSequence{ 0 };
    uint32_t serverTick{ 0 };
};

/// Player animation synchronization state
struct PlayerAnimationState {
    NetworkEntityIdentity identity{};
    uint16_t animationId{ 0 };
    uint8_t direction{ 0 };
    bool looping{ true };
    uint32_t animationStartTick{ 0 };
    uint32_t serverTick{ 0 };
};

/// Server-controlled Enemy state
struct EnemyNetworkState {
    NetworkEntityIdentity identity{};
    float posX{ 0.0f };
    float posY{ 0.0f };
    float velX{ 0.0f };
    float velY{ 0.0f };
    float facingAngle{ 0.0f };
    float health{ 100.0f };
    float maxHealth{ 100.0f };
    uint8_t aiState{ 0 };
    uint8_t animState{ 0 };
    bool isAlive{ true };
    uint32_t serverTick{ 0 };
};

/// Replicated Loot / World Item state
struct LootNetworkState {
    NetworkEntityIdentity identity{};
    uint32_t itemDefinitionId{ 0 };
    float posX{ 0.0f };
    float posY{ 0.0f };
    uint32_t quantity{ 1 };
    uint8_t rarity{ 0 };
    bool isAvailable{ true };
    uint32_t serverTick{ 0 };
};

/// Replicated Environmental & Interactive state (doors, chests, traps, storm)
struct EnvironmentalNetworkState {
    NetworkEntityIdentity identity{};
    uint16_t stateId{ 0 };
    bool active{ false };
    bool destroyed{ false };
    uint32_t stateVersion{ 0 };
    uint32_t serverTick{ 0 };
};

/// Combat outcome network event
struct CombatNetworkEvent {
    uint64_t eventId{ 0 };
    NetworkEntityIdentity sourceEntity{};
    NetworkEntityIdentity targetEntity{};
    uint16_t eventType{ 0 }; // 1=AttackStart, 2=HitConfirmed, 3=DamageApplied, 4=Eliminated
    float damage{ 0.0f };
    uint32_t serverTick{ 0 };
};

} // namespace wildbound::network

