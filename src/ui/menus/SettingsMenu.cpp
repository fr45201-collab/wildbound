#include "SettingsMenu.hpp"
#include "../../audio/AudioSystem.hpp"
#include "../../input/InputManager.hpp"
#include "../../input/InputSettings.hpp"
#include <cstdio>
#include <algorithm>

namespace wildbound::ui {

SettingsMenu::SettingsMenu() {
}

bool SettingsMenu::CheckHover(Rectangle r) const {
    return (m_mousePos.x >= r.x && m_mousePos.x <= r.x + r.width &&
            m_mousePos.y >= r.y && m_mousePos.y <= r.y + r.height);
}

void SettingsMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown,
                          input::InputManager* inputMgr,
                          audio::AudioSystem* audioSys) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;
    m_mouseDown = mouseLeftDown;

    // Pull current audio values from AudioSystem on first or regular update
    if (audioSys) {
        if (!m_mouseDown) { // Don't overwrite while dragging slider
            m_volMaster = audioSys->GetBusVolume(audio::AudioBus::Master);
            m_volMusic = audioSys->GetBusVolume(audio::AudioBus::Music);
            m_volSFX = audioSys->GetBusVolume(audio::AudioBus::SFX);
            m_volUI = audioSys->GetBusVolume(audio::AudioBus::UI);
            m_volEnv = audioSys->GetBusVolume(audio::AudioBus::Environment);
            m_muteAll = audioSys->IsMuteAll();
        }
    }

    // Tab Bar click handling
    const char* tabs[] = { "GRAPHICS", "AUDIO", "CONTROLS", "GAMEPLAY" };
    for (int i = 0; i < 4; ++i) {
        Rectangle tabRect{ 160.0f + i * 85.0f, 40.0f, 80.0f, 22.0f };
        if (m_mousePressed && CheckHover(tabRect)) {
            m_currentTab = static_cast<SettingsTab>(i);
        }
    }

    // Back button click handling
    Rectangle backBtnRect{ 24.0f, 20.0f, 60.0f, 22.0f };
    if (m_mousePressed && CheckHover(backBtnRect)) {
        m_requestBack = true;
    }
}

bool SettingsMenu::DrawSlider(Rectangle bounds, const std::string& label, float& value, float minVal, float maxVal) {
    DrawText(label.c_str(), static_cast<int>(bounds.x), static_cast<int>(bounds.y - 12), UITheme::FontSmall, UITheme::TextLight);

    // Track
    DrawRectangleRec(bounds, UITheme::DarkBgOpaque);
    DrawRectangleLinesEx(bounds, 1.0f, UITheme::BorderNormal);

    // Fill
    float norm = std::clamp((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y),
                  static_cast<int>(bounds.width * norm), static_cast<int>(bounds.height), UITheme::Gold);

    // Handle
    float handleX = bounds.x + bounds.width * norm;
    DrawRectangle(static_cast<int>(handleX - 3), static_cast<int>(bounds.y - 2), 6, static_cast<int>(bounds.height + 4), UITheme::TextLight);

    // Value percentage text
    char valBuf[16];
    std::snprintf(valBuf, sizeof(valBuf), "%d%%", static_cast<int>(norm * 100.0f));
    DrawText(valBuf, static_cast<int>(bounds.x + bounds.width + 12), static_cast<int>(bounds.y), UITheme::FontSmall, UITheme::TextMuted);

    // Drag interaction
    Rectangle hitArea = { bounds.x - 5, bounds.y - 5, bounds.width + 10, bounds.height + 10 };
    if (m_mouseDown && CheckHover(hitArea)) {
        float newNorm = std::clamp((m_mousePos.x - bounds.x) / bounds.width, 0.0f, 1.0f);
        value = minVal + newNorm * (maxVal - minVal);
        return true;
    }
    return false;
}

