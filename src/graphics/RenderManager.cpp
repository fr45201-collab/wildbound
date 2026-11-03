#include "RenderManager.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::graphics {

RenderManager::RenderManager() = default;

RenderManager::~RenderManager() {
    Shutdown();
}

bool RenderManager::Initialize(const core::EngineConfig& config) {
    if (m_initialized) {
        Logger::Warn("GRAPHICS", "RenderManager already initialized.");
        return true;
    }

    m_windowWidth = config.windowWidth;
    m_windowHeight = config.windowHeight;
    m_internalWidth = config.internalWidth;
    m_internalHeight = config.internalHeight;

    // Configure raylib window flags
    unsigned int flags = FLAG_WINDOW_HIGHDPI;
    if (config.resizable) {
        flags |= FLAG_WINDOW_RESIZABLE;
    }
    if (config.vsync) {
        flags |= FLAG_VSYNC_HINT;
    }
    SetConfigFlags(flags);

    // Suppress verbose raylib internal logs in favor of engine logger
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(m_windowWidth, m_windowHeight, config.gameTitle.c_str());

    if (!IsWindowReady()) {
        Logger::Error("GRAPHICS", "Failed to initialize raylib window.");
        return false;
    }

    if (config.targetFps > 0) {
        SetTargetFPS(config.targetFps);
    }

    // Create virtual render target for pixel-perfect internal resolution
    m_virtualCanvas = LoadRenderTexture(m_internalWidth, m_internalHeight);
    if (m_virtualCanvas.id == 0) {
        Logger::Error("GRAPHICS", "Failed to create virtual canvas RenderTexture2D.");
        CloseWindow();
        return false;
    }

    // Set point filtering for authentic pixel-art rendering
    SetTextureFilter(m_virtualCanvas.texture, TEXTURE_FILTER_POINT);

    UpdateScaling();

    m_initialized = true;
    Logger::Info("GRAPHICS", "raylib initialized.");
    Logger::Info("GRAPHICS", "Virtual canvas: " + std::to_string(m_internalWidth) + "x" +
                 std::to_string(m_internalHeight) + ", Window: " +
                 std::to_string(m_windowWidth) + "x" + std::to_string(m_windowHeight) +
                 " (Scale: " + std::to_string(m_renderScale) + "x, Point Filtering)");

    return true;
}

void RenderManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("GRAPHICS", "Releasing graphics resources and closing window.");
        if (m_virtualCanvas.id != 0) {
            UnloadRenderTexture(m_virtualCanvas);
            m_virtualCanvas = {};
        }
        if (IsWindowReady()) {
            CloseWindow();
        }
        m_initialized = false;
    }
}

void RenderManager::UpdateScaling() {
    int currentWinWidth = GetScreenWidth();
    int currentWinHeight = GetScreenHeight();

    if (currentWinWidth > 0 && currentWinHeight > 0) {
        m_windowWidth = currentWinWidth;
        m_windowHeight = currentWinHeight;
    }

    float scaleX = static_cast<float>(m_windowWidth) / static_cast<float>(m_internalWidth);
    float scaleY = static_cast<float>(m_windowHeight) / static_cast<float>(m_internalHeight);
    m_renderScale = std::min(scaleX, scaleY);

    // Coordinate system: raylib render texture has inverted Y coordinates
    m_srcRect = {
        0.0f,
        0.0f,
        static_cast<float>(m_internalWidth),
        -static_cast<float>(m_internalHeight)
    };

    float scaledWidth = static_cast<float>(m_internalWidth) * m_renderScale;
    float scaledHeight = static_cast<float>(m_internalHeight) * m_renderScale;

    m_dstRect = {
        (static_cast<float>(m_windowWidth) - scaledWidth) * 0.5f,
        (static_cast<float>(m_windowHeight) - scaledHeight) * 0.5f,
        scaledWidth,
        scaledHeight
    };
}

void RenderManager::BeginVirtualCanvas() {
    BeginTextureMode(m_virtualCanvas);
}

void RenderManager::EndVirtualCanvas() {
    EndTextureMode();
}

void RenderManager::RenderCanvasToWindow() {
    // If window resized, recalculate letterboxing
    if (IsWindowResized()) {
        UpdateScaling();
    }

    // Letterbox/pillarbox background color (solid dark border)
    ClearBackground(BLACK);

    // Draw virtual render target scaled to destination viewport
    DrawTexturePro(m_virtualCanvas.texture, m_srcRect, m_dstRect, m_origin, 0.0f, WHITE);
}

