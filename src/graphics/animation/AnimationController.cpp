#include "AnimationController.hpp"
#include "../../resources/AssetManager.hpp"
#include "../../resources/AnimationMetadata.hpp"
#include "../../resources/SpriteSheet.hpp"
#include <cmath>

namespace wildbound::graphics::animation {

AnimationController::AnimationController() = default;

void AnimationController::Play(const std::string& clipName, bool resetIfSame, float speed) {
    if (clipName.empty()) return;

    if (m_currentClipName == clipName && !resetIfSame && m_isPlaying) {
        // Already playing this clip smoothly; update speed if changed
        m_playbackSpeed = speed > 0.0f ? speed : 1.0f;
        return;
    }

    m_currentClipName = clipName;
    m_playbackSpeed = speed > 0.0f ? speed : 1.0f;
    m_elapsedTime = 0.0f;
    m_currentFrameIndex = 0;
    m_spritesheetFrameIndex = 0;
    m_isPlaying = true;
    m_isFinished = false;
}

void AnimationController::Stop() {
    m_isPlaying = false;
}

void AnimationController::Reset() {
    m_elapsedTime = 0.0f;
    m_currentFrameIndex = 0;
    m_isFinished = false;
}

void AnimationController::Update(float dt, const resources::AssetManager& assetMgr) {
    if (!m_isPlaying || m_currentClipName.empty()) return;

    const auto* clip = assetMgr.GetAnimation(m_currentClipName);
    if (!clip || !clip->IsValid()) return;

    float totalDuration = clip->GetTotalDuration();
    if (totalDuration <= 0.0f) return;

    m_elapsedTime += (dt * m_playbackSpeed);

    if (clip->loop) {
        m_elapsedTime = std::fmod(m_elapsedTime, totalDuration);
        if (m_elapsedTime < 0.0f) m_elapsedTime += totalDuration;

        m_currentFrameIndex = static_cast<int>(m_elapsedTime / clip->frameDuration);
        if (m_currentFrameIndex >= static_cast<int>(clip->frames.size())) {
            m_currentFrameIndex = static_cast<int>(clip->frames.size()) - 1;
        }
        m_isFinished = false;
    } else {
        if (m_elapsedTime >= totalDuration) {
            m_elapsedTime = totalDuration;
            m_currentFrameIndex = static_cast<int>(clip->frames.size()) - 1;
            m_isFinished = true;
        } else {
            m_currentFrameIndex = static_cast<int>(m_elapsedTime / clip->frameDuration);
            m_isFinished = false;
        }
    }

    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < static_cast<int>(clip->frames.size())) {
        m_spritesheetFrameIndex = clip->frames[m_currentFrameIndex];
    }
}

Rectangle AnimationController::GetCurrentFrameRect(const resources::AssetManager& assetMgr) const {
    const auto* clip = assetMgr.GetAnimation(m_currentClipName);
    if (!clip) return Rectangle{ 0.0f, 0.0f, 32.0f, 48.0f };

    const auto* sheet = assetMgr.GetSpriteSheet(clip->spritesheet);
    if (!sheet) return Rectangle{ 0.0f, 0.0f, 32.0f, 48.0f };

    return sheet->GetFrameRect(m_spritesheetFrameIndex);
}

Vector2 AnimationController::GetCurrentFrameOrigin(const resources::AssetManager& assetMgr) const {
    const auto* clip = assetMgr.GetAnimation(m_currentClipName);
    if (!clip) return Vector2{ 16.0f, 48.0f };

    const auto* sheet = assetMgr.GetSpriteSheet(clip->spritesheet);
    if (!sheet) return Vector2{ 16.0f, 48.0f };

    return sheet->GetFrameOrigin(m_spritesheetFrameIndex);
}

} // namespace wildbound::graphics::animation

