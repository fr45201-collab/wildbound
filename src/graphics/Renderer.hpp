#pragma once

#include "Viewport.hpp"
#include "RenderTexture.hpp"
#include "Camera2D.hpp"
#include "DrawContext.hpp"
#include "../core/EngineConfig.hpp"
#include <raylib.h>
#include <memory>

namespace wildbound::graphics {

/**
 * Core 2D Renderer for Wildbound.
 * Coordinates virtual resolution render targets, camera passes,
 * pixel-perfect viewport scaling, and presentation.
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Initialize(const core::EngineConfig& config);
    void Shutdown();

    // Frame lifecycle
    void BeginFrame();
    void BeginWorld(Camera2DSystem& camera);
    void EndWorld(Camera2DSystem& camera);
    void BeginUI();
    void EndUI();
    void EndFrame();
    void Present();

    void HandleWindowResize();

    // Reusable Sprite Rendering Pipeline (Phase 2)
    void DrawSprite(
        const Texture2D& texture,
        Rectangle sourceRect,
        Rectangle destRect,
        Vector2 origin = { 0.0f, 0.0f },
        float rotation = 0.0f,
        Color tint = WHITE,
        bool flipX = false,
        bool flipY = false
    );

    void DrawSprite(
        const Texture2D& texture,
        Rectangle sourceRect,
        Vector2 worldPosition,
        Vector2 origin = { 0.0f, 0.0f },
        float scale = 1.0f,
        float rotation = 0.0f,
        Color tint = WHITE,
        bool flipX = false,
        bool flipY = false
    );

    // Accessors
    const Viewport& GetViewport() const { return m_viewport; }
    Viewport& GetViewport() { return m_viewport; }
    const RenderTexture& GetVirtualCanvas() const { return m_canvas; }
    DrawContext& GetDrawContext() { return m_drawContext; }
    const DrawContext& GetDrawContext() const { return m_drawContext; }

    bool IsWindowClosed() const;
    bool IsInitialized() const { return m_initialized; }

private:
    Viewport m_viewport;
    RenderTexture m_canvas;
    DrawContext m_drawContext;

    bool m_initialized = false;
    bool m_inFrame = false;
    bool m_inWorld = false;
};

} // namespace wildbound::graphics
