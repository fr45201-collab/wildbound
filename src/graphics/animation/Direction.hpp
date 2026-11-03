#pragma once

#include <raylib.h>
#include <string_view>
#include <cmath>

namespace wildbound::graphics::animation {

enum class Direction {
    Down = 0,
    Up,
    Left,
    Right
};

inline std::string_view DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::Down:  return "down";
        case Direction::Up:    return "up";
        case Direction::Left:  return "left";
        case Direction::Right: return "right";
        default:               return "down";
    }
}

inline Direction StringToDirection(std::string_view str) {
    if (str == "up")    return Direction::Up;
    if (str == "left")  return Direction::Left;
    if (str == "right") return Direction::Right;
    return Direction::Down;
}

inline Vector2 DirectionToVector(Direction dir) {
    switch (dir) {
        case Direction::Down:  return Vector2{ 0.0f, 1.0f };
        case Direction::Up:    return Vector2{ 0.0f, -1.0f };
        case Direction::Left:  return Vector2{ -1.0f, 0.0f };
        case Direction::Right: return Vector2{ 1.0f, 0.0f };
        default:               return Vector2{ 0.0f, 1.0f };
    }
}

/**
 * Determines directional orientation from movement vector with bias to previous facing
 * to prevent flickering along exact 45-degree diagonals.
 */
inline Direction VectorToDirection(Vector2 vec, Direction currentFacing = Direction::Down) {
    constexpr float threshold = 0.001f;
    float ax = std::fabs(vec.x);
    float ay = std::fabs(vec.y);

    if (ax < threshold && ay < threshold) {
        return currentFacing;
    }

    // Give slight bias to current facing direction on diagonal ties
    if (std::fabs(ax - ay) < 0.05f) {
        if ((currentFacing == Direction::Left && vec.x < 0) ||
            (currentFacing == Direction::Right && vec.x > 0)) {
            return currentFacing;
        }
        if ((currentFacing == Direction::Up && vec.y < 0) ||
            (currentFacing == Direction::Down && vec.y > 0)) {
            return currentFacing;
        }
    }

    if (ax > ay) {
        return (vec.x > 0) ? Direction::Right : Direction::Left;
    } else {
        return (vec.y > 0) ? Direction::Down : Direction::Up;
    }
}

} // namespace wildbound::graphics::animation

