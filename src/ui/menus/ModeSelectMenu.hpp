#pragma once

#include "../UITheme.hpp"
#include "gamemodes/GameModeDefinition.hpp"
#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::audio {
class AudioSystem;
}

namespace wildbound::gamemodes {
class GameModeRegistry;
class GameModeManager;
}

namespace wildbound::ui {

/**
 * Player-Facing Game Mode Selection Menu.
 * Allows browsing available Battle Royale, Practice, and Bot modes,
 * configuring match parameters, and launching into gameplay.
 */
class ModeSelectMenu {
public:
    ModeSelectMenu();
    ~ModeSelectMenu() = default;

    void Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                bool navUp = false, bool navDown = false,
                bool navLeft = false, bool navRight = false,
                bool navConfirm = false, bool navCancel = false,
                audio::AudioSystem* audioSys = nullptr);

    void Render(const gamemodes::GameModeRegistry& registry);

    [[nodiscard]] int GetSelectedModeIndex() const noexcept { return m_selectedModeIndex; }
    void SetSelectedModeIndex(int index) noexcept { m_selectedModeIndex = index; }

    [[nodiscard]] const std::string& GetSelectedModeId() const;

    [[nodiscard]] uint32_t GetConfiguredBotCount() const noexcept { return m_botCount; }
    [[nodiscard]] const std::string& GetConfiguredDifficulty() const;

    [[nodiscard]] bool RequestStartMatch() const noexcept { return m_requestStart; }
    void ClearRequestStartMatch() noexcept { m_requestStart = false; }

    [[nodiscard]] bool RequestBack() const noexcept { return m_requestBack; }
    void ClearRequestBack() noexcept { m_requestBack = false; }

    void SyncFromActiveMode(const gamemodes::GameModeDefinition& def);

private:
    struct ModeCardEntry {
        std::string id;
        std::string title;
        std::string subtitle;
    };

    std::vector<ModeCardEntry> m_modeEntries;
    int m_selectedModeIndex = 0;
    uint32_t m_botCount = 15;
    int m_difficultyIndex = 1; // 0 = Easy, 1 = Normal, 2 = Hard
    bool m_stormEnabled = true;

    bool m_requestStart = false;
    bool m_requestBack = false;

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed = false;

    bool CheckHover(Rectangle r) const;
    void DrawCard(Rectangle bounds, const ModeCardEntry& entry, bool isSelected, bool isHovered);
};

} // namespace wildbound::ui

