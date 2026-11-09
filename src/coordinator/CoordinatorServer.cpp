#include "CoordinatorServer.hpp"
#include "../core/Logger.hpp"
#include <iostream>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

using wildbound::core::Logger;

namespace {

std::string Base64UrlDecode(const std::string& input) {
    std::string base64 = input;
    for (char& c : base64) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    while (base64.size() % 4 != 0) {
        base64.push_back('=');
    }

    static const std::string b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; ++i) T[static_cast<unsigned char>(b64chars[i])] = i;

    int val = 0, valb = -8;
    for (unsigned char c : base64) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

bool ExtractJwtClaims(const std::string& token, std::string& outUserId, std::string& outUsername, int64_t& outExp) {
    if (token.empty()) return false;
    size_t firstDot = token.find('.');
    if (firstDot == std::string::npos) return false;
    size_t secondDot = token.find('.', firstDot + 1);
    if (secondDot == std::string::npos) return false;

    std::string payloadB64 = token.substr(firstDot + 1, secondDot - firstDot - 1);
    std::string payloadJson = Base64UrlDecode(payloadB64);
    if (payloadJson.empty()) return false;

    try {
        auto j = nlohmann::json::parse(payloadJson);
        if (j.contains("sub") && j["sub"].is_string()) {
            outUserId = j["sub"].get<std::string>();
        }
        if (j.contains("usr") && j["usr"].is_string()) {
            outUsername = j["usr"].get<std::string>();
        }
        if (j.contains("exp") && j["exp"].is_number()) {
            outExp = j["exp"].get<int64_t>();
        }
        return !outUserId.empty();
    } catch (...) {
        return false;
    }
}

} // anonymous namespace

