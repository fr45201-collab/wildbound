#include "Team.hpp"
#include "match/MatchParticipantRegistry.hpp"

namespace wildbound::gamemodes {

size_t Team::GetAliveMemberCount(const match::MatchParticipantRegistry& registry) const {
    size_t alive = 0;
    for (uint32_t pid : memberParticipantIds) {
        const auto* participant = registry.FindParticipant(pid);
        if (participant && participant->IsAlive()) {
            ++alive;
        }
    }
    return alive;
}

std::vector<uint32_t> Team::GetAliveMembers(const match::MatchParticipantRegistry& registry) const {
    std::vector<uint32_t> alive;
    alive.reserve(memberParticipantIds.size());
    for (uint32_t pid : memberParticipantIds) {
        const auto* participant = registry.FindParticipant(pid);
        if (participant && participant->IsAlive()) {
            alive.push_back(pid);
        }
    }
    return alive;
}

} // namespace wildbound::gamemodes

