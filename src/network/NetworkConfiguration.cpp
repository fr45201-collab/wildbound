#include "NetworkConfiguration.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace wildbound::network {

bool NetworkConfiguration::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("version")) {
            version = j["version"].get<uint32_t>();
        }

        if (j.contains("server")) {
            const auto& s = j["server"];
            server.bindPort = s.value("bind_port", server.bindPort);
            server.maxClients = s.value("max_clients", server.maxClients);
            server.simulationTickRate = s.value("simulation_tick_rate", server.simulationTickRate);
            server.snapshotBroadcastRate = s.value("snapshot_broadcast_rate", server.snapshotBroadcastRate);
            server.connectionTimeoutMs = s.value("connection_timeout_ms", server.connectionTimeoutMs);
            server.handshakeTimeoutMs = s.value("handshake_timeout_ms", server.handshakeTimeoutMs);
            server.reconnectGracePeriodMs = s.value("reconnect_grace_period_ms", server.reconnectGracePeriodMs);
            server.maxPacketRatePerClient = s.value("max_packet_rate_per_client", server.maxPacketRatePerClient);
        }

        if (j.contains("client")) {
            const auto& c = j["client"];
            client.serverAddress = c.value("server_address", client.serverAddress);
            client.serverPort = c.value("server_port", client.serverPort);
            client.connectionTimeoutMs = c.value("connection_timeout_ms", client.connectionTimeoutMs);
            client.inputSamplingRate = c.value("input_sampling_rate", client.inputSamplingRate);
            client.interpolationDelayMs = c.value("interpolation_delay_ms", client.interpolationDelayMs);
            client.predictionEnabled = c.value("prediction_enabled", client.predictionEnabled);
            client.reconciliationToleranceUnits = c.value("reconciliation_tolerance_units", client.reconciliationToleranceUnits);
            client.maxExtrapolationMs = c.value("max_extrapolation_ms", client.maxExtrapolationMs);
            client.heartbeatIntervalMs = c.value("heartbeat_interval_ms", client.heartbeatIntervalMs);
        }

        if (j.contains("protocol")) {
            const auto& p = j["protocol"];
            protocol.protocolMagic = p.value("protocol_magic", protocol.protocolMagic);
            protocol.protocolVersion = p.value("protocol_version", protocol.protocolVersion);
            protocol.maxPacketPayloadBytes = p.value("max_packet_payload_bytes", protocol.maxPacketPayloadBytes);
            protocol.channelCount = p.value("channel_count", protocol.channelCount);
        }

        if (j.contains("interest_management")) {
            const auto& im = j["interest_management"];
            interest.enabled = im.value("enabled", interest.enabled);
            interest.cellWidth = im.value("cell_width", interest.cellWidth);
            interest.cellHeight = im.value("cell_height", interest.cellHeight);
            interest.activeCellRadius = im.value("active_cell_radius", interest.activeCellRadius);
            interest.highPriorityRadius = im.value("high_priority_radius", interest.highPriorityRadius);
            interest.mediumPriorityRadius = im.value("medium_priority_radius", interest.mediumPriorityRadius);
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace wildbound::network

