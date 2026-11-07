#include "ServerConfig.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace wildbound::server {

ServerConfig ServerConfig::GetDefault() noexcept {
    return ServerConfig{};
}

bool ServerConfig::Validate() {
    bool modified = false;

    if (port < 1024) {
        port = 7777;
        modified = true;
    }
    if (maxClients < 1 || maxClients > 64) {
        maxClients = std::clamp(maxClients, 1u, 64u);
        modified = true;
    }
    if (tickRate < 10 || tickRate > 120) {
        tickRate = std::clamp(tickRate, 10u, 120u);
        modified = true;
    }
    if (maxFrameTimeMs < 50 || maxFrameTimeMs > 1000) {
        maxFrameTimeMs = std::clamp(maxFrameTimeMs, 50u, 1000u);
        modified = true;
    }
    if (connectionTimeoutSeconds < 5 || connectionTimeoutSeconds > 120) {
        connectionTimeoutSeconds = std::clamp(connectionTimeoutSeconds, 5u, 120u);
        modified = true;
    }
    if (maxPacketSizeBytes < 512 || maxPacketSizeBytes > 65535) {
        maxPacketSizeBytes = std::clamp(maxPacketSizeBytes, 512u, 65535u);
        modified = true;
    }
    if (channelCount < 1 || channelCount > 10) {
        channelCount = std::clamp(channelCount, static_cast<uint8_t>(1), static_cast<uint8_t>(10));
        modified = true;
    }
    if (bindAddress.empty()) {
        bindAddress = "0.0.0.0";
        modified = true;
    }
    if (mapName.empty()) {
        mapName = "data/maps/forest_01.tmj";
        modified = true;
    }
    if (gameMode.empty()) {
        gameMode = "Solo Battle Royale";
        modified = true;
    }

    return !modified;
}

bool ServerConfig::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("server")) {
            const auto& s = j["server"];
            serverName = s.value("name", serverName);
            bindAddress = s.value("host", bindAddress);
            port = s.value("port", port);
            maxClients = s.value("max_clients", maxClients);
            tickRate = s.value("tick_rate", tickRate);
            maxFrameTimeMs = s.value("max_frame_time_ms", maxFrameTimeMs);
            enableConsole = s.value("enable_console", enableConsole);
            enableFileLogging = s.value("enable_file_logging", enableFileLogging);
            logLevel = s.value("log_level", logLevel);
        }

        if (j.contains("world")) {
            const auto& w = j["world"];
            mapName = w.value("map", mapName);
            gameMode = w.value("game_mode", gameMode);
            autoStartMatch = w.value("auto_start", autoStartMatch);
        }

        if (j.contains("network")) {
            const auto& n = j["network"];
            connectionTimeoutSeconds = n.value("connection_timeout_seconds", connectionTimeoutSeconds);
            maxPacketSizeBytes = n.value("max_packet_size_bytes", maxPacketSizeBytes);
            channelCount = n.value("channel_count", channelCount);
        }

        if (j.contains("coordinator")) {
            const auto& c = j["coordinator"];
            enableCoordinator = c.value("enabled", enableCoordinator);
            coordinatorHost = c.value("host", coordinatorHost);
            coordinatorPort = c.value("port", coordinatorPort);
        }

        Validate();
        return true;
    } catch (...) {
        return false;
    }
}

bool ServerConfig::SaveToFile(const std::string& filepath) const {
    nlohmann::json j;
    j["server"] = {
        { "name", serverName },
        { "host", bindAddress },
        { "port", port },
        { "max_clients", maxClients },
        { "tick_rate", tickRate },
        { "max_frame_time_ms", maxFrameTimeMs },
        { "enable_console", enableConsole },
        { "enable_file_logging", enableFileLogging },
        { "log_level", logLevel }
    };
    j["world"] = {
        { "map", mapName },
        { "game_mode", gameMode },
        { "auto_start", autoStartMatch }
    };
    j["network"] = {
        { "connection_timeout_seconds", connectionTimeoutSeconds },
        { "max_packet_size_bytes", maxPacketSizeBytes },
        { "channel_count", channelCount }
    };
    j["coordinator"] = {
        { "enabled", enableCoordinator },
        { "host", coordinatorHost },
        { "port", coordinatorPort }
    };

    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(2);
    return true;
}

} // namespace wildbound::server

