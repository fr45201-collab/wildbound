#pragma once

#include "MatchParticipant.hpp"
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

namespace wildbound::match {

/**
 * Authoritative registry of match competitors.
 * Ensures stable IDs, prevents duplicate registrations, and provides fast status queries.
 */
class MatchParticipantRegistry {
public:
    MatchParticipantRegistry() = default;
    ~MatchParticipantRegistry() = default;

    bool RegisterParticipant(const MatchParticipant& participant);
    bool UnregisterParticipant(uint32_t participantId);

    [[nodiscard]] MatchParticipant* FindParticipant(uint32_t participantId);
    [[nodiscard]] const MatchParticipant* FindParticipant(uint32_t participantId) const;

    [[nodiscard]] MatchParticipant* FindParticipantByEntityId(uint32_t entityId);
    [[nodiscard]] const MatchParticipant* FindParticipantByEntityId(uint32_t entityId) const;

    [[nodiscard]] std::vector<MatchParticipant>& GetAllParticipants() noexcept { return m_participants; }
    [[nodiscard]] const std::vector<MatchParticipant>& GetAllParticipants() const noexcept { return m_participants; }

    [[nodiscard]] std::vector<uint32_t> GetAliveParticipantIds() const;
    [[nodiscard]] std::vector<uint32_t> GetEliminatedParticipantIds() const;

    [[nodiscard]] size_t GetAliveCount() const;
    [[nodiscard]] size_t GetTotalCount() const noexcept { return m_participants.size(); }

    bool UpdateParticipantStatus(uint32_t participantId, ParticipantStatus newStatus);
    void ResetRegistry();

private:
    std::vector<MatchParticipant> m_participants;
    std::unordered_map<uint32_t, size_t> m_idToIndexMap;
    std::unordered_map<uint32_t, size_t> m_entityIdToIndexMap;
};

} // namespace wildbound::match