void RenderManager::DrawInitialScreen(const core::EngineConfig& config, float runningTime) {
    // Fantasy night background: #131722
    Color bgDark = { 19, 23, 34, 255 };
    ClearBackground(bgDark);

    // Subtle atmospheric pixel background elements
    // Ambient decorative grid / border
    DrawRectangleLines(8, 8, m_internalWidth - 16, m_internalHeight - 16, Color{ 36, 44, 66, 255 });
    DrawRectangleLines(10, 10, m_internalWidth - 20, m_internalHeight - 20, Color{ 28, 34, 52, 255 });

    // Subtle ambient decorative corner accents
    DrawRectangle(12, 12, 6, 6, Color{ 70, 90, 125, 255 });
    DrawRectangle(m_internalWidth - 18, 12, 6, 6, Color{ 70, 90, 125, 255 });
    DrawRectangle(12, m_internalHeight - 18, 6, 6, Color{ 70, 90, 125, 255 });
    DrawRectangle(m_internalWidth - 18, m_internalHeight - 18, 6, 6, Color{ 70, 90, 125, 255 });

    // Animated glow pulse
    float pulse = (std::sin(runningTime * 2.0f) + 1.0f) * 0.5f;
    unsigned char alphaGlow = static_cast<unsigned char>(180 + pulse * 75);

    // Main Title: "WILDBOUND"
    const char* titleText = config.gameTitle.c_str();
    int titleFontSize = 40;
    int titleWidth = MeasureText(titleText, titleFontSize);
    int titleX = (m_internalWidth - titleWidth) / 2;
    int titleY = 110;

    // Drop shadow
    DrawText(titleText, titleX + 2, titleY + 2, titleFontSize, Color{ 8, 10, 15, 255 });
    // Title text with gold/amber fantasy tone: #E5A93C
    Color titleColor = { 229, 169, 60, alphaGlow };
    DrawText(titleText, titleX, titleY, titleFontSize, titleColor);

    // Subtitle: Phase 0 Engine Foundation
    const char* subTitle = "PHASE 0: FOUNDATION & ARCHITECTURE";
    int subFontSize = 14;
    int subWidth = MeasureText(subTitle, subFontSize);
    int subX = (m_internalWidth - subWidth) / 2;
    int subY = 165;
    DrawText(subTitle, subX + 1, subY + 1, subFontSize, Color{ 15, 18, 25, 255 });
    DrawText(subTitle, subX, subY, subFontSize, Color{ 150, 175, 205, 255 });

    // Status Banner
    const char* statusText = "[ ENGINE INITIALIZATION SUCCESSFUL ]";
    int statusFontSize = 12;
    int statusWidth = MeasureText(statusText, statusFontSize);
    int statusX = (m_internalWidth - statusWidth) / 2;
    int statusY = 210;
    DrawText(statusText, statusX, statusY, statusFontSize, Color{ 90, 205, 130, 255 });

    // Specs readout (internal resolution, window, scale)
    std::string specInfo = "Internal: " + std::to_string(m_internalWidth) + "x" + std::to_string(m_internalHeight) +
                           "  |  Window: " + std::to_string(m_windowWidth) + "x" + std::to_string(m_windowHeight) +
                           " (Scale " + std::to_string(static_cast<int>(m_renderScale)) + "x)";
    int specFontSize = 10;
    int specWidth = MeasureText(specInfo.c_str(), specFontSize);
    int specX = (m_internalWidth - specWidth) / 2;
    int specY = 270;
    DrawText(specInfo.c_str(), specX, specY, specFontSize, Color{ 110, 130, 160, 255 });

    // Controls hint
    const char* hintText = "Press [F1] to toggle Developer ImGui Overlay | Press [ESC] to Exit";
    int hintFontSize = 10;
    int hintWidth = MeasureText(hintText, hintFontSize);
    int hintX = (m_internalWidth - hintWidth) / 2;
    int hintY = 320;
    DrawText(hintText, hintX, hintY, hintFontSize, Color{ 80, 95, 120, 255 });
}

bool RenderManager::IsWindowClosed() const {
    return WindowShouldClose();
}

} // namespace wildbound::graphics

