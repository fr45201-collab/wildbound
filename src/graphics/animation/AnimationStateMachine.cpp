#include "AnimationStateMachine.hpp"
#include "AnimationController.hpp"
#include "../../resources/AssetManager.hpp"
#include <cmath>

namespace wildbound::graphics::animation {

AnimationStateMachine::AnimationStateMachine(const std::string& characterPrefix)
    : m_prefix(characterPrefix)
{
}

void AnimationStateMachine::SetCustomClip(MovementState state, Direction dir, const std::string& clipName) {
    std::string key = std::string(MovementStateToString(state)) + "_" + std::string(DirectionToString(dir));
    m_customClips[key] = clipName;
}

std::string AnimationStateMachine::GetClipForState(MovementState state, Direction dir) const {
    std::string key = std::string(MovementStateToString(state)) + "_" + std::string(DirectionToString(dir));
    auto it = m_customClips.find(key);
    if (it != m_customClips.end()) {
        return it->second;
    }
    return MakeAnimationKey(state, dir, m_prefix);
}

void AnimationStateMachine::Update(Vector2 velocity, bool isRunningRequest, AnimationController& controller,
                                  const resources::AssetManager& assetMgr)
{
    float speedSq = (velocity.x * velocity.x) + (velocity.y * velocity.y);
    constexpr float stopThresholdSq = 1.0f; // 1 pixel/second threshold

    MovementState nextState = MovementState::Idle;
    if (speedSq > stopThresholdSq) {
        nextState = isRunningRequest ? MovementState::Running : MovementState::Walking;
        m_currentFacing = VectorToDirection(velocity, m_currentFacing);
    } else {
        nextState = MovementState::Idle;
    }

    m_currentState = nextState;

    std::string clipName = GetClipForState(m_currentState, m_currentFacing);
    float playbackSpeed = 1.0f;

    // Check if the requested clip exists in AssetManager
    if (!assetMgr.GetAnimation(clipName)) {
        // If it's a running clip that doesn't exist, fall back to walking with accelerated timing (1.65x)
        if (m_currentState == MovementState::Running) {
            clipName = GetClipForState(MovementState::Walking, m_currentFacing);
            playbackSpeed = 1.65f;
        } else if (m_currentState == MovementState::Walking) {
            // If walk clip doesn't exist, fallback to idle
            clipName = GetClipForState(MovementState::Idle, m_currentFacing);
        }
    }

    // Play clip (resetIfSame is false so continuing the same walk/idle direction preserves smooth cycle)
    controller.Play(clipName, false, playbackSpeed);
}

void AnimationStateMachine::ForceState(MovementState state, Direction dir, AnimationController& controller,
                                       const resources::AssetManager& assetMgr)
{
    m_currentState = state;
    m_currentFacing = dir;
    std::string clip = GetClipForState(m_currentState, m_currentFacing);
    float speed = (m_currentState == MovementState::Running && !assetMgr.GetAnimation(clip)) ? 1.65f : 1.0f;
    if (speed > 1.0f) {
        clip = GetClipForState(MovementState::Walking, m_currentFacing);
    }
    controller.Play(clip, true, speed);
}

} // namespace wildbound::graphics::animation

