#pragma once

#include <string>
#include <cstdint>

namespace wildbound::coordinator {

struct CoordinatorDiagnostics {
    uint32_t activeSessions{ 0 };
    uint32_t activeParties{ 0 };
    uint32_t activeLobbies{ 0 };
    uint32_t pendingInvitations{ 0 };
    uint32_t queuedParties{ 0 };
    uint32_t registeredServers{ 0 };
    uint32_t availableServers{ 0 };
    uint32_t matchesFormed{ 0 };
    uint32_t matchesCompleted{ 0 };
    uint32_t reservationFailures{ 0 };

    float uptimeSeconds{ 0.0f };
    uint64_t totalPacketsProcessed{ 0 };

    void Reset();
    void Update(float dt);
    [[nodiscard]] std::string FormatSummary() const;
};

} // namespace wildbound::coordinator