namespace wildbound::coordinator {

CoordinatorServer::CoordinatorServer()
    : m_server(std::make_unique<network::ENetServer>())
{
}

CoordinatorServer::~CoordinatorServer() {
    Stop();
}

bool CoordinatorServer::Initialize(const CoordinatorConfig& config) {
    m_config = config;
    m_config.Validate();

    m_matchmakingMgr.SetMatchSizeRequirement("solo", m_config.soloMinParticipants);
    m_matchmakingMgr.SetMatchSizeRequirement("duos", m_config.duosMinParticipants);
    m_matchmakingMgr.SetMatchSizeRequirement("squads", m_config.squadsMinParticipants);

    m_diagnostics.Reset();
    return true;
}

bool CoordinatorServer::Initialize(const std::string& configFilePath) {
    CoordinatorConfig cfg = CoordinatorConfig::GetDefault();
    if (!cfg.LoadFromFile(configFilePath)) {
        (void)cfg.LoadFromFile("assets/config/coordinator.json");
    }
    return Initialize(cfg);
}

bool CoordinatorServer::Start() {
    if (m_isRunning) return true;

    Logger::Info("COORDINATOR", "Starting Wildbound Coordinator on port " + std::to_string(m_config.port) + "...");

    if (!m_server->Start(m_config.port, m_config.maxConnections)) {
        Logger::Error("COORDINATOR", "Failed to bind Coordinator port " + std::to_string(m_config.port));
        return false;
    }

    m_server->SetEventCallback([this](const network::ConnectionEvent& ev) {
        HandleNetworkEvent(ev);
    });

    m_isRunning = true;
    Logger::Info("COORDINATOR", "Coordinator service is now RUNNING and accepting connections.");
    return true;
}

void CoordinatorServer::Stop() {
    if (!m_isRunning) return;

    Logger::Info("COORDINATOR", "Stopping Wildbound Coordinator service...");
    if (m_server) {
        m_server->Stop();
    }

    m_sessionMgr.Clear();
    m_partyMgr.Clear();
    m_lobbyMgr.Clear();
    m_invitationMgr.Clear();
    m_matchmakingMgr.Clear();
    m_serverRegistry.Clear();
    m_assignmentMgr.Clear();

    m_isRunning = false;
    Logger::Info("COORDINATOR", "Coordinator service stopped.");
}

void CoordinatorServer::SendMessage(network::ClientId clientId, CoordinatorMsgType type, const network::ByteStreamWriter& payload) {
    if (!m_server || !m_isRunning) return;

    network::ByteStreamWriter packetWriter;
    network::PacketHeader hdr{
        .magic = network::PROTOCOL_MAGIC,
        .version = network::PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(type),
        .sequenceNumber = 0,
        .senderClientId = network::SERVER_CLIENT_ID,
        .payloadLength = static_cast<uint16_t>(payload.GetSize())
    };

    network::SerializePacketHeader(packetWriter, hdr);
    if (payload.GetSize() > 0) {
        packetWriter.WriteBytes(payload.GetData(), payload.GetSize());
    }

    m_server->Send(clientId, network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                   packetWriter.GetData(), packetWriter.GetSize());
}

void CoordinatorServer::HandleNetworkEvent(const network::ConnectionEvent& event) {
    switch (event.type) {
        case network::ConnectionEventType::ClientConnected: {
            Logger::Info("COORDINATOR", "New connection accepted: ClientId=" + std::to_string(event.connectionId) +
                                        " from " + event.remoteAddress + ":" + std::to_string(event.remotePort));
            break;
        }
        case network::ConnectionEventType::ClientDisconnected: {
            Logger::Info("COORDINATOR", "Connection closed: ClientId=" + std::to_string(event.connectionId));

            // 1. Check if it was a player session
            auto* s = m_sessionMgr.GetSessionByConnection(event.connectionId);
            if (s) {
                SessionId sid = s->sessionId;
                // If in queue, cancel queue
                if (s->partyId != INVALID_PARTY_ID) {
                    m_matchmakingMgr.DequeueParty(s->partyId);
                    bool disbanded = false;
                    SessionId newLead = INVALID_SESSION_ID;
                    m_partyMgr.RemoveMember(s->partyId, sid, disbanded, newLead);
                    if (!disbanded) {
                        BroadcastPartyUpdate(s->partyId);
                    }
                }
                if (s->lobbyId != INVALID_LOBBY_ID) {
                    bool disbanded = false;
                    SessionId newHost = INVALID_SESSION_ID;
                    m_lobbyMgr.RemoveMember(s->lobbyId, sid, disbanded, newHost);
                }
                m_sessionMgr.RemoveSession(sid);
            }

            // 2. Check if it was a registered dedicated server
            auto* dServer = m_serverRegistry.GetServerByConnection(event.connectionId);
            if (dServer) {
                Logger::Warn("COORDINATOR", "Dedicated server disconnected: " + dServer->serverName +
                                            " (ServerId=" + std::to_string(dServer->serverId) + ")");
                m_serverRegistry.DeregisterServer(dServer->serverId);
            }
            break;
        }
        case network::ConnectionEventType::PacketReceived: {
            ProcessPacket(event.connectionId, event.messageType, event.payload.data(), event.payload.size());
            break;
        }
        default:
            break;
    }
}

void CoordinatorServer::ProcessPacket(network::ClientId clientId, uint16_t messageType, const uint8_t* data, size_t size) {
    m_diagnostics.totalPacketsProcessed++;
    CoordinatorMsgType type = static_cast<CoordinatorMsgType>(messageType);
    network::ByteStreamReader reader(data, size);

    switch (type) {
        case CoordinatorMsgType::SessionRegisterRequest:     HandleSessionRegister(clientId, reader); break;
        case CoordinatorMsgType::SessionHeartbeat:           HandleSessionHeartbeat(clientId, reader); break;
        case CoordinatorMsgType::PartyCreateRequest:         HandlePartyCreate(clientId, reader); break;
        case CoordinatorMsgType::PartyJoinRequest:           HandlePartyJoin(clientId, reader); break;
        case CoordinatorMsgType::PartyLeaveRequest:          HandlePartyLeave(clientId, reader); break;
        case CoordinatorMsgType::PartySetReadyRequest:       HandlePartySetReady(clientId, reader); break;
        case CoordinatorMsgType::PartySetModeRequest:        HandlePartySetMode(clientId, reader); break;
        case CoordinatorMsgType::PartyKickRequest:           HandlePartyKick(clientId, reader); break;
        case CoordinatorMsgType::PartyTransferLeaderRequest: HandlePartyTransferLeader(clientId, reader); break;
        case CoordinatorMsgType::InviteSendRequest:          HandleInviteSend(clientId, reader); break;
        case CoordinatorMsgType::InviteRespondRequest:       HandleInviteRespond(clientId, reader); break;
        case CoordinatorMsgType::MatchmakingQueueRequest:    HandleMatchmakingQueue(clientId, reader); break;
        case CoordinatorMsgType::MatchmakingCancelRequest:   HandleMatchmakingCancel(clientId, reader); break;
        case CoordinatorMsgType::ServerRegisterRequest:      HandleServerRegister(clientId, reader); break;
        case CoordinatorMsgType::ServerHeartbeat:            HandleServerHeartbeat(clientId, reader); break;
        case CoordinatorMsgType::ServerDeregister:           HandleServerDeregister(clientId, reader); break;
        case CoordinatorMsgType::MatchReserveResponse:       HandleMatchReserveResponse(clientId, reader); break;
        case CoordinatorMsgType::MatchCompletedNotification: HandleMatchCompleted(clientId, reader); break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// Packet Handlers
// ----------------------------------------------------------------------------

void CoordinatorServer::HandleSessionRegister(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgSessionRegisterRequest req;
    if (!req.Deserialize(reader)) return;

    SessionId sid = m_sessionMgr.CreateSession(cid, req.displayName, m_currentTime);
    std::string errorMsg;
    if (sid != INVALID_SESSION_ID) {
        auto* session = m_sessionMgr.GetSession(sid);
        if (session && !req.authToken.empty()) {
            std::string userId, username;
            int64_t exp = 0;
            if (ExtractJwtClaims(req.authToken, userId, username, exp)) {
                int64_t nowSec = static_cast<int64_t>(std::time(nullptr));
                if (exp > 0 && nowSec > exp) {
                    m_sessionMgr.RemoveSession(sid);
                    sid = INVALID_SESSION_ID;
                    errorMsg = "Authentication token expired";
                } else {
                    session->authToken = req.authToken;
                    session->persistentUserId = userId;
                    if (!username.empty() && session->displayName.empty()) {
                        session->displayName = username;
                    }
                    Logger::Info("COORDINATOR", "Validated contender session: user_id=" + userId + ", display=" + session->displayName);
                }
            }
        }
    }

    MsgSessionRegisterResponse resp{
        .success = (sid != INVALID_SESSION_ID),
        .sessionId = sid,
        .errorMessage = (sid != INVALID_SESSION_ID) ? "" : (errorMsg.empty() ? "Failed to create session" : errorMsg)
    };

    network::ByteStreamWriter writer;
    resp.Serialize(writer);
    SendMessage(cid, CoordinatorMsgType::SessionRegisterResponse, writer);
}

void CoordinatorServer::HandleSessionHeartbeat(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgSessionHeartbeat req;
    if (!req.Deserialize(reader)) return;
    m_sessionMgr.UpdateHeartbeat(req.sessionId, m_currentTime);
}

void CoordinatorServer::HandlePartyCreate(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgPartyCreateRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s) {
        MsgPartyCreateResponse resp{ .success = false, .partyId = INVALID_PARTY_ID, .errorMessage = "Invalid session" };
        network::ByteStreamWriter w; resp.Serialize(w);
        SendMessage(cid, CoordinatorMsgType::PartyCreateResponse, w);
        return;
    }

    PartyId pid = m_partyMgr.CreateParty(req.sessionId, s->displayName, req.gameMode);
    bool success = (pid != INVALID_PARTY_ID);
    if (success) {
        m_sessionMgr.SetSessionParty(req.sessionId, pid);
    }

    MsgPartyCreateResponse resp{
        .success = success,
        .partyId = pid,
        .errorMessage = success ? "" : "Failed to create party"
    };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::PartyCreateResponse, w);

    if (success) {
        BroadcastPartyUpdate(pid);
    }
}

void CoordinatorServer::HandlePartyJoin(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgPartyJoinRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s) return;

