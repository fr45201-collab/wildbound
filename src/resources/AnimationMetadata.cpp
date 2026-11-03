#include "AnimationMetadata.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::resources {

int AnimationClip::GetFrameIndexAtTime(float timeSeconds) const {
    if (frames.empty()) return 0;
    if (frames.size() == 1 || frameDuration <= 0.0f) return frames[0];

    float totalDuration = GetTotalDuration();
    if (totalDuration <= 0.0f) return frames[0];

    float effectiveTime = timeSeconds;
    if (loop) {
        effectiveTime = std::fmod(timeSeconds, totalDuration);
        if (effectiveTime < 0.0f) {
            effectiveTime += totalDuration;
        }
    } else {
        effectiveTime = std::clamp(timeSeconds, 0.0f, totalDuration - 0.0001f);
    }

    size_t index = static_cast<size_t>(effectiveTime / frameDuration);
    if (index >= frames.size()) {
        index = loop ? 0 : frames.size() - 1;
    }

    return frames[index];
}

nlohmann::json AnimationClip::ToJson() const {
    nlohmann::json j;
    j["animation"] = name;
    j["spritesheet"] = spritesheet;
    j["frames"] = frames;
    j["frame_duration"] = frameDuration;
    j["loop"] = loop;
    return j;
}

AnimationClip AnimationClip::FromJson(const nlohmann::json& j) {
    AnimationClip clip;
    clip.name = j.value("animation", "");
    clip.spritesheet = j.value("spritesheet", "");
    clip.frames = j.value("frames", std::vector<int>{});
    clip.frameDuration = j.value("frame_duration", 0.12f);
    clip.loop = j.value("loop", true);
    return clip;
}

} // namespace wildbound::resources

