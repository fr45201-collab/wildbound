#pragma once

#include "AnimationState.hpp"
#include "Direction.hpp"
#include <string>
#include <unordered_map>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::graphics::animation {

class AnimationController;

/**
 * State machine managing Idle, Walking, and Running states across 4 directions.
 * Handles state transitions, direction updates, and run animation fallbacks.
 */
class AnimationStateMachine {
public:
    explicit AnimationStateMachine(const std::string& characterPrefix = "player");

    void SetCharacterPrefix(const std::string& prefix) { m_prefix = prefix; }
    const std::string& GetCharacterPrefix() const { return m_prefix; }

    void SetCustomClip(MovementState state, Direction dir, const std::string& clipName);
    std::string GetClipForState(MovementState state, Direction dir) const;

    void Update(Vector2 velocity, bool isRunningRequest, AnimationController& controller,
                const resources::AssetManager& assetMgr);

    // Current State Queries
    MovementState GetCurrentMovementState() const { return m_currentState; }
    Direction GetCurrentDirection() const { return m_currentFacing; }

    void SetFacing(Direction dir) { m_currentFacing = dir; }
    void ForceState(MovementState state, Direction dir, AnimationController& controller,
                    const resources::AssetManager& assetMgr);

private:
    std::string m_prefix = "player";
    MovementState m_currentState = MovementState::Idle;
    Direction m_currentFacing = Direction::Down;

    // Custom clip name overrides (e.g. from JSON config)
    std::unordered_map<std::string, std::string> m_customClips;
};

} // namespace wildbound::graphics::animation