    bool ok = m_partyMgr.AddMember(req.partyId, req.sessionId, s->displayName);
    if (ok) {
        m_sessionMgr.SetSessionParty(req.sessionId, req.partyId);
    }

    MsgPartyJoinResponse resp{
        .success = ok,
        .partyId = ok ? req.partyId : INVALID_PARTY_ID,
        .errorMessage = ok ? "" : "Failed to join party (party full, in queue, or invalid)"
    };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::PartyJoinResponse, w);

    if (ok) {
        BroadcastPartyUpdate(req.partyId);
    }
}

void CoordinatorServer::HandlePartyLeave(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgPartyLeaveRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    PartyId pid = s->partyId;
    bool disbanded = false;
    SessionId newLeader = INVALID_SESSION_ID;

    // If party was queued, dequeue
    m_matchmakingMgr.DequeueParty(pid);

    bool ok = m_partyMgr.RemoveMember(pid, req.sessionId, disbanded, newLeader);
    if (ok) {
        m_sessionMgr.SetSessionParty(req.sessionId, INVALID_PARTY_ID);
    }

    MsgPartyLeaveResponse resp{ .success = ok, .errorMessage = ok ? "" : "Failed to leave party" };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::PartyLeaveResponse, w);

    if (ok && !disbanded) {
        BroadcastPartyUpdate(pid);
    }
}

