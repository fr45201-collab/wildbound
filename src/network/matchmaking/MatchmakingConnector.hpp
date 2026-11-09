#pragma once

#include <string>
#include <memory>
#include <functional>
#include <cstdint>
#include <vector>

#include "MatchmakingTypes.hpp"
#include "../../coordinator/CoordinatorProtocol.hpp"

namespace wildbound::network {
    class ENetClient;
    struct ConnectionEvent;
    class ByteStreamWriter;
}

namespace wildbound::network::matchmaking {

/**
 * Client Matchmaking Connector.
 * Authoritative client-side interface to the Wildbound Matchmaking Coordinator.
 *
 * Responsibilities:
 * - Manages the control-plane connection over ENet to CoordinatorServer.
 * - Handles session registration with player credentials / auth token.
 * - Manages explicit state machine transitions (Idle -> Requesting -> Queued -> MatchFound).
 * - Handles queue cancellation safely and prevents duplicate submissions.
 * - Validates incoming match redirection assignments.
 * - Dispatches callbacks safely on the main thread via Update(float dt).
 * - Enforces lifetime safety token to prevent callback dispatch after destruction.
 */
class MatchmakingConnector {
public:
    using StateChangedCallback = std::function<void(MatchmakingState oldState, MatchmakingState newState)>;
    using MatchFoundCallback = std::function<void(const MatchAssignmentInfo& assignment)>;
    using ErrorCallback = std::function<void(const std::string& errorMsg)>;

    MatchmakingConnector();
    ~MatchmakingConnector();

    // Non-copyable, non-movable
    MatchmakingConnector(const MatchmakingConnector&) = delete;
    MatchmakingConnector& operator=(const MatchmakingConnector&) = delete;
    MatchmakingConnector(MatchmakingConnector&&) = delete;
    MatchmakingConnector& operator=(MatchmakingConnector&&) = delete;

    /**
     * Connect to the Coordinator service.
     * @param host Target coordinator host address (e.g. "127.0.0.1")
     * @param port Target coordinator UDP port (default 7780)
     * @param displayName Player's display name
     * @param authToken Player's session authentication token (from AuthSession)
     * @return true if connection initiation succeeded
     */
    bool Connect(const std::string& host = "127.0.0.1",
                 uint16_t port = 7780,
                 const std::string& displayName = "Player",
                 const std::string& authToken = "");

    /**
     * Disconnects and cleanly shuts down the connector.
     */
    void Shutdown();

    /**
     * Main-thread per-frame tick. Pumps transport events and state timeouts.
     */
    void Update(float dt);

    /**
     * Submit a matchmaking queue request for the specified game mode.
     * @param gameMode "solo", "duos", or "squads"
     * @return true if request was successfully dispatched
     */
    bool RequestMatchmaking(const std::string& gameMode = "solo");

    /**
     * Cancel an active matchmaking queue request.
     * @return true if cancellation request was dispatched
     */
    bool CancelMatchmaking();

    /**
     * Reset state back to InLobby or Idle after Cancelled or Failed.
     */
    void Reset();

    // Query state
    [[nodiscard]] MatchmakingState GetState() const noexcept { return m_state; }
    [[nodiscard]] bool IsConnected() const noexcept;
    [[nodiscard]] bool IsRegistered() const noexcept { return m_sessionId != coordinator::INVALID_SESSION_ID; }
    [[nodiscard]] bool IsQueued() const noexcept { return m_state == MatchmakingState::Queued; }
    [[nodiscard]] coordinator::SessionId GetSessionId() const noexcept { return m_sessionId; }
    [[nodiscard]] const std::string& GetDisplayName() const noexcept { return m_displayName; }
    [[nodiscard]] const std::string& GetQueuedMode() const noexcept { return m_queuedMode; }
    [[nodiscard]] uint32_t GetQueuePosition() const noexcept { return m_queuePosition; }
    [[nodiscard]] float GetQueueTime() const noexcept { return m_queueTimer; }
    [[nodiscard]] uint32_t GetEstimatedWaitSeconds() const noexcept { return m_estimatedWaitSeconds; }
    [[nodiscard]] const std::string& GetLastErrorMessage() const noexcept { return m_lastErrorMessage; }
    [[nodiscard]] const MatchAssignmentInfo& GetMatchAssignment() const noexcept { return m_assignment; }
    [[nodiscard]] bool HasParty() const noexcept { return m_hasParty; }
    [[nodiscard]] const coordinator::MsgPartyStateNotification& GetPartyState() const noexcept { return m_partyState; }

    // Configuration / context setters
    void SetCredentials(const std::string& displayName, const std::string& authToken) {
        m_displayName = displayName;
        m_authToken = authToken;
    }

    void SetStateChangedCallback(StateChangedCallback cb) { m_onStateChanged = std::move(cb); }
    void SetMatchFoundCallback(MatchFoundCallback cb) { m_onMatchFound = std::move(cb); }
    void SetErrorCallback(ErrorCallback cb) { m_onError = std::move(cb); }

    // Direct message injection / testing helper
    void InjectPacket(uint16_t messageType, const uint8_t* data, size_t size);

    // Direct transition helper for unit testing state transitions
    bool TransitionTo(MatchmakingState newState);

private:
    void HandleNetworkEvent(const ConnectionEvent& ev);
    void HandleCoordinatorPacket(uint16_t messageType, const uint8_t* data, size_t size);
    void SendCoordinatorMsg(coordinator::CoordinatorMsgType type, const ByteStreamWriter& payload);

    bool ValidateAssignment(const coordinator::MsgMatchRedirectNotification& notif, MatchAssignmentInfo& outInfo);

    std::unique_ptr<ENetClient> m_client;
    MatchmakingState m_state{ MatchmakingState::Idle };

    std::string m_coordinatorHost{ "127.0.0.1" };
    uint16_t m_coordinatorPort{ 7780 };
    std::string m_displayName{ "Player" };
    std::string m_authToken;

    coordinator::SessionId m_sessionId{ coordinator::INVALID_SESSION_ID };
    coordinator::PartyId m_partyId{ coordinator::INVALID_PARTY_ID };
    bool m_hasParty{ false };
    coordinator::MsgPartyStateNotification m_partyState;

    std::string m_queuedMode{ "solo" };
    uint32_t m_queuePosition{ 0 };
    uint32_t m_estimatedWaitSeconds{ 0 };
    float m_queueTimer{ 0.0f };

    float m_requestTimeoutTimer{ 0.0f };
    static constexpr float REQUEST_TIMEOUT_SECONDS = 10.0f;

    std::string m_lastErrorMessage;
    MatchAssignmentInfo m_assignment;

    StateChangedCallback m_onStateChanged;
    MatchFoundCallback m_onMatchFound;
    ErrorCallback m_onError;

    std::shared_ptr<bool> m_alive;
};

} // namespace wildbound::network::matchmaking
