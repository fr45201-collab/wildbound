#include "DevUI.hpp"
#include "../core/Logger.hpp"
#include "../resources/AssetManager.hpp"
#include "../resources/AssetValidator.hpp"

#include <imgui.h>
#include <rlImGui.h>
#include <algorithm>

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
        Logger::Info("DEV_UI", "Dear ImGui and rlImGui initialized for Phase 2 dev tools.");
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
    Vector2 mouseWorld,
    resources::AssetManager* assetMgr)
{
    if (!m_initialized || !m_visible) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, 520), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Wildbound Developer Suite (F1)", &m_visible)) {
        if (ImGui::BeginTabBar("DevSuiteTabs")) {
            // Tab 1: Engine Metrics & Camera
            if (ImGui::BeginTabItem("Metrics & Camera")) {
                ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "%s (v%s)", config.gameTitle.c_str(), config.gameVersion.c_str());
                ImGui::Separator();

                // Timing
                float frameTimeMs = dt * 1000.0f;
                ImGui::Text("FPS: %.1f | Frame Time: %.2f ms", fps, frameTimeMs);
                ImGui::Separator();

                // Viewport
                ImGui::Text("Virtual Resolution: %d x %d", viewport.GetVirtualWidth(), viewport.GetVirtualHeight());
                ImGui::Text("Window Resolution:  %d x %d", viewport.GetWindowWidth(), viewport.GetWindowHeight());
                Rectangle dst = viewport.GetDestRect();
                ImGui::Text("Viewport: [%.0f, %.0f, %.0f, %.0f] (Scale: %.2fx)", dst.x, dst.y, dst.width, dst.height, viewport.GetScale());
                ImGui::Separator();

                // Camera
                const char* modeStr = (camera.GetMode() == graphics::CameraMode::Follow) ? "FOLLOW [Target]" : "FREE [WASD]";
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.95f, 1.0f), "Camera Mode: %s", modeStr);
                Vector2 camPos = camera.GetTarget();
                ImGui::Text("Camera Pos:  (%.1f, %.1f) | Zoom: %.2fx", camPos.x, camPos.y, camera.GetZoom());
                ImGui::Separator();

                // Coordinates
                ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.5f, 1.0f), "Coordinates:");
                ImGui::Text("Screen (Window):  (%.1f, %.1f)", mouseScreen.x, mouseScreen.y);
                ImGui::Text("Virtual (Canvas): (%.1f, %.1f)", mouseVirtual.x, mouseVirtual.y);
                ImGui::Text("World Space:      (%.1f, %.1f)", mouseWorld.x, mouseWorld.y);
                ImGui::Separator();

                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Controls: WASD=Pan, Q/E=Zoom, R=Reset, F2=Toggle Follow, F1=Toggle Overlay");
                ImGui::EndTabItem();
            }

            // Tab 2: Asset Browser
            if (ImGui::BeginTabItem("Asset Browser")) {
                RenderAssetBrowser(assetMgr);
                ImGui::EndTabItem();
            }

            // Tab 3: Asset Validation & Licensing
            if (ImGui::BeginTabItem("Asset Validation")) {
                RenderAssetValidation(assetMgr);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void DevUI::RenderAssetBrowser(resources::AssetManager* assetMgr) {
    if (!assetMgr) {
        ImGui::Text("AssetManager is not active.");
        return;
    }

    auto& registry = assetMgr->GetRegistry();
    auto allAssets = registry.GetAll();

    // Category filter
    auto categories = registry.GetCategories();
    std::vector<std::string> catFilterOptions;
    catFilterOptions.push_back("All");
    for (const auto& c : categories) catFilterOptions.push_back(c);

    if (ImGui::BeginCombo("Category", m_selectedCategory.c_str())) {
        for (const auto& opt : catFilterOptions) {
            bool isSelected = (m_selectedCategory == opt);
            if (ImGui::Selectable(opt.c_str(), isSelected)) {
                m_selectedCategory = opt;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Search input
    ImGui::InputTextWithHint("Search ID", "Type to search...", m_searchBuffer, sizeof(m_searchBuffer));
    std::string searchFilter(m_searchBuffer);
    std::transform(searchFilter.begin(), searchFilter.end(), searchFilter.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    ImGui::Separator();

    // Split view: Left list, Right preview
    ImGui::Columns(2, "AssetBrowserColumns", true);
    ImGui::SetColumnWidth(0, 180.0f);

    // Left List
    ImGui::BeginChild("AssetListScroll", ImVec2(0, 260), true);
    for (const auto& meta : allAssets) {
        // Apply category filter
        if (m_selectedCategory != "All" && meta.category != m_selectedCategory) {
            continue;
        }

        // Apply search filter
        if (!searchFilter.empty()) {
            std::string idLower = meta.id;
            std::transform(idLower.begin(), idLower.end(), idLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (idLower.find(searchFilter) == std::string::npos) {
                continue;
            }
        }

        bool isSelected = (m_selectedAssetId == meta.id);
        ImVec4 color = (meta.status == resources::AssetStatus::Approved)
                       ? ImVec4(0.4f, 0.9f, 0.5f, 1.0f)
                       : ImVec4(0.9f, 0.7f, 0.3f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (ImGui::Selectable(meta.id.c_str(), isSelected)) {
            m_selectedAssetId = meta.id;
            m_previewFrameIndex = 0;
        }
        ImGui::PopStyleColor();

        if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right Details & Preview
    ImGui::BeginChild("AssetDetailScroll", ImVec2(0, 260), true);
    const resources::AssetMetadata* selectedMeta = registry.GetMetadata(m_selectedAssetId);
    if (selectedMeta) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "ID: %s", selectedMeta->id.c_str());
        ImGui::Text("File: %s", selectedMeta->file.c_str());
        ImGui::Text("Type: %s", std::string(resources::AssetMetadata::TypeToString(selectedMeta->type)).c_str());
        ImGui::Text("Category: %s", selectedMeta->category.c_str());
        ImGui::Text("License: %s", selectedMeta->license.c_str());
        ImGui::Text("Author: %s (%s)", selectedMeta->author.c_str(), selectedMeta->source.c_str());
        ImGui::Text("Commercial: %s | Attribution: %s",
                    selectedMeta->commercialUse ? "Yes" : "No",
                    selectedMeta->attributionRequired ? "Yes" : "No");

        // Texture Thumbnail Preview
        if (selectedMeta->type == resources::AssetType::Texture) {
            Texture2D tex = assetMgr->GetTexture(selectedMeta->id);
            if (tex.id != 0) {
                ImGui::Separator();
                ImGui::Text("Dimensions: %d x %d px", tex.width, tex.height);
                float previewScale = std::min(128.0f / tex.width, 128.0f / tex.height);
                if (previewScale <= 0.0f || previewScale > 4.0f) previewScale = 2.0f;
                int pw = static_cast<int>(tex.width * previewScale);
                int ph = static_cast<int>(tex.height * previewScale);
                rlImGuiImageSize(&tex, pw, ph);
            }
        }
        // Spritesheet Preview
        else if (selectedMeta->type == resources::AssetType::SpriteSheet) {
            const resources::SpriteSheet* sheet = assetMgr->GetSpriteSheet(selectedMeta->id);
            if (sheet) {
                Texture2D tex = assetMgr->GetTexture(sheet->GetTextureId());
                ImGui::Separator();
                ImGui::Text("Frames: %d (%d x %d)", sheet->GetFrameCount(), sheet->GetFrameWidth(), sheet->GetFrameHeight());
                if (sheet->GetFrameCount() > 1) {
                    ImGui::SliderInt("Frame", &m_previewFrameIndex, 0, sheet->GetFrameCount() - 1);
                }
                if (tex.id != 0) {
                    Rectangle srcRect = sheet->GetFrameRect(m_previewFrameIndex);
                    rlImGuiImageRect(&tex, sheet->GetFrameWidth() * 2, sheet->GetFrameHeight() * 2, srcRect);
                }
            }
        }
    } else {
        ImGui::TextDisabled("Select an asset on the left to inspect.");
    }
    ImGui::EndChild();

    ImGui::Columns(1);
}

void DevUI::RenderAssetValidation(resources::AssetManager* assetMgr) {
    if (!assetMgr) return;

    if (ImGui::Button("Run Asset Validation Scan")) {
        // Trigger scan
    }

    resources::ValidationReport report = assetMgr->ValidateAssets();
    ImGui::Separator();

    ImGui::Text("Total Registered Assets: %zu", report.totalAssetsChecked);
    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Passed: %zu", report.passedCount);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "Warnings: %zu", report.warningsCount);
    ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Errors: %zu", report.errorsCount);
    ImGui::Separator();

    ImGui::BeginChild("ValidationIssuesList", ImVec2(0, 200), true);
    if (report.issues.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "All manifest assets verified clean and valid.");
    } else {
        for (const auto& issue : report.issues) {
            if (issue.severity == resources::ValidationSeverity::Error) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "[ERROR] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            } else if (issue.severity == resources::ValidationSeverity::Warning) {
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "[WARN] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            } else {
                ImGui::Text("[INFO] %s: %s", issue.assetId.c_str(), issue.message.c_str());
            }
        }
    }
    ImGui::EndChild();
}

} // namespace wildbound::ui
