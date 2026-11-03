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
        // Setup rlImGui with dark theme enabled
        rlImGuiSetup(true);
        m_initialized = true;
        Logger::Info("DEV_UI", "Dear ImGui and rlImGui initialized for internal dev tools.");
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

void DevUI::RenderDebugOverlay(const core::EngineConfig& config, float fps, float dt, const std::string& luaStatus) {
    if (!m_initialized || !m_visible) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 260), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Wildbound - Developer Metrics", &m_visible)) {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "%s (v%s)", config.gameTitle.c_str(), config.gameVersion.c_str());
        ImGui::Separator();

        ImGui::Text("Engine Status: RUNNING");
        ImGui::Text("FPS: %.1f (dt: %.4f s)", fps, dt);
        ImGui::Separator();

        ImGui::Text("Internal Canvas: %d x %d", config.internalWidth, config.internalHeight);
        ImGui::Text("Window Size:     %d x %d", config.windowWidth, config.windowHeight);
        float scale = static_cast<float>(config.windowWidth) / static_cast<float>(config.internalWidth);
        ImGui::Text("Render Scale:    %.1fx", scale);
        ImGui::Text("Texture Filter:  POINT");
        ImGui::Separator();

        ImGui::Text("Scripting: Lua 5.4 + sol2");
        ImGui::Text("Lua Status: %s", luaStatus.c_str());
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), "[F1] Toggle Overlay");
    }
    ImGui::End();
}

} // namespace wildbound::ui

