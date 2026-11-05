#pragma once

#include "StormTypes.hpp"
#include <raylib.h>
#include <random>

namespace wildbound::storm {

class SafeZoneHelper {
public:
    /**
     * Determines whether a circular safe zone is completely contained inside a rectangular boundary.
     */
    static bool IsCircleContainedInRect(Vector2 center, float radius, Rectangle rect) noexcept;

    /**
     * Clamps a circle's center so that its circumference remains strictly inside the bounding rectangle.
     */
    static Vector2 ClampCircleCenterToRect(Vector2 center, float radius, Rectangle rect) noexcept;

    /**
     * Calculates a valid next safe zone center inside the current safe zone that also fits within world bounds.
     */
    static Vector2 CalculateNextCenter(const SafeZone& currentZone, float nextRadius,
                                       Rectangle playableBounds, float driftRatio = 0.4f, uint32_t seed = 42);
};

} // namespace wildbound::storm

