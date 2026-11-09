#pragma once

#include "../UITheme.hpp"
#include "../../coordinator/CoordinatorProtocol.hpp"
#include "../../network/matchmaking/MatchmakingTypes.hpp"
#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::network::matchmaking {
    class MatchmakingConnector;
}

namespace wildbound::ui {

enum class MultiplayerMenuAction {
    None,
    BackToMain,
    ToggleReady,
    StartQueue,
    CancelQueue,
    LeaveParty,
    HostLobby,
    AcceptInvite,
    DeclineInvite
};

/**
 * Player-Facing Multiplayer & Matchmaking Menu.
 * Provides party management, mode selection, matchmaking queue monitoring,
 * invitation handling, match assignment display, and coordinator integration.
 */
class MultiplayerMenu {
public:
    MultiplayerMenu();
    ~MultiplayerMenu() = default;

    MultiplayerMenuAction Update(float dt, Vector2 mouseVirtual, bool mouseLeftPressed);
    void Render();

    // Synchronization from Client Matchmaking Connector
    void SyncFromConnector(const network::matchmaking::MatchmakingConnector& connector);

    // State setters (updated by coordinator client callbacks or direct testing)
    void SetConnected(bool connected) noexcept { m_isConnected = connected; }
    [[nodiscard]] bool IsConnected() const noexcept { return m_isConnected; }

    void SetSessionInfo(coordinator::SessionId sid, const std::string& name) {
        m_sessionId = sid;
        m_displayName = name;
    }
    [[nodiscard]] coordinator::SessionId GetSessionId() const noexcept { return m_sessionId; }
    [[nodiscard]] const std::string& GetDisplayName() const noexcept { return m_displayName; }

    void SetPartyState(const coordinator::MsgPartyStateNotification& partyState) {
        m_hasParty = true;
        m_partyState = partyState;
    }

    void ClearParty() {
        m_hasParty = false;
        m_partyState = coordinator::MsgPartyStateNotification{};
    }

    void SetMatchmakingState(network::matchmaking::MatchmakingState state) {
        m_matchState = state;
        m_isQueued = (state == network::matchmaking::MatchmakingState::Queued);
    }
    [[nodiscard]] network::matchmaking::MatchmakingState GetMatchmakingState() const noexcept { return m_matchState; }

    void SetQueueStatus(bool queued, const std::string& mode = "solo", uint32_t pos = 0) {
        m_isQueued = queued;
        m_queuedMode = mode;
        m_queuePosition = pos;
        if (!queued) m_queueTimer = 0.0f;
    }

    void SetMatchAssignment(const network::matchmaking::MatchAssignmentInfo& info) {
        m_assignment = info;
    }
    [[nodiscard]] const network::matchmaking::MatchAssignmentInfo& GetMatchAssignment() const noexcept { return m_assignment; }

    void SetErrorMessage(const std::string& err) {
        m_errorMessage = err;
    }
    [[nodiscard]] const std::string& GetErrorMessage() const noexcept { return m_errorMessage; }

    void SetPendingInvite(const coordinator::MsgInviteReceivedNotification& notif) {
        m_hasPendingInvite = true;
        m_pendingInvite = notif;
    }

    void ClearPendingInvite() {
        m_hasPendingInvite = false;
    }

    [[nodiscard]] const std::string& GetSelectedGameMode() const noexcept { return m_selectedMode; }
    void SetSelectedGameMode(const std::string& mode) { m_selectedMode = mode; }

    [[nodiscard]] bool HasPendingInvite() const noexcept { return m_hasPendingInvite; }
    [[nodiscard]] const coordinator::MsgInviteReceivedNotification& GetPendingInvite() const noexcept { return m_pendingInvite; }
    [[nodiscard]] bool IsQueued() const noexcept { return m_isQueued; }

private:
    bool CheckHover(Rectangle r) const;
    void DrawButton(Rectangle r, const std::string& text, bool isHovered, bool isSelected = false);

    bool m_isConnected{ false };
    coordinator::SessionId m_sessionId{ coordinator::INVALID_SESSION_ID };
    std::string m_displayName{ "Player" };

    bool m_hasParty{ false };
    coordinator::MsgPartyStateNotification m_partyState;
    std::string m_selectedMode{ "solo" };

    network::matchmaking::MatchmakingState m_matchState{ network::matchmaking::MatchmakingState::Idle };
    bool m_isQueued{ false };
    std::string m_queuedMode{ "solo" };
    uint32_t m_queuePosition{ 0 };
    float m_queueTimer{ 0.0f };

    network::matchmaking::MatchAssignmentInfo m_assignment;
    std::string m_errorMessage;

    bool m_hasPendingInvite{ false };
    coordinator::MsgInviteReceivedNotification m_pendingInvite;

    Vector2 m_mousePos{ 0.0f, 0.0f };
    bool m_mousePressed{ false };
};

} // namespace wildbound::ui
