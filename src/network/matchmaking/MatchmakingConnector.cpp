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

#include "MatchmakingConnector.hpp"
#include "../ENetClient.hpp"
#include "../../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::network::matchmaking {

MatchmakingConnector::MatchmakingConnector()
    : m_client(std::make_unique<ENetClient>())
    , m_alive(std::make_shared<bool>(true)) {
}

MatchmakingConnector::~MatchmakingConnector() {
    if (m_alive) {
        *m_alive = false;
    }
    Shutdown();
}

bool MatchmakingConnector::IsConnected() const noexcept {
    return m_client && m_client->IsConnected();
}

bool MatchmakingConnector::Connect(const std::string& host,
                                   uint16_t port,
                                   const std::string& displayName,
                                   const std::string& authToken) {
    m_coordinatorHost = host;
    m_coordinatorPort = port;
    m_displayName = displayName;
    m_authToken = authToken;
    m_lastErrorMessage.clear();
    m_requestTimeoutTimer = 0.0f;

    if (!m_client) {
        m_client = std::make_unique<ENetClient>();
    }

    auto alive = m_alive;
    m_client->SetEventCallback([this, alive](const ConnectionEvent& ev) {
        if (!*alive) return;
        HandleNetworkEvent(ev);
    });

    if (m_client->IsConnected()) {
        if (m_sessionId != coordinator::INVALID_SESSION_ID) {
            TransitionTo(MatchmakingState::InLobby);
            return true;
        }
        TransitionTo(MatchmakingState::RegisteringSession);
        coordinator::MsgSessionRegisterRequest req{
            .displayName = m_displayName,
            .clientVersion = "1.0.0",
            .authToken = m_authToken
        };
        ByteStreamWriter w;
        req.Serialize(w);
        SendCoordinatorMsg(coordinator::CoordinatorMsgType::SessionRegisterRequest, w);
        return true;
    }

    TransitionTo(MatchmakingState::Connecting);
    if (!m_client->Connect(host, port)) {
        m_lastErrorMessage = "Failed to initiate connection to Coordinator at " + host + ":" + std::to_string(port);
        Logger::Error("MATCH_CONN", m_lastErrorMessage);
        TransitionTo(MatchmakingState::Failed);
        return false;
    }

    Logger::Info("MATCH_CONN", "Connecting to Coordinator at " + host + ":" + std::to_string(port) + "...");
    return true;
}

void MatchmakingConnector::Shutdown() {
    if (m_client && m_client->IsConnected()) {
        m_client->Disconnect();
    }
    m_state = MatchmakingState::Idle;
    m_sessionId = coordinator::INVALID_SESSION_ID;
    m_partyId = coordinator::INVALID_PARTY_ID;
    m_hasParty = false;
    m_queueTimer = 0.0f;
    m_queuePosition = 0;
    m_estimatedWaitSeconds = 0;
    m_assignment = MatchAssignmentInfo{};
}

void MatchmakingConnector::Reset() {
    m_lastErrorMessage.clear();
    m_queueTimer = 0.0f;
    m_queuePosition = 0;
    m_assignment = MatchAssignmentInfo{};

    if (m_client && m_client->IsConnected() && m_sessionId != coordinator::INVALID_SESSION_ID) {
        TransitionTo(MatchmakingState::InLobby);
    } else {
        TransitionTo(MatchmakingState::Idle);
    }
}

bool MatchmakingConnector::RequestMatchmaking(const std::string& gameMode) {
    if (m_state == MatchmakingState::RequestingMatch || m_state == MatchmakingState::Queued) {
        Logger::Warn("MATCH_CONN", "Duplicate matchmaking request rejected: already in queue or requesting.");
        return false;
    }

    if (gameMode != "solo" && gameMode != "duos" && gameMode != "squads") {
        m_lastErrorMessage = "Invalid game mode requested: " + gameMode;
        Logger::Error("MATCH_CONN", m_lastErrorMessage);
        return false;
    }

    if (!m_client || !m_client->IsConnected() || m_sessionId == coordinator::INVALID_SESSION_ID) {
        m_lastErrorMessage = "Cannot queue: Not connected or registered with Coordinator.";
        Logger::Error("MATCH_CONN", m_lastErrorMessage);
        TransitionTo(MatchmakingState::Failed);
        return false;
    }

    m_queuedMode = gameMode;
    m_queueTimer = 0.0f;
    m_queuePosition = 0;
    m_requestTimeoutTimer = 0.0f;
    m_assignment = MatchAssignmentInfo{};
    m_lastErrorMessage.clear();

    if (!TransitionTo(MatchmakingState::RequestingMatch)) {
        return false;
    }

    if (m_hasParty) {
        coordinator::MsgPartySetModeRequest modeReq{
            .sessionId = m_sessionId,
            .gameMode = gameMode
        };
        ByteStreamWriter mw;
        modeReq.Serialize(mw);
        SendCoordinatorMsg(coordinator::CoordinatorMsgType::PartySetModeRequest, mw);
    }

    coordinator::MsgMatchmakingQueueRequest req{
        .sessionId = m_sessionId,
        .gameMode = gameMode
    };
    ByteStreamWriter w;
    req.Serialize(w);
    SendCoordinatorMsg(coordinator::CoordinatorMsgType::MatchmakingQueueRequest, w);
    Logger::Info("MATCH_CONN", "Dispatched MatchmakingQueueRequest for mode '" + gameMode + "' (SessionId=" + std::to_string(m_sessionId) + ")");
    return true;
}