void CoordinatorServer::HandlePartySetReady(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgPartySetReadyRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    if (m_partyMgr.SetMemberReady(s->partyId, req.sessionId, req.isReady)) {
        BroadcastPartyUpdate(s->partyId);
    }
}

void CoordinatorServer::HandlePartySetMode(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgPartySetModeRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    if (m_partyMgr.SetPartyGameMode(s->partyId, req.sessionId, req.gameMode)) {
        BroadcastPartyUpdate(s->partyId);
    }
}

void CoordinatorServer::HandlePartyKick(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgPartyKickRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.senderSessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    PartyId pid = s->partyId;
    if (m_partyMgr.KickMember(pid, req.senderSessionId, req.targetSessionId)) {
        m_sessionMgr.SetSessionParty(req.targetSessionId, INVALID_PARTY_ID);
        BroadcastPartyUpdate(pid);

        // Notify kicked member
        auto* targetS = m_sessionMgr.GetSession(req.targetSessionId);
        if (targetS) {
            MsgPartyLeaveResponse leaveResp{ .success = true, .errorMessage = "You were kicked from the party" };
            network::ByteStreamWriter w; leaveResp.Serialize(w);
            SendMessage(targetS->connectionId, CoordinatorMsgType::PartyLeaveResponse, w);
        }
    }
}

void CoordinatorServer::HandlePartyTransferLeader(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgPartyTransferLeaderRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.currentLeaderSessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    if (m_partyMgr.TransferLeader(s->partyId, req.currentLeaderSessionId, req.newLeaderSessionId)) {
        BroadcastPartyUpdate(s->partyId);
    }
}

void CoordinatorServer::BroadcastPartyUpdate(PartyId partyId) {
    auto* party = m_partyMgr.GetParty(partyId);
    if (!party) return;

    MsgPartyStateNotification notif{
        .partyId = party->partyId,
        .leaderSessionId = party->leaderSessionId,
        .gameMode = party->gameMode,
        .maxCapacity = party->maxCapacity
    };

    for (const auto& m : party->members) {
        notif.members.push_back(PartyMemberInfo{
            .sessionId = m.sessionId,
            .displayName = m.displayName,
            .isLeader = (m.sessionId == party->leaderSessionId),
            .isReady = m.isReady
        });
    }

    network::ByteStreamWriter writer;
    notif.Serialize(writer);

    for (const auto& m : party->members) {
        auto* s = m_sessionMgr.GetSession(m.sessionId);
        if (s && s->connectionId != network::INVALID_CLIENT_ID) {
            SendMessage(s->connectionId, CoordinatorMsgType::PartyStateNotification, writer);
        }
    }
}

