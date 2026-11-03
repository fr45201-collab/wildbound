#pragma once

#include "../core/EngineConfig.hpp"
#include "../graphics/Camera2D.hpp"
#include "../graphics/Viewport.hpp"
#include <string>

namespace wildbound::ui {

class DevUI {
public:
    DevUI();
    ~DevUI();

    DevUI(const DevUI&) = delete;
    DevUI& operator=(const DevUI&) = delete;

    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderDebugOverlay(
        const core::EngineConfig& config,
        float fps,
        float dt,
        const graphics::Camera2DSystem& camera,
        const graphics::Viewport& viewport,
        Vector2 mouseScreen,
        Vector2 mouseVirtual,
        Vector2 mouseWorld
    );

    void ToggleVisible() { m_visible = !m_visible; }
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

private:
    bool m_initialized = false;
    bool m_visible = false;
};

} // namespace wildbound::ui
