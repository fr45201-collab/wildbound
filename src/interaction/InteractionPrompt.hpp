#pragma once

#include "InteractionTypes.hpp"
#include <raylib.h>
#include <string>

namespace wildbound::interaction {

/**
 * UI presentation state representing the current contextual prompt.
 */
struct InteractionPrompt {
    bool visible = false;
    std::string keyGlyph = "E";               // Localized key glyph (e.g. "E" or "X")
    std::string verb = "Interact";            // Action verb (e.g. "Open", "Enter", "Collect")
    std::string targetName = "";              // Target display name (e.g. "Wooden Door")
    std::string description = "";             // Extended details (e.g. "Press [E] to Enter")
    Vector2 worldPosition{ 0.0f, 0.0f };      // World coordinates of target
    float distance = 0.0f;                    // Distance to player
    InteractionCategory category = InteractionCategory::Environmental;

    // Failure / feedback message
    std::string feedbackMessage = "";
    float feedbackTimer = 0.0f;
    InteractionResult lastResult = InteractionResult::Success;

    void SetFeedback(const std::string& msg, InteractionResult result, float duration = 2.0f) {
        feedbackMessage = msg;
        lastResult = result;
        feedbackTimer = duration;
    }

    void Update(float dt) {
        if (feedbackTimer > 0.0f) {
            feedbackTimer -= dt;
            if (feedbackTimer <= 0.0f) {
                feedbackTimer = 0.0f;
                feedbackMessage.clear();
            }
        }
    }

    [[nodiscard]] std::string GetFormattedPrompt() const {
        if (!visible) return "";
        if (!targetName.empty()) {
            return "[" + keyGlyph + "] " + verb + " " + targetName;
        }
        return "[" + keyGlyph + "] " + verb;
    }
};

} // namespace wildbound::interaction

