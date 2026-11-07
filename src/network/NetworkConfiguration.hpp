#pragma once

#include <cstdint>
#include <string>

namespace wildbound::network {

struct ServerConfig {
    uint16_t bindPort{ 7777 };
    uint32_t maxClients{ 64 };
    uint16_t simulationTickRate{ 60 };
    uint16_t snapshotBroadcastRate{ 30 };
    uint32_t connectionTimeoutMs{ 15000 };
    uint32_t handshakeTimeoutMs{ 5000 };
    uint32_t reconnectGracePeriodMs{ 15000 };
    uint32_t maxPacketRatePerClient{ 80 };
};

struct ClientConfig {
    std::string serverAddress{ "127.0.0.1" };
    uint16_t serverPort{ 7777 };
    uint32_t connectionTimeoutMs{ 5000 };
    uint16_t inputSamplingRate{ 60 };
    uint32_t interpolationDelayMs{ 100 };
    bool predictionEnabled{ true };
    float reconciliationToleranceUnits{ 1.0f };
    uint32_t maxExtrapolationMs{ 150 };
    uint32_t heartbeatIntervalMs{ 1000 };
};

struct ProtocolConfig {
    uint32_t protocolMagic{ 0x57424E44 };
    uint16_t protocolVersion{ 1 };
    uint16_t maxPacketPayloadBytes{ 1200 };
    uint8_t channelCount{ 5 };
};

struct InterestConfig {
    bool enabled{ true };
    float cellWidth{ 640.0f };
    float cellHeight{ 360.0f };
    int activeCellRadius{ 1 };
    float highPriorityRadius{ 800.0f };
    float mediumPriorityRadius{ 1500.0f };
};

struct NetworkConfiguration {
    uint32_t version{ 1 };
    ServerConfig server;
    ClientConfig client;
    ProtocolConfig protocol;
    InterestConfig interest;

    [[nodiscard]] bool LoadFromFile(const std::string& filepath);
};

} // namespace wildbound::network

