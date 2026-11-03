#pragma once

#include "../core/EngineConfig.hpp"
#include "../graphics/Camera2D.hpp"
#include "../graphics/Viewport.hpp"
#include <string>

namespace wildbound::resources {
    class AssetManager;
}

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
        Vector2 mouseWorld,
        resources::AssetManager* assetMgr = nullptr
    );

    void ToggleVisible() { m_visible = !m_visible; }
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }

private:
    void RenderAssetBrowser(resources::AssetManager* assetMgr);
    void RenderAssetValidation(resources::AssetManager* assetMgr);

    bool m_initialized = false;
    bool m_visible = false;

    // Asset Browser state
    char m_searchBuffer[64]{ 0 };
    std::string m_selectedCategory = "All";
    std::string m_selectedAssetId;
    int m_previewFrameIndex = 0;
};

} // namespace wildbound::ui
