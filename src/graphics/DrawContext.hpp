#pragma once

#include <cstdint>

namespace wildbound::graphics {

/**
 * Standard Rendering Layers defined for Wildbound engine.
 */
enum class RenderLayer : uint8_t {
    Background = 0,
    Ground,
    GroundDecorations,
    WorldObjects,
    Characters,
    ForegroundObjects,
    GameEffects,
    GameUI,
    DebugOverlay,
    Count
};

/**
 * Context passed to rendering operations.
 */
struct DrawContext {
    float deltaTime = 0.0f;
    float runningTime = 0.0f;
    uint64_t frameIndex = 0;
    int virtualWidth = 640;
    int virtualHeight = 360;
    bool debugMode = false;
};

} // namespace wildbound::graphics

