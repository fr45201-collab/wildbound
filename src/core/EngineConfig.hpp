#pragma once

#include <string>

namespace wildbound::core {

struct EngineConfig {
    // Game info
    std::string gameTitle = "Wildbound";
    std::string gameVersion = "0.1.0-phase0";
    std::string author = "Wildbound Dev Team";

    // Display configuration
    int windowWidth = 1280;
    int windowHeight = 720;
    int internalWidth = 640;
    int internalHeight = 360;
    int targetFps = 60;
    bool vsync = true;
    bool resizable = false;
    bool fullscreen = false;

    // Debug & logging
    bool debugMode = true;
    bool showMetricsOverlay = true;
    std::string logLevel = "DEBUG";

    static EngineConfig CreateDefault();
    bool Validate() const;
};

} // namespace wildbound::core