bool SettingsMenu::DrawToggle(Rectangle bounds, const std::string& label, bool& state) {
    DrawText(label.c_str(), static_cast<int>(bounds.x), static_cast<int>(bounds.y + 3), UITheme::FontSmall, UITheme::TextLight);

    Rectangle toggleRect = { bounds.x + bounds.width - 40.0f, bounds.y, 40.0f, 18.0f };
    DrawRectangleRec(toggleRect, state ? UITheme::Gold : UITheme::SlotEmpty);
    DrawRectangleLinesEx(toggleRect, 1.0f, UITheme::BorderNormal);

    float knobX = state ? (toggleRect.x + toggleRect.width - 16.0f) : (toggleRect.x + 2.0f);
    DrawRectangle(static_cast<int>(knobX), static_cast<int>(toggleRect.y + 2), 14, 14, UITheme::TextLight);

    if (m_mousePressed && CheckHover(toggleRect)) {
        state = !state;
        return true;
    }
    return false;
}

void SettingsMenu::Render(audio::AudioSystem* audioSys, input::InputManager* inputMgr) {
    // 1. Modal Background
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 10, 12, 16, 235 });

    // Header Panel
    Rectangle panelRect{ 20.0f, 15.0f, 600.0f, 330.0f };
    DrawRectangleRec(panelRect, UITheme::PanelBg);
    DrawRectangleLinesEx(panelRect, 1.0f, UITheme::BorderGoldDim);

    // Title
    DrawText("SETTINGS", static_cast<int>(panelRect.x + 260.0f), 20, UITheme::FontLarge, UITheme::Gold);

    // Back Button
    Rectangle backRect{ 30.0f, 20.0f, 60.0f, 20.0f };
    bool backHov = CheckHover(backRect);
    DrawRectangleRec(backRect, backHov ? UITheme::SlotHovered : UITheme::SlotEmpty);
    DrawRectangleLinesEx(backRect, 1.0f, backHov ? UITheme::Gold : UITheme::BorderNormal);
    DrawText("< BACK", static_cast<int>(backRect.x + 10.0f), static_cast<int>(backRect.y + 4.0f), UITheme::FontSmall, backHov ? UITheme::Gold : UITheme::TextLight);

    // 2. Tab Navigation Bar
    const char* tabs[] = { "GRAPHICS", "AUDIO", "CONTROLS", "GAMEPLAY" };
    for (int i = 0; i < 4; ++i) {
        Rectangle tabRect{ 145.0f + i * 90.0f, 44.0f, 82.0f, 22.0f };
        bool isSelected = (static_cast<int>(m_currentTab) == i);
        bool isHov = CheckHover(tabRect);

        DrawRectangleRec(tabRect, isSelected ? UITheme::SlotSelected : (isHov ? UITheme::SlotHovered : UITheme::SlotEmpty));
        DrawRectangleLinesEx(tabRect, 1.0f, isSelected ? UITheme::BorderGold : UITheme::BorderNormal);

        int tw = MeasureText(tabs[i], UITheme::FontSmall);
        DrawText(tabs[i], static_cast<int>(tabRect.x + (tabRect.width - tw) * 0.5f), static_cast<int>(tabRect.y + 5.0f),
                 UITheme::FontSmall, isSelected ? UITheme::Gold : UITheme::TextLight);
    }

    // Horizontal Divider
    DrawLine(30, 72, 610, 72, UITheme::BorderNormal);

    // 3. Tab Contents
    switch (m_currentTab) {
        case SettingsTab::Graphics: RenderGraphicsTab(); break;
        case SettingsTab::Audio:    RenderAudioTab(audioSys); break;
        case SettingsTab::Controls: RenderControlsTab(inputMgr); break;
        case SettingsTab::Gameplay: RenderGameplayTab(); break;
    }
}

