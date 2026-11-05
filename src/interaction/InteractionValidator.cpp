#include "InteractionValidator.hpp"
#include "../entities/Player.hpp"
#include <cmath>

namespace wildbound::interaction {

InteractionResult InteractionValidator::Validate(const InteractableComponent* target,
                                                const Vector2& playerPos,
                                                InteractionContext& context) {
    if (!target) {
        return InteractionResult::InvalidTarget;
    }

    if (!target->definition.enabled) {
        return InteractionResult::TargetUnavailable;
    }

    if (!target->isAvailable) {
        return InteractionResult::TargetUnavailable;
    }

    if (target->definition.oneTimeOnly && target->hasBeenExecuted) {
        return InteractionResult::TargetUnavailable;
    }

    if (target->isExecuting) {
        return InteractionResult::AlreadyExecuting;
    }

    if (target->currentCooldown > 0.0f) {
        return InteractionResult::CooldownActive;
    }

    // Distance check
    float dx = target->position.x - playerPos.x;
    float dy = target->position.y - playerPos.y;
    float distSq = dx * dx + dy * dy;
    float maxDist = target->definition.range;
    if (distSq > maxDist * maxDist) {
        return InteractionResult::OutOfRange;
    }

    // Player status check
    if (context.player && context.player->GetHealth().IsDead()) {
        return InteractionResult::Blocked;
    }

    // Key / item requirement check
    if (!target->definition.requiredItemId.empty() && context.player) {
        const auto& inv = context.player->GetInventory();
        if (!inv.HasItem(target->definition.requiredItemId)) {
            return InteractionResult::MissingRequirement;
        }
    }

    return InteractionResult::Success;
}

} // namespace wildbound::interaction
