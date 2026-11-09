#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#endif

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "coordinator/CoordinatorServer.hpp"
#include "coordinator/CoordinatorProtocol.hpp"
#include "server/DedicatedServer.hpp"
#include "network/ENetClient.hpp"
#include "network/NetworkProtocol.hpp"
#include "network/NetworkManager.hpp"
#include "network/matchmaking/MatchmakingConnector.hpp"
#include "network/matchmaking/MatchmakingTypes.hpp"
#include "core/Logger.hpp"

using namespace wildbound::coordinator;
using namespace wildbound::server;
using namespace wildbound::network;
using namespace wildbound::network::matchmaking;
using namespace wildbound::core;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILURE] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            std::abort(); \
        } \
    } while (0)

static constexpr uint16_t TEST_COORD_PORT = 17780;
static constexpr uint16_t TEST_SERVER_PORT = 17777;

// Simple test helper client wrapper for coordinator protocol
struct TestClientSession {
    std::string name;
    ENetClient client;
    SessionId sessionId{ INVALID_SESSION_ID };
    PartyId partyId{ INVALID_PARTY_ID };
    std::vector<InvitationId> pendingInvites;
    bool isReady{ false };
    bool isQueued{ false };
    bool wasRedirected{ false };
    std::string redirectServerHost;
    uint16_t redirectServerPort{ 0 };
    std::string matchAuthToken;
    std::string assignedMatchId;

    void SendCoordinatorMsg(CoordinatorMsgType type, const ByteStreamWriter& payload) {
        ByteStreamWriter packetWriter;
        PacketHeader hdr{
            .magic = PROTOCOL_MAGIC,
            .version = PROTOCOL_VERSION,
            .messageType = static_cast<uint16_t>(type),
            .sequenceNumber = 0,
            .senderClientId = 0,
            .payloadLength = static_cast<uint16_t>(payload.GetSize())
        };
        SerializePacketHeader(packetWriter, hdr);
        if (payload.GetSize() > 0) {
            packetWriter.WriteBytes(payload.GetData(), payload.GetSize());
        }
        client.Send(ChannelType::Control, DeliveryMode::ReliableOrdered,
                    packetWriter.GetData(), packetWriter.GetSize());
    }

    void OnPacket(uint16_t messageType, const uint8_t* data, size_t size) {
        ByteStreamReader reader(data, size);
        auto type = static_cast<CoordinatorMsgType>(messageType);
        if (type == CoordinatorMsgType::SessionRegisterResponse) {
            MsgSessionRegisterResponse resp;
            if (resp.Deserialize(reader)) {
                sessionId = resp.sessionId;
            }
        } else if (type == CoordinatorMsgType::PartyCreateResponse) {
            MsgPartyCreateResponse resp;
            if (resp.Deserialize(reader)) {
                partyId = resp.partyId;
            }
        } else if (type == CoordinatorMsgType::PartyJoinResponse) {
            MsgPartyJoinResponse resp;
            if (resp.Deserialize(reader)) {
                if (resp.success) partyId = resp.partyId;
            }
        } else if (type == CoordinatorMsgType::PartyStateNotification) {
            MsgPartyStateNotification notif;
            if (notif.Deserialize(reader)) {
                partyId = notif.partyId;
                for (const auto& m : notif.members) {
                    if (m.sessionId == sessionId) {
                        isReady = m.isReady;
                    }
                }
            }
        } else if (type == CoordinatorMsgType::InviteReceivedNotification) {
            MsgInviteReceivedNotification notif;
            if (notif.Deserialize(reader)) {
                pendingInvites.push_back(notif.invitationId);
            }
        } else if (type == CoordinatorMsgType::MatchmakingStatusUpdate) {
            MsgMatchmakingStatusUpdate notif;
            if (notif.Deserialize(reader)) {
                isQueued = notif.isQueued;
            }
        } else if (type == CoordinatorMsgType::MatchRedirectNotification) {
            MsgMatchRedirectNotification notif;
            if (notif.Deserialize(reader)) {
                wasRedirected = true;
                redirectServerHost = notif.serverHost;
                redirectServerPort = notif.serverPort;
                matchAuthToken = notif.playerAuthToken;
                assignedMatchId = notif.matchId;
            }
        }
    }
};

