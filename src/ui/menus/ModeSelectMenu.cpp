#include "ModeSelectMenu.hpp"
#include "gamemodes/GameModeRegistry.hpp"
#include "audio/AudioSystem.hpp"
#include <algorithm>

namespace wildbound::ui {

ModeSelectMenu::ModeSelectMenu() {
    m_modeEntries = {
        { "solo", "Solo Battle Royale", "Every warrior for themselves" },
        { "duos", "Duos Battle Royale", "Cooperate in 2-player teams" },
        { "squads", "Squads Battle Royale", "Dominate in 4-player teams" },
        { "practice", "Practice Arena", "Freeplay combat & testing" },
        { "offline_bots", "Offline Bot Matches", "Custom offline AI competitors" }
    };
    m_selectedModeIndex = 0;
    m_botCount = 15;
    m_difficultyIndex = 1;
    m_stormEnabled = true;
}

const std::string& ModeSelectMenu::GetSelectedModeId() const {
    if (m_selectedModeIndex >= 0 && m_selectedModeIndex < static_cast<int>(m_modeEntries.size())) {
        return m_modeEntries[m_selectedModeIndex].id;
    }
    static const std::string fallback = "solo";
    return fallback;
}

const std::string& ModeSelectMenu::GetConfiguredDifficulty() const {
    static const std::string easy = "easy";
    static const std::string normal = "normal";
    static const std::string hard = "hard";
    if (m_difficultyIndex == 0) return easy;
    if (m_difficultyIndex == 2) return hard;
    return normal;
}

void ModeSelectMenu::SyncFromActiveMode(const gamemodes::GameModeDefinition& def) {
    for (size_t i = 0; i < m_modeEntries.size(); ++i) {
        if (m_modeEntries[i].id == def.id) {
            m_selectedModeIndex = static_cast<int>(i);
            break;
        }
    }
    m_botCount = def.defaultBotCount;
    if (def.botDifficulty == "easy") m_difficultyIndex = 0;
    else if (def.botDifficulty == "hard") m_difficultyIndex = 2;
    else m_difficultyIndex = 1;
    m_stormEnabled = def.stormEnabled;
}

bool ModeSelectMenu::CheckHover(Rectangle r) const {
    return (m_mousePos.x >= r.x && m_mousePos.x <= (r.x + r.width) &&
            m_mousePos.y >= r.y && m_mousePos.y <= (r.y + r.height));
}

void ModeSelectMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed,
                            bool navUp, bool navDown,
                            bool navLeft, bool navRight,
                            bool navConfirm, bool navCancel,
                            audio::AudioSystem* audioSys) {
    (void)dt;
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;

    if (navCancel) {
        m_requestBack = true;
        if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
        return;
    }

    if (navUp) {
        if (m_selectedModeIndex > 0) {
            --m_selectedModeIndex;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }
    if (navDown) {
        if (m_selectedModeIndex < static_cast<int>(m_modeEntries.size()) - 1) {
            ++m_selectedModeIndex;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }

    if (navLeft) {
        if (m_botCount > 1) {
            --m_botCount;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }
    if (navRight) {
        if (m_botCount < 32) {
            ++m_botCount;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }

    if (navConfirm) {
        m_requestStart = true;
        if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
        return;
    }

    // Check mode card clicking
    float startY = 60.0f;
    float cardH = 46.0f;
    float cardW = 220.0f;
    float gap = 8.0f;

    for (int i = 0; i < static_cast<int>(m_modeEntries.size()); ++i) {
        Rectangle cardRect{ 24.0f, startY + i * (cardH + gap), cardW, cardH };
        if (CheckHover(cardRect) && m_mousePressed) {
            if (m_selectedModeIndex != i) {
                m_selectedModeIndex = i;
                if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
            }
        }
    }

    // Bot count adjustment buttons
    Rectangle btnDecBot{ 430.0f, 185.0f, 22.0f, 20.0f };
    Rectangle btnIncBot{ 530.0f, 185.0f, 22.0f, 20.0f };
    if (CheckHover(btnDecBot) && m_mousePressed) {
        if (m_botCount > 1) {
            --m_botCount;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }
    if (CheckHover(btnIncBot) && m_mousePressed) {
        if (m_botCount < 32) {
            ++m_botCount;
            if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
        }
    }

    // Difficulty toggle button
    Rectangle btnDiff{ 430.0f, 215.0f, 122.0f, 20.0f };
    if (CheckHover(btnDiff) && m_mousePressed) {
        m_difficultyIndex = (m_difficultyIndex + 1) % 3;
        if (audioSys) audioSys->DispatchEvent("UI.ButtonHover");
    }

    // Bottom action buttons
    Rectangle btnPlay{ 420.0f, 305.0f, 180.0f, 34.0f };
    Rectangle btnBack{ 260.0f, 305.0f, 140.0f, 34.0f };

    if (CheckHover(btnPlay) && m_mousePressed) {
        m_requestStart = true;
        if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
    }
    if (CheckHover(btnBack) && m_mousePressed) {
        m_requestBack = true;
        if (audioSys) audioSys->DispatchEvent("UI.ButtonClick");
    }
}

void ModeSelectMenu::DrawCard(Rectangle bounds, const ModeCardEntry& entry, bool isSelected, bool isHovered) {
    Color bg = isSelected ? Color{ 50, 40, 20, 230 } : (isHovered ? Color{ 35, 35, 45, 220 } : Color{ 22, 22, 30, 200 });
    Color border = isSelected ? UITheme::Gold : (isHovered ? UITheme::BorderLight : UITheme::BorderNormal);

    DrawRectangleRec(bounds, bg);
    DrawRectangleLinesEx(bounds, isSelected ? 2.0f : 1.0f, border);

    Color textCol = isSelected ? UITheme::Gold : (isHovered ? UITheme::TextLight : UITheme::TextMuted);
    DrawText(entry.title.c_str(), static_cast<int>(bounds.x + 10.0f), static_cast<int>(bounds.y + 8.0f), 12, textCol);
    DrawText(entry.subtitle.c_str(), static_cast<int>(bounds.x + 10.0f), static_cast<int>(bounds.y + 24.0f), 9, UITheme::TextDim);
}

void ModeSelectMenu::Render(const gamemodes::GameModeRegistry& registry) {
    // Backdrop
    DrawRectangle(0, 0, 640, 360, Color{ 10, 10, 16, 235 });

    // Header banner
    DrawRectangle(0, 0, 640, 42, Color{ 18, 18, 26, 255 });
    DrawRectangle(0, 40, 640, 2, UITheme::Gold);
    DrawText("SELECT GAME MODE", 24, 12, 18, UITheme::Gold);

    // Left Column: Mode Cards
    float startY = 56.0f;
    float cardH = 46.0f;
    float cardW = 220.0f;
    float gap = 8.0f;

    for (int i = 0; i < static_cast<int>(m_modeEntries.size()); ++i) {
        Rectangle cardRect{ 24.0f, startY + i * (cardH + gap), cardW, cardH };
        bool isSelected = (m_selectedModeIndex == i);
        bool isHovered = CheckHover(cardRect);
        DrawCard(cardRect, m_modeEntries[i], isSelected, isHovered);
    }

    // Right Column: Details Panel
    Rectangle detailsRect{ 260.0f, 56.0f, 356.0f, 236.0f };
    DrawRectangleRec(detailsRect, Color{ 18, 18, 26, 220 });
    DrawRectangleLinesEx(detailsRect, 1.0f, UITheme::BorderNormal);

    const std::string& currentId = GetSelectedModeId();
    const auto* modeDef = registry.GetMode(currentId);

    if (modeDef) {
        // Mode Title
        DrawText(modeDef->displayName.c_str(), 276, 70, 16, UITheme::Gold);

        // Status badge
        const char* statusBadge = modeDef->isAvailable ? "[ PLAYABLE OFFLINE ]" : "[ FUTURE ONLINE ]";
        Color badgeColor = modeDef->isAvailable ? UITheme::TextSuccess : UITheme::TextWarning;
        DrawText(statusBadge, 480, 72, 9, badgeColor);

        // Description
        DrawText(modeDef->description.c_str(), 276, 96, 10, UITheme::TextMuted);

        // Parameters table
        DrawLine(276, 130, 600, 130, UITheme::BorderNormal);

        std::string teamStr = modeDef->teamsEnabled ? ("Enabled (" + std::to_string(modeDef->teamSize) + " players / squad)") : "Disabled (Solo)";
        DrawText("Team Structure:", 276, 140, 10, UITheme::TextDim);
        DrawText(teamStr.c_str(), 430, 140, 10, UITheme::TextLight);

        std::string partStr = std::to_string(modeDef->minimumParticipants) + " - " + std::to_string(modeDef->maximumParticipants);
        DrawText("Participant Range:", 276, 160, 10, UITheme::TextDim);
        DrawText(partStr.c_str(), 430, 160, 10, UITheme::TextLight);

        if (modeDef->botsAllowed) {
            DrawText("AI Competitors:", 276, 188, 10, UITheme::TextDim);

            // [-] [count] [+]
            Rectangle btnDecBot{ 430.0f, 185.0f, 22.0f, 20.0f };
            Rectangle btnIncBot{ 530.0f, 185.0f, 22.0f, 20.0f };

            DrawRectangleRec(btnDecBot, CheckHover(btnDecBot) ? UITheme::SlotHovered : UITheme::SlotFilled);
            DrawRectangleLinesEx(btnDecBot, 1.0f, UITheme::BorderNormal);
            DrawText("-", 438, 188, 12, UITheme::TextLight);

            std::string botCntStr = std::to_string(m_botCount) + " bots";
            DrawText(botCntStr.c_str(), 462, 188, 10, UITheme::TextLight);

            DrawRectangleRec(btnIncBot, CheckHover(btnIncBot) ? UITheme::SlotHovered : UITheme::SlotFilled);
            DrawRectangleLinesEx(btnIncBot, 1.0f, UITheme::BorderNormal);
            DrawText("+", 538, 188, 12, UITheme::TextLight);

            // Difficulty toggle
            DrawText("AI Difficulty:", 276, 218, 10, UITheme::TextDim);
            Rectangle btnDiff{ 430.0f, 215.0f, 122.0f, 20.0f };
            DrawRectangleRec(btnDiff, CheckHover(btnDiff) ? UITheme::SlotHovered : UITheme::SlotFilled);
            DrawRectangleLinesEx(btnDiff, 1.0f, UITheme::BorderNormal);

            std::string diffLabel = "< " + GetConfiguredDifficulty() + " >";
            DrawText(diffLabel.c_str(), 442, 218, 10, UITheme::Gold);
        } else {
            DrawText("AI Competitors:", 276, 188, 10, UITheme::TextDim);
            DrawText("Disabled", 430, 188, 10, UITheme::TextMuted);

            DrawText("Practice Hazard:", 276, 218, 10, UITheme::TextDim);
            DrawText("Free Exploration", 430, 218, 10, UITheme::TextMuted);
        }

        std::string stormStr = modeDef->stormEnabled ? "Corruption Storm Active" : "Disabled";
        DrawText("Storm System:", 276, 248, 10, UITheme::TextDim);
        DrawText(stormStr.c_str(), 430, 248, 10, modeDef->stormEnabled ? UITheme::TextWarning : UITheme::TextMuted);
    }

    // Action Buttons
    Rectangle btnBack{ 260.0f, 305.0f, 140.0f, 34.0f };
    Rectangle btnPlay{ 420.0f, 305.0f, 196.0f, 34.0f };

    bool hoverBack = CheckHover(btnBack);
    DrawRectangleRec(btnBack, hoverBack ? UITheme::SlotHovered : UITheme::SlotFilled);
    DrawRectangleLinesEx(btnBack, 1.0f, hoverBack ? UITheme::BorderLight : UITheme::BorderNormal);
    DrawText("BACK", 310, 316, 12, hoverBack ? UITheme::TextLight : UITheme::TextMuted);

    bool hoverPlay = CheckHover(btnPlay);
    DrawRectangleRec(btnPlay, hoverPlay ? Color{ 80, 60, 20, 255 } : Color{ 50, 40, 15, 255 });
    DrawRectangleLinesEx(btnPlay, 1.5f, hoverPlay ? UITheme::Gold : Color{ 160, 120, 40, 255 });
    DrawText("START MATCH", 474, 316, 12, UITheme::Gold);
}

} // namespace wildbound::ui
