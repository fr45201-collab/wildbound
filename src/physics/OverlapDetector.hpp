#pragma once

#include <raylib.h>
#include <algorithm>
#include <cmath>

namespace wildbound::physics {

/**
 * Geometric intersection and overlap computation utilities for 2D Axis-Aligned Bounding Boxes.
 * Zero-allocation, deterministic floating-point operations.
 */
class OverlapDetector {
public:
    /**
     * Fast boolean intersection test between two rectangles.
     */
    static constexpr bool Intersects(const Rectangle& a, const Rectangle& b) noexcept {
        return (a.x < b.x + b.width) &&
               (a.x + a.width > b.x) &&
               (a.y < b.y + b.height) &&
               (a.y + a.height > b.y);
    }

    /**
     * Computes the overlapping intersection rectangle between two intersecting AABBs.
     * Returns a zero-area rectangle if they do not intersect.
     */
    static Rectangle ComputeIntersection(const Rectangle& a, const Rectangle& b) noexcept {
        if (!Intersects(a, b)) {
            return Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
        }

        float left   = std::max(a.x, b.x);
        float top    = std::max(a.y, b.y);
        float right  = std::min(a.x + a.width, b.x + b.width);
        float bottom = std::min(a.y + a.height, b.y + b.height);

        return Rectangle{
            left,
            top,
            std::max(0.0f, right - left),
            std::max(0.0f, bottom - top)
        };
    }

    /**
     * Checks if a point is contained inside an AABB.
     */
    static constexpr bool ContainsPoint(const Rectangle& box, Vector2 point) noexcept {
        return (point.x >= box.x) &&
               (point.x <= box.x + box.width) &&
               (point.y >= box.y) &&
               (point.y <= box.y + box.height);
    }

    /**
     * Computes penetration depth and minimum translation vector (MTV) to push box A out of box B.
     * Returns Vector2 where magnitude is penetration along the axis of minimum penetration.
     */
    static Vector2 ComputePenetration(const Rectangle& a, const Rectangle& b) noexcept {
        if (!Intersects(a, b)) {
            return Vector2{ 0.0f, 0.0f };
        }

        float centerAX = a.x + a.width * 0.5f;
        float centerAY = a.y + a.height * 0.5f;
        float centerBX = b.x + b.width * 0.5f;
        float centerBY = b.y + b.height * 0.5f;

        float overlapX = (a.width * 0.5f + b.width * 0.5f) - std::fabs(centerAX - centerBX);
        float overlapY = (a.height * 0.5f + b.height * 0.5f) - std::fabs(centerAY - centerBY);

        if (overlapX < overlapY) {
            float signX = (centerAX < centerBX) ? -1.0f : 1.0f;
            return Vector2{ overlapX * signX, 0.0f };
        } else {
            float signY = (centerAY < centerBY) ? -1.0f : 1.0f;
            return Vector2{ 0.0f, overlapY * signY };
        }
    }
};

} // namespace wildbound::physics

