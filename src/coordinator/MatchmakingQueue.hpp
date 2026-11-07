#pragma once

#include "CoordinatorProtocol.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

struct QueuedParty {
    PartyId partyId{ INVALID_PARTY_ID };
    std::string gameMode{ "solo" };
    uint32_t memberCount{ 1 };
    float queuedTime{ 0.0f };
    std::vector<SessionId> members;
};

struct FormedMatch {
    std::string matchId;
    std::string gameMode;
    std::vector<QueuedParty> parties;
    uint32_t totalParticipants{ 0 };
};

} // namespace wildbound::coordinator

