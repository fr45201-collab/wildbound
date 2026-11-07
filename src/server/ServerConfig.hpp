#pragma once

#include <string>
#include <cstdint>

namespace wildbound::server {

struct ServerConfig {
    // Server section
    std::string serverName{ "Wildbound Dedicated Server" };
    std::string bindAddress{ "0.0.0.0" };
    uint16_t port{ 7777 };
    uint32_t maxClients{ 32 };
    uint32_t tickRate{ 30 };
    uint32_t maxFrameTimeMs{ 250 };
    bool enableConsole{ true };
    bool enableFileLogging{ true };
    std::string logLevel{ "info" };

    // World & Match section
    std::string mapName{ "maps/world/battle_royale_island.json" };
    std::string gameMode{ "solo" };
    bool autoStartMatch{ false };

    // Network section
    uint32_t connectionTimeoutSeconds{ 30 };
    uint32_t maxPacketSizeBytes{ 65536 };
    uint8_t channelCount{ 5 };

    // Coordinator section (Phase 41)
    bool enableCoordinator{ false };
    std::string coordinatorHost{ "127.0.0.1" };
    uint16_t coordinatorPort{ 7780 };

    /// Load server config from a JSON file
    bool LoadFromFile(const std::string& filepath);

    /// Save server config to a JSON file
    bool SaveToFile(const std::string& filepath) const;

    /// Validate values against hard safety bounds and clamp if needed
    bool Validate();

    /// Get factory defaults
    static ServerConfig GetDefault() noexcept;
};

} // namespace wildbound::server