void CoordinatorServer::HandleInviteSend(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgInviteSendRequest req;
    if (!req.Deserialize(reader)) return;

    auto* sender = m_sessionMgr.GetSession(req.senderSessionId);
    auto* recipient = m_sessionMgr.GetSession(req.recipientSessionId);

    if (!sender || !recipient) {
        MsgInviteSendResponse resp{ .success = false, .invitationId = INVALID_INVITATION_ID, .errorMessage = "Invalid sender or recipient" };
        network::ByteStreamWriter w; resp.Serialize(w);
        SendMessage(cid, CoordinatorMsgType::InviteSendResponse, w);
        return;
    }

    uint64_t targetId = (req.type == InvitationType::Party) ? sender->partyId : sender->lobbyId;
    if (targetId == 0) {
        // Automatically create a party for sender if they don't have one
        targetId = m_partyMgr.CreateParty(sender->sessionId, sender->displayName, "solo");
        m_sessionMgr.SetSessionParty(sender->sessionId, targetId);
        BroadcastPartyUpdate(targetId);
    }

    InvitationId invId = m_invitationMgr.SendInvitation(
        sender->sessionId, sender->displayName,
        recipient->sessionId, req.type, targetId,
        m_currentTime, m_config.invitationTimeoutSeconds
    );

    bool ok = (invId != INVALID_INVITATION_ID);
    MsgInviteSendResponse resp{
        .success = ok,
        .invitationId = invId,
        .errorMessage = ok ? "" : "Failed to create invitation"
    };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::InviteSendResponse, w);

    if (ok) {
        // Notify recipient
        MsgInviteReceivedNotification notif{
            .invitationId = invId,
            .senderSessionId = sender->sessionId,
            .senderDisplayName = sender->displayName,
            .type = req.type,
            .targetId = targetId,
            .expirationSeconds = static_cast<uint32_t>(m_config.invitationTimeoutSeconds)
        };
        network::ByteStreamWriter notifWriter;
        notif.Serialize(notifWriter);
        SendMessage(recipient->connectionId, CoordinatorMsgType::InviteReceivedNotification, notifWriter);
    }
}

void CoordinatorServer::HandleInviteRespond(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgInviteRespondRequest req;
    if (!req.Deserialize(reader)) return;

    auto* inv = m_invitationMgr.GetInvitation(req.invitationId);
    if (!inv) {
        MsgInviteRespondResponse resp{ .success = false, .errorMessage = "Invitation not found" };
        network::ByteStreamWriter w; resp.Serialize(w);
        SendMessage(cid, CoordinatorMsgType::InviteRespondResponse, w);
        return;
    }

    bool ok = false;
    if (req.accept) {
        ok = m_invitationMgr.AcceptInvitation(req.invitationId, req.recipientSessionId, m_currentTime);
        if (ok) {
            auto* recipient = m_sessionMgr.GetSession(req.recipientSessionId);
            if (recipient) {
                if (inv->type == InvitationType::Party) {
                    if (m_partyMgr.AddMember(inv->targetId, recipient->sessionId, recipient->displayName)) {
                        m_sessionMgr.SetSessionParty(recipient->sessionId, inv->targetId);
                        BroadcastPartyUpdate(inv->targetId);
                    }
                } else if (inv->type == InvitationType::Lobby) {
                    if (m_lobbyMgr.AddMember(inv->targetId, recipient->sessionId, recipient->displayName)) {
                        m_sessionMgr.SetSessionLobby(recipient->sessionId, inv->targetId);
                    }
                }
            }
        }
    } else {
        ok = m_invitationMgr.DeclineInvitation(req.invitationId, req.recipientSessionId, m_currentTime);
    }

    MsgInviteRespondResponse resp{ .success = ok, .errorMessage = ok ? "" : "Failed to process invitation response" };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::InviteRespondResponse, w);
}

