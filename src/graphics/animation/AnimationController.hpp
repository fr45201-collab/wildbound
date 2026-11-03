#pragma once

#include <raylib.h>
#include <string>

namespace wildbound::resources {
    class AssetManager;
    struct AnimationClip;
    class SpriteSheet;
}

namespace wildbound::graphics::animation {

/**
 * FPS-independent animation playback engine.
 * Computes frame progression using elapsed delta time.
 */
class AnimationController {
public:
    AnimationController();

    void Play(const std::string& clipName, bool resetIfSame = false, float speed = 1.0f);
    void Stop();
    void Reset();

    void Update(float dt, const resources::AssetManager& assetMgr);

    // Queries
    const std::string& GetCurrentClipName() const { return m_currentClipName; }
    int GetCurrentFrameIndex() const { return m_currentFrameIndex; }
    int GetSpritesheetFrameIndex() const { return m_spritesheetFrameIndex; }
    float GetElapsedTime() const { return m_elapsedTime; }
    float GetPlaybackSpeed() const { return m_playbackSpeed; }
    void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed > 0.0f ? speed : 1.0f; }

    bool IsPlaying() const { return m_isPlaying; }
    bool IsFinished() const { return m_isFinished; }

    // Visual frame query
    Rectangle GetCurrentFrameRect(const resources::AssetManager& assetMgr) const;
    Vector2 GetCurrentFrameOrigin(const resources::AssetManager& assetMgr) const;

private:
    std::string m_currentClipName;
    float m_elapsedTime = 0.0f;
    float m_playbackSpeed = 1.0f;

    int m_currentFrameIndex = 0;       // Index into AnimationClip::frames
    int m_spritesheetFrameIndex = 0;   // The resolved frame index in the SpriteSheet

    bool m_isPlaying = false;
    bool m_isFinished = false;
};

} // namespace wildbound::graphics::animation

