#pragma once

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

#include "ServerConfig.hpp"
#include "ServerCommandLine.hpp"
#include "ServerLifecycle.hpp"
#include "ServerDiagnostics.hpp"
#include "ServerLoop.hpp"
#include "../platform/Platform.hpp"
#include "../coordinator/CoordinatorProtocol.hpp"
#include "../network/ENetClient.hpp"
#include "../network/NetworkMessage.hpp"
#include "../network/http/HttpClient.hpp"
#include "../customization/CosmeticRegistry.hpp"
#include "../security/ServerSecurityManager.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace wildbound::physics { class CollisionManager; }
namespace wildbound::world { class World; }
namespace wildbound::combat { class CombatSystem; }
namespace wildbound::items { class ItemManager; }
namespace wildbound::storm { class StormManager; }
namespace wildbound::match { class MatchManager; }
namespace wildbound::gamemodes { class GameModeManager; }
namespace wildbound::network { class NetworkManager; }
namespace wildbound::scripting { class LuaManager; }
namespace wildbound::entities { class Player; }

namespace wildbound::server {
class MatchResultsOutbox;
}

namespace wildbound::server {

/**
 * Standalone Headless Dedicated Server for Wildbound.
 * Runs completely decoupled from graphical, window, and audio subsystems.
 * Executes authoritative gameplay simulation, replication, and participant management.
 */
class DedicatedServer {
public:
    DedicatedServer();
    ~DedicatedServer();

    DedicatedServer(const DedicatedServer&) = delete;
    DedicatedServer& operator=(const DedicatedServer&) = delete;

    /// Parse arguments, load configuration, and initialize server subsystems
    bool Initialize(int argc = 0, char* argv[] = nullptr);

    /// Start listening on configured port and bind simulation loop hooks
    bool Start();

    /// Run the simulation loop until shutdown is signaled
    void Run();

    /// Request orderly server termination (thread-safe and signal-safe)
    void RequestShutdown(const std::string& reason = "Server Shutdown Requested");

    /// Graceful shutdown releasing all resources and disconnecting clients
    void Shutdown();

    /// Advance server tick and poll network sockets (for headless tests)
    void PollNetwork();
    void UpdateServer(float dt);

    // Queries
    [[nodiscard]] ServerLifecycleState GetState() const noexcept { return m_state; }
    [[nodiscard]] bool IsRunning() const noexcept { return m_state == ServerLifecycleState::Running; }
    [[nodiscard]] const ServerConfig& GetConfig() const noexcept { return m_config; }
    [[nodiscard]] ServerConfig& GetConfig() noexcept { return m_config; }
    [[nodiscard]] const ServerDiagnostics& GetDiagnostics() const noexcept { return m_diagnostics; }
    [[nodiscard]] ServerDiagnostics& GetDiagnostics() noexcept { return m_diagnostics; }
    [[nodiscard]] const ServerCommandLine& GetCommandLine() const noexcept { return m_cli; }

    // Coordinator Integration (Phase 41)
    bool ConnectToCoordinator(const std::string& host, uint16_t port);
    void DisconnectFromCoordinator();
    void SendCoordinatorHeartbeat();
    [[nodiscard]] bool HasReservedParticipant(const std::string& token) const noexcept;
    bool ValidateClientAuthToken(const std::string& token, std::string& outPlayerName, uint32_t& outTeamId) const;
    void SetReservedMatch(const coordinator::MsgMatchReserveRequest& reservation);
    void ReportMatchCompletedToCoordinator(const std::string& winnerName, uint32_t durationSeconds);
    [[nodiscard]] coordinator::ServerInstanceId GetCoordinatorServerId() const noexcept { return m_registeredServerId; }
    [[nodiscard]] bool IsConnectedToCoordinator() const noexcept;

