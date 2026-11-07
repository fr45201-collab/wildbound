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
#include "core/Logger.hpp"

using namespace wildbound::coordinator;
using namespace wildbound::server;
using namespace wildbound::network;
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

    std::cout << "\n========================================================\n"
              << "[SUCCESS] Coordinator E2E Integration Suite PASSED!\n"
              << "========================================================" << std::endl;
    return 0;
}

