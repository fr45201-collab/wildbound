#include "DevUI.hpp"
#include "../core/Logger.hpp"

#include <imgui.h>
#include <rlImGui.h>

using wildbound::core::Logger;

namespace wildbound::ui {

DevUI::DevUI() = default;

DevUI::~DevUI() {
    Shutdown();
}

bool DevUI::Initialize() {
    if (m_initialized) {
        return true;
    }

    try {
        rlImGuiSetup(true);
        m_initialized = true;
        Logger::Info("DEV_UI", "Dear ImGui and rlImGui initialized for Phase 1 dev tools.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("DEV_UI", std::string("Failed to initialize rlImGui: ") + ex.what());
        return false;
    }
}

void DevUI::Shutdown() {
    if (m_initialized) {
        Logger::Info("DEV_UI", "Shutting down rlImGui dev tools.");
        rlImGuiShutdown();
        m_initialized = false;
    }
}

void DevUI::BeginFrame() {
    if (m_initialized) {
        rlImGuiBegin();
    }
}

void DevUI::EndFrame() {
    if (m_initialized) {
        rlImGuiEnd();
    }
}

void DevUI::RenderDebugOverlay(
    const core::EngineConfig& config,
    float fps,
    float dt,
    const graphics::Camera2DSystem& camera,
    const graphics::Viewport& viewport,
    Vector2 mouseScreen,
    Vector2 mouseVirtual,
    Vector2 mouseWorld)
{
    if (!m_initialized || !m_visible) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 420), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Wildbound Phase 1 — Diagnostics", &m_visible)) {
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "%s (v%s)", config.gameTitle.c_str(), config.gameVersion.c_str());
        ImGui::Separator();

        // Timing & Performance
        float frameTimeMs = dt * 1000.0f;
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.2f ms (dt: %.4f s)", frameTimeMs, dt);
        ImGui::Separator();

        // Resolution & Viewport
        ImGui::Text("Virtual Resolution: %d x %d", viewport.GetVirtualWidth(), viewport.GetVirtualHeight());
        ImGui::Text("Window Resolution:  %d x %d", viewport.GetWindowWidth(), viewport.GetWindowHeight());
        Rectangle dst = viewport.GetDestRect();
        ImGui::Text("Viewport Rect:      [%.0f, %.0f, %.0f, %.0f]", dst.x, dst.y, dst.width, dst.height);
        ImGui::Text("Rendering Scale:    %.2fx %s", viewport.GetScale(),
                    viewport.IsLetterboxed() ? "(Letterboxed)" : "(Exact)");
        ImGui::Text("Texture Filter:     POINT (Nearest-Neighbor)");
        ImGui::Separator();

        // Camera System
        const char* modeStr = "UNKNOWN";
        switch (camera.GetMode()) {
            case graphics::CameraMode::Free:   modeStr = "FREE (WASD)"; break;
            case graphics::CameraMode::Follow: modeStr = "FOLLOW (Target)"; break;
            case graphics::CameraMode::Fixed:  modeStr = "FIXED"; break;
        }
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Camera Mode: %s", modeStr);
        Vector2 camPos = camera.GetTarget();
        ImGui::Text("Camera Pos:  (%.1f, %.1f)", camPos.x, camPos.y);
        ImGui::Text("Camera Zoom: %.2fx (Min: %.1fx, Max: %.1fx)",
                    camera.GetZoom(), camera.GetMinZoom(), camera.GetMaxZoom());
        if (camera.HasWorldBounds()) {
            Rectangle b = camera.GetWorldBounds();
            ImGui::Text("World Bounds: [%.0f, %.0f, %.0f, %.0f]", b.x, b.y, b.width, b.height);
        }
        ImGui::Separator();

        // Multi-Space Coordinates
        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.5f, 1.0f), "Coordinate Inspector:");
        ImGui::Text("Screen (Window):  (%.1f, %.1f)", mouseScreen.x, mouseScreen.y);
        ImGui::Text("Virtual (Canvas): (%.1f, %.1f)", mouseVirtual.x, mouseVirtual.y);
        ImGui::Text("World Space:      (%.1f, %.1f)", mouseWorld.x, mouseWorld.y);
        ImGui::Separator();

        // Controls Reference
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Keyboard Controls:");
        ImGui::BulletText("W / A / S / D - Pan Camera (Free mode)");
        ImGui::BulletText("Q / E - Zoom Out / Zoom In");
        ImGui::BulletText("R - Reset Camera");
        ImGui::BulletText("F2 - Toggle Follow Camera");
        ImGui::BulletText("F1 - Toggle Diagnostics Overlay");
        ImGui::BulletText("ESC - Exit");
    }
    ImGui::End();
}

} // namespace wildbound::ui