bool MatchmakingConnector::CancelMatchmaking() {
    if (m_state != MatchmakingState::Queued && m_state != MatchmakingState::RequestingMatch) {
        Logger::Warn("MATCH_CONN", "Cannot cancel matchmaking: Not actively queued or requesting.");
        return false;
    }

    if (!TransitionTo(MatchmakingState::Cancelling)) {
        return false;
    }

    m_requestTimeoutTimer = 0.0f;

    coordinator::MsgMatchmakingCancelRequest req{
        .sessionId = m_sessionId
    };
    ByteStreamWriter w;
    req.Serialize(w);
    SendCoordinatorMsg(coordinator::CoordinatorMsgType::MatchmakingCancelRequest, w);
    Logger::Info("MATCH_CONN", "Dispatched MatchmakingCancelRequest (SessionId=" + std::to_string(m_sessionId) + ")");
    return true;
}

void MatchmakingConnector::Update(float dt) {
    if (m_client) {
        m_client->Update(dt);
    }

    if (m_state == MatchmakingState::Queued) {
        m_queueTimer += dt;
    }

    if (m_state == MatchmakingState::Connecting ||
        m_state == MatchmakingState::RegisteringSession ||
        m_state == MatchmakingState::RequestingMatch ||
        m_state == MatchmakingState::Cancelling) {
        m_requestTimeoutTimer += dt;
        if (m_requestTimeoutTimer >= REQUEST_TIMEOUT_SECONDS) {
            if (m_state == MatchmakingState::Cancelling) {
                Logger::Warn("MATCH_CONN", "Matchmaking cancellation request timed out; forcing CANCELLED.");
                TransitionTo(MatchmakingState::Cancelled);
            } else {
                m_lastErrorMessage = "Coordinator request timed out after " + std::to_string(static_cast<int>(REQUEST_TIMEOUT_SECONDS)) + " seconds.";
                Logger::Error("MATCH_CONN", m_lastErrorMessage);
                TransitionTo(MatchmakingState::Failed);
            }
        }
    }
}

bool MatchmakingConnector::TransitionTo(MatchmakingState newState) {
    if (m_state == newState) {
        return true;
    }

    bool valid = false;
    switch (m_state) {
        case MatchmakingState::Idle:
            valid = (newState == MatchmakingState::Connecting ||
                     newState == MatchmakingState::RegisteringSession ||
                     newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Failed);
            break;

        case MatchmakingState::Connecting:
            valid = (newState == MatchmakingState::RegisteringSession ||
                     newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Failed ||
                     newState == MatchmakingState::Idle);
            break;

        case MatchmakingState::RegisteringSession:
            valid = (newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Failed ||
                     newState == MatchmakingState::Idle);
            break;

        case MatchmakingState::InLobby:
            valid = (newState == MatchmakingState::RequestingMatch ||
                     newState == MatchmakingState::Idle ||
                     newState == MatchmakingState::Connecting ||
                     newState == MatchmakingState::Failed);
            break;

        case MatchmakingState::RequestingMatch:
            valid = (newState == MatchmakingState::Queued ||
                     newState == MatchmakingState::Cancelling ||
                     newState == MatchmakingState::Cancelled ||
                     newState == MatchmakingState::Failed ||
                     newState == MatchmakingState::InLobby);
            break;

        case MatchmakingState::Queued:
            valid = (newState == MatchmakingState::MatchFound ||
                     newState == MatchmakingState::Cancelling ||
                     newState == MatchmakingState::Cancelled ||
                     newState == MatchmakingState::Failed ||
                     newState == MatchmakingState::InLobby);
            break;

        case MatchmakingState::Cancelling:
            valid = (newState == MatchmakingState::Cancelled ||
                     newState == MatchmakingState::MatchFound ||
                     newState == MatchmakingState::Failed ||
                     newState == MatchmakingState::InLobby);
            break;

        case MatchmakingState::Cancelled:
            valid = (newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Idle ||
                     newState == MatchmakingState::RequestingMatch);
            break;

        case MatchmakingState::Failed:
            valid = (newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Idle ||
                     newState == MatchmakingState::Connecting ||
                     newState == MatchmakingState::RequestingMatch);
            break;

        case MatchmakingState::MatchFound:
            valid = (newState == MatchmakingState::InLobby ||
                     newState == MatchmakingState::Idle);
            break;
    }

    if (!valid) {
        Logger::Warn("MATCH_CONN", "Invalid state transition requested: " +
                     std::string(MatchmakingStateToString(m_state)) + " -> " +
                     std::string(MatchmakingStateToString(newState)));
        return false;
    }

    MatchmakingState oldState = m_state;
    m_state = newState;
    m_requestTimeoutTimer = 0.0f;

    Logger::Info("MATCH_CONN", "State transition: " +
                 std::string(MatchmakingStateToString(oldState)) + " -> " +
                 std::string(MatchmakingStateToString(newState)));

    if (m_onStateChanged) {
        m_onStateChanged(oldState, newState);
    }

    if (newState == MatchmakingState::Failed && m_onError) {
        m_onError(m_lastErrorMessage);
    }

    return true;
}

