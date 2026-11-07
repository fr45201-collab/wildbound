#pragma once

#include "CoordinatorProtocol.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

enum class AssignmentState : uint8_t {
    PendingReservation,
    Confirmed,
    Failed,
    Completed
};

struct MatchAssignment {
    std::string matchId;
    ServerInstanceId assignedServerId{ INVALID_SERVER_ID };
    std::string serverHost{ "127.0.0.1" };
    uint16_t serverPort{ 7777 };
    std::string gameMode{ "solo" };
    std::string mapName{ "maps/world/battle_royale_island.json" };
    std::vector<ParticipantReservation> participants;
    AssignmentState state{ AssignmentState::PendingReservation };
    float reservationStartTime{ 0.0f };
    float timeoutSeconds{ 5.0f };

    [[nodiscard]] bool IsTimedOut(float currentTime) const noexcept {
        return state == AssignmentState::PendingReservation &&
               (currentTime - reservationStartTime) >= timeoutSeconds;
    }

    [[nodiscard]] const ParticipantReservation* FindParticipant(SessionId sid) const noexcept {
        for (const auto& p : participants) {
            if (p.sessionId == sid) return &p;
        }
        return nullptr;
    }
};

} // namespace wildbound::coordinator