int main() {
    std::cout << "========================================================\n"
              << "TEST SUITE: Coordinator End-to-End Integration (Phase 41)\n"
              << "4 Clients + 1 Coordinator + 1 Dedicated Game Server\n"
              << "========================================================" << std::endl;

    Logger::Init(LogLevel::Info);

    // 1. Initialize and Start Coordinator Server
    std::cout << "[STEP 1] Starting Coordinator Server on port " << TEST_COORD_PORT << "..." << std::endl;
    CoordinatorServer coordinator;
    CoordinatorConfig cConfig = CoordinatorConfig::GetDefault();
    cConfig.port = TEST_COORD_PORT;
    cConfig.squadsMinParticipants = 4; // Require 4 players to form Squads match
    TEST_ASSERT(coordinator.Initialize(cConfig), "Coordinator init must succeed");
    TEST_ASSERT(coordinator.Start(), "Coordinator start must succeed");

    // 2. Initialize and Start Dedicated Server
    std::cout << "[STEP 2] Starting Dedicated Server on port " << TEST_SERVER_PORT << " and registering with Coordinator..." << std::endl;
    DedicatedServer server;
    char arg0[] = "WildboundServer";
    char arg1[] = "--port";
    char arg2[] = "17777";
    char arg3[] = "--coordinator";
    char arg4[] = "127.0.0.1:17780";
    char arg5[] = "--mode";
    char arg6[] = "squads";
    char* argv[] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6 };

    TEST_ASSERT(server.Initialize(7, argv), "Dedicated server init must succeed");
    TEST_ASSERT(server.Start(), "Dedicated server start must succeed");

    // Pump loop to establish connection and register
    for (int i = 0; i < 100 && coordinator.GetServerRegistry().GetServerCount() == 0; ++i) {
        server.UpdateServer(0.016f);
        coordinator.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(coordinator.GetServerRegistry().GetServerCount() == 1, "Coordinator must have 1 registered server");
    TEST_ASSERT(coordinator.GetServerRegistry().GetAvailableServerCount() == 1, "Server must be in Available state");
    TEST_ASSERT(server.IsConnectedToCoordinator(), "Server must be connected to coordinator");
    std::cout << "  -> Dedicated Server registered with ServerId=" << server.GetCoordinatorServerId() << std::endl;

    // 3. Connect 4 Clients
    std::cout << "[STEP 3] Connecting 4 Clients (Alpha, Bravo, Charlie, Delta)..." << std::endl;
    std::vector<std::string> names = { "Player_Alpha", "Player_Bravo", "Player_Charlie", "Player_Delta" };
    std::vector<TestClientSession> clients(4);

    for (size_t i = 0; i < 4; ++i) {
        clients[i].name = names[i];
        clients[i].client.SetEventCallback([&clients, i](const ConnectionEvent& ev) {
            if (ev.type == ConnectionEventType::PacketReceived) {
                clients[i].OnPacket(ev.messageType, ev.payload.data(), ev.payload.size());
            }
        });
        TEST_ASSERT(clients[i].client.Connect("127.0.0.1", TEST_COORD_PORT), "Client connect must succeed");
    }

    // Pump network for connection handshakes
    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        bool allConnected = true;
        for (auto& c : clients) { if (!c.client.IsConnected()) allConnected = false; }
        if (allConnected) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Register sessions
    for (auto& c : clients) {
        MsgSessionRegisterRequest req{ .displayName = c.name, .clientVersion = "1.0.0" };
        ByteStreamWriter w; req.Serialize(w);
        c.SendCoordinatorMsg(CoordinatorMsgType::SessionRegisterRequest, w);
    }

    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        bool allRegistered = true;
        for (auto& c : clients) { if (c.sessionId == INVALID_SESSION_ID) allRegistered = false; }
        if (allRegistered) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT(clients[i].sessionId != INVALID_SESSION_ID, "Client must receive valid SessionId");
        std::cout << "  -> Client " << clients[i].name << " registered: SessionId=" << clients[i].sessionId << std::endl;
    }

    // 4. Client A (Alpha) Creates Squad Party
    std::cout << "[STEP 4] Client Alpha creates Squads party..." << std::endl;
    MsgPartyCreateRequest pCreateReq{ .sessionId = clients[0].sessionId, .gameMode = "squads" };
    ByteStreamWriter pcw; pCreateReq.Serialize(pcw);
    clients[0].SendCoordinatorMsg(CoordinatorMsgType::PartyCreateRequest, pcw);

    for (int loop = 0; loop < 100 && clients[0].partyId == INVALID_PARTY_ID; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    PartyId squadPartyId = clients[0].partyId;
    TEST_ASSERT(squadPartyId != INVALID_PARTY_ID, "Party ID must be valid");
    std::cout << "  -> Party created: PartyId=" << squadPartyId << std::endl;

    // 5. Client A Invites B, C, D
    std::cout << "[STEP 5] Client Alpha invites Bravo, Charlie, and Delta..." << std::endl;
    for (size_t i = 1; i < 4; ++i) {
        MsgInviteSendRequest invReq{
            .senderSessionId = clients[0].sessionId,
            .recipientSessionId = clients[i].sessionId,
            .type = InvitationType::Party
        };
        ByteStreamWriter iw; invReq.Serialize(iw);
        clients[0].SendCoordinatorMsg(CoordinatorMsgType::InviteSendRequest, iw);
    }

    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        bool allInvited = true;
        for (size_t i = 1; i < 4; ++i) { if (clients[i].pendingInvites.empty()) allInvited = false; }
        if (allInvited) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (size_t i = 1; i < 4; ++i) {
        TEST_ASSERT(!clients[i].pendingInvites.empty(), "Client must receive pending invitation");
    }

    // 6. Clients B, C, D Accept Invitations
    std::cout << "[STEP 6] Clients Bravo, Charlie, and Delta accept invitations..." << std::endl;
    for (size_t i = 1; i < 4; ++i) {
        MsgInviteRespondRequest respReq{
            .invitationId = clients[i].pendingInvites.front(),
            .recipientSessionId = clients[i].sessionId,
            .accept = true
        };
        ByteStreamWriter rw; respReq.Serialize(rw);
        clients[i].SendCoordinatorMsg(CoordinatorMsgType::InviteRespondRequest, rw);
    }

    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        auto* p = coordinator.GetPartyManager().GetParty(squadPartyId);
        if (p && p->GetMemberCount() == 4) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto* party = coordinator.GetPartyManager().GetParty(squadPartyId);
    TEST_ASSERT(party != nullptr, "Party must exist");
    TEST_ASSERT(party->GetMemberCount() == 4, "Party must have all 4 members");
    std::cout << "  -> Full 4-player squad assembled in party " << squadPartyId << std::endl;

    // 7. Clients B, C, D Mark Ready
    std::cout << "[STEP 7] Party members toggle ready..." << std::endl;
    for (size_t i = 1; i < 4; ++i) {
        MsgPartySetReadyRequest readyReq{ .sessionId = clients[i].sessionId, .isReady = true };
        ByteStreamWriter rw; readyReq.Serialize(rw);
        clients[i].SendCoordinatorMsg(CoordinatorMsgType::PartySetReadyRequest, rw);
    }

    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        auto* p = coordinator.GetPartyManager().GetParty(squadPartyId);
        if (p && p->AreAllMembersReady()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(party->AreAllMembersReady(), "All party members must now be READY");
    std::cout << "  -> All squad members are READY." << std::endl;

    // 8. Party Leader Queues for Matchmaking
    std::cout << "[STEP 8] Squad leader enters matchmaking queue..." << std::endl;
    MsgMatchmakingQueueRequest qReq{ .sessionId = clients[0].sessionId, .gameMode = "squads" };
    ByteStreamWriter qw; qReq.Serialize(qw);
    clients[0].SendCoordinatorMsg(CoordinatorMsgType::MatchmakingQueueRequest, qw);

    // 9. Match Formation, Server Reservation & Redirection Handshake
    std::cout << "[STEP 9] Coordinator forms match, reserves DedicatedServer, and redirects clients..." << std::endl;
    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        server.UpdateServer(0.016f);
        for (auto& c : clients) c.client.Update(0.016f);
        bool allRedirected = true;
        for (auto& c : clients) { if (!c.wasRedirected) allRedirected = false; }
        if (allRedirected) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify all 4 clients received redirection
    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT(clients[i].wasRedirected, "Client must receive MsgMatchRedirectNotification");
        TEST_ASSERT(clients[i].redirectServerPort == TEST_SERVER_PORT, "Redirect port must match dedicated server port");
        TEST_ASSERT(!clients[i].matchAuthToken.empty(), "Client must receive personal match auth token");
        TEST_ASSERT(!clients[i].assignedMatchId.empty(), "Client must receive match ID");
    }
    std::cout << "  -> All 4 clients successfully redirected to dedicated server with tokens." << std::endl;

    // 10. Server Validation of Assigned Credentials
    std::cout << "[STEP 10] Validating credentials on Dedicated Server..." << std::endl;
    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT(server.HasReservedParticipant(clients[i].matchAuthToken), "Dedicated server must validate auth token");
        std::string outName;
        uint32_t outTeam = 0;
        TEST_ASSERT(server.ValidateClientAuthToken(clients[i].matchAuthToken, outName, outTeam), "Token validation must succeed");
        TEST_ASSERT(outName == clients[i].name, "Validated participant name must match");
        TEST_ASSERT(outTeam == 1, "All squad members must be assigned to Team 1");
    }
    std::cout << "  -> Dedicated Server validated all 4 client authentication tokens and Team 1 assignment." << std::endl;

    // 11. Match Completion & Capacity Release
    std::cout << "[STEP 11] Dedicated Server reports match completion..." << std::endl;
    server.ReportMatchCompletedToCoordinator("Squad_Alpha", 240);

    for (int loop = 0; loop < 100; ++loop) {
        coordinator.Update(0.016f);
        server.UpdateServer(0.016f);
        if (coordinator.GetServerRegistry().GetAvailableServerCount() == 1) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(coordinator.GetServerRegistry().GetAvailableServerCount() == 1, "Dedicated server must be released back to Available");
    TEST_ASSERT(coordinator.GetDiagnostics().matchesCompleted == 1, "Coordinator must record 1 completed match");
    std::cout << "  -> Server capacity successfully released. Status is back to AVAILABLE." << std::endl;

    // 12. Orderly Teardown
    std::cout << "[STEP 12] Clean teardown..." << std::endl;
    for (auto& c : clients) c.client.Disconnect();
    server.Shutdown();
    coordinator.Stop();

    // ------------------------------------------------------------------------
    // STAGE 6B: Client Matchmaking Connector Verification
    // ------------------------------------------------------------------------

    // STEP 13: State Machine Validation
    std::cout << "[STEP 13] Client Matchmaking Connector — State Machine Validation..." << std::endl;
    {
        MatchmakingConnector testConn;
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Idle, "Initial state must be IDLE");

        // Invalid transitions from IDLE
        TEST_ASSERT(!testConn.TransitionTo(MatchmakingState::MatchFound), "IDLE -> MATCH_FOUND must be rejected");
        TEST_ASSERT(!testConn.TransitionTo(MatchmakingState::Cancelling), "IDLE -> CANCELLING must be rejected");
        TEST_ASSERT(!testConn.TransitionTo(MatchmakingState::Queued), "IDLE -> QUEUED must be rejected");

        // Valid forward path: IDLE -> IN_LOBBY -> REQUESTING_MATCH -> QUEUED -> MATCH_FOUND
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "IDLE -> IN_LOBBY must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "IN_LOBBY -> REQUESTING_MATCH must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "REQUESTING_MATCH -> QUEUED must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::MatchFound), "QUEUED -> MATCH_FOUND must succeed");

        // Reset back to IN_LOBBY
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "MATCH_FOUND -> IN_LOBBY must succeed");

        // Cancellation path: IN_LOBBY -> REQUESTING_MATCH -> QUEUED -> CANCELLING -> CANCELLED
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "IN_LOBBY -> REQUESTING_MATCH must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "REQUESTING_MATCH -> QUEUED must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Cancelling), "QUEUED -> CANCELLING must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Cancelled), "CANCELLING -> CANCELLED must succeed");
        TEST_ASSERT(!testConn.TransitionTo(MatchmakingState::MatchFound), "CANCELLED -> MATCH_FOUND must be rejected");

        // Failure path: CANCELLED -> IN_LOBBY -> REQUESTING_MATCH -> FAILED -> IN_LOBBY
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "CANCELLED -> IN_LOBBY must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "IN_LOBBY -> REQUESTING_MATCH must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Failed), "REQUESTING_MATCH -> FAILED must succeed");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "FAILED -> IN_LOBBY must succeed");

        // Duplicate submission prevention
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "Must transition to REQUESTING_MATCH");
        TEST_ASSERT(!testConn.RequestMatchmaking("solo"), "Duplicate request while REQUESTING_MATCH must be rejected");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "Must transition to QUEUED");
        TEST_ASSERT(!testConn.RequestMatchmaking("solo"), "Duplicate request while QUEUED must be rejected");

        // Cancellation bounds
        testConn.Shutdown();
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Idle, "Shutdown resets to IDLE");
        TEST_ASSERT(!testConn.CancelMatchmaking(), "Cancel while IDLE must be rejected");

        // Timeout handling in REQUESTING_MATCH
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "IDLE -> IN_LOBBY");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "IN_LOBBY -> REQUESTING_MATCH");
        testConn.Update(11.0f); // Advance past 10s request timeout
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Timeout in REQUESTING_MATCH must force FAILED");
        TEST_ASSERT(!testConn.GetLastErrorMessage().empty(), "Timeout must populate error message");

        // Timeout handling in CANCELLING
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "FAILED -> IN_LOBBY");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "IN_LOBBY -> REQUESTING_MATCH");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "REQUESTING_MATCH -> QUEUED");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Cancelling), "QUEUED -> CANCELLING");
        testConn.Update(11.0f); // Advance past timeout
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Cancelled, "Timeout in CANCELLING must force CANCELLED");
    }
    std::cout << "  -> State machine transitions, duplicate rejection & timeouts validated." << std::endl;

    // STEP 14: Assignment Validation & Malformed Payload Handling
    std::cout << "[STEP 14] Client Matchmaking Connector — Assignment Validation..." << std::endl;
    {
        MatchmakingConnector testConn;
        bool matchFoundCallbackFired = false;
        testConn.SetMatchFoundCallback([&](const MatchAssignmentInfo& info) {
            matchFoundCallbackFired = true;
            TEST_ASSERT(info.IsValid(), "Callback assignment must be valid");
        });

        // Advance to QUEUED state
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Init to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");

        // 1. Valid Assignment Injection
        MsgMatchRedirectNotification validNotif{
            .matchId = "match_s6b_alpha",
            .serverHost = "127.0.0.1",
            .serverPort = 17777,
            .playerAuthToken = "auth_tok_valid_123",
            .gameMode = "solo",
            .mapName = "maps/world/battle_royale_island.json"
        };
        ByteStreamWriter vw;
        validNotif.Serialize(vw);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), vw.GetData(), vw.GetSize());

        TEST_ASSERT(testConn.GetState() == MatchmakingState::MatchFound, "Valid assignment must transition to MATCH_FOUND");
        TEST_ASSERT(matchFoundCallbackFired, "Match found callback must fire");
        TEST_ASSERT(testConn.GetMatchAssignment().IsValid(), "Retained assignment must be valid");
        TEST_ASSERT(testConn.GetMatchAssignment().matchId == "match_s6b_alpha", "Match ID must match");
        TEST_ASSERT(testConn.GetMatchAssignment().serverPort == 17777, "Port must match");

        // 2. Reject empty matchId
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        MsgMatchRedirectNotification badMatchId = validNotif;
        badMatchId.matchId = "";
        ByteStreamWriter bw1; badMatchId.Serialize(bw1);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), bw1.GetData(), bw1.GetSize());
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Empty matchId must fail validation");

        // 3. Reject empty serverHost
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        MsgMatchRedirectNotification badHost = validNotif;
        badHost.serverHost = "";
        ByteStreamWriter bw2; badHost.Serialize(bw2);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), bw2.GetData(), bw2.GetSize());
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Empty serverHost must fail validation");

        // 4. Reject invalid port (0)
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        MsgMatchRedirectNotification badPort = validNotif;
        badPort.serverPort = 0;
        ByteStreamWriter bw3; badPort.Serialize(bw3);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), bw3.GetData(), bw3.GetSize());
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Port 0 must fail validation");

        // 5. Reject empty playerAuthToken
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        MsgMatchRedirectNotification badToken = validNotif;
        badToken.playerAuthToken = "";
        ByteStreamWriter bw4; badToken.Serialize(bw4);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), bw4.GetData(), bw4.GetSize());
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Empty token must fail validation");

        // 6. Reject invalid game mode
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        MsgMatchRedirectNotification badMode = validNotif;
        badMode.gameMode = "invalid_battle_mode";
        ByteStreamWriter bw5; badMode.Serialize(bw5);
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), bw5.GetData(), bw5.GetSize());
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Invalid gameMode must fail validation");

        // 7. Malformed / corrupted packet
        testConn.Reset();
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::InLobby), "Reset to InLobby");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::RequestingMatch), "To Requesting");
        TEST_ASSERT(testConn.TransitionTo(MatchmakingState::Queued), "To Queued");
        uint8_t garbage[5] = { 0xFF, 0x00, 0xAA, 0xBB, 0xCC };
        testConn.InjectPacket(static_cast<uint16_t>(CoordinatorMsgType::MatchRedirectNotification), garbage, sizeof(garbage));
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Failed, "Malformed payload must fail gracefully to FAILED");
    }
    std::cout << "  -> Assignment validation (valid, missing fields, corrupted packets) verified." << std::endl;

    // STEP 15: Lifecycle & Safety Verification
    std::cout << "[STEP 15] Client Matchmaking Connector — Lifecycle & Memory Safety..." << std::endl;
    {
        // Safe destruction with pending callback registration
        {
            auto scopedConn = std::make_unique<MatchmakingConnector>();
            scopedConn->SetStateChangedCallback([](MatchmakingState, MatchmakingState) {});
            scopedConn->SetMatchFoundCallback([](const MatchAssignmentInfo&) {});
            scopedConn->SetErrorCallback([](const std::string&) {});
            scopedConn->Connect("127.0.0.1", 19999, "Temp", "tok");
            scopedConn.reset(); // Destroy with connection pending
        }

        // Idempotent repeated shutdown
        MatchmakingConnector testConn;
        testConn.Shutdown();
        testConn.Shutdown();
        testConn.Shutdown();
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Idle, "Repeated shutdown must be idempotent");

        // Repeated reset
        testConn.Reset();
        testConn.Reset();
        TEST_ASSERT(testConn.GetState() == MatchmakingState::Idle, "Repeated reset must be idempotent");
    }
    std::cout << "  -> Lifecycle safety & shutdown idempotence verified." << std::endl;

    // STEP 16: Live Coordinator Integration Workflow
    std::cout << "[STEP 16] Live Matchmaking Connector E2E Workflow with CoordinatorServer..." << std::endl;
    {
        // Restart Coordinator & DedicatedServer on fresh ports
        static constexpr uint16_t LIVE_COORD_PORT = 17781;
        static constexpr uint16_t LIVE_SERVER_PORT = 17778;

        CoordinatorServer coord;
        CoordinatorConfig cfg = CoordinatorConfig::GetDefault();
        cfg.port = LIVE_COORD_PORT;
        cfg.soloMinParticipants = 1; // Solo match forms with 1 player
        TEST_ASSERT(coord.Initialize(cfg), "Live Coordinator init must succeed");
        TEST_ASSERT(coord.Start(), "Live Coordinator start must succeed");

        DedicatedServer srv;
        char a0[] = "WildboundServer";
        char a1[] = "--port";
        char a2[] = "17778";
        char a3[] = "--coordinator";
        char a4[] = "127.0.0.1:17781";
        char a5[] = "--mode";
        char a6[] = "solo";
        char* av[] = { a0, a1, a2, a3, a4, a5, a6 };
        TEST_ASSERT(srv.Initialize(7, av), "Dedicated server init must succeed");
        TEST_ASSERT(srv.Start(), "Dedicated server start must succeed");

        // Pump until server registers
        for (int i = 0; i < 100 && coord.GetServerRegistry().GetServerCount() == 0; ++i) {
            srv.UpdateServer(0.016f);
            coord.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        TEST_ASSERT(coord.GetServerRegistry().GetServerCount() == 1, "Coordinator must have registered server");

        // Instantiate live client connector
        MatchmakingConnector liveConn;
        TEST_ASSERT(liveConn.Connect("127.0.0.1", LIVE_COORD_PORT, "Explorer_Live", "jwt_auth_session_valid"), "Connect must succeed");

        // Pump until connected and session registered
        for (int i = 0; i < 100 && liveConn.GetState() != MatchmakingState::InLobby; ++i) {
            coord.Update(0.016f);
            liveConn.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        TEST_ASSERT(liveConn.IsConnected(), "Live connector must be connected to Coordinator");
        TEST_ASSERT(liveConn.IsRegistered(), "Live connector must have registered session");
        TEST_ASSERT(liveConn.GetState() == MatchmakingState::InLobby, "Live connector state must be IN_LOBBY");
        std::cout << "  -> Live connector connected and session registered: SessionId=" << liveConn.GetSessionId() << std::endl;

        // Test Queue & Cancel workflow live
        std::cout << "  -> Testing live queue and cancel..." << std::endl;
        // Squads requires 4 players, so single player will remain queued
        TEST_ASSERT(liveConn.RequestMatchmaking("squads"), "Request matchmaking for squads must succeed");

        for (int i = 0; i < 50 && liveConn.GetState() != MatchmakingState::Queued; ++i) {
            coord.Update(0.016f);
            liveConn.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        TEST_ASSERT(liveConn.IsQueued(), "Live connector must be QUEUED");
        TEST_ASSERT(liveConn.GetQueuedMode() == "squads", "Queued mode must be squads");
        std::cout << "  -> Connector is actively queued for squads." << std::endl;

        // Cancel queue
        TEST_ASSERT(liveConn.CancelMatchmaking(), "Cancel matchmaking must succeed");
        for (int i = 0; i < 50 && liveConn.GetState() != MatchmakingState::Cancelled; ++i) {
            coord.Update(0.016f);
            liveConn.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        TEST_ASSERT(liveConn.GetState() == MatchmakingState::Cancelled, "Live connector must be CANCELLED");
        std::cout << "  -> Live queue cancellation verified." << std::endl;

        // Reset and re-queue for solo match formation (soloMinParticipants = 1)
        liveConn.Reset();
        TEST_ASSERT(liveConn.GetState() == MatchmakingState::InLobby, "Reset must restore IN_LOBBY");

        std::cout << "  -> Re-queuing for solo match formation & dedicated server assignment..." << std::endl;
        TEST_ASSERT(liveConn.RequestMatchmaking("solo"), "Re-queue for solo must succeed");

        for (int i = 0; i < 100 && liveConn.GetState() != MatchmakingState::MatchFound; ++i) {
            coord.Update(0.016f);
            srv.UpdateServer(0.016f);
            liveConn.Update(0.016f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        TEST_ASSERT(liveConn.GetState() == MatchmakingState::MatchFound, "Live connector must transition to MATCH_FOUND");
        const auto& assignment = liveConn.GetMatchAssignment();
        TEST_ASSERT(assignment.IsValid(), "Live assignment must be valid");
        TEST_ASSERT(assignment.serverPort == LIVE_SERVER_PORT, "Assignment port must match dedicated server");
        TEST_ASSERT(!assignment.matchId.empty(), "Assignment matchId must be non-empty");
        TEST_ASSERT(!assignment.playerAuthToken.empty(), "Assignment playerAuthToken must be non-empty");

        // Verify Dedicated Server holds reservation for assigned token
        TEST_ASSERT(srv.HasReservedParticipant(assignment.playerAuthToken), "Dedicated server must hold participant reservation");
        std::string validatedName;
        uint32_t validatedTeam = 0;
        TEST_ASSERT(srv.ValidateClientAuthToken(assignment.playerAuthToken, validatedName, validatedTeam), "Token validation must succeed");
        TEST_ASSERT(validatedName == "Explorer_Live", "Validated participant name must match");

        std::cout << "  -> Live match assignment validated: MatchId=" << assignment.matchId
                  << " Endpoint=" << assignment.serverHost << ":" << assignment.serverPort
                  << " Token=" << assignment.playerAuthToken << std::endl;

        // Orderly teardown
        liveConn.Shutdown();
        srv.Shutdown();
        coord.Stop();
    }
    std::cout << "  -> Live Coordinator integration workflow passed." << std::endl;

    std::cout << "\n========================================================\n"
              << "[SUCCESS] Coordinator E2E Integration Suite PASSED!\n"
              << "========================================================" << std::endl;
    return 0;
}