void CoordinatorServer::HandleMatchmakingQueue(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgMatchmakingQueueRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s) return;

    PartyId pid = s->partyId;
    if (pid == INVALID_PARTY_ID) {
        // Create 1-person party for solo player
        pid = m_partyMgr.CreateParty(s->sessionId, s->displayName, req.gameMode);
        m_sessionMgr.SetSessionParty(s->sessionId, pid);
    }

    auto* party = m_partyMgr.GetParty(pid);
    if (!party) return;

    if (party->IsLeader(s->sessionId) && party->gameMode != req.gameMode) {
        m_partyMgr.SetPartyGameMode(pid, s->sessionId, req.gameMode);
    }

    if (!party->IsLeader(s->sessionId)) {
        MsgMatchmakingQueueResponse resp{ .success = false, .gameMode = req.gameMode, .errorMessage = "Only party leader can queue" };
        network::ByteStreamWriter w; resp.Serialize(w);
        SendMessage(cid, CoordinatorMsgType::MatchmakingQueueResponse, w);
        return;
    }

    if (!party->AreAllMembersReady()) {
        MsgMatchmakingQueueResponse resp{ .success = false, .gameMode = req.gameMode, .errorMessage = "All party members must be ready" };
        network::ByteStreamWriter w; resp.Serialize(w);
        SendMessage(cid, CoordinatorMsgType::MatchmakingQueueResponse, w);
        return;
    }

    QueuedParty qParty{
        .partyId = party->partyId,
        .gameMode = party->gameMode,
        .memberCount = static_cast<uint32_t>(party->GetMemberCount()),
        .queuedTime = m_currentTime
    };
    for (const auto& m : party->members) {
        qParty.members.push_back(m.sessionId);
    }

    bool queued = m_matchmakingMgr.EnqueueParty(qParty);
    if (queued) {
        m_partyMgr.SetPartyInQueue(pid, true);
        for (SessionId sid : qParty.members) {
            m_sessionMgr.SetSessionState(sid, SessionState::InQueue);
        }
    }

    MsgMatchmakingQueueResponse resp{
        .success = queued,
        .gameMode = party->gameMode,
        .errorMessage = queued ? "" : "Failed to queue for matchmaking"
    };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::MatchmakingQueueResponse, w);

    // Notify all members of queue status
    if (queued) {
        MsgMatchmakingStatusUpdate status{
            .isQueued = true,
            .gameMode = party->gameMode,
            .queuePosition = static_cast<uint32_t>(m_matchmakingMgr.GetQueueSize(party->gameMode)),
            .estimatedWaitSeconds = 10,
            .elapsedTimeSeconds = 0.0f
        };
        network::ByteStreamWriter sw; status.Serialize(sw);
        for (const auto& m : party->members) {
            auto* memberSession = m_sessionMgr.GetSession(m.sessionId);
            if (memberSession) {
                SendMessage(memberSession->connectionId, CoordinatorMsgType::MatchmakingStatusUpdate, sw);
            }
        }
    }
}

void CoordinatorServer::HandleMatchmakingCancel(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgMatchmakingCancelRequest req;
    if (!req.Deserialize(reader)) return;

    auto* s = m_sessionMgr.GetSession(req.sessionId);
    if (!s || s->partyId == INVALID_PARTY_ID) return;

    auto* party = m_partyMgr.GetParty(s->partyId);
    if (!party) return;

    bool ok = m_matchmakingMgr.DequeueParty(party->partyId);
    if (ok) {
        m_partyMgr.SetPartyInQueue(party->partyId, false);
        for (const auto& m : party->members) {
            m_sessionMgr.SetSessionState(m.sessionId, SessionState::InParty);
        }
    }

    MsgMatchmakingCancelResponse resp{ .success = ok, .errorMessage = ok ? "" : "Party was not queued" };
    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::MatchmakingCancelResponse, w);

    // Notify all members of cancel
    if (ok) {
        MsgMatchmakingStatusUpdate status{ .isQueued = false, .gameMode = party->gameMode, .queuePosition = 0, .estimatedWaitSeconds = 0, .elapsedTimeSeconds = 0.0f };
        network::ByteStreamWriter sw; status.Serialize(sw);
        for (const auto& m : party->members) {
            auto* memberSession = m_sessionMgr.GetSession(m.sessionId);
            if (memberSession) {
                SendMessage(memberSession->connectionId, CoordinatorMsgType::MatchmakingStatusUpdate, sw);
            }
        }
    }
}

