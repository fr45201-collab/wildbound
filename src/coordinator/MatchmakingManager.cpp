#include "MatchmakingManager.hpp"
#include <algorithm>

namespace wildbound::coordinator {

MatchmakingManager::MatchmakingManager() {
    // Defaults: min participants required to form match
    // Configurable (default to 2 for Solo/Duos and 4 for Squads for fast matching & tests)
    m_modeMinParticipants["solo"] = 2;
    m_modeMinParticipants["duos"] = 2;
    m_modeMinParticipants["squads"] = 4;
}

std::string MatchmakingManager::GenerateMatchId() noexcept {
    return "match_" + std::to_string(m_nextMatchSeq++);
}

bool MatchmakingManager::EnqueueParty(const QueuedParty& party) {
    if (party.partyId == INVALID_PARTY_ID) return false;
    if (party.members.empty()) return false;
    if (IsPartyQueued(party.partyId)) return false;

    std::string mode = party.gameMode.empty() ? "solo" : party.gameMode;
    m_queues[mode].push_back(party);
    m_partyToQueueMode[party.partyId] = mode;
    return true;
}

bool MatchmakingManager::DequeueParty(PartyId partyId) {
    auto it = m_partyToQueueMode.find(partyId);
    if (it == m_partyToQueueMode.end()) return false;

    std::string mode = it->second;
    m_partyToQueueMode.erase(it);

    auto& q = m_queues[mode];
    auto qIt = std::find_if(q.begin(), q.end(), [partyId](const QueuedParty& p) {
        return p.partyId == partyId;
    });

    if (qIt != q.end()) {
        q.erase(qIt);
        return true;
    }
    return false;
}

bool MatchmakingManager::IsPartyQueued(PartyId partyId) const noexcept {
    return m_partyToQueueMode.find(partyId) != m_partyToQueueMode.end();
}

const QueuedParty* MatchmakingManager::GetQueuedParty(PartyId partyId) const noexcept {
    auto it = m_partyToQueueMode.find(partyId);
    if (it == m_partyToQueueMode.end()) return nullptr;

    auto qIt = m_queues.find(it->second);
    if (qIt == m_queues.end()) return nullptr;

    for (const auto& p : qIt->second) {
        if (p.partyId == partyId) return &p;
    }
    return nullptr;
}

size_t MatchmakingManager::GetQueueSize(const std::string& gameMode) const noexcept {
    auto it = m_queues.find(gameMode);
    return (it != m_queues.end()) ? it->second.size() : 0;
}

void MatchmakingManager::SetMatchSizeRequirement(const std::string& gameMode, uint32_t minParticipants) {
    m_modeMinParticipants[gameMode] = minParticipants;
}

uint32_t MatchmakingManager::GetMatchSizeRequirement(const std::string& gameMode) const noexcept {
    auto it = m_modeMinParticipants.find(gameMode);
    return (it != m_modeMinParticipants.end()) ? it->second : 2;
}

void MatchmakingManager::Update(float currentTime, std::vector<FormedMatch>& outFormedMatches) {
    (void)currentTime;

    for (auto& [mode, queue] : m_queues) {
        uint32_t minNeeded = GetMatchSizeRequirement(mode);
        if (minNeeded == 0) minNeeded = 2;

        // Count available participants while preserving whole parties
        uint32_t totalAvailable = 0;
        for (const auto& party : queue) {
            totalAvailable += party.memberCount;
        }

        while (totalAvailable >= minNeeded && !queue.empty()) {
            FormedMatch match;
            match.matchId = GenerateMatchId();
            match.gameMode = mode;
            match.totalParticipants = 0;

            std::vector<size_t> consumedIndices;
            for (size_t i = 0; i < queue.size(); ++i) {
                // NEVER SPLIT PARTIES: Entire party is either included or not
                match.parties.push_back(queue[i]);
                match.totalParticipants += queue[i].memberCount;
                consumedIndices.push_back(i);

                if (match.totalParticipants >= minNeeded) {
                    break;
                }
            }

            if (match.totalParticipants >= minNeeded) {
                // Remove consumed parties from queue in reverse order
                for (auto rIt = consumedIndices.rbegin(); rIt != consumedIndices.rend(); ++rIt) {
                    PartyId pid = queue[*rIt].partyId;
                    m_partyToQueueMode.erase(pid);
                    queue.erase(queue.begin() + *rIt);
                }
                totalAvailable -= match.totalParticipants;
                outFormedMatches.push_back(std::move(match));
            } else {
                // Not enough whole parties to satisfy requirement without split
                break;
            }
        }
    }
}

void MatchmakingManager::Clear() {
    m_queues.clear();
    m_partyToQueueMode.clear();
}

} // namespace wildbound::coordinator

