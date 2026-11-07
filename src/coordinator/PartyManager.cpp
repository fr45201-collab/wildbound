#include "PartyManager.hpp"
#include <algorithm>

namespace wildbound::coordinator {

uint32_t PartyManager::GetCapacityForMode(const std::string& gameMode) noexcept {
    if (gameMode == "solo") return 1;
    if (gameMode == "duos") return 2;
    if (gameMode == "squads") return 4;
    return 4; // default
}

PartyId PartyManager::GeneratePartyId() noexcept {
    return m_nextPartyId++;
}

PartyId PartyManager::CreateParty(SessionId leaderSessionId, const std::string& leaderName, const std::string& gameMode) {
    if (leaderSessionId == INVALID_SESSION_ID) return INVALID_PARTY_ID;

    // If player already in a party, leave it first
    auto it = m_sessionToParty.find(leaderSessionId);
    if (it != m_sessionToParty.end()) {
        bool disbanded = false;
        SessionId newLeader = INVALID_SESSION_ID;
        RemoveMember(it->second, leaderSessionId, disbanded, newLeader);
    }

    PartyId newId = GeneratePartyId();
    uint32_t cap = GetCapacityForMode(gameMode);

    Party party{
        .partyId = newId,
        .leaderSessionId = leaderSessionId,
        .gameMode = gameMode,
        .maxCapacity = cap,
        .members = { PartyMember{ leaderSessionId, leaderName, true } }, // Leader defaults to ready
        .isInQueue = false
    };

    m_parties[newId] = std::move(party);
    m_sessionToParty[leaderSessionId] = newId;
    return newId;
}

Party* PartyManager::GetParty(PartyId id) noexcept {
    auto it = m_parties.find(id);
    return (it != m_parties.end()) ? &it->second : nullptr;
}

const Party* PartyManager::GetParty(PartyId id) const noexcept {
    auto it = m_parties.find(id);
    return (it != m_parties.end()) ? &it->second : nullptr;
}

Party* PartyManager::GetPartyBySession(SessionId sessionId) noexcept {
    auto it = m_sessionToParty.find(sessionId);
    if (it == m_sessionToParty.end()) return nullptr;
    return GetParty(it->second);
}

const Party* PartyManager::GetPartyBySession(SessionId sessionId) const noexcept {
    auto it = m_sessionToParty.find(sessionId);
    if (it == m_sessionToParty.end()) return nullptr;
    return GetParty(it->second);
}

bool PartyManager::AddMember(PartyId partyId, SessionId sessionId, const std::string& displayName) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    if (party->IsFull()) return false;
    if (party->HasMember(sessionId)) return false;
    if (party->isInQueue) return false; // Cannot join queued party

    // Remove from previous party if any
    auto it = m_sessionToParty.find(sessionId);
    if (it != m_sessionToParty.end()) {
        bool disbanded = false;
        SessionId newLeader = INVALID_SESSION_ID;
        RemoveMember(it->second, sessionId, disbanded, newLeader);
    }

    party->members.push_back(PartyMember{
        .sessionId = sessionId,
        .displayName = displayName,
        .isReady = false
    });

    m_sessionToParty[sessionId] = partyId;
    return true;
}

bool PartyManager::RemoveMember(PartyId partyId, SessionId sessionId, bool& outDisbanded, SessionId& outNewLeader) {
    outDisbanded = false;
    outNewLeader = INVALID_SESSION_ID;

    auto* party = GetParty(partyId);
    if (!party) return false;

    auto it = std::find_if(party->members.begin(), party->members.end(), [sessionId](const PartyMember& m) {
        return m.sessionId == sessionId;
    });

    if (it == party->members.end()) return false;

    party->members.erase(it);
    m_sessionToParty.erase(sessionId);

    if (party->members.empty()) {
        // Disband party
        m_parties.erase(partyId);
        outDisbanded = true;
        return true;
    }

    // If leader left, assign next member as leader
    if (party->leaderSessionId == sessionId) {
        party->leaderSessionId = party->members.front().sessionId;
        party->members.front().isReady = true; // New leader is ready
        outNewLeader = party->leaderSessionId;
    }

    return true;
}

bool PartyManager::SetMemberReady(PartyId partyId, SessionId sessionId, bool isReady) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    auto* member = party->FindMember(sessionId);
    if (!member) return false;
    member->isReady = isReady;
    return true;
}

bool PartyManager::SetPartyGameMode(PartyId partyId, SessionId requesterSessionId, const std::string& gameMode) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    if (!party->IsLeader(requesterSessionId)) return false;
    if (party->isInQueue) return false;

    uint32_t newCap = GetCapacityForMode(gameMode);
    // If party has more members than new capacity, cannot change to smaller mode
    if (party->GetMemberCount() > newCap) return false;

    party->gameMode = gameMode;
    party->maxCapacity = newCap;
    return true;
}

bool PartyManager::KickMember(PartyId partyId, SessionId leaderSessionId, SessionId targetSessionId) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    if (!party->IsLeader(leaderSessionId)) return false;
    if (leaderSessionId == targetSessionId) return false;
    if (!party->HasMember(targetSessionId)) return false;

    bool disbanded = false;
    SessionId newLeader = INVALID_SESSION_ID;
    return RemoveMember(partyId, targetSessionId, disbanded, newLeader);
}

bool PartyManager::TransferLeader(PartyId partyId, SessionId currentLeaderSessionId, SessionId newLeaderSessionId) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    if (!party->IsLeader(currentLeaderSessionId)) return false;
    if (!party->HasMember(newLeaderSessionId)) return false;

    party->leaderSessionId = newLeaderSessionId;
    auto* newLead = party->FindMember(newLeaderSessionId);
    if (newLead) newLead->isReady = true;
    return true;
}

bool PartyManager::DisbandParty(PartyId partyId) {
    auto it = m_parties.find(partyId);
    if (it == m_parties.end()) return false;

    for (const auto& m : it->second.members) {
        m_sessionToParty.erase(m.sessionId);
    }
    m_parties.erase(it);
    return true;
}

bool PartyManager::SetPartyInQueue(PartyId partyId, bool inQueue) {
    auto* party = GetParty(partyId);
    if (!party) return false;
    party->isInQueue = inQueue;
    return true;
}

void PartyManager::Clear() {
    m_parties.clear();
    m_sessionToParty.clear();
}

} // namespace wildbound::coordinator

