#pragma once

#include "MatchmakingQueue.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace wildbound::coordinator {

class MatchmakingManager {
public:
    MatchmakingManager();
    ~MatchmakingManager() = default;

    bool EnqueueParty(const QueuedParty& party);
    bool DequeueParty(PartyId partyId);
    [[nodiscard]] bool IsPartyQueued(PartyId partyId) const noexcept;
    [[nodiscard]] const QueuedParty* GetQueuedParty(PartyId partyId) const noexcept;

    [[nodiscard]] size_t GetQueueSize(const std::string& gameMode) const noexcept;
    [[nodiscard]] size_t GetTotalQueuedParties() const noexcept { return m_partyToQueueMode.size(); }

    void SetMatchSizeRequirement(const std::string& gameMode, uint32_t minParticipants);
    [[nodiscard]] uint32_t GetMatchSizeRequirement(const std::string& gameMode) const noexcept;

    void Update(float currentTime, std::vector<FormedMatch>& outFormedMatches);

    void Clear();

private:
    std::string GenerateMatchId() noexcept;

    // Queues grouped by game mode ("solo", "duos", "squads")
    std::unordered_map<std::string, std::vector<QueuedParty>> m_queues;
    std::unordered_map<PartyId, std::string> m_partyToQueueMode;
    std::unordered_map<std::string, uint32_t> m_modeMinParticipants;

    uint64_t m_nextMatchSeq{ 50001 };
};

} // namespace wildbound::coordinator

