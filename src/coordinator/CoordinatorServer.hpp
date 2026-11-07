#pragma once

#include "CoordinatorConfig.hpp"
#include "CoordinatorDiagnostics.hpp"
#include "CoordinatorProtocol.hpp"
#include "SessionManager.hpp"
#include "PartyManager.hpp"
#include "LobbyManager.hpp"
#include "InvitationManager.hpp"
#include "MatchmakingManager.hpp"
#include "DedicatedServerRegistry.hpp"
#include "MatchAssignmentManager.hpp"
#include "../network/ENetServer.hpp"
#include "../network/NetworkProtocol.hpp"

#include <memory>
#include <string>

namespace wildbound::coordinator {

class CoordinatorServer {
public:
    CoordinatorServer();
    ~CoordinatorServer();

    bool Initialize(const CoordinatorConfig& config);
    bool Initialize(const std::string& configFilePath = "assets/config/coordinator.json");
    bool Start();
    void Stop();

    void Update(float dt);

    void SendMessage(network::ClientId clientId, CoordinatorMsgType type, const network::ByteStreamWriter& payload);

    // Queries
    [[nodiscard]] bool IsRunning() const noexcept { return m_isRunning; }
    [[nodiscard]] const CoordinatorConfig& GetConfig() const noexcept { return m_config; }
    [[nodiscard]] CoordinatorConfig& GetConfig() noexcept { return m_config; }
    [[nodiscard]] const CoordinatorDiagnostics& GetDiagnostics() const noexcept { return m_diagnostics; }
    [[nodiscard]] CoordinatorDiagnostics& GetDiagnostics() noexcept { return m_diagnostics; }

    // Managers
    [[nodiscard]] SessionManager& GetSessionManager() noexcept { return m_sessionMgr; }
    [[nodiscard]] const SessionManager& GetSessionManager() const noexcept { return m_sessionMgr; }

    [[nodiscard]] PartyManager& GetPartyManager() noexcept { return m_partyMgr; }
    [[nodiscard]] const PartyManager& GetPartyManager() const noexcept { return m_partyMgr; }

    [[nodiscard]] LobbyManager& GetLobbyManager() noexcept { return m_lobbyMgr; }
    [[nodiscard]] const LobbyManager& GetLobbyManager() const noexcept { return m_lobbyMgr; }

    [[nodiscard]] InvitationManager& GetInvitationManager() noexcept { return m_invitationMgr; }
    [[nodiscard]] const InvitationManager& GetInvitationManager() const noexcept { return m_invitationMgr; }

    [[nodiscard]] MatchmakingManager& GetMatchmakingManager() noexcept { return m_matchmakingMgr; }
    [[nodiscard]] const MatchmakingManager& GetMatchmakingManager() const noexcept { return m_matchmakingMgr; }

    [[nodiscard]] DedicatedServerRegistry& GetServerRegistry() noexcept { return m_serverRegistry; }
    [[nodiscard]] const DedicatedServerRegistry& GetServerRegistry() const noexcept { return m_serverRegistry; }

    [[nodiscard]] MatchAssignmentManager& GetAssignmentManager() noexcept { return m_assignmentMgr; }
    [[nodiscard]] const MatchAssignmentManager& GetAssignmentManager() const noexcept { return m_assignmentMgr; }

    void BroadcastPartyUpdate(PartyId partyId);

private:
    void HandleNetworkEvent(const network::ConnectionEvent& event);
    void ProcessPacket(network::ClientId clientId, uint16_t messageType, const uint8_t* data, size_t size);

    // Packet Handlers
    void HandleSessionRegister(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleSessionHeartbeat(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartyCreate(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartyJoin(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartyLeave(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartySetReady(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartySetMode(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartyKick(network::ClientId cid, network::ByteStreamReader& reader);
    void HandlePartyTransferLeader(network::ClientId cid, network::ByteStreamReader& reader);

    void HandleInviteSend(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleInviteRespond(network::ClientId cid, network::ByteStreamReader& reader);

    void HandleMatchmakingQueue(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleMatchmakingCancel(network::ClientId cid, network::ByteStreamReader& reader);

    void HandleServerRegister(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleServerHeartbeat(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleServerDeregister(network::ClientId cid, network::ByteStreamReader& reader);

    void HandleMatchReserveResponse(network::ClientId cid, network::ByteStreamReader& reader);
    void HandleMatchCompleted(network::ClientId cid, network::ByteStreamReader& reader);

    void ProcessFormedMatches();

    CoordinatorConfig m_config;
    CoordinatorDiagnostics m_diagnostics;
    std::unique_ptr<network::ENetServer> m_server;

    SessionManager m_sessionMgr;
    PartyManager m_partyMgr;
    LobbyManager m_lobbyMgr;
    InvitationManager m_invitationMgr;
    MatchmakingManager m_matchmakingMgr;
    DedicatedServerRegistry m_serverRegistry;
    MatchAssignmentManager m_assignmentMgr;

    float m_currentTime{ 0.0f };
    float m_timeoutCheckAccumulator{ 0.0f };
    bool m_isRunning{ false };
};

} // namespace wildbound::coordinator