void SettingsMenu::RenderGraphicsTab() {
    float startY = 95.0f;

    // Resolution Choice
    DrawText("Resolution Target", 45, static_cast<int>(startY), UITheme::FontSmall, UITheme::TextLight);
    const char* resOptions[] = { "960 x 540", "1280 x 720 (Default)", "1920 x 1080" };
    for (int i = 0; i < 3; ++i) {
        Rectangle r{ 220.0f + i * 125.0f, startY - 4.0f, 115.0f, 20.0f };
        bool sel = (m_selectedResolutionIdx == i);
        bool hov = CheckHover(r);
        DrawRectangleRec(r, sel ? UITheme::SlotSelected : (hov ? UITheme::SlotHovered : UITheme::SlotEmpty));
        DrawRectangleLinesEx(r, 1.0f, sel ? UITheme::BorderGold : UITheme::BorderNormal);
        int tw = MeasureText(resOptions[i], UITheme::FontMicro);
        DrawText(resOptions[i], static_cast<int>(r.x + (r.width - tw) * 0.5f), static_cast<int>(r.y + 5.0f), UITheme::FontMicro, sel ? UITheme::Gold : UITheme::TextLight);
        if (m_mousePressed && hov) m_selectedResolutionIdx = i;
    }

    // Fullscreen Toggle
    startY += 36.0f;
    Rectangle fsRect{ 45.0f, startY, 520.0f, 20.0f };
    DrawToggle(fsRect, "Fullscreen Mode", m_fullscreen);

    // VSync Toggle
    startY += 32.0f;
    Rectangle vsyncRect{ 45.0f, startY, 520.0f, 20.0f };
    DrawToggle(vsyncRect, "Vertical Sync (VSync)", m_vsync);

    // Target Framerate
    startY += 36.0f;
    DrawText("Target Framerate", 45, static_cast<int>(startY), UITheme::FontSmall, UITheme::TextLight);
    const char* fpsOptions[] = { "30 FPS", "60 FPS", "144 FPS", "Max Unlimited" };
    for (int i = 0; i < 4; ++i) {
        Rectangle r{ 220.0f + i * 90.0f, startY - 4.0f, 82.0f, 20.0f };
        bool sel = (m_targetFpsIdx == i);
        bool hov = CheckHover(r);
        DrawRectangleRec(r, sel ? UITheme::SlotSelected : (hov ? UITheme::SlotHovered : UITheme::SlotEmpty));
        DrawRectangleLinesEx(r, 1.0f, sel ? UITheme::BorderGold : UITheme::BorderNormal);
        int tw = MeasureText(fpsOptions[i], UITheme::FontMicro);
        DrawText(fpsOptions[i], static_cast<int>(r.x + (r.width - tw) * 0.5f), static_cast<int>(r.y + 5.0f), UITheme::FontMicro, sel ? UITheme::Gold : UITheme::TextLight);
        if (m_mousePressed && hov) m_targetFpsIdx = i;
    }
}

void SettingsMenu::RenderAudioTab(audio::AudioSystem* audioSys) {
    float startY = 100.0f;

    // Master Volume
    Rectangle rMaster{ 180.0f, startY, 320.0f, 12.0f };
    if (DrawSlider(rMaster, "Master Volume", m_volMaster) && audioSys) {
        audioSys->SetMasterVolume(m_volMaster);
    }

    // Music Volume
    startY += 34.0f;
    Rectangle rMusic{ 180.0f, startY, 320.0f, 12.0f };
    if (DrawSlider(rMusic, "Music Volume", m_volMusic) && audioSys) {
        audioSys->SetBusVolume(audio::AudioBus::Music, m_volMusic);
    }

    // SFX Volume
    startY += 34.0f;
    Rectangle rSFX{ 180.0f, startY, 320.0f, 12.0f };
    if (DrawSlider(rSFX, "SFX Volume", m_volSFX) && audioSys) {
        audioSys->SetBusVolume(audio::AudioBus::SFX, m_volSFX);
    }

    // UI Volume
    startY += 34.0f;
    Rectangle rUI{ 180.0f, startY, 320.0f, 12.0f };
    if (DrawSlider(rUI, "UI Volume", m_volUI) && audioSys) {
        audioSys->SetBusVolume(audio::AudioBus::UI, m_volUI);
    }

    // Environment Volume
    startY += 34.0f;
    Rectangle rEnv{ 180.0f, startY, 320.0f, 12.0f };
    if (DrawSlider(rEnv, "Ambience & Environment", m_volEnv) && audioSys) {
        audioSys->SetBusVolume(audio::AudioBus::Environment, m_volEnv);
    }

    // Mute All Toggle
    startY += 32.0f;
    Rectangle muteRect{ 180.0f, startY, 320.0f, 20.0f };
    if (DrawToggle(muteRect, "Mute All Sound", m_muteAll) && audioSys) {
        audioSys->SetMuteAll(m_muteAll);
    }
}

