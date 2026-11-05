#include "HitDetection.hpp"
#include <algorithm>
#include <cmath>

namespace wildbound::combat {

Rectangle HitDetection::ComputeAttackHitbox(
    Vector2 attackerPos,
    graphics::animation::Direction facing,
    const AttackDefinition& attack)
{
    float w = attack.hitboxWidth;
    float h = attack.hitboxHeight;
    float offset = attack.forwardOffset;

    // Attacker position represents logical feet anchor (bottom-center of body)
    // Vertical body midpoint is approximately attackerPos.y - 24.0f
    float bodyMidY = attackerPos.y - 24.0f;

    Rectangle hitbox{ 0.0f, 0.0f, w, h };

    switch (facing) {
        case graphics::animation::Direction::Down:
            hitbox.x = attackerPos.x - (w * 0.5f);
            hitbox.y = attackerPos.y + offset - (h * 0.5f);
            break;

        case graphics::animation::Direction::Up:
            hitbox.x = attackerPos.x - (w * 0.5f);
            hitbox.y = attackerPos.y - 48.0f - offset;
            break;

        case graphics::animation::Direction::Left:
            hitbox.x = attackerPos.x - offset - w;
            hitbox.y = bodyMidY - (h * 0.5f);
            break;

        case graphics::animation::Direction::Right:
            hitbox.x = attackerPos.x + offset;
            hitbox.y = bodyMidY - (h * 0.5f);
            break;
    }

    return hitbox;
}

bool HitDetection::Intersects(const Rectangle& attackHitbox, const Rectangle& targetHurtbox) {
    return CheckCollisionRecs(attackHitbox, targetHurtbox);
}

Vector2 HitDetection::ComputeImpactPoint(const Rectangle& a, const Rectangle& b) {
    float left = std::max(a.x, b.x);
    float right = std::min(a.x + a.width, b.x + b.width);
    float top = std::max(a.y, b.y);
    float bottom = std::min(a.y + a.height, b.y + b.height);

    if (left <= right && top <= bottom) {
        return Vector2{
            (left + right) * 0.5f,
            (top + bottom) * 0.5f
        };
    }

    return Vector2{ (a.x + a.width * 0.5f + b.x + b.width * 0.5f) * 0.5f,
                   (a.y + a.height * 0.5f + b.y + b.height * 0.5f) * 0.5f };
}

} // namespace wildbound::combat

