#pragma once

#include "InteractionTypes.hpp"
#include "InteractableComponent.hpp"
#include "InteractionContext.hpp"

namespace wildbound::interaction {

class Door;
class Mechanism;

/**
 * Executes validated interactions by delegating to corresponding domain systems.
 */
class InteractionExecutor {
public:
    static InteractionResult Execute(InteractableComponent* target,
                                    InteractionContext& context,
                                    Door* door = nullptr,
                                    Mechanism* mechanism = nullptr);
};

} // namespace wildbound::interaction

