#include "ObstacleAvoidance.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::ai {

ObstacleAvoidance::ObstacleAvoidance(AvoidanceConfig config)
    : m_config(config)
{
}

Vector2 ObstacleAvoidance::ComputeAvoidanceForce(uint32_t selfId, Vector2 selfPos, const std::vector<AvoidanceEntity>& nearbyEntities) const {
    Vector2 totalForce{ 0.0f, 0.0f };

    for (const auto& other : nearbyEntities) {
        if (other.id == selfId) continue;

        float dx = selfPos.x - other.position.x;
        float dy = selfPos.y - other.position.y;
        float distSq = dx * dx + dy * dy;

        float effectiveRadius = m_config.separationRadius + other.radius;
        float radiusSq = effectiveRadius * effectiveRadius;

        if (distSq > 0.0001f && distSq < radiusSq) {
            float dist = std::sqrt(distSq);
            // Inverse distance falloff: stronger when closer
            float strength = (1.0f - (dist / effectiveRadius));
            float invDist = 1.0f / dist;

            totalForce.x += (dx * invDist) * strength;
            totalForce.y += (dy * invDist) * strength;
        }
    }

    // Clamp total force to maxForce
    float len = std::sqrt(totalForce.x * totalForce.x + totalForce.y * totalForce.y);
    if (len > m_config.maxForce) {
        totalForce.x = (totalForce.x / len) * m_config.maxForce;
        totalForce.y = (totalForce.y / len) * m_config.maxForce;
    }

    totalForce.x *= m_config.weight;
    totalForce.y *= m_config.weight;

    return totalForce;
}

} // namespace wildbound::ai

