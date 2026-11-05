#pragma once

#include "CollisionTypes.hpp"
#include <raylib.h>
#include <functional>
#include <string_view>

namespace wildbound::physics {

/**
 * Lifecycle event types for physical collisions and trigger volume overlaps.
 */
enum class CollisionEventType : uint8_t {
    CollisionEnter = 0,
    CollisionStay  = 1,
    CollisionExit  = 2,
    TriggerEnter   = 3,
    TriggerStay    = 4,
    TriggerExit    = 5
};

inline std::string_view CollisionEventTypeToString(CollisionEventType type) {
    switch (type) {
        case CollisionEventType::CollisionEnter: return "CollisionEnter";
        case CollisionEventType::CollisionStay:  return "CollisionStay";
        case CollisionEventType::CollisionExit:  return "CollisionExit";
        case CollisionEventType::TriggerEnter:   return "TriggerEnter";
        case CollisionEventType::TriggerStay:    return "TriggerStay";
        case CollisionEventType::TriggerExit:    return "TriggerExit";
        default:                                 return "Unknown";
    }
}

/**
 * Authoritative record of an interaction between two colliders.
 */
struct CollisionEvent {
    CollisionEventType type = CollisionEventType::CollisionEnter;
    ColliderId colliderIdA = INVALID_COLLIDER_ID;
    ColliderId colliderIdB = INVALID_COLLIDER_ID;
    EntityId entityIdA = INVALID_ENTITY_ID;
    EntityId entityIdB = INVALID_ENTITY_ID;
    CollisionLayer layerA = CollisionLayer::None;
    CollisionLayer layerB = CollisionLayer::None;
    Rectangle intersection{ 0.0f, 0.0f, 0.0f, 0.0f };
    bool isTrigger = false;

    // Helper to check if a specific entity participated in this event
    bool InvolvesEntity(EntityId id) const noexcept {
        return entityIdA == id || entityIdB == id;
    }

    // Helper to check if a specific collider participated in this event
    bool InvolvesCollider(ColliderId id) const noexcept {
        return colliderIdA == id || colliderIdB == id;
    }

    // Helper to get the other entity ID relative to an observer entity
    EntityId GetOtherEntity(EntityId observerId) const noexcept {
        return (entityIdA == observerId) ? entityIdB : entityIdA;
    }

    // Helper to get the other collider ID relative to an observer collider
    ColliderId GetOtherCollider(ColliderId observerId) const noexcept {
        return (colliderIdA == observerId) ? colliderIdB : colliderIdA;
    }
};

/**
 * Interface for components or systems subscribing to collision/trigger events.
 */
class ICollisionListener {
public:
    virtual ~ICollisionListener() = default;
    virtual void OnCollisionEvent(const CollisionEvent& event) = 0;
};

using CollisionCallback = std::function<void(const CollisionEvent&)>;

} // namespace wildbound::physics

