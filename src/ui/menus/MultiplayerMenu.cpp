#include "MultiplayerMenu.hpp"
#include <string>

namespace wildbound::ui {

MultiplayerMenu::MultiplayerMenu() = default;

bool MultiplayerMenu::CheckHover(Rectangle r) const {
    return CheckCollisionPointRec(m_mousePos, r);
}

void MultiplayerMenu::DrawButton(Rectangle r, const std::string& text, bool isHovered, bool isSelected) {
    Color bg = isSelected ? UITheme::SlotSelected : (isHovered ? UITheme::SlotHovered : UITheme::SlotFilled);
    Color border = isSelected ? UITheme::Gold : (isHovered ? UITheme::BorderGold : UITheme::BorderNormal);
    Color txtColor = isSelected ? UITheme::Gold : (isHovered ? UITheme::TextLight : UITheme::TextMuted);

    DrawRectangleRounded(r, 0.2f, 4, bg);
    DrawRectangleRoundedLines(r, 0.2f, 4, border);

    int textW = MeasureText(text.c_str(), 10);
    int posX = static_cast<int>(r.x + (r.width - textW) * 0.5f);
    int posY = static_cast<int>(r.y + (r.height - 10) * 0.5f);
    DrawText(text.c_str(), posX, posY, 10, txtColor);
}

MultiplayerMenuAction MultiplayerMenu::Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed) {
    m_mousePos = mouseVirtual;
    m_mousePressed = mouseLeftPressed;

    if (m_isQueued) {
        m_queueTimer += dt;
    }

    // Modal invite check
    if (m_hasPendingInvite) {
        Rectangle acceptBtn{ 240.0f, 200.0f, 70.0f, 22.0f };
        Rectangle declineBtn{ 330.0f, 200.0f, 70.0f, 22.0f };

        if (m_mousePressed) {
            if (CheckHover(acceptBtn)) {
                m_hasPendingInvite = false;
                return MultiplayerMenuAction::AcceptInvite;
            }
            if (CheckHover(declineBtn)) {
                m_hasPendingInvite = false;
                return MultiplayerMenuAction::DeclineInvite;
            }
        }
        return MultiplayerMenuAction::None;
    }

    // Mode Buttons
    Rectangle soloBtn{ 220.0f, 60.0f, 60.0f, 22.0f };
    Rectangle duosBtn{ 290.0f, 60.0f, 60.0f, 22.0f };
    Rectangle squadsBtn{ 360.0f, 60.0f, 60.0f, 22.0f };

    if (m_mousePressed && !m_isQueued) {
        if (CheckHover(soloBtn)) m_selectedMode = "solo";
        else if (CheckHover(duosBtn)) m_selectedMode = "duos";
        else if (CheckHover(squadsBtn)) m_selectedMode = "squads";
    }

    // Ready Button
    Rectangle readyBtn{ 40.0f, 280.0f, 80.0f, 26.0f };
    if (m_mousePressed && CheckHover(readyBtn)) {
        return MultiplayerMenuAction::ToggleReady;
    }

    // Queue / Cancel Button
    Rectangle queueBtn{ 220.0f, 280.0f, 130.0f, 26.0f };
    if (m_mousePressed && CheckHover(queueBtn)) {
        return m_isQueued ? MultiplayerMenuAction::CancelQueue : MultiplayerMenuAction::StartQueue;
    }

    // Leave Party Button
    Rectangle leaveBtn{ 130.0f, 280.0f, 80.0f, 26.0f };
    if (m_mousePressed && CheckHover(leaveBtn) && m_hasParty) {
        return MultiplayerMenuAction::LeaveParty;
    }

    // Back Button
    Rectangle backBtn{ 520.0f, 280.0f, 80.0f, 26.0f };
    if (m_mousePressed && CheckHover(backBtn)) {
        return MultiplayerMenuAction::BackToMain;
    }

    return MultiplayerMenuAction::None;
}

