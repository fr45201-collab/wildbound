#include "MultiplayerMenu.hpp"
#include "../../network/matchmaking/MatchmakingConnector.hpp"
#include <string>

namespace wildbound::ui {

MultiplayerMenu::MultiplayerMenu() = default;

void MultiplayerMenu::SyncFromConnector(const network::matchmaking::MatchmakingConnector& connector) {
    m_isConnected = connector.IsConnected();
    m_sessionId = connector.GetSessionId();
    m_displayName = connector.GetDisplayName();
    m_hasParty = connector.HasParty();
    if (m_hasParty) {
        m_partyState = connector.GetPartyState();
    }
    m_matchState = connector.GetState();
    m_isQueued = connector.IsQueued();
    m_queuedMode = connector.GetQueuedMode();
    m_queuePosition = connector.GetQueuePosition();
    m_queueTimer = connector.GetQueueTime();
    m_assignment = connector.GetMatchAssignment();
    m_errorMessage = connector.GetLastErrorMessage();
}

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
        Rectangle acceptBtn{ 220.0f, 195.0f, 90.0f, 24.0f };
        Rectangle declineBtn{ 330.0f, 195.0f, 90.0f, 24.0f };

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
    Rectangle soloBtn{ 220.0f, 80.0f, 80.0f, 24.0f };
    Rectangle duosBtn{ 310.0f, 80.0f, 80.0f, 24.0f };
    Rectangle squadsBtn{ 400.0f, 80.0f, 80.0f, 24.0f };

    if (m_mousePressed && !m_isQueued && m_matchState != network::matchmaking::MatchmakingState::RequestingMatch) {
        if (CheckHover(soloBtn)) m_selectedMode = "solo";
        else if (CheckHover(duosBtn)) m_selectedMode = "duos";
        else if (CheckHover(squadsBtn)) m_selectedMode = "squads";
    }

    // Ready Button
    Rectangle readyBtn{ 20.0f, 280.0f, 80.0f, 26.0f };
    if (m_mousePressed && CheckHover(readyBtn)) {
        return MultiplayerMenuAction::ToggleReady;
    }

    // Queue / Cancel Button
    Rectangle queueBtn{ 210.0f, 280.0f, 150.0f, 26.0f };
    if (m_mousePressed && CheckHover(queueBtn)) {
        if (m_isQueued || m_matchState == network::matchmaking::MatchmakingState::RequestingMatch) {
            return MultiplayerMenuAction::CancelQueue;
        } else {
            return MultiplayerMenuAction::StartQueue;
        }
    }

    // Leave Party Button
    if (m_hasParty) {
        Rectangle leaveBtn{ 110.0f, 280.0f, 90.0f, 26.0f };
        if (m_mousePressed && CheckHover(leaveBtn)) {
            return MultiplayerMenuAction::LeaveParty;
        }
    }

    // Back Button
    Rectangle backBtn{ 520.0f, 280.0f, 100.0f, 26.0f };
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
    DrawText(statusText.c_str(), static_cast<int>(UITheme::CanvasWidth) - 170, 13, 10, statusCol);

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
    DrawLine(220, 115, 610, 115, UITheme::BorderNormal);

    switch (m_matchState) {
        case network::matchmaking::MatchmakingState::RequestingMatch: {
            DrawText("REQUESTING MATCH...", 220, 130, 12, UITheme::Gold);
            std::string line = "Mode: " + m_selectedMode + " — Contacting Coordinator...";
            DrawText(line.c_str(), 220, 150, 10, UITheme::TextLight);
            break;
        }

        case network::matchmaking::MatchmakingState::Queued: {
            DrawText("MATCHMAKING IN PROGRESS...", 220, 130, 12, UITheme::Gold);
            std::string modeLine = "Mode: " + m_queuedMode;
            DrawText(modeLine.c_str(), 220, 150, 10, UITheme::TextLight);

            std::string timerLine = "Time in Queue: " + std::to_string(static_cast<int>(m_queueTimer)) + "s";
            DrawText(timerLine.c_str(), 220, 170, 10, UITheme::TextMuted);

            std::string queuePos = "Queue position: " + std::to_string(m_queuePosition + 1);
            DrawText(queuePos.c_str(), 220, 190, 10, UITheme::TextInfo);
            break;
        }

        case network::matchmaking::MatchmakingState::Cancelling: {
            DrawText("CANCELLING QUEUE...", 220, 130, 12, UITheme::Gold);
            DrawText("Notifying Coordinator...", 220, 150, 10, UITheme::TextMuted);
            break;
        }

        case network::matchmaking::MatchmakingState::Cancelled: {
            DrawText("STATUS: QUEUE CANCELLED", 220, 130, 12, UITheme::TextLight);
            DrawText("Matchmaking was cancelled. Ready to queue again.", 220, 150, 10, UITheme::TextMuted);
            break;
        }

        case network::matchmaking::MatchmakingState::MatchFound: {
            DrawText("MATCH ASSIGNED!", 220, 125, 12, UITheme::Gold);
            std::string mLine = "Match ID: " + m_assignment.matchId;
            DrawText(mLine.c_str(), 220, 145, 10, UITheme::TextSuccess);

            std::string sLine = "Dedicated Server: " + m_assignment.serverHost + ":" + std::to_string(m_assignment.serverPort);
            DrawText(sLine.c_str(), 220, 165, 10, UITheme::TextLight);

            std::string dLine = "Mode: " + m_assignment.gameMode + " | Map: " + m_assignment.mapName;
            DrawText(dLine.c_str(), 220, 185, 9, UITheme::TextMuted);

            DrawText("Awaiting Stage 6C gameplay connection...", 220, 205, 9, UITheme::TextDim);
            break;
        }

        case network::matchmaking::MatchmakingState::Failed: {
            DrawText("STATUS: ERROR", 220, 130, 12, UITheme::TextDanger);
            std::string err = m_errorMessage.empty() ? "Coordinator communication error." : m_errorMessage;
            DrawText(err.c_str(), 220, 150, 9, UITheme::TextMuted);
            DrawText("Click FIND MATCH to retry.", 220, 175, 9, UITheme::TextLight);
            break;
        }

        default: {
            DrawText("STATUS: IN LOBBY", 220, 130, 12, UITheme::TextLight);
            DrawText("Select game mode and click FIND MATCH.", 220, 150, 9, UITheme::TextMuted);
            DrawText("Party leader can queue for dedicated server match.", 220, 170, 9, UITheme::TextMuted);
            break;
        }
    }

    // Bottom Action Buttons
    Rectangle readyBtn{ 20.0f, 280.0f, 80.0f, 26.0f };
    DrawButton(readyBtn, "TOGGLE READY", CheckHover(readyBtn));

    if (m_hasParty) {
        Rectangle leaveBtn{ 110.0f, 280.0f, 90.0f, 26.0f };
        DrawButton(leaveBtn, "LEAVE PARTY", CheckHover(leaveBtn));
    }

    Rectangle queueBtn{ 210.0f, 280.0f, 150.0f, 26.0f };
    std::string qText = (m_isQueued || m_matchState == network::matchmaking::MatchmakingState::RequestingMatch) ? "CANCEL QUEUE" : "FIND MATCH";
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
