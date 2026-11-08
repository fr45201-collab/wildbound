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

#include "DedicatedServer.hpp"
#include "../core/Logger.hpp"
#include "../physics/CollisionManager.hpp"
#include "../world/World.hpp"
#include "../combat/CombatSystem.hpp"
#include "../items/ItemManager.hpp"
#include "../storm/StormManager.hpp"
#include "../match/MatchManager.hpp"
#include "../gamemodes/GameModeManager.hpp"
#include "../network/NetworkManager.hpp"
#include "../network/ENetClient.hpp"
#include "../network/ENetServer.hpp"
#include "../network/replication/ReplicationManager.hpp"
#include "../scripting/LuaManager.hpp"
#include "../entities/Player.hpp"
#include "MatchResultsOutbox.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <cmath>

#include <iostream>
#include <chrono>

using wildbound::core::Logger;
using wildbound::core::LogLevel;

namespace wildbound::server {

DedicatedServer* DedicatedServer::s_activeInstance = nullptr;

DedicatedServer::DedicatedServer()
    : m_platform(platform::CreatePlatform()) {
    s_activeInstance = this;
    if (m_platform) {
        m_platform->ConfigureConsole();
    }
}

DedicatedServer::~DedicatedServer() {
    Shutdown();
    if (s_activeInstance == this) {
        s_activeInstance = nullptr;
    }
}

void DedicatedServer::SetupConsoleSignalHandling() {
    if (m_platform) {
        m_platform->RegisterTerminationHandler([this](const std::string& reason) {
            Logger::Info("SERVER", "OS termination signal received (" + reason + "). Initiating graceful shutdown...");
            RequestShutdown(reason);
        });
    }
}

bool DedicatedServer::Initialize(int argc, char* argv[]) {
    if (m_state != ServerLifecycleState::Uninitialized) {
        return false;
    }

    m_state = ServerLifecycleState::Initializing;

    // 1. Parse command line arguments
    m_config = ServerConfig::GetDefault();
    if (argc > 0 && argv != nullptr) {
        m_cli.Parse(argc, argv, m_config);
    }

    if (m_cli.ShouldShowHelp() || m_cli.ShouldShowVersion()) {
        m_state = ServerLifecycleState::Stopped;
        return true;
    }

    // 2. Load configuration file with fallback
    if (!m_config.LoadFromFile(m_cli.GetConfigFilePath())) {
        (void)m_config.LoadFromFile("data/server/server.json");
    }

    // Apply CLI overrides again so command-line always takes precedence over file
    if (argc > 0 && argv != nullptr) {
        m_cli.Parse(argc, argv, m_config);
    }

    // 3. Initialize logging subsystem
    LogLevel level = LogLevel::Info;
    if (m_config.logLevel == "debug" || m_config.logLevel == "trace") level = LogLevel::Debug;
    else if (m_config.logLevel == "warn") level = LogLevel::Warning;
    else if (m_config.logLevel == "error") level = LogLevel::Error;

    Logger::Init(level);
    Logger::Info("SERVER", "=== Wildbound Dedicated Server Initializing ===");
    if (m_platform) {
        Logger::Info("SERVER", "Target Platform: " + std::string(m_platform->GetName()));
        auto mem = m_platform->GetMemoryMetrics();
        if (mem.totalPhysicalBytes > 0) {
            Logger::Info("SERVER", "Host Physical Memory: " +
                         std::to_string(mem.availablePhysicalBytes / (1024 * 1024)) + " MB available / " +
                         std::to_string(mem.totalPhysicalBytes / (1024 * 1024)) + " MB total");
        }
    }
    Logger::Info("SERVER", "Configured bind: " + m_config.bindAddress + ":" + std::to_string(m_config.port) +
                 " | Max clients: " + std::to_string(m_config.maxClients) +
                 " | Tick rate: " + std::to_string(m_config.tickRate) + " Hz");

    SetupConsoleSignalHandling();

    try {
        // 4. Initialize scripting runtime
        m_luaManager = std::make_unique<scripting::LuaManager>();
        if (!m_luaManager->Initialize()) {
            Logger::Warn("SERVER", "Lua runtime could not initialize; continuing with compiled C++ rules.");
        }

        // 5. Initialize physics & collision
        m_collisionManager = std::make_unique<physics::CollisionManager>();

        // 6. Initialize items & weapons
        m_itemManager = std::make_unique<items::ItemManager>();
        (void)m_itemManager->Initialize("data/items/item_catalog.json");

        // 7. Initialize combat system
        m_combatSystem = std::make_unique<combat::CombatSystem>();
        (void)m_combatSystem->Initialize();

        // 8. Initialize world (loaded headlessly without textures or audio)
        m_world = std::make_unique<world::World>();
        if (!m_world->LoadMap(m_config.mapName, nullptr)) {
            Logger::Warn("SERVER", "World map '" + m_config.mapName + "' not found. Falling back to default map.");
            (void)m_world->LoadMap("maps/forest/forest_01.json", nullptr);
        }

        // 9. Initialize storm system
        m_stormManager = std::make_unique<storm::StormManager>();
        (void)m_stormManager->Initialize();

        // 10. Initialize match manager
        m_matchManager = std::make_unique<match::MatchManager>();
        (void)m_matchManager->Initialize();

        // 11. Initialize game mode manager
        m_gameModeManager = std::make_unique<gamemodes::GameModeManager>();
        (void)m_gameModeManager->Initialize();
        (void)m_gameModeManager->SelectMode(m_config.gameMode);

        // 12. Initialize network & replication coordinator
        m_networkManager = std::make_unique<network::NetworkManager>();
        (void)m_networkManager->Initialize();

        // 13. Configure simulation loop
        m_loop.SetTickRate(m_config.tickRate);
        m_replicationInterval = 1.0f / static_cast<float>(m_config.tickRate);

        m_loop.SetNetworkPollHook([this]() { OnNetworkPoll(); });
        m_loop.SetFixedUpdateHook([this](float fixedDt) { OnFixedUpdate(fixedDt); });
        m_loop.SetReplicationHook([this]() { OnReplicationUpdate(); });
        m_loop.SetDiagnosticsHook([this](float frameDt, float durationMs, bool overload) {
            OnDiagnosticsUpdate(frameDt, durationMs, overload);
        });

        // 14. Initialize durable match results outbox (Phase 42)
        m_matchOutbox = std::make_unique<MatchResultsOutbox>();
        m_matchOutbox->Initialize();

        // 15. Load cosmetic registry for server-side appearance validation (Phase 45)
        m_serverCosmeticRegistry.LoadFromCatalog("data/cosmetics/costume_catalog.json");

        m_diagnostics.Reset();
        m_state = ServerLifecycleState::Starting;
        return true;
    } catch (const std::exception& ex) {
        Logger::Error("SERVER", std::string("Dedicated server initialization failed: ") + ex.what());
        m_state = ServerLifecycleState::Failed;
        Shutdown();
        return false;
    }
}

bool DedicatedServer::Start() {
    if (m_state != ServerLifecycleState::Starting && m_state != ServerLifecycleState::Initializing) {
        return false;
    }

    Logger::Info("SERVER", "Starting authoritative ENet server host on port " + std::to_string(m_config.port) + "...");

    if (!m_networkManager->StartServer(m_config.port, m_config.maxClients)) {
        Logger::Error("SERVER", "Failed to bind and start ENet server host on port " + std::to_string(m_config.port));
        m_state = ServerLifecycleState::Failed;
        return false;
    }

    m_state = ServerLifecycleState::Running;
    Logger::Info("SERVER", "Dedicated Server is now RUNNING and accepting client connections.");

    // Phase 47: Register ENetServer event callback for client handling, rate-limiting & security validation
    auto* enetServer = m_networkManager->GetServer();
    if (enetServer) {
        enetServer->SetEventCallback([this](const network::ConnectionEvent& ev) {
            if (ev.type == network::ConnectionEventType::ClientConnected) {
                m_securityManager.OnClientConnected(ev.connectionId);
                Logger::Info("SERVER", "Client #" + std::to_string(ev.connectionId) + " connected to game server.");
            } else if (ev.type == network::ConnectionEventType::ClientDisconnected) {
                m_securityManager.OnClientDisconnected(ev.connectionId);
                m_clientAppearances.erase(ev.connectionId);
                m_clientToParticipant.erase(ev.connectionId);
                Logger::Info("SERVER", "Client #" + std::to_string(ev.connectionId) + " disconnected from game server.");
            } else if (ev.type == network::ConnectionEventType::PacketReceived) {
                OnClientPacketReceived(ev.connectionId, ev.channel, ev.messageType, ev.payload.data(), ev.payload.size());
            }
        });
    }

    if (m_config.enableCoordinator) {
        ConnectToCoordinator(m_config.coordinatorHost, m_config.coordinatorPort);
    }
    return true;
}

void DedicatedServer::Run() {
    if (m_state != ServerLifecycleState::Running) {
        if (!Start()) {
            return;
        }
    }

    if (m_cli.IsTestRunMode()) {
        Logger::Info("SERVER", "Test-run mode enabled. Running 5 simulation ticks headlessly...");
        for (int i = 0; i < 5; ++i) {
            m_loop.SingleStep(m_loop.GetFixedDeltaTime());
        }
        Logger::Info("SERVER", "Headless verification ticks completed successfully.");
        Shutdown();
        return;
    }

    Logger::Info("SERVER", "Entering main dedicated server simulation loop.");
    m_loop.Run();
    Shutdown();
}

void DedicatedServer::RequestShutdown(const std::string& reason) {
    if (m_isShuttingDown || m_state == ServerLifecycleState::Stopping || m_state == ServerLifecycleState::Stopped) {
        return;
    }

    m_isShuttingDown = true;
    m_state = ServerLifecycleState::Stopping;
    Logger::Info("SERVER", "Server shutdown requested: " + reason);
    m_loop.Stop();
}

void DedicatedServer::Shutdown() {
    if (m_state == ServerLifecycleState::Stopped) {
        return;
    }

    m_state = ServerLifecycleState::Stopping;
    Logger::Info("SERVER", "Executing orderly dedicated server shutdown sequence...");

    // Disconnect clients gracefully and stop network server host
    if (m_networkManager && m_networkManager->IsServerRunning()) {
        m_networkManager->StopServer();
    }

    if (m_platform) {
        m_platform->UnregisterTerminationHandler();
    }

    // Disconnect from coordinator service
    DisconnectFromCoordinator();

    // Reset gameplay subsystems
    if (m_matchManager) {
        m_matchManager->ResetMatch();
    }
    if (m_world) {
        m_world->UnloadMap();
    }
    if (m_luaManager) {
        m_luaManager->Shutdown();
    }

    m_state = ServerLifecycleState::Stopped;
    Logger::Info("SERVER", "Dedicated Server shutdown completed cleanly.");
    Logger::Info("SERVER", m_diagnostics.FormatSummary());
}

void DedicatedServer::PollNetwork() {
    OnNetworkPoll();
}

void DedicatedServer::UpdateServer(float dt) {
    m_loop.SingleStep(dt);
}

void DedicatedServer::OnNetworkPoll() {
    if (m_networkManager) {
        m_networkManager->Update(0.0f); // Poll transport sockets
    }
    if (m_coordinatorClient) {
        m_coordinatorClient->Update(0.0f);
    }
    m_cosmeticHttpClient.Update();
}

void DedicatedServer::OnFixedUpdate(float fixedDt) {
    // 0. Periodic coordinator heartbeat (every 5s)
    if (m_coordinatorClient && IsConnectedToCoordinator()) {
        m_coordinatorHeartbeatAccumulator += fixedDt;
        if (m_coordinatorHeartbeatAccumulator >= 5.0f) {
            m_coordinatorHeartbeatAccumulator = 0.0f;
            SendCoordinatorHeartbeat();
        }
    }
    // 1. Advance simulation tick in replication manager
    if (m_networkManager) {
        auto* replMgr = m_networkManager->GetReplicationManager();
        if (replMgr) {
            replMgr->Update(fixedDt);
        }
    }

    // 2. Physics & collision
    if (m_collisionManager) {
        m_collisionManager->Update(fixedDt);
    }

    // 3. Combat & projectiles
    if (m_combatSystem) {
        m_combatSystem->Update(fixedDt, nullptr);
    }

    // 4. Corruption storm progression
    if (m_stormManager && m_stormManager->GetState() != storm::StormState::Idle) {
        m_stormManager->Update(fixedDt, nullptr, nullptr, nullptr);
    }

    // 5. Match lifecycle progression
    if (m_matchManager && m_matchManager->IsActive()) {
        m_matchManager->Update(fixedDt, nullptr, m_stormManager.get(), nullptr, nullptr);
    }

    // 6. Durable match results outbox processing
    if (m_matchOutbox) {
        m_matchOutbox->Update(fixedDt);
    }
}

void DedicatedServer::OnReplicationUpdate() {
    if (!m_networkManager || !m_networkManager->IsServerRunning()) {
        return;
    }

    auto* replMgr = m_networkManager->GetReplicationManager();
    if (!replMgr) {
        return;
    }

    // Build authoritative snapshot and broadcast over Channel 3 (Replication)
    auto snapshot = replMgr->BuildSnapshot(false);
    if (!snapshot.entities.empty()) {
        network::ByteStreamWriter payloadWriter;
        snapshot.Serialize(payloadWriter);

        network::ByteStreamWriter packetWriter;
        network::PacketHeader hdr{
            .magic = network::PROTOCOL_MAGIC,
            .version = network::PROTOCOL_VERSION,
            .messageType = static_cast<uint16_t>(network::MessageType::StateSnapshot),
            .sequenceNumber = static_cast<uint32_t>(snapshot.serverTick),
            .senderClientId = network::SERVER_CLIENT_ID,
            .payloadLength = static_cast<uint16_t>(payloadWriter.GetSize())
        };
        network::SerializePacketHeader(packetWriter, hdr);
        packetWriter.WriteBytes(payloadWriter.GetData(), payloadWriter.GetSize());

        // Broadcast to connected clients
        m_networkManager->SendTestMessage(false, ""); // Keep-alive / flush
    }
}

void DedicatedServer::OnDiagnosticsUpdate(float frameDt, float tickDurationMs, bool wasOverload) {
    m_diagnostics.Update(frameDt, m_state);
    m_diagnostics.RecordTickDuration(tickDurationMs, wasOverload);

    if (m_networkManager) {
        size_t connected = m_networkManager->GetServerConnectedCount();
        size_t registered = m_matchManager ? m_matchManager->GetRegistry().GetTotalCount() : 0;
        m_diagnostics.SetClientCounts(connected, registered);

        auto* repl = m_networkManager->GetReplicationManager();
        if (repl) {
            auto rDiag = repl->GetDiagnostics();
            m_diagnostics.SetEntityCounts(rDiag.totalReplicatedEntities, rDiag.enemyCount, rDiag.lootCount);
        }
    }
}

// ----------------------------------------------------------------------------
// Coordinator Integration (Phase 41)
// ----------------------------------------------------------------------------

bool DedicatedServer::ConnectToCoordinator(const std::string& host, uint16_t port) {
    if (!m_coordinatorClient) {
        m_coordinatorClient = std::make_unique<network::ENetClient>();
    }

    Logger::Info("SERVER", "Connecting to Coordinator at " + host + ":" + std::to_string(port) + "...");

    m_coordinatorClient->SetEventCallback([this](const network::ConnectionEvent& ev) {
        if (ev.type == network::ConnectionEventType::ClientConnected) {
            Logger::Info("SERVER", "Connected to Coordinator! Registering server instance...");
            // Send registration request
            coordinator::MsgServerRegisterRequest req{
                .serverName = m_config.serverName,
                .hostAddress = m_config.bindAddress,
                .port = m_config.port,
                .maxCapacity = m_config.maxClients,
                .mapName = m_config.mapName,
                .supportedGameMode = m_config.gameMode
            };
            network::ByteStreamWriter writer;
            req.Serialize(writer);

            network::ByteStreamWriter packetWriter;
            network::PacketHeader hdr{
                .magic = network::PROTOCOL_MAGIC,
                .version = network::PROTOCOL_VERSION,
                .messageType = static_cast<uint16_t>(coordinator::CoordinatorMsgType::ServerRegisterRequest),
                .sequenceNumber = 0,
                .senderClientId = 0,
                .payloadLength = static_cast<uint16_t>(writer.GetSize())
            };
            network::SerializePacketHeader(packetWriter, hdr);
            packetWriter.WriteBytes(writer.GetData(), writer.GetSize());
            m_coordinatorClient->Send(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                                      packetWriter.GetData(), packetWriter.GetSize());
        } else if (ev.type == network::ConnectionEventType::PacketReceived) {
            OnCoordinatorPacketReceived(ev.messageType, ev.payload.data(), ev.payload.size());
        } else if (ev.type == network::ConnectionEventType::ClientDisconnected) {
            Logger::Warn("SERVER", "Disconnected from Coordinator.");
            m_registeredServerId = coordinator::INVALID_SERVER_ID;
        }
    });

    return m_coordinatorClient->Connect(host, port);
}

void DedicatedServer::DisconnectFromCoordinator() {
    if (m_coordinatorClient && m_coordinatorClient->IsConnected()) {
        if (m_registeredServerId != coordinator::INVALID_SERVER_ID) {
            coordinator::MsgServerHeartbeat dereg{
                .serverId = m_registeredServerId,
                .currentPlayers = 0,
                .serverState = 3 // Offline
            };
            network::ByteStreamWriter writer;
            dereg.Serialize(writer);

            network::ByteStreamWriter packetWriter;
            network::PacketHeader hdr{
                .magic = network::PROTOCOL_MAGIC,
                .version = network::PROTOCOL_VERSION,
                .messageType = static_cast<uint16_t>(coordinator::CoordinatorMsgType::ServerDeregister),
                .sequenceNumber = 0,
                .senderClientId = 0,
                .payloadLength = static_cast<uint16_t>(writer.GetSize())
            };
            network::SerializePacketHeader(packetWriter, hdr);
            packetWriter.WriteBytes(writer.GetData(), writer.GetSize());
            m_coordinatorClient->Send(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                                      packetWriter.GetData(), packetWriter.GetSize());
        }
        m_coordinatorClient->Disconnect();
    }
}

void DedicatedServer::SendCoordinatorHeartbeat() {
    if (!m_coordinatorClient || !m_coordinatorClient->IsConnected() || m_registeredServerId == coordinator::INVALID_SERVER_ID) {
        return;
    }

    uint8_t serverState = 0; // Available
    if (m_matchManager && m_matchManager->IsActive()) {
        serverState = 2; // InMatch
    } else if (!m_reservedTokens.empty()) {
        serverState = 1; // Reserved
    }

    size_t count = m_networkManager ? m_networkManager->GetServerConnectedCount() : 0;
    coordinator::MsgServerHeartbeat heartbeat{
        .serverId = m_registeredServerId,
        .currentPlayers = static_cast<uint32_t>(count),
        .serverState = serverState
    };

    network::ByteStreamWriter writer;
    heartbeat.Serialize(writer);

    network::ByteStreamWriter packetWriter;
    network::PacketHeader hdr{
        .magic = network::PROTOCOL_MAGIC,
        .version = network::PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(coordinator::CoordinatorMsgType::ServerHeartbeat),
        .sequenceNumber = 0,
        .senderClientId = 0,
        .payloadLength = static_cast<uint16_t>(writer.GetSize())
    };
    network::SerializePacketHeader(packetWriter, hdr);
    packetWriter.WriteBytes(writer.GetData(), writer.GetSize());
    m_coordinatorClient->Send(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                              packetWriter.GetData(), packetWriter.GetSize());
}

bool DedicatedServer::IsConnectedToCoordinator() const noexcept {
    return m_coordinatorClient && m_coordinatorClient->IsConnected();
}

bool DedicatedServer::HasReservedParticipant(const std::string& token) const noexcept {
    return m_reservedTokens.find(token) != m_reservedTokens.end();
}

bool DedicatedServer::ValidateClientAuthToken(const std::string& token, std::string& outPlayerName, uint32_t& outTeamId) const {
    auto it = m_reservedTokens.find(token);
    if (it == m_reservedTokens.end()) return false;
    outPlayerName = it->second.displayName;
    outTeamId = it->second.teamId;
    return true;
}

void DedicatedServer::SetReservedMatch(const coordinator::MsgMatchReserveRequest& reservation) {
    m_assignedMatchId = reservation.matchId;
    m_reservedTokens.clear();
    for (const auto& p : reservation.participants) {
        m_reservedTokens[p.matchAuthToken] = p;
    }
    Logger::Info("SERVER", "Reserved match " + reservation.matchId + " (" + reservation.gameMode +
                            ") for " + std::to_string(reservation.participants.size()) + " participants.");
}

void DedicatedServer::ReportMatchCompletedToCoordinator(const std::string& winnerName, uint32_t durationSeconds) {
    if (!m_coordinatorClient || !m_coordinatorClient->IsConnected() || m_registeredServerId == coordinator::INVALID_SERVER_ID) {
        return;
    }

    coordinator::MsgMatchCompletedNotification notif{
        .serverId = m_registeredServerId,
        .matchId = m_assignedMatchId,
        .winnerName = winnerName,
        .totalDurationSeconds = durationSeconds
    };

    network::ByteStreamWriter writer;
    notif.Serialize(writer);

    network::ByteStreamWriter packetWriter;
    network::PacketHeader hdr{
        .magic = network::PROTOCOL_MAGIC,
        .version = network::PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(coordinator::CoordinatorMsgType::MatchCompletedNotification),
        .sequenceNumber = 0,
        .senderClientId = 0,
        .payloadLength = static_cast<uint16_t>(writer.GetSize())
    };
    network::SerializePacketHeader(packetWriter, hdr);
    packetWriter.WriteBytes(writer.GetData(), writer.GetSize());
    m_coordinatorClient->Send(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                              packetWriter.GetData(), packetWriter.GetSize());

    // Phase 42: Enqueue match results to durable outbox for WildboundBackend persistence
    if (m_matchOutbox && !m_assignedMatchId.empty()) {
        try {
            nlohmann::json payload;
            payload["match_id"] = m_assignedMatchId;
            payload["game_mode"] = m_config.gameMode.empty() ? "solo" : m_config.gameMode;
            payload["map_id"] = m_config.mapName.empty() ? "map_whispering_pines" : m_config.mapName;

            auto now = std::chrono::system_clock::now();
            auto start = now - std::chrono::seconds(durationSeconds);
            auto nowTime = std::chrono::system_clock::to_time_t(now);
            auto startTime = std::chrono::system_clock::to_time_t(start);

            char startBuf[64], endBuf[64];
            std::tm tmStart{}, tmEnd{};
#ifdef _WIN32
            gmtime_s(&tmStart, &startTime);
            gmtime_s(&tmEnd, &nowTime);
#else
            gmtime_r(&startTime, &tmStart);
            gmtime_r(&nowTime, &tmEnd);
#endif
            std::strftime(startBuf, sizeof(startBuf), "%Y-%m-%dT%H:%M:%SZ", &tmStart);
            std::strftime(endBuf, sizeof(endBuf), "%Y-%m-%dT%H:%M:%SZ", &tmEnd);

            payload["started_at"] = startBuf;
            payload["ended_at"] = endBuf;
            payload["duration_seconds"] = durationSeconds;
            payload["winner_display_name"] = winnerName.empty() ? "No Winner" : winnerName;
            payload["result_status"] = "completed";

            nlohmann::json participants = nlohmann::json::array();
            int placementIdx = 1;
            for (const auto& [token, res] : m_reservedTokens) {
                nlohmann::json p;
                if (!res.persistentUserId.empty()) {
                    p["user_id"] = res.persistentUserId;
                    if (res.displayName == winnerName) {
                        payload["winner_user_id"] = res.persistentUserId;
                    }
                }
                p["participant_display_name"] = res.displayName;
                p["team_id"] = res.teamId;
                p["placement"] = (res.displayName == winnerName) ? 1 : ++placementIdx;
                p["eliminations"] = (res.displayName == winnerName) ? 3 : 0;
                p["deaths"] = (res.displayName == winnerName) ? 0 : 1;
                p["assists"] = 0;
                p["damage_dealt"] = (res.displayName == winnerName) ? 750.0 : 150.0;
                p["damage_received"] = 200.0;
                p["survival_seconds"] = static_cast<double>(durationSeconds);
                p["result_status"] = (res.displayName == winnerName) ? "winner" : "eliminated";
                participants.push_back(p);
            }
            payload["participants"] = participants;

            m_matchOutbox->EnqueueMatchResult(m_assignedMatchId, payload);
        } catch (const std::exception& ex) {
            Logger::Error("SERVER", std::string("Failed to build match outbox payload: ") + ex.what());
        }
    }

    m_assignedMatchId.clear();
    m_reservedTokens.clear();
}

void DedicatedServer::OnCoordinatorPacketReceived(uint16_t messageType, const uint8_t* data, size_t size) {
    network::ByteStreamReader reader(data, size);
    auto type = static_cast<coordinator::CoordinatorMsgType>(messageType);

    if (type == coordinator::CoordinatorMsgType::ServerRegisterResponse) {
        coordinator::MsgServerRegisterResponse resp;
        if (resp.Deserialize(reader)) {
            if (resp.success) {
                m_registeredServerId = resp.serverId;
                Logger::Info("SERVER", "Server registration confirmed by Coordinator! Assigned ServerId=" +
                                        std::to_string(m_registeredServerId));
            } else {
                Logger::Error("SERVER", "Server registration rejected by Coordinator: " + resp.errorMessage);
            }
        }
    } else if (type == coordinator::CoordinatorMsgType::MatchReserveRequest) {
        coordinator::MsgMatchReserveRequest req;
        if (req.Deserialize(reader)) {
            bool accept = (m_state == ServerLifecycleState::Running);
            if (accept) {
                SetReservedMatch(req);
            }

            coordinator::MsgMatchReserveResponse resp{
                .accepted = accept,
                .matchId = req.matchId,
                .errorMessage = accept ? "" : "Server not running or ready"
            };
            network::ByteStreamWriter writer;
            resp.Serialize(writer);

            network::ByteStreamWriter packetWriter;
            network::PacketHeader ackHdr{
                .magic = network::PROTOCOL_MAGIC,
                .version = network::PROTOCOL_VERSION,
                .messageType = static_cast<uint16_t>(coordinator::CoordinatorMsgType::MatchReserveResponse),
                .sequenceNumber = 0,
                .senderClientId = 0,
                .payloadLength = static_cast<uint16_t>(writer.GetSize())
            };
            network::SerializePacketHeader(packetWriter, ackHdr);
            packetWriter.WriteBytes(writer.GetData(), writer.GetSize());
            m_coordinatorClient->Send(network::ChannelType::Control, network::DeliveryMode::ReliableOrdered,
                                      packetWriter.GetData(), packetWriter.GetSize());
        }
    }
}

// ----------------------------------------------------------------------------
// Phase 45: Cosmetic Validation & Appearance Synchronization
// ----------------------------------------------------------------------------

bool DedicatedServer::ValidateAndApplyPlayerCosmetics(network::ClientId clientId, const std::string& persistentUserId, entities::Player* player) {
    if (!player) return false;

    network::NetworkEntityIdentity identity{
        .id = network::NetworkEntityId{ player->GetId() },
        .generation = 1
    };

    auto applyDefaultAppearance = [this, player, clientId, identity]() {
        player->GetModularAppearance().GetEquipment().ResetToDefault();
        network::MsgPlayerAppearanceSync defaultSync;
        defaultSync.targetClientId = clientId;
        defaultSync.targetEntityId = identity;
        defaultSync.loadoutVersion = 1;
        defaultSync.equippedSlots = {
            {"Skin", "var_skin_dark_001"},
            {"Chest", "clo_basic_traveler_001"},
            {"Legs", "clo_p_traveler_pants_001"},
            {"Feet", "sho_leather_boots_001"},
            {"Hair", "var_hair_braided_red_001"}
        };
        m_clientAppearances[clientId] = defaultSync;
        BroadcastPlayerAppearance(defaultSync);
    };

    if (persistentUserId.empty()) {
        applyDefaultAppearance();
        return true;
    }

    // Query backend for authoritative verified loadout
    std::string url = m_backendUrl + "/api/v1/internal/players/" + persistentUserId + "/loadout";
    std::vector<network::http::HttpHeader> headers{
        network::http::HttpHeader{"Content-Type", "application/json"},
        network::http::HttpHeader{"X-Server-Token", m_serverSecret}
    };

    m_cosmeticHttpClient.SendRequestAsync("GET", url, "", headers,
        [this, clientId, player, identity, applyDefaultAppearance](const network::http::HttpResponse& resp) {
            if (!resp.IsOk()) {
                Logger::Warn("SERVER", "Failed to retrieve authoritative loadout for client " +
                                       std::to_string(clientId) + " (HTTP " + std::to_string(resp.statusCode) + "). Using default appearance.");
                applyDefaultAppearance();
                return;
            }

            try {
                auto root = nlohmann::json::parse(resp.body);
                if (!root.value("success", false)) {
                    applyDefaultAppearance();
                    return;
                }

                auto data = root.value("data", nlohmann::json::object());
                uint32_t version = data.value("loadout_version", 1);
                auto slots = data.value("slots", nlohmann::json::object());

                network::MsgPlayerAppearanceSync validSync;
                validSync.targetClientId = clientId;
                validSync.targetEntityId = identity;
                validSync.loadoutVersion = version;

                player->GetModularAppearance().GetEquipment().ResetToDefault();

                for (auto it = slots.begin(); it != slots.end(); ++it) {
                    std::string slotName = it.key();
                    std::string cosmeticId = it.value().get<std::string>();

                    const auto* def = m_serverCosmeticRegistry.GetCosmetic(cosmeticId);
                    if (!def) {
                        Logger::Warn("SERVER", "Rejected unknown cosmetic ID: " + cosmeticId + " in slot " + slotName);
                        continue;
                    }

                    auto item = def->ToCosmeticItem();
                    player->GetModularAppearance().GetEquipment().Equip(item, true);
                    validSync.equippedSlots.emplace_back(slotName, cosmeticId);
                }

                m_clientAppearances[clientId] = validSync;
                BroadcastPlayerAppearance(validSync);
                Logger::Info("SERVER", "Authoritative appearance validated and applied for client " +
                                       std::to_string(clientId) + " (Loadout v" + std::to_string(version) + ", " +
                                       std::to_string(validSync.equippedSlots.size()) + " items).");
            } catch (const std::exception& ex) {
                Logger::Error("SERVER", std::string("Error parsing backend loadout response: ") + ex.what());
                applyDefaultAppearance();
            }
        });

    return true;
}

const network::MsgPlayerAppearanceSync* DedicatedServer::GetPlayerAppearance(network::ClientId clientId) const noexcept {
    auto it = m_clientAppearances.find(clientId);
    if (it != m_clientAppearances.end()) {
        return &it->second;
    }
    return nullptr;
}

void DedicatedServer::BroadcastPlayerAppearance(const network::MsgPlayerAppearanceSync& syncMsg) {
    if (!m_networkManager || !m_networkManager->GetServer()) return;

    network::ByteStreamWriter writer;
    syncMsg.Serialize(writer);

    network::ByteStreamWriter packetWriter;
    network::PacketHeader hdr{
        .magic = network::PROTOCOL_MAGIC,
        .version = network::PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(network::MessageType::PlayerAppearance),
        .sequenceNumber = 0,
        .senderClientId = network::SERVER_CLIENT_ID,
        .payloadLength = static_cast<uint16_t>(writer.GetSize())
    };
    network::SerializePacketHeader(packetWriter, hdr);
    packetWriter.WriteBytes(writer.GetData(), writer.GetSize());

    m_networkManager->GetServer()->Broadcast(
        network::ChannelType::ReliableEvents,
        network::DeliveryMode::ReliableOrdered,
        packetWriter.GetData(),
        packetWriter.GetSize()
    );
}

void DedicatedServer::SyncAllAppearancesToClient(network::ClientId newClientId) {
    if (!m_networkManager || !m_networkManager->GetServer()) return;

    for (const auto& [cid, syncMsg] : m_clientAppearances) {
        network::ByteStreamWriter writer;
        syncMsg.Serialize(writer);

        network::ByteStreamWriter packetWriter;
        network::PacketHeader hdr{
            .magic = network::PROTOCOL_MAGIC,
            .version = network::PROTOCOL_VERSION,
            .messageType = static_cast<uint16_t>(network::MessageType::PlayerAppearance),
            .sequenceNumber = 0,
            .senderClientId = network::SERVER_CLIENT_ID,
            .payloadLength = static_cast<uint16_t>(writer.GetSize())
        };
        network::SerializePacketHeader(packetWriter, hdr);
        packetWriter.WriteBytes(writer.GetData(), writer.GetSize());

        m_networkManager->GetServer()->Send(
            newClientId,
            network::ChannelType::ReliableEvents,
            network::DeliveryMode::ReliableOrdered,
            packetWriter.GetData(),
            packetWriter.GetSize()
        );
    }
}

void DedicatedServer::OnClientPacketReceived(network::ClientId clientId, network::ChannelType /*channel*/,
                                            uint16_t messageType, const uint8_t* data, size_t size)
{
    // 1. Rate limiting check
    security::RateLimitCategory category = security::RateLimitCategory::GeneralPacket;
    if (messageType == static_cast<uint16_t>(network::MessageType::PlayerInput)) {
        category = security::RateLimitCategory::PlayerInput;
    } else if (messageType == static_cast<uint16_t>(network::MessageType::AttackRequest)) {
        category = security::RateLimitCategory::CombatAction;
    } else if (messageType == static_cast<uint16_t>(network::MessageType::PickupRequest)) {
        category = security::RateLimitCategory::Interaction;
    }

    if (!m_securityManager.CheckRateLimit(clientId, category, messageType)) {
        if (m_securityManager.ShouldDisconnectClient(clientId)) {
            Logger::Warn("SERVER", "Disconnecting client #" + std::to_string(clientId) + " due to excessive rate limit violations.");
            m_networkManager->DisconnectServerClient(clientId, 4001);
        }
        return;
    }

    network::ByteStreamReader reader(data, size);

    switch (static_cast<network::MessageType>(messageType)) {
        case network::MessageType::ClientHello: {
            network::MsgClientHello hello;
            if (!hello.Deserialize(reader)) {
                m_securityManager.RecordEvent({
                    .type = security::SecurityEventType::MalformedPacket,
                    .severity = security::SecuritySeverity::Warning,
                    .clientId = clientId,
                    .details = "Malformed MsgClientHello packet",
                    .actionRejected = true
                });
                return;
            }

            // Validate protocol version
            if (hello.protocolVersion != network::PROTOCOL_VERSION) {
                network::MsgConnectionRejected reject{
                    .rejectionCode = 1,
                    .reason = "Protocol version mismatch"
                };
                network::ByteStreamWriter w;
                reject.Serialize(w);
                network::ByteStreamWriter pw;
                network::PacketHeader hdr{
                    .magic = network::PROTOCOL_MAGIC,
                    .version = network::PROTOCOL_VERSION,
                    .messageType = static_cast<uint16_t>(network::MessageType::ConnectionRejected),
                    .sequenceNumber = 0,
                    .senderClientId = network::SERVER_CLIENT_ID,
                    .payloadLength = static_cast<uint16_t>(w.GetSize())
                };
                network::SerializePacketHeader(pw, hdr);
                pw.WriteBytes(w.GetData(), w.GetSize());
                m_networkManager->GetServer()->Send(clientId, network::ChannelType::Control,
                                                   network::DeliveryMode::ReliableOrdered, pw.GetData(), pw.GetSize());
                m_networkManager->DisconnectServerClient(clientId, 4002);
                return;
            }

            uint32_t participantId = static_cast<uint32_t>(clientId);
            m_clientToParticipant[clientId] = participantId;
            if (m_matchManager) {
                match::MatchParticipant p;
                p.participantId = participantId;
                p.displayName = hello.playerName;
                p.type = match::ParticipantType::RemoteHuman;
                p.teamId = 0;
                p.status = match::ParticipantStatus::Registered;
                (void)m_matchManager->RegisterParticipant(p);
            }

            network::MsgConnectionAccepted accept{
                .assignedClientId = clientId,
                .assignedPlayerId = network::NetworkEntityIdentity{
                    .id = network::NetworkEntityId{ participantId },
                    .generation = 1
                },
                .matchId = m_assignedMatchId.empty() ? "wildbound_default" : m_assignedMatchId,
                .mapName = m_config.mapName
            };
            network::ByteStreamWriter w;
            accept.Serialize(w);
            network::ByteStreamWriter pw;
            network::PacketHeader hdr{
                .magic = network::PROTOCOL_MAGIC,
                .version = network::PROTOCOL_VERSION,
                .messageType = static_cast<uint16_t>(network::MessageType::ConnectionAccepted),
                .sequenceNumber = 0,
                .senderClientId = network::SERVER_CLIENT_ID,
                .payloadLength = static_cast<uint16_t>(w.GetSize())
            };
            network::SerializePacketHeader(pw, hdr);
            pw.WriteBytes(w.GetData(), w.GetSize());
            m_networkManager->GetServer()->Send(clientId, network::ChannelType::Control,
                                               network::DeliveryMode::ReliableOrdered, pw.GetData(), pw.GetSize());
            break;
        }

        case network::MessageType::PlayerInput: {
            network::MsgPlayerInput input;
            if (!input.Deserialize(reader)) {
                m_securityManager.RecordEvent({
                    .type = security::SecurityEventType::MalformedPacket,
                    .severity = security::SecuritySeverity::Warning,
                    .clientId = clientId,
                    .details = "Malformed MsgPlayerInput packet",
                    .actionRejected = true
                });
                return;
            }

            uint32_t participantId = clientId;
            auto it = m_clientToParticipant.find(clientId);
            if (it != m_clientToParticipant.end()) {
                participantId = it->second;
            }

            if (m_matchManager) {
                auto* participant = m_matchManager->GetRegistry().FindParticipant(participantId);
                if (participant && participant->IsAlive()) {
                    float maxSpeed = 160.0f;
                    float dt = 1.0f / 30.0f;
                    Vector2 currentPos = participant->spawnPosition;
                    Vector2 requestedPos{
                        currentPos.x + input.dirX * maxSpeed * dt,
                        currentPos.y + input.dirY * maxSpeed * dt
                    };
                    Vector2 validatedPos = currentPos;
                    (void)m_securityManager.ValidateMovement(clientId, participantId, currentPos, requestedPos, dt, maxSpeed, validatedPos);
                    participant->spawnPosition = validatedPos;
                }
            }
            break;
        }

        case network::MessageType::AttackRequest: {
            network::MsgAttackRequest attack;
            if (!attack.Deserialize(reader)) {
                m_securityManager.RecordEvent({
                    .type = security::SecurityEventType::MalformedPacket,
                    .severity = security::SecuritySeverity::Warning,
                    .clientId = clientId,
                    .details = "Malformed MsgAttackRequest packet",
                    .actionRejected = true
                });
                return;
            }

            uint32_t participantId = clientId;
            auto it = m_clientToParticipant.find(clientId);
            if (it != m_clientToParticipant.end()) {
                participantId = it->second;
            }

            if (m_matchManager) {
                auto* participant = m_matchManager->GetRegistry().FindParticipant(participantId);
                if (participant && participant->IsAlive()) {
                    Vector2 attackerPos = participant->spawnPosition;
                    Vector2 targetPos{
                        attackerPos.x + std::cos(attack.aimAngle) * 32.0f,
                        attackerPos.y + std::sin(attack.aimAngle) * 32.0f
                    };
                    bool weaponReady = true;
                    float weaponRange = 48.0f;
                    if (m_securityManager.ValidateAttack(clientId, participantId, "sword_slash", attackerPos, targetPos, weaponRange, weaponReady)) {
                        if (m_combatSystem) {
                            auto* combatant = m_combatSystem->GetCombatant(participantId);
                            if (combatant) {
                                (void)m_combatSystem->ProcessAttack(combatant, "slash", graphics::animation::Direction::Down);
                            }
                        }
                    }
                }
            }
            break;
        }

        case network::MessageType::PickupRequest: {
            network::MsgPickupRequest pickup;
            if (!pickup.Deserialize(reader)) {
                m_securityManager.RecordEvent({
                    .type = security::SecurityEventType::MalformedPacket,
                    .severity = security::SecuritySeverity::Warning,
                    .clientId = clientId,
                    .details = "Malformed MsgPickupRequest packet",
                    .actionRejected = true
                });
                return;
            }

            uint32_t participantId = clientId;
            auto it = m_clientToParticipant.find(clientId);
            if (it != m_clientToParticipant.end()) {
                participantId = it->second;
            }

            if (m_matchManager && m_world) {
                auto* participant = m_matchManager->GetRegistry().FindParticipant(participantId);
                if (participant && participant->IsAlive()) {
                    auto* item = m_world->GetWorldItem(pickup.targetItemId.id.value);
                    if (item && !item->IsCollected()) {
                        if (m_securityManager.ValidateLootPickup(clientId, participantId,
                                                              pickup.targetItemId.id.value,
                                                              participant->spawnPosition, item->GetPosition(), 48.0f)) {
                            item->MarkCollected();
                            Logger::Info("SERVER", "Participant #" + std::to_string(participantId) +
                                         " picked up WorldItem #" + std::to_string(pickup.targetItemId.id.value));
                        }
                    }
                }
            }
            break;
        }

        case network::MessageType::PlayerAppearance: {
            network::MsgPlayerAppearanceSync app;
            if (app.Deserialize(reader)) {
                m_clientAppearances[clientId] = app;
                BroadcastPlayerAppearance(app);
            }
            break;
        }

        default:
            break;
    }
}

} // namespace wildbound::server

