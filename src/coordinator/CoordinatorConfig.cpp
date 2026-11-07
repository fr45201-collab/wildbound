#include "CoordinatorConfig.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace wildbound::coordinator {

CoordinatorConfig CoordinatorConfig::GetDefault() noexcept {
    return CoordinatorConfig{};
}

bool CoordinatorConfig::Validate() {
    bool valid = true;
    if (port == 0) {
        port = 7780;
        valid = false;
    }
    if (maxConnections < 2) {
        maxConnections = 2;
        valid = false;
    } else if (maxConnections > 2048) {
        maxConnections = 2048;
        valid = false;
    }

    if (tickRate < 10) {
        tickRate = 10;
        valid = false;
    } else if (tickRate > 120) {
        tickRate = 120;
        valid = false;
    }

    if (sessionTimeoutSeconds < 5.0f) {
        sessionTimeoutSeconds = 5.0f;
        valid = false;
    }
    if (serverHeartbeatTimeoutSeconds < 3.0f) {
        serverHeartbeatTimeoutSeconds = 3.0f;
        valid = false;
    }
    if (invitationTimeoutSeconds < 5.0f) {
        invitationTimeoutSeconds = 5.0f;
        valid = false;
    }
    if (matchReservationTimeoutSeconds < 1.0f) {
        matchReservationTimeoutSeconds = 1.0f;
        valid = false;
    }

    if (soloMinParticipants < 1) soloMinParticipants = 1;
    if (duosMinParticipants < 1) duosMinParticipants = 1;
    if (squadsMinParticipants < 1) squadsMinParticipants = 1;

    return valid;
}

bool CoordinatorConfig::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("coordinatorName")) coordinatorName = j["coordinatorName"].get<std::string>();
        if (j.contains("bindAddress")) bindAddress = j["bindAddress"].get<std::string>();
        if (j.contains("port")) port = j["port"].get<uint16_t>();
        if (j.contains("maxConnections")) maxConnections = j["maxConnections"].get<uint32_t>();
        if (j.contains("tickRate")) tickRate = j["tickRate"].get<uint32_t>();
        if (j.contains("logLevel")) logLevel = j["logLevel"].get<std::string>();

        if (j.contains("sessionTimeoutSeconds")) sessionTimeoutSeconds = j["sessionTimeoutSeconds"].get<float>();
        if (j.contains("serverHeartbeatTimeoutSeconds")) serverHeartbeatTimeoutSeconds = j["serverHeartbeatTimeoutSeconds"].get<float>();
        if (j.contains("invitationTimeoutSeconds")) invitationTimeoutSeconds = j["invitationTimeoutSeconds"].get<float>();
        if (j.contains("matchReservationTimeoutSeconds")) matchReservationTimeoutSeconds = j["matchReservationTimeoutSeconds"].get<float>();

        if (j.contains("soloMinParticipants")) soloMinParticipants = j["soloMinParticipants"].get<uint32_t>();
        if (j.contains("duosMinParticipants")) duosMinParticipants = j["duosMinParticipants"].get<uint32_t>();
        if (j.contains("squadsMinParticipants")) squadsMinParticipants = j["squadsMinParticipants"].get<uint32_t>();

        Validate();
        return true;
    } catch (...) {
        return false;
    }
}

bool CoordinatorConfig::SaveToFile(const std::string& filepath) const {
    try {
        nlohmann::json j;
        j["coordinatorName"] = coordinatorName;
        j["bindAddress"] = bindAddress;
        j["port"] = port;
        j["maxConnections"] = maxConnections;
        j["tickRate"] = tickRate;
        j["logLevel"] = logLevel;

        j["sessionTimeoutSeconds"] = sessionTimeoutSeconds;
        j["serverHeartbeatTimeoutSeconds"] = serverHeartbeatTimeoutSeconds;
        j["invitationTimeoutSeconds"] = invitationTimeoutSeconds;
        j["matchReservationTimeoutSeconds"] = matchReservationTimeoutSeconds;

        j["soloMinParticipants"] = soloMinParticipants;
        j["duosMinParticipants"] = duosMinParticipants;
        j["squadsMinParticipants"] = squadsMinParticipants;

        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace wildbound::coordinator

