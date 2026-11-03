#include "EngineConfig.hpp"
#include "Logger.hpp"

namespace wildbound::core {

EngineConfig EngineConfig::CreateDefault() {
    return EngineConfig{};
}

bool EngineConfig::Validate() const {
    if (windowWidth <= 0 || windowHeight <= 0) {
        Logger::Error("CONFIG", "Window dimensions must be positive integers.");
        return false;
    }
    if (internalWidth <= 0 || internalHeight <= 0) {
        Logger::Error("CONFIG", "Internal render dimensions must be positive integers.");
        return false;
    }
    if (targetFps <= 0 && targetFps != -1) {
        Logger::Error("CONFIG", "Target FPS must be positive or -1 for uncapped.");
        return false;
    }
    if (gameTitle.empty()) {
        Logger::Warn("CONFIG", "Game title is empty, falling back to 'Wildbound'.");
    }
    return true;
}

} // namespace wildbound::core

