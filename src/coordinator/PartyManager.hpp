#pragma once

#include "Party.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>

namespace wildbound::coordinator {

class PartyManager {
public:
    PartyManager() = default;
    ~PartyManager() = default;

    PartyId CreateParty(SessionId leaderSessionId, const std::string& leaderName, const std::string& gameMode = "solo");
    [[nodiscard]] Party* GetParty(PartyId id) noexcept;
    [[nodiscard]] const Party* GetParty(PartyId id) const noexcept;
    [[nodiscard]] Party* GetPartyBySession(SessionId sessionId) noexcept;
    [[nodiscard]] const Party* GetPartyBySession(SessionId sessionId) const noexcept;

    bool AddMember(PartyId partyId, SessionId sessionId, const std::string& displayName);
    bool RemoveMember(PartyId partyId, SessionId sessionId, bool& outDisbanded, SessionId& outNewLeader);
    bool SetMemberReady(PartyId partyId, SessionId sessionId, bool isReady);
    bool SetPartyGameMode(PartyId partyId, SessionId requesterSessionId, const std::string& gameMode);
    bool KickMember(PartyId partyId, SessionId leaderSessionId, SessionId targetSessionId);
    bool TransferLeader(PartyId partyId, SessionId currentLeaderSessionId, SessionId newLeaderSessionId);
    bool DisbandParty(PartyId partyId);
    bool SetPartyInQueue(PartyId partyId, bool inQueue);

    [[nodiscard]] static uint32_t GetCapacityForMode(const std::string& gameMode) noexcept;
    [[nodiscard]] size_t GetPartyCount() const noexcept { return m_parties.size(); }
    [[nodiscard]] const std::unordered_map<PartyId, Party>& GetAllParties() const noexcept { return m_parties; }

    void Clear();

private:
    PartyId GeneratePartyId() noexcept;

    std::unordered_map<PartyId, Party> m_parties;
    std::unordered_map<SessionId, PartyId> m_sessionToParty;
    PartyId m_nextPartyId{ 20001 };
};

} // namespace wildbound::coordinator

