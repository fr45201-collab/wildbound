#pragma once

#include "CoordinatorProtocol.hpp"
#include <vector>
#include <string>
#include <algorithm>

namespace wildbound::coordinator {

struct PartyMember {
    SessionId sessionId{ INVALID_SESSION_ID };
    std::string displayName;
    bool isReady{ false };
};

struct Party {
    PartyId partyId{ INVALID_PARTY_ID };
    SessionId leaderSessionId{ INVALID_SESSION_ID };
    std::string gameMode{ "solo" };
    uint32_t maxCapacity{ 1 };
    std::vector<PartyMember> members;
    bool isInQueue{ false };

    [[nodiscard]] size_t GetMemberCount() const noexcept { return members.size(); }
    [[nodiscard]] bool IsFull() const noexcept { return members.size() >= maxCapacity; }

    [[nodiscard]] bool HasMember(SessionId sessionId) const noexcept {
        return std::any_of(members.begin(), members.end(), [sessionId](const PartyMember& m) {
            return m.sessionId == sessionId;
        });
    }

    [[nodiscard]] bool IsLeader(SessionId sessionId) const noexcept {
        return leaderSessionId == sessionId;
    }

    [[nodiscard]] bool AreAllMembersReady() const noexcept {
        if (members.empty()) return false;
        return std::all_of(members.begin(), members.end(), [](const PartyMember& m) {
            return m.isReady;
        });
    }

    [[nodiscard]] PartyMember* FindMember(SessionId sessionId) noexcept {
        for (auto& m : members) {
            if (m.sessionId == sessionId) return &m;
        }
        return nullptr;
    }

    [[nodiscard]] const PartyMember* FindMember(SessionId sessionId) const noexcept {
        for (const auto& m : members) {
            if (m.sessionId == sessionId) return &m;
        }
        return nullptr;
    }
};

} // namespace wildbound::coordinator

