#include "MatchParticipantRegistry.hpp"
#include <algorithm>

namespace wildbound::match {

bool MatchParticipantRegistry::RegisterParticipant(const MatchParticipant& participant) {
    if (participant.participantId == 0) {
        return false;
    }
    if (m_idToIndexMap.find(participant.participantId) != m_idToIndexMap.end()) {
        return false; // Prevent duplicate IDs
    }

    size_t index = m_participants.size();
    m_participants.push_back(participant);
    m_idToIndexMap[participant.participantId] = index;

    if (participant.entityId != 0) {
        m_entityIdToIndexMap[participant.entityId] = index;
    }

    return true;
}

bool MatchParticipantRegistry::UnregisterParticipant(uint32_t participantId) {
    auto it = m_idToIndexMap.find(participantId);
    if (it == m_idToIndexMap.end()) {
        return false;
    }

    size_t indexToRemove = it->second;
    uint32_t entityIdToRemove = m_participants[indexToRemove].entityId;

    // Swap and pop with last element
    size_t lastIndex = m_participants.size() - 1;
    if (indexToRemove != lastIndex) {
        m_participants[indexToRemove] = std::move(m_participants[lastIndex]);
        uint32_t movedId = m_participants[indexToRemove].participantId;
        uint32_t movedEntityId = m_participants[indexToRemove].entityId;
        m_idToIndexMap[movedId] = indexToRemove;
        if (movedEntityId != 0) {
            m_entityIdToIndexMap[movedEntityId] = indexToRemove;
        }
    }

    m_participants.pop_back();
    m_idToIndexMap.erase(participantId);
    if (entityIdToRemove != 0) {
        m_entityIdToIndexMap.erase(entityIdToRemove);
    }

    return true;
}

MatchParticipant* MatchParticipantRegistry::FindParticipant(uint32_t participantId) {
    auto it = m_idToIndexMap.find(participantId);
    if (it != m_idToIndexMap.end() && it->second < m_participants.size()) {
        return &m_participants[it->second];
    }
    return nullptr;
}

const MatchParticipant* MatchParticipantRegistry::FindParticipant(uint32_t participantId) const {
    auto it = m_idToIndexMap.find(participantId);
    if (it != m_idToIndexMap.end() && it->second < m_participants.size()) {
        return &m_participants[it->second];
    }
    return nullptr;
}

MatchParticipant* MatchParticipantRegistry::FindParticipantByEntityId(uint32_t entityId) {
    if (entityId == 0) return nullptr;
    auto it = m_entityIdToIndexMap.find(entityId);
    if (it != m_entityIdToIndexMap.end() && it->second < m_participants.size()) {
        return &m_participants[it->second];
    }
    return nullptr;
}

const MatchParticipant* MatchParticipantRegistry::FindParticipantByEntityId(uint32_t entityId) const {
    if (entityId == 0) return nullptr;
    auto it = m_entityIdToIndexMap.find(entityId);
    if (it != m_entityIdToIndexMap.end() && it->second < m_participants.size()) {
        return &m_participants[it->second];
    }
    return nullptr;
}

std::vector<uint32_t> MatchParticipantRegistry::GetAliveParticipantIds() const {
    std::vector<uint32_t> aliveIds;
    aliveIds.reserve(m_participants.size());
    for (const auto& p : m_participants) {
        if (p.IsAlive()) {
            aliveIds.push_back(p.participantId);
        }
    }
    return aliveIds;
}

std::vector<uint32_t> MatchParticipantRegistry::GetEliminatedParticipantIds() const {
    std::vector<uint32_t> elimIds;
    elimIds.reserve(m_participants.size());
    for (const auto& p : m_participants) {
        if (p.IsEliminated()) {
            elimIds.push_back(p.participantId);
        }
    }
    return elimIds;
}

size_t MatchParticipantRegistry::GetAliveCount() const {
    size_t count = 0;
    for (const auto& p : m_participants) {
        if (p.IsAlive()) {
            ++count;
        }
    }
    return count;
}

bool MatchParticipantRegistry::UpdateParticipantStatus(uint32_t participantId, ParticipantStatus newStatus) {
    MatchParticipant* p = FindParticipant(participantId);
    if (!p) {
        return false;
    }
    p->status = newStatus;
    return true;
}

void MatchParticipantRegistry::ResetRegistry() {
    m_participants.clear();
    m_idToIndexMap.clear();
    m_entityIdToIndexMap.clear();
}

} // namespace wildbound::match

