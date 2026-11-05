#pragma once

#include "CollisionTypes.hpp"

namespace wildbound::physics {

/**
 * High-performance bilateral collision filtering engine.
 * Ensures consistent, symmetric interaction between two colliders:
 * (A.mask & B.layer) != 0 AND (B.mask & A.layer) != 0
 */
class CollisionFilter {
public:
    /**
     * Evaluates bilateral layer and mask compatibility between two entities.
     * Both sides must consent to the interaction.
     */
    static constexpr bool ShouldCollide(CollisionLayer layerA, uint32_t maskA,
                                        CollisionLayer layerB, uint32_t maskB) noexcept {
        if (layerA == CollisionLayer::None || layerB == CollisionLayer::None) {
            return false;
        }

        uint32_t valA = static_cast<uint32_t>(layerA);
        uint32_t valB = static_cast<uint32_t>(layerB);

        bool aAcceptsB = (maskA & valB) != 0;
        bool bAcceptsA = (maskB & valA) != 0;

        return aAcceptsB && bAcceptsA;
    }

    /**
     * Determines whether two colliders should be tested, taking into account
     * enabled state, layer/mask filtering, and self-collision rules.
     */
    static constexpr bool ShouldTestPair(bool enabledA, EntityId entityA, CollisionLayer layerA, uint32_t maskA,
                                         bool enabledB, EntityId entityB, CollisionLayer layerB, uint32_t maskB,
                                         bool allowSelfCollision = false) noexcept {
        if (!enabledA || !enabledB) {
            return false;
        }

        // Prevent self-collision between different colliders on the same entity
        if (!allowSelfCollision && entityA != INVALID_ENTITY_ID && entityA == entityB) {
            return false;
        }

        return ShouldCollide(layerA, maskA, layerB, maskB);
    }
};

} // namespace wildbound::physics