void CoordinatorServer::HandleServerRegister(network::ClientId cid, network::ByteStreamReader& reader) {
    MsgServerRegisterRequest req;
    if (!req.Deserialize(reader)) return;

    ServerInstanceId sid = m_serverRegistry.RegisterServer(
        cid, req.serverName, req.hostAddress, req.port,
        req.maxCapacity, req.mapName, req.supportedGameMode, m_currentTime
    );

    bool ok = (sid != INVALID_SERVER_ID);
    MsgServerRegisterResponse resp{
        .success = ok,
        .serverId = sid,
        .errorMessage = ok ? "" : "Failed to register server"
    };

    network::ByteStreamWriter w; resp.Serialize(w);
    SendMessage(cid, CoordinatorMsgType::ServerRegisterResponse, w);
    Logger::Info("COORDINATOR", "Dedicated server registered: " + req.serverName +
                                " (ServerId=" + std::to_string(sid) + ") on " + req.hostAddress + ":" + std::to_string(req.port));
}

void CoordinatorServer::HandleServerHeartbeat(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgServerHeartbeat req;
    if (!req.Deserialize(reader)) return;

    m_serverRegistry.UpdateHeartbeat(
        req.serverId, req.currentPlayers,
        static_cast<ServerRegistryState>(req.serverState), m_currentTime
    );
}

void CoordinatorServer::HandleServerDeregister(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgServerHeartbeat req;
    if (!req.Deserialize(reader)) return;

    m_serverRegistry.DeregisterServer(req.serverId);
    Logger::Info("COORDINATOR", "Dedicated server deregistered: ServerId=" + std::to_string(req.serverId));
}

void CoordinatorServer::HandleMatchReserveResponse(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgMatchReserveResponse resp;
    if (!resp.Deserialize(reader)) return;

    auto* assignment = m_assignmentMgr.GetAssignment(resp.matchId);
    if (!assignment) return;

    if (resp.accepted) {
        m_assignmentMgr.ConfirmReservation(resp.matchId);
        Logger::Info("COORDINATOR", "Match reservation CONFIRMED by server for match " + resp.matchId);

        // Redirect all participants to the dedicated server
        for (const auto& p : assignment->participants) {
            auto* s = m_sessionMgr.GetSession(p.sessionId);
            if (s && s->connectionId != network::INVALID_CLIENT_ID) {
                MsgMatchRedirectNotification redirect{
                    .matchId = assignment->matchId,
                    .serverHost = assignment->serverHost,
                    .serverPort = assignment->serverPort,
                    .playerAuthToken = p.matchAuthToken,
                    .gameMode = assignment->gameMode,
                    .mapName = assignment->mapName
                };
                network::ByteStreamWriter w;
                redirect.Serialize(w);
                SendMessage(s->connectionId, CoordinatorMsgType::MatchRedirectNotification, w);
                m_sessionMgr.SetSessionMatch(s->sessionId, assignment->matchId);
            }
        }
    } else {
        m_assignmentMgr.RejectReservation(resp.matchId);
        Logger::Warn("COORDINATOR", "Match reservation REJECTED by server: " + resp.errorMessage);
        m_diagnostics.reservationFailures++;
    }
}

void CoordinatorServer::HandleMatchCompleted(network::ClientId cid, network::ByteStreamReader& reader) {
    (void)cid;
    MsgMatchCompletedNotification notif;
    if (!notif.Deserialize(reader)) return;

    Logger::Info("COORDINATOR", "Match " + notif.matchId + " completed on server " +
                                std::to_string(notif.serverId) + ". Winner: " + notif.winnerName +
                                ", Duration: " + std::to_string(notif.totalDurationSeconds) + "s");

    m_assignmentMgr.CompleteMatch(notif.matchId);
    m_serverRegistry.SetServerState(notif.serverId, ServerRegistryState::Available);
    m_serverRegistry.SetAssignedMatch(notif.serverId, "");
    m_diagnostics.matchesCompleted++;
}

// ----------------------------------------------------------------------------
// Matchmaking & Loop Logic
// ----------------------------------------------------------------------------

