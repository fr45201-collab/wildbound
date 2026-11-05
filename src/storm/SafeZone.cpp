#include "SafeZone.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::storm {

bool SafeZoneHelper::IsCircleContainedInRect(Vector2 center, float radius, Rectangle rect) noexcept {
    return (center.x - radius >= rect.x) &&
           (center.x + radius <= rect.x + rect.width) &&
           (center.y - radius >= rect.y) &&
           (center.y + radius <= rect.y + rect.height);
}

Vector2 SafeZoneHelper::ClampCircleCenterToRect(Vector2 center, float radius, Rectangle rect) noexcept {
    float minX = rect.x + radius;
    float maxX = rect.x + rect.width - radius;
    float minY = rect.y + radius;
    float maxY = rect.y + rect.height - radius;

    // If diameter is larger than rect, clamp to center of rect
    if (minX > maxX) {
        center.x = rect.x + rect.width * 0.5f;
    } else {
        center.x = std::clamp(center.x, minX, maxX);
    }

    if (minY > maxY) {
        center.y = rect.y + rect.height * 0.5f;
    } else {
        center.y = std::clamp(center.y, minY, maxY);
    }

    return center;
}

Vector2 SafeZoneHelper::CalculateNextCenter(const SafeZone& currentZone, float nextRadius,
                                           Rectangle playableBounds, float driftRatio, uint32_t seed) {
    // Maximum offset allowed from current center such that the next circle is contained in current circle:
    // maxAllowedOffset = currentZone.radius - nextRadius
    float maxAllowedOffset = std::max(0.0f, (currentZone.radius - nextRadius) * std::clamp(driftRatio, 0.0f, 1.0f));

    if (maxAllowedOffset <= 1.0f) {
        return ClampCircleCenterToRect(currentZone.center, nextRadius, playableBounds);
    }

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
    std::uniform_real_distribution<float> radDist(0.0f, maxAllowedOffset);

    // Attempt up to 16 samples to find a center that keeps nextRadius completely inside playableBounds
    for (int attempt = 0; attempt < 16; ++attempt) {
        float angle = angleDist(rng);
        float dist = radDist(rng);
        Vector2 candidate{
            currentZone.center.x + dist * std::cos(angle),
            currentZone.center.y + dist * std::sin(angle)
        };

        if (IsCircleContainedInRect(candidate, nextRadius, playableBounds)) {
            return candidate;
        }
    }

    // Fallback: Clamp current center or candidate to playable bounds
    return ClampCircleCenterToRect(currentZone.center, nextRadius, playableBounds);
}

} // namespace wildbound::storm

