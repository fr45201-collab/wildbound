#pragma once

#include "InteractionTypes.hpp"
#include "InteractableComponent.hpp"
#include "InteractionContext.hpp"

namespace wildbound::interaction {

/**
 * Validates interaction preconditions without executing side-effects.
 */
class InteractionValidator {
public:
    static InteractionResult Validate(const InteractableComponent* target,
                                      const Vector2& playerPos,
                                      InteractionContext& context);
};

} // namespace wildbound::interaction

