#pragma once

#include "CoordinatorProtocol.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

enum class ServerRegistryState : uint8_t {
    Available = 0,
    Reserved  = 1,
    InMatch   = 2,
    Offline   = 3
};

struct ServerInstance {
    ServerInstanceId serverId{ INVALID_SERVER_ID };
    network::ClientId connectionId{ network::INVALID_CLIENT_ID };
    std::string serverName;
    std::string hostAddress{ "127.0.0.1" };
    uint16_t port{ 7777 };
    uint32_t maxCapacity{ 32 };
    uint32_t currentPlayers{ 0 };
    std::string assignedMatchId;
    std::string supportedGameMode{ "solo" };
    std::string mapName{ "maps/world/battle_royale_island.json" };
    ServerRegistryState state{ ServerRegistryState::Available };
    float lastHeartbeatTime{ 0.0f };
    float registeredTime{ 0.0f };

    [[nodiscard]] bool IsAvailableFor(uint32_t participantCount) const noexcept {
        return state == ServerRegistryState::Available &&
               (currentPlayers + participantCount) <= maxCapacity;
    }
};

class DedicatedServerRegistry {
public:
    DedicatedServerRegistry() = default;
    ~DedicatedServerRegistry() = default;

    ServerInstanceId RegisterServer(network::ClientId connectionId,
                                    const std::string& serverName,
                                    const std::string& hostAddress,
                                    uint16_t port,
                                    uint32_t maxCapacity,
                                    const std::string& mapName,
                                    const std::string& supportedGameMode,
                                    float currentTime = 0.0f);

    [[nodiscard]] ServerInstance* GetServer(ServerInstanceId id) noexcept;
    [[nodiscard]] const ServerInstance* GetServer(ServerInstanceId id) const noexcept;
    [[nodiscard]] ServerInstance* GetServerByConnection(network::ClientId connectionId) noexcept;
    [[nodiscard]] const ServerInstance* GetServerByConnection(network::ClientId connectionId) const noexcept;

    bool UpdateHeartbeat(ServerInstanceId id, uint32_t currentPlayers, ServerRegistryState state, float currentTime);
    bool SetServerState(ServerInstanceId id, ServerRegistryState state);
    bool SetAssignedMatch(ServerInstanceId id, const std::string& matchId);

    [[nodiscard]] ServerInstance* FindAvailableServer(uint32_t requiredCapacity, const std::string& gameMode = "");
    void CheckHeartbeatTimeouts(float currentTime, float timeoutSeconds, std::vector<ServerInstanceId>& outTimedOut);

    bool DeregisterServer(ServerInstanceId id);
    bool DeregisterServerByConnection(network::ClientId connectionId);

    [[nodiscard]] size_t GetServerCount() const noexcept { return m_servers.size(); }
    [[nodiscard]] size_t GetAvailableServerCount() const noexcept;
    [[nodiscard]] const std::unordered_map<ServerInstanceId, ServerInstance>& GetAllServers() const noexcept { return m_servers; }

    void Clear();

private:
    ServerInstanceId GenerateServerId() noexcept;

    std::unordered_map<ServerInstanceId, ServerInstance> m_servers;
    std::unordered_map<network::ClientId, ServerInstanceId> m_connectionToServer;
    ServerInstanceId m_nextServerId{ 60001 };
};

} // namespace wildbound::coordinator

