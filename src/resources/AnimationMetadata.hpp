#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::resources {

/**
 * Lightweight animation metadata foundation.
 * Defines sequential frame timing, looping parameters, and spritesheet bindings.
 */
struct AnimationClip {
    std::string name;
    std::string spritesheet;
    std::vector<int> frames;
    float frameDuration = 0.12f;
    bool loop = true;

    bool IsValid() const {
        return !name.empty() && !spritesheet.empty() && !frames.empty() && frameDuration > 0.0f;
    }

    float GetTotalDuration() const {
        return static_cast<float>(frames.size()) * frameDuration;
    }

    int GetFrameIndexAtTime(float timeSeconds) const;

    nlohmann::json ToJson() const;
    static AnimationClip FromJson(const nlohmann::json& j);
};

} // namespace wildbound::resources