void MultiplayerMenu::Render() {
    // Backdrop
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), UITheme::DarkBg);

    // Title Header
    DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), 36, UITheme::PanelHeader);
    DrawLine(0, 36, static_cast<int>(UITheme::CanvasWidth), 36, UITheme::BorderGold);
    DrawText("WILDBOUND MULTIPLAYER — LOBBY & MATCHMAKING", 20, 11, 14, UITheme::Gold);

    std::string statusText = m_isConnected ? "ONLINE (COORDINATOR)" : "OFFLINE";
    Color statusCol = m_isConnected ? UITheme::TextSuccess : UITheme::TextDanger;
    DrawText(statusText.c_str(), static_cast<int>(UITheme::CanvasWidth) - 150, 13, 10, statusCol);

    // Left Panel: Party Members
    Rectangle partyBox{ 20.0f, 50.0f, 180.0f, 215.0f };
    DrawRectangleRounded(partyBox, 0.05f, 4, UITheme::PanelBg);
    DrawRectangleRoundedLines(partyBox, 0.05f, 4, UITheme::BorderNormal);
    DrawText("PARTY MEMBERS", 30, 60, 11, UITheme::Gold);

    int memberY = 85;
    if (m_hasParty && !m_partyState.members.empty()) {
        for (const auto& m : m_partyState.members) {
            std::string label = m.displayName + (m.isLeader ? " [LEADER]" : "");
            Color nameCol = (m.sessionId == m_sessionId) ? UITheme::TextGold : UITheme::TextLight;
            DrawText(label.c_str(), 30, memberY, 10, nameCol);

            std::string rText = m.isReady ? "READY" : "NOT READY";
            Color rCol = m.isReady ? UITheme::TextSuccess : UITheme::TextMuted;
            DrawText(rText.c_str(), 140, memberY, 9, rCol);
            memberY += 24;
        }
    } else {
        DrawText(m_displayName.c_str(), 30, memberY, 10, UITheme::TextGold);
        DrawText("READY", 140, memberY, 9, UITheme::TextSuccess);
        DrawText("(Solo party)", 30, memberY + 20, 9, UITheme::TextDim);
    }

    // Center Panel: Game Mode & Matchmaking Status
    Rectangle mainBox{ 210.0f, 50.0f, 410.0f, 215.0f };
    DrawRectangleRounded(mainBox, 0.05f, 4, UITheme::PanelBg);
    DrawRectangleRoundedLines(mainBox, 0.05f, 4, UITheme::BorderNormal);

    DrawText("GAME MODE SELECTION", 220, 60, 11, UITheme::Gold);
    DrawButton(Rectangle{ 220.0f, 80.0f, 80.0f, 24.0f }, "SOLO", CheckHover(Rectangle{ 220.0f, 80.0f, 80.0f, 24.0f }), m_selectedMode == "solo");
    DrawButton(Rectangle{ 310.0f, 80.0f, 80.0f, 24.0f }, "DUOS", CheckHover(Rectangle{ 310.0f, 80.0f, 80.0f, 24.0f }), m_selectedMode == "duos");
    DrawButton(Rectangle{ 400.0f, 80.0f, 80.0f, 24.0f }, "SQUADS", CheckHover(Rectangle{ 400.0f, 80.0f, 80.0f, 24.0f }), m_selectedMode == "squads");

    // Queue / Match Info Display
    DrawLine(220, 120, 610, 120, UITheme::BorderNormal);
    if (m_isQueued) {
        DrawText("MATCHMAKING IN PROGRESS...", 220, 135, 12, UITheme::Gold);
        std::string modeLine = "Mode: " + m_queuedMode;
        DrawText(modeLine.c_str(), 220, 155, 10, UITheme::TextLight);

        std::string timerLine = "Time in Queue: " + std::to_string(static_cast<int>(m_queueTimer)) + "s";
        DrawText(timerLine.c_str(), 220, 175, 10, UITheme::TextMuted);

        std::string queuePos = "Queue position: " + std::to_string(m_queuePosition + 1);
        DrawText(queuePos.c_str(), 220, 195, 10, UITheme::TextInfo);
    } else {
        DrawText("STATUS: IN LOBBY", 220, 135, 12, UITheme::TextLight);
        DrawText("Select game mode and ensure all party members are READY.", 220, 155, 9, UITheme::TextMuted);
        DrawText("Party leader can queue for dedicated server match.", 220, 175, 9, UITheme::TextMuted);
    }

    // Bottom Action Buttons
    Rectangle readyBtn{ 20.0f, 280.0f, 80.0f, 26.0f };
    DrawButton(readyBtn, "TOGGLE READY", CheckHover(readyBtn));

    if (m_hasParty) {
        Rectangle leaveBtn{ 110.0f, 280.0f, 90.0f, 26.0f };
        DrawButton(leaveBtn, "LEAVE PARTY", CheckHover(leaveBtn));
    }

    Rectangle queueBtn{ 210.0f, 280.0f, 150.0f, 26.0f };
    std::string qText = m_isQueued ? "CANCEL QUEUE" : "FIND MATCH";
    DrawButton(queueBtn, qText, CheckHover(queueBtn), m_isQueued);

    Rectangle backBtn{ 520.0f, 280.0f, 100.0f, 26.0f };
    DrawButton(backBtn, "MAIN MENU", CheckHover(backBtn));

    // Modal Invitation Dialog
    if (m_hasPendingInvite) {
        DrawRectangle(0, 0, static_cast<int>(UITheme::CanvasWidth), static_cast<int>(UITheme::CanvasHeight), Color{ 0, 0, 0, 180 });
        Rectangle inviteBox{ 180.0f, 120.0f, 280.0f, 120.0f };
        DrawRectangleRounded(inviteBox, 0.1f, 4, UITheme::PanelBg);
        DrawRectangleRoundedLines(inviteBox, 0.1f, 4, UITheme::BorderGold);

        DrawText("PARTY INVITATION", 240, 135, 12, UITheme::Gold);
        std::string invText = m_pendingInvite.senderDisplayName + " invited you to join!";
        DrawText(invText.c_str(), 200, 160, 10, UITheme::TextLight);

        DrawButton(Rectangle{ 220.0f, 195.0f, 90.0f, 24.0f }, "ACCEPT", CheckHover(Rectangle{ 220.0f, 195.0f, 90.0f, 24.0f }), true);
        DrawButton(Rectangle{ 330.0f, 195.0f, 90.0f, 24.0f }, "DECLINE", CheckHover(Rectangle{ 330.0f, 195.0f, 90.0f, 24.0f }));
    }
}

} // namespace wildbound::ui

