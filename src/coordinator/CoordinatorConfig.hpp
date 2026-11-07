#pragma once

#include <string>
#include <cstdint>

namespace wildbound::coordinator {

struct CoordinatorConfig {
    std::string coordinatorName{ "Wildbound Coordinator Service" };
    std::string bindAddress{ "0.0.0.0" };
    uint16_t port{ 7780 };
    uint32_t maxConnections{ 128 };
    uint32_t tickRate{ 30 };
    std::string logLevel{ "info" };

    float sessionTimeoutSeconds{ 30.0f };
    float serverHeartbeatTimeoutSeconds{ 15.0f };
    float invitationTimeoutSeconds{ 60.0f };
    float matchReservationTimeoutSeconds{ 5.0f };

    uint32_t soloMinParticipants{ 2 };
    uint32_t duosMinParticipants{ 2 };
    uint32_t squadsMinParticipants{ 4 };

    bool LoadFromFile(const std::string& filepath);
    bool SaveToFile(const std::string& filepath) const;
    bool Validate();

    static CoordinatorConfig GetDefault() noexcept;
};

} // namespace wildbound::coordinator

