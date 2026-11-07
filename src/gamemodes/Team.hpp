#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace wildbound::match {
class MatchParticipantRegistry;
}

namespace wildbound::gamemodes {

using TeamId = uint32_t;

/**
 * Team model representing an affiliated squad or duo of participants.
 */
struct Team {
    TeamId id = 0;
    std::string name;
    std::vector<uint32_t> memberParticipantIds;
    bool eliminated = false;
    uint32_t placement = 0;
    float eliminationTimestamp = 0.0f;

    [[nodiscard]] bool IsAlive() const noexcept { return !eliminated; }
    [[nodiscard]] size_t GetMemberCount() const noexcept { return memberParticipantIds.size(); }
    [[nodiscard]] bool ContainsMember(uint32_t participantId) const noexcept {
        return std::find(memberParticipantIds.begin(), memberParticipantIds.end(), participantId) != memberParticipantIds.end();
    }

    [[nodiscard]] size_t GetAliveMemberCount(const match::MatchParticipantRegistry& registry) const;
    [[nodiscard]] std::vector<uint32_t> GetAliveMembers(const match::MatchParticipantRegistry& registry) const;
};

} // namespace wildbound::gamemodes