void MatchmakingConnector::HandleNetworkEvent(const ConnectionEvent& ev) {
    switch (ev.type) {
        case ConnectionEventType::ClientConnected: {
            Logger::Info("MATCH_CONN", "Connected to Matchmaking Coordinator transport.");
            TransitionTo(MatchmakingState::RegisteringSession);
            coordinator::MsgSessionRegisterRequest req{
                .displayName = m_displayName,
                .clientVersion = "1.0.0",
                .authToken = m_authToken
            };
            ByteStreamWriter w;
            req.Serialize(w);
            SendCoordinatorMsg(coordinator::CoordinatorMsgType::SessionRegisterRequest, w);
            break;
        }

        case ConnectionEventType::ClientDisconnected: {
            Logger::Warn("MATCH_CONN", "Coordinator transport disconnected.");
            if (m_state != MatchmakingState::Idle && m_state != MatchmakingState::MatchFound) {
                m_lastErrorMessage = "Connection to Coordinator was lost.";
                TransitionTo(MatchmakingState::Failed);
            }
            break;
        }

        case ConnectionEventType::PacketReceived: {
            HandleCoordinatorPacket(ev.messageType, ev.payload.data(), ev.payload.size());
            break;
        }

        default:
            break;
    }
}

void MatchmakingConnector::HandleCoordinatorPacket(uint16_t messageType, const uint8_t* data, size_t size) {
    ByteStreamReader reader(data, size);
    auto type = static_cast<coordinator::CoordinatorMsgType>(messageType);

    switch (type) {
        case coordinator::CoordinatorMsgType::SessionRegisterResponse: {
            coordinator::MsgSessionRegisterResponse resp;
            if (resp.Deserialize(reader)) {
                if (resp.success) {
                    m_sessionId = resp.sessionId;
                    Logger::Info("MATCH_CONN", "Session registered successfully. SessionId=" + std::to_string(m_sessionId));
                    TransitionTo(MatchmakingState::InLobby);
                } else {
                    m_lastErrorMessage = resp.errorMessage.empty() ? "Session registration rejected by Coordinator." : resp.errorMessage;
                    Logger::Error("MATCH_CONN", m_lastErrorMessage);
                    TransitionTo(MatchmakingState::Failed);
                }
            } else {
                m_lastErrorMessage = "Malformed SessionRegisterResponse received.";
                Logger::Error("MATCH_CONN", m_lastErrorMessage);
                TransitionTo(MatchmakingState::Failed);
            }
            break;
        }

        case coordinator::CoordinatorMsgType::MatchmakingQueueResponse: {
            coordinator::MsgMatchmakingQueueResponse resp;
            if (resp.Deserialize(reader)) {
                if (resp.success) {
                    Logger::Info("MATCH_CONN", "Queue request accepted by Coordinator for mode '" + resp.gameMode + "'.");
                    TransitionTo(MatchmakingState::Queued);
                } else {
                    m_lastErrorMessage = resp.errorMessage.empty() ? "Matchmaking queue request rejected." : resp.errorMessage;
                    Logger::Error("MATCH_CONN", m_lastErrorMessage);
                    TransitionTo(MatchmakingState::Failed);
                }
            } else {
                m_lastErrorMessage = "Malformed MatchmakingQueueResponse received.";
                Logger::Error("MATCH_CONN", m_lastErrorMessage);
                TransitionTo(MatchmakingState::Failed);
            }
            break;
        }

        case coordinator::CoordinatorMsgType::MatchmakingCancelResponse: {
            coordinator::MsgMatchmakingCancelResponse resp;
            if (resp.Deserialize(reader)) {
                if (resp.success) {
                    Logger::Info("MATCH_CONN", "Queue cancellation confirmed by Coordinator.");
                    TransitionTo(MatchmakingState::Cancelled);
                } else {
                    m_lastErrorMessage = resp.errorMessage.empty() ? "Queue cancellation rejected." : resp.errorMessage;
                    Logger::Warn("MATCH_CONN", m_lastErrorMessage);
                    TransitionTo(MatchmakingState::Cancelled);
                }
            } else {
                TransitionTo(MatchmakingState::Cancelled);
            }
            break;
        }

        case coordinator::CoordinatorMsgType::MatchmakingStatusUpdate: {
            coordinator::MsgMatchmakingStatusUpdate notif;
            if (notif.Deserialize(reader)) {
                m_queuePosition = notif.queuePosition;
                m_estimatedWaitSeconds = notif.estimatedWaitSeconds;
                if (notif.elapsedTimeSeconds > 0.0f) {
                    m_queueTimer = notif.elapsedTimeSeconds;
                }
                if (!notif.isQueued && m_state == MatchmakingState::Queued) {
                    Logger::Info("MATCH_CONN", "Status update reports no longer queued.");
                    TransitionTo(MatchmakingState::Cancelled);
                }
            }
            break;
        }

        case coordinator::CoordinatorMsgType::MatchRedirectNotification: {
            coordinator::MsgMatchRedirectNotification notif;
            if (notif.Deserialize(reader)) {
                MatchAssignmentInfo info;
                if (ValidateAssignment(notif, info)) {
                    m_assignment = info;
                    Logger::Info("MATCH_CONN", "Received valid match assignment! MatchId=" + info.matchId +
                                 " Server=" + info.serverHost + ":" + std::to_string(info.serverPort));
                    TransitionTo(MatchmakingState::MatchFound);
                    if (m_onMatchFound) {
                        m_onMatchFound(m_assignment);
                    }
                } else {
                    m_lastErrorMessage = "Received invalid or malformed MatchRedirectNotification from Coordinator.";
                    Logger::Error("MATCH_CONN", m_lastErrorMessage);
                    TransitionTo(MatchmakingState::Failed);
                }
            } else {
                m_lastErrorMessage = "Failed to deserialize MatchRedirectNotification.";
                Logger::Error("MATCH_CONN", m_lastErrorMessage);
                TransitionTo(MatchmakingState::Failed);
            }
            break;
        }

        case coordinator::CoordinatorMsgType::PartyStateNotification: {
            coordinator::MsgPartyStateNotification notif;
            if (notif.Deserialize(reader)) {
                m_hasParty = true;
                m_partyId = notif.partyId;
                m_partyState = notif;
                Logger::Info("MATCH_CONN", "Party state updated. PartyId=" + std::to_string(m_partyId) +
                             " Members=" + std::to_string(notif.members.size()));
            }
            break;
        }

        default:
            break;
    }
}