    // Phase 45: Server-side cosmetic validation & appearance sync
    bool ValidateAndApplyPlayerCosmetics(network::ClientId clientId, const std::string& persistentUserId, entities::Player* player);
    [[nodiscard]] const network::MsgPlayerAppearanceSync* GetPlayerAppearance(network::ClientId clientId) const noexcept;
    void BroadcastPlayerAppearance(const network::MsgPlayerAppearanceSync& syncMsg);
    void SyncAllAppearancesToClient(network::ClientId newClientId);
    [[nodiscard]] customization::CosmeticRegistry& GetCosmeticRegistry() noexcept { return m_serverCosmeticRegistry; }

    // Subsystem access (for tests)
    [[nodiscard]] platform::IPlatform* GetPlatform() noexcept { return m_platform.get(); }
    [[nodiscard]] network::NetworkManager* GetNetworkManager() noexcept { return m_networkManager.get(); }
    [[nodiscard]] match::MatchManager* GetMatchManager() noexcept { return m_matchManager.get(); }
    [[nodiscard]] storm::StormManager* GetStormManager() noexcept { return m_stormManager.get(); }
    [[nodiscard]] world::World* GetWorld() noexcept { return m_world.get(); }
    [[nodiscard]] security::ServerSecurityManager& GetSecurityManager() noexcept { return m_securityManager; }
    [[nodiscard]] const security::ServerSecurityManager& GetSecurityManager() const noexcept { return m_securityManager; }

    /// Static singleton pointer for OS signal interception
    static DedicatedServer* s_activeInstance;

private:
    void SetupConsoleSignalHandling();
    void OnFixedUpdate(float fixedDt);
    void OnNetworkPoll();
    void OnReplicationUpdate();
    void OnDiagnosticsUpdate(float frameDt, float tickDurationMs, bool wasOverload);
    void OnCoordinatorPacketReceived(uint16_t messageType, const uint8_t* data, size_t size);
    void OnClientPacketReceived(network::ClientId clientId, network::ChannelType channel, uint16_t messageType, const uint8_t* data, size_t size);

    std::unique_ptr<platform::IPlatform> m_platform;
    ServerConfig m_config;
    ServerCommandLine m_cli;
    ServerLifecycleState m_state{ ServerLifecycleState::Uninitialized };
    ServerDiagnostics m_diagnostics;
    ServerLoop m_loop;

    std::unique_ptr<physics::CollisionManager> m_collisionManager;
    std::unique_ptr<world::World> m_world;
    std::unique_ptr<combat::CombatSystem> m_combatSystem;
    std::unique_ptr<items::ItemManager> m_itemManager;
    std::unique_ptr<storm::StormManager> m_stormManager;
    std::unique_ptr<match::MatchManager> m_matchManager;
    std::unique_ptr<gamemodes::GameModeManager> m_gameModeManager;
    std::unique_ptr<network::NetworkManager> m_networkManager;
    std::unique_ptr<scripting::LuaManager> m_luaManager;

    float m_replicationAccumulator{ 0.0f };
    float m_replicationInterval{ 1.0f / 30.0f };
    bool m_isShuttingDown{ false };

    // Coordinator Client State (Phase 41)
    std::unique_ptr<network::ENetClient> m_coordinatorClient;
    coordinator::ServerInstanceId m_registeredServerId{ coordinator::INVALID_SERVER_ID };
    std::unordered_map<std::string, coordinator::ParticipantReservation> m_reservedTokens;
    std::string m_assignedMatchId;
    float m_coordinatorHeartbeatAccumulator{ 0.0f };
    std::unique_ptr<MatchResultsOutbox> m_matchOutbox;

    // Phase 45: Cosmetic Validation & Appearance Replication
    customization::CosmeticRegistry m_serverCosmeticRegistry;
    std::unordered_map<network::ClientId, network::MsgPlayerAppearanceSync> m_clientAppearances;
    network::http::HttpClient m_cosmeticHttpClient;
    std::string m_backendUrl{ "http://127.0.0.1:8080" };
    std::string m_serverSecret{ "dev_server_secret_change_in_production" };

    // Phase 47: Security & Rate Limiting Subsystem
    security::ServerSecurityManager m_securityManager;
    std::unordered_map<network::ClientId, uint32_t> m_clientToParticipant;
};

} // namespace wildbound::server