void SettingsMenu::RenderControlsTab(input::InputManager* inputMgr) {
    float startY = 88.0f;

    DrawText("Action", 50, static_cast<int>(startY), UITheme::FontSmall, UITheme::Gold);
    DrawText("Primary Key", 240, static_cast<int>(startY), UITheme::FontSmall, UITheme::Gold);
    DrawText("Alternate Key", 380, static_cast<int>(startY), UITheme::FontSmall, UITheme::Gold);

    DrawLine(45, static_cast<int>(startY + 15), 590, static_cast<int>(startY + 15), UITheme::BorderNormal);
    startY += 20.0f;

    struct BindingDisplay {
        const char* name;
        const char* primary;
        const char* alternate;
    };

    static const BindingDisplay bindings[] = {
        { "Move Up", "W", "Up Arrow" },
        { "Move Down", "S", "Down Arrow" },
        { "Move Left", "A", "Left Arrow" },
        { "Move Right", "D", "Right Arrow" },
        { "Attack / Fire", "Left Mouse", "Space" },
        { "Interact", "E", "Enter" },
        { "Sprint", "Left Shift", "Right Shift" },
        { "Dodge / Roll", "Space", "Num 0" },
        { "Inventory", "I", "Tab" },
        { "Wardrobe", "K", "None" }
    };

    for (size_t i = 0; i < 7; ++i) {
        float rowY = startY + i * 22.0f;
        DrawText(bindings[i].name, 50, static_cast<int>(rowY + 2), UITheme::FontMicro, UITheme::TextLight);

        // Primary key pill
        Rectangle pRect{ 235.0f, rowY, 90.0f, 18.0f };
        DrawRectangleRec(pRect, UITheme::SlotEmpty);
        DrawRectangleLinesEx(pRect, 1.0f, UITheme::BorderNormal);
        int tw1 = MeasureText(bindings[i].primary, UITheme::FontMicro);
        DrawText(bindings[i].primary, static_cast<int>(pRect.x + (pRect.width - tw1) * 0.5f), static_cast<int>(pRect.y + 4), UITheme::FontMicro, UITheme::Gold);

        // Alternate key pill
        Rectangle aRect{ 375.0f, rowY, 90.0f, 18.0f };
        DrawRectangleRec(aRect, UITheme::SlotEmpty);
        DrawRectangleLinesEx(aRect, 1.0f, UITheme::BorderNormal);
        int tw2 = MeasureText(bindings[i].alternate, UITheme::FontMicro);
        DrawText(bindings[i].alternate, static_cast<int>(aRect.x + (aRect.width - tw2) * 0.5f), static_cast<int>(aRect.y + 4), UITheme::FontMicro, UITheme::TextMuted);
    }

    // Reset Defaults Button
    Rectangle resetRect{ 240.0f, 310.0f, 140.0f, 22.0f };
    bool resetHov = CheckHover(resetRect);
    DrawRectangleRec(resetRect, resetHov ? UITheme::SlotHovered : UITheme::SlotEmpty);
    DrawRectangleLinesEx(resetRect, 1.0f, resetHov ? UITheme::Gold : UITheme::BorderNormal);
    int rw = MeasureText("Reset To Defaults", UITheme::FontSmall);
    DrawText("Reset To Defaults", static_cast<int>(resetRect.x + (resetRect.width - rw) * 0.5f), static_cast<int>(resetRect.y + 5), UITheme::FontSmall, resetHov ? UITheme::Gold : UITheme::TextLight);

    if (m_mousePressed && resetHov && inputMgr) {
        inputMgr->GetSettings().ResetToDefaults();
    }
}

void SettingsMenu::RenderGameplayTab() {
    float startY = 100.0f;

    // Minimap Toggle
    Rectangle mmRect{ 45.0f, startY, 520.0f, 20.0f };
    DrawToggle(mmRect, "Enable Minimap Radar", m_minimapEnabled);

    // HUD Toggle
    startY += 34.0f;
    Rectangle hudRect{ 45.0f, startY, 520.0f, 20.0f };
    DrawToggle(hudRect, "Enable Player Vitals HUD", m_hudEnabled);

    // Damage Numbers
    startY += 34.0f;
    Rectangle dmgRect{ 45.0f, startY, 520.0f, 20.0f };
    DrawToggle(dmgRect, "Floating Combat Damage Numbers", m_damageNumbersEnabled);
}

} // namespace wildbound::ui