bool MatchmakingConnector::ValidateAssignment(const coordinator::MsgMatchRedirectNotification& notif, MatchAssignmentInfo& outInfo) {
    if (notif.matchId.empty()) {
        Logger::Error("MATCH_CONN", "Assignment validation failed: empty matchId");
        return false;
    }
    if (notif.serverHost.empty()) {
        Logger::Error("MATCH_CONN", "Assignment validation failed: empty serverHost");
        return false;
    }
    if (notif.serverPort == 0) {
        Logger::Error("MATCH_CONN", "Assignment validation failed: invalid serverPort (0)");
        return false;
    }
    if (notif.playerAuthToken.empty()) {
        Logger::Error("MATCH_CONN", "Assignment validation failed: empty playerAuthToken");
        return false;
    }
    if (notif.gameMode != "solo" && notif.gameMode != "duos" && notif.gameMode != "squads") {
        Logger::Error("MATCH_CONN", "Assignment validation failed: invalid gameMode '" + notif.gameMode + "'");
        return false;
    }
    if (notif.mapName.empty()) {
        Logger::Error("MATCH_CONN", "Assignment validation failed: empty mapName");
        return false;
    }

    outInfo.matchId = notif.matchId;
    outInfo.serverHost = notif.serverHost;
    outInfo.serverPort = notif.serverPort;
    outInfo.playerAuthToken = notif.playerAuthToken;
    outInfo.gameMode = notif.gameMode;
    outInfo.mapName = notif.mapName;
    outInfo.assignedTimestamp = 1; // Mark as populated
    return true;
}

void MatchmakingConnector::SendCoordinatorMsg(coordinator::CoordinatorMsgType type, const ByteStreamWriter& payload) {
    if (!m_client) return;

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
    m_client->Send(ChannelType::Control, DeliveryMode::ReliableOrdered,
                  packetWriter.GetData(), packetWriter.GetSize());
}

void MatchmakingConnector::InjectPacket(uint16_t messageType, const uint8_t* data, size_t size) {
    HandleCoordinatorPacket(messageType, data, size);
}

} // namespace wildbound::network::matchmaking
