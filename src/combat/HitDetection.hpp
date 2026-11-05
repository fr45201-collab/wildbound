#pragma once

#include "CombatTypes.hpp"
#include "AttackSystem.hpp"
#include "../graphics/animation/Direction.hpp"
#include <raylib.h>
#include <vector>

namespace wildbound::combat {

struct HitDebugEntry {
    Rectangle attackHitbox{ 0.0f, 0.0f, 0.0f, 0.0f };
    Rectangle targetHurtbox{ 0.0f, 0.0f, 0.0f, 0.0f };
    Vector2 hitPoint{ 0.0f, 0.0f };
    bool didHit = false;
    float lifetime = 0.5f; // Duration to display in debug overlay
};

class HitDetection {
public:
    // Generate an attack hitbox in front of the attacker based on facing direction
    static Rectangle ComputeAttackHitbox(
        Vector2 attackerPos,
        graphics::animation::Direction facing,
        const AttackDefinition& attack
    );

    // Test intersection between attack hitbox and target hurtbox
    static bool Intersects(const Rectangle& attackHitbox, const Rectangle& targetHurtbox);

    // Compute contact/impact point between two overlapping rectangles
    static Vector2 ComputeImpactPoint(const Rectangle& a, const Rectangle& b);
};

} // namespace wildbound::combat

