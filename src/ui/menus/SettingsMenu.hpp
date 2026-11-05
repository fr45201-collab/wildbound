#pragma once

#include "../UITheme.hpp"
#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::input {
    class InputManager;
    class InputSettings;
}

namespace wildbound::ui {

enum class SettingsTab {
    Graphics = 0,
    Audio,
    Controls,
    Gameplay
};

/**
 * Full-featured Settings Menu with Graphics, Audio (5 buses + Mute All),
 * Controls rebinding (with InputSettings conflict detection), and Gameplay toggles.
 */
class SettingsMenu {
public:
    SettingsMenu();
    ~SettingsMenu() = default;

    void Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed, bool mouseLeftDown,
                input::InputManager* inputMgr = nullptr,
                audio::AudioSystem* audioSys = nullptr);

    void Render(audio::AudioSystem* audioSys = nullptr,
                input::InputManager* inputMgr = nullptr);

    [[nodiscard]] SettingsTab GetCurrentTab() const { return m_currentTab; }
    void SetCurrentTab(SettingsTab tab) { m_currentTab = tab; }

    [[nodiscard]] bool RequestBack() const { return m_requestBack; }
    void ClearRequestBack() { m_requestBack = false; }

    // Gameplay settings toggles
    [[nodiscard]] bool IsMinimapEnabled() const { return m_minimapEnabled; }
    [[nodiscard]] bool IsHUDEnabled() const { return m_hudEnabled; }
    [[nodiscard]] bool AreDamageNumbersEnabled() const { return m_damageNumbersEnabled; }

    void SetMinimapEnabled(bool enabled) { m_minimapEnabled = enabled; }
    void SetHUDEnabled(bool enabled) { m_hudEnabled = enabled; }
    void SetDamageNumbersEnabled(bool enabled) { m_damageNumbersEnabled = enabled; }

private:
    void RenderGraphicsTab();
    void RenderAudioTab(audio::AudioSystem* audioSys);
    void RenderControlsTab(input::InputManager* inputMgr);
    void RenderGameplayTab();

    bool DrawSlider(Rectangle bounds, const std::string& label, float& value, float minVal = 0.0f, float maxVal = 1.0f);
    bool DrawToggle(Rectangle bounds, const std::string& label, bool& state);
    bool CheckHover(Rectangle r) const;

    SettingsTab m_currentTab = SettingsTab::Audio;
    bool m_requestBack = false;

    // Graphics Settings
    int m_selectedResolutionIdx = 1; // 0=960x540, 1=1280x720, 2=1920x1080
    bool m_fullscreen = false;
    bool m_vsync = true;
    int m_targetFpsIdx = 2; // 0=30, 1=60, 2=144, 3=Unlimited

    // Audio Cache (synchronizes with AudioSystem)
    float m_volMaster = 1.0f;
    float m_volMusic = 0.7f;
    float m_volSFX = 0.8f;
    float m_volUI = 0.9f;
    float m_volEnv = 0.75f;
    bool m_muteAll = false;

    // Gameplay Settings
    bool m_minimapEnabled = true;
    bool m_hudEnabled = true;
    bool m_damageNumbersEnabled = true;

    // Controls Rebinding
    int m_controlsScrollOffset = 0;
    bool m_isRebinding = false;
    int m_rebindingActionIdx = -1;

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;
    bool m_mouseDown = false;
};

} // namespace wildbound::ui