void CoordinatorServer::ProcessFormedMatches() {
    std::vector<FormedMatch> formed;
    m_matchmakingMgr.Update(m_currentTime, formed);

    for (const auto& match : formed) {
        m_diagnostics.matchesFormed++;
        Logger::Info("COORDINATOR", "Match formed: " + match.matchId + " (" + match.gameMode +
                                    ") with " + std::to_string(match.totalParticipants) + " participants.");

        // Find available dedicated server
        auto* server = m_serverRegistry.FindAvailableServer(match.totalParticipants, match.gameMode);
        if (!server) {
            Logger::Warn("COORDINATOR", "No available server for match " + match.matchId + "! Re-queuing parties...");
            for (const auto& p : match.parties) {
                m_matchmakingMgr.EnqueueParty(p);
            }
            continue;
        }

        // Create assignment and reserve capacity
        auto* assignment = m_assignmentMgr.CreateAssignment(match, *server, m_sessionMgr, m_currentTime, m_config.matchReservationTimeoutSeconds);
        if (!assignment) continue;

        m_serverRegistry.SetServerState(server->serverId, ServerRegistryState::Reserved);
        m_serverRegistry.SetAssignedMatch(server->serverId, match.matchId);

        // Send MsgMatchReserveRequest to dedicated server
        MsgMatchReserveRequest reserveReq{
            .matchId = match.matchId,
            .gameMode = match.gameMode,
            .mapName = server->mapName,
            .participants = assignment->participants
        };
        network::ByteStreamWriter w;
        reserveReq.Serialize(w);
        SendMessage(server->connectionId, CoordinatorMsgType::MatchReserveRequest, w);
    }
}

void CoordinatorServer::Update(float dt) {
    if (!m_isRunning) return;

    m_currentTime += dt;
    m_diagnostics.Update(dt);

    // 1. Poll network messages
    if (m_server) {
        m_server->Update(dt);
    }

    // 2. Process matchmaking queues and match formation
    ProcessFormedMatches();

    // 3. Periodic timeout checks (every 1 second)
    m_timeoutCheckAccumulator += dt;
    if (m_timeoutCheckAccumulator >= 1.0f) {
        m_timeoutCheckAccumulator = 0.0f;

        // Session inactivity timeouts
        std::vector<SessionId> timedOutSessions;
        m_sessionMgr.CheckTimeouts(m_currentTime, m_config.sessionTimeoutSeconds, timedOutSessions);
        for (SessionId sid : timedOutSessions) {
            Logger::Info("COORDINATOR", "Session timed out: " + std::to_string(sid));
        }

        // Dedicated server heartbeat timeouts
        std::vector<ServerInstanceId> timedOutServers;
        m_serverRegistry.CheckHeartbeatTimeouts(m_currentTime, m_config.serverHeartbeatTimeoutSeconds, timedOutServers);
        for (ServerInstanceId sid : timedOutServers) {
            Logger::Warn("COORDINATOR", "Dedicated server timed out (marked offline): " + std::to_string(sid));
        }

        // Invitation expirations
        std::vector<InvitationId> expiredInvites;
        m_invitationMgr.CheckExpirations(m_currentTime, expiredInvites);

        // Match reservation timeouts
        std::vector<std::string> timedOutMatches;
        m_assignmentMgr.CheckReservationTimeouts(m_currentTime, timedOutMatches);
        for (const auto& mid : timedOutMatches) {
            Logger::Warn("COORDINATOR", "Match reservation timed out for match: " + mid);
            m_diagnostics.reservationFailures++;
        }

        // Update diagnostics counts
        m_diagnostics.activeSessions = static_cast<uint32_t>(m_sessionMgr.GetActiveSessionCount());
        m_diagnostics.activeParties = static_cast<uint32_t>(m_partyMgr.GetPartyCount());
        m_diagnostics.activeLobbies = static_cast<uint32_t>(m_lobbyMgr.GetLobbyCount());
        m_diagnostics.queuedParties = static_cast<uint32_t>(m_matchmakingMgr.GetTotalQueuedParties());
        m_diagnostics.registeredServers = static_cast<uint32_t>(m_serverRegistry.GetServerCount());
        m_diagnostics.availableServers = static_cast<uint32_t>(m_serverRegistry.GetAvailableServerCount());
    }
}

} // namespace wildbound::coordinator

