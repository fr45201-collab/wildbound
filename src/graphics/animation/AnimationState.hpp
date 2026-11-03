#pragma once

#include "Direction.hpp"
#include <string>
#include <string_view>

namespace wildbound::graphics::animation {

enum class MovementState {
    Idle = 0,
    Walking,
    Running
};

inline std::string_view MovementStateToString(MovementState state) {
    switch (state) {
        case MovementState::Idle:    return "idle";
        case MovementState::Walking: return "walk";
        case MovementState::Running: return "run";
        default:                     return "idle";
    }
}

inline MovementState StringToMovementState(std::string_view str) {
    if (str == "walk" || str == "walking") return MovementState::Walking;
    if (str == "run" || str == "running")  return MovementState::Running;
    return MovementState::Idle;
}

/**
 * Generates standardized animation clip key from state and direction.
 * e.g., (Walking, Down, "player") -> "player_walk_down"
 */
inline std::string MakeAnimationKey(MovementState state, Direction dir, const std::string& prefix = "player") {
    std::string key = prefix;
    if (!key.empty()) key += "_";
    key += MovementStateToString(state);
    key += "_";
    key += DirectionToString(dir);
    return key;
}

} // namespace wildbound::graphics::animation

