#include "Renderer.hpp"
#include "../core/Logger.hpp"
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::graphics {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(const core::EngineConfig& config) {
    if (m_initialized) {
        Logger::Warn("RENDERER", "Renderer is already initialized.");
        return true;
    }

    unsigned int flags = FLAG_WINDOW_HIGHDPI;
    if (config.resizable) {
        flags |= FLAG_WINDOW_RESIZABLE;
    }
    if (config.vsync) {
        flags |= FLAG_VSYNC_HINT;
    }
    SetConfigFlags(flags);

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(config.windowWidth, config.windowHeight, config.gameTitle.c_str());

    if (!IsWindowReady()) {
        Logger::Error("RENDERER", "Failed to initialize raylib window.");
        return false;
    }

    if (config.targetFps > 0) {
        SetTargetFPS(config.targetFps);
    }

    m_viewport = Viewport(
        config.internalWidth,
        config.internalHeight,
        config.windowWidth,
        config.windowHeight,
        false
    );

    if (!m_canvas.Load(config.internalWidth, config.internalHeight)) {
        Logger::Error("RENDERER", "Failed to allocate virtual canvas RenderTexture.");
        CloseWindow();
        return false;
    }

    m_drawContext.virtualWidth = config.internalWidth;
    m_drawContext.virtualHeight = config.internalHeight;
    m_drawContext.debugMode = config.debugMode;

    m_initialized = true;
    Logger::Info("RENDERER", "Renderer initialized. Virtual: " +
                 std::to_string(config.internalWidth) + "x" + std::to_string(config.internalHeight) +
                 ", Window: " + std::to_string(config.windowWidth) + "x" + std::to_string(config.windowHeight) +
                 " (Scale: " + std::to_string(m_viewport.GetScale()) + "x, Point Filtering)");

    return true;
}

void Renderer::Shutdown() {
    if (m_initialized) {
        Logger::Info("RENDERER", "Shutting down Renderer resources.");
        m_canvas.Unload();
        if (IsWindowReady()) {
            CloseWindow();
        }
        m_initialized = false;
    }
}

void Renderer::HandleWindowResize() {
    if (IsWindowResized()) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        if (w > 0 && h > 0) {
            m_viewport.Update(w, h);
            Logger::Debug("RENDERER", "Window resized to " + std::to_string(w) + "x" +
                          std::to_string(h) + " (New Scale: " + std::to_string(m_viewport.GetScale()) + "x)");
        }
    }
}

void Renderer::BeginFrame() {
    HandleWindowResize();
    m_canvas.Begin();
    m_inFrame = true;
}

void Renderer::BeginWorld(Camera2DSystem& camera) {
    if (m_inFrame) {
        camera.Begin();
        m_inWorld = true;
    }
}

void Renderer::EndWorld(Camera2DSystem& camera) {
    if (m_inWorld) {
        camera.End();
        m_inWorld = false;
    }
}

void Renderer::BeginUI() {
}

void Renderer::EndUI() {
}

void Renderer::EndFrame() {
    if (m_inFrame) {
        m_canvas.End();
        m_inFrame = false;
    }
}

void Renderer::Present() {
    BeginDrawing();
    ClearBackground(BLACK); // Letterbox border area

    DrawTexturePro(
        m_canvas.GetTexture(),
        m_viewport.GetSourceRect(),
        m_viewport.GetDestRect(),
        m_viewport.GetOrigin(),
        0.0f,
        WHITE
    );
}

void Renderer::DrawSprite(
    const Texture2D& texture,
    Rectangle sourceRect,
    Rectangle destRect,
    Vector2 origin,
    float rotation,
    Color tint,
    bool flipX,
    bool flipY)
{
    if (texture.id == 0) return;

    Rectangle src = sourceRect;
    if (flipX) src.width = -src.width;
    if (flipY) src.height = -src.height;

    DrawTexturePro(texture, src, destRect, origin, rotation, tint);
}

void Renderer::DrawSprite(
    const Texture2D& texture,
    Rectangle sourceRect,
    Vector2 worldPosition,
    Vector2 origin,
    float scale,
    float rotation,
    Color tint,
    bool flipX,
    bool flipY)
{
    if (texture.id == 0) return;

    float destWidth = std::abs(sourceRect.width) * scale;
    float destHeight = std::abs(sourceRect.height) * scale;

    Rectangle destRect{
        worldPosition.x,
        worldPosition.y,
        destWidth,
        destHeight
    };

    DrawSprite(texture, sourceRect, destRect, origin, rotation, tint, flipX, flipY);
}

bool Renderer::IsWindowClosed() const {
    return WindowShouldClose();
}

} // namespace wildbound::graphics
