#pragma once

#include "../core/EngineConfig.hpp"
#include <raylib.h>

namespace wildbound::graphics {

class RenderManager {
public:
    RenderManager();
    ~RenderManager();

    // Non-copyable
    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

    bool Initialize(const core::EngineConfig& config);
    void Shutdown();

    void BeginVirtualCanvas();
    void EndVirtualCanvas();

    void RenderCanvasToWindow();

    void DrawInitialScreen(const core::EngineConfig& config, float runningTime);

    int GetWindowWidth() const { return m_windowWidth; }
    int GetWindowHeight() const { return m_windowHeight; }
    int GetInternalWidth() const { return m_internalWidth; }
    int GetInternalHeight() const { return m_internalHeight; }
    float GetRenderScale() const { return m_renderScale; }

    bool IsWindowClosed() const;

private:
    void UpdateScaling();

    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    int m_internalWidth = 640;
    int m_internalHeight = 360;
    float m_renderScale = 2.0f;

    RenderTexture2D m_virtualCanvas{};
    Rectangle m_srcRect{};
    Rectangle m_dstRect{};
    Vector2 m_origin{0.0f, 0.0f};

    bool m_initialized = false;
};

} // namespace wildbound::graphics

