#pragma once

#include "InteractionTypes.hpp"
#include "InteractionDefinition.hpp"
#include "InteractionContext.hpp"
#include <raylib.h>
#include <functional>
#include <string>

namespace wildbound::interaction {

using InteractionHandler = std::function<InteractionResult(InteractableId id, InteractionContext& context)>;

/**
 * Component or descriptor attached to entities that can be targeted for interaction.
 */
struct InteractableComponent {
    InteractableId id = INVALID_INTERACTABLE_ID;
    Vector2 position{ 0.0f, 0.0f };
    Rectangle bounds{ 0.0f, 0.0f, 32.0f, 32.0f };
    InteractionDefinition definition;

    // Runtime state
    float currentCooldown = 0.0f;
    bool isExecuting = false;
    bool isAvailable = true;
    bool hasBeenExecuted = false;

    // Domain entity linkage
    std::string linkedEntityId = ""; // E.g. Building ID, Door ID, LootContainer ID, WorldItem ID
    InteractionHandler customHandler = nullptr;

    [[nodiscard]] bool CanInteract() const noexcept {
        return definition.enabled && isAvailable && currentCooldown <= 0.0f && !isExecuting &&
               (!definition.oneTimeOnly || !hasBeenExecuted);
    }

    void Update(float dt) noexcept {
        if (currentCooldown > 0.0f) {
            currentCooldown -= dt;
            if (currentCooldown < 0.0f) currentCooldown = 0.0f;
        }
    }
};

} // namespace wildbound::interaction

