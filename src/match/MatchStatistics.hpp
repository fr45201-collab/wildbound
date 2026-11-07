#pragma once

#include <cstdint>
#include <string>

namespace wildbound::match {

class MatchParticipantRegistry;

/**
 * Aggregator for Match-Scoped Statistics.
 */
class MatchStatistics {
public:
    MatchStatistics() = default;
    ~MatchStatistics() = default;

    void Reset();

    void RecordDamage(uint32_t attackerId, uint32_t victimId, float amount, MatchParticipantRegistry& registry, bool isStorm = false);
    void RecordElimination(uint32_t victimId, uint32_t killerId, MatchParticipantRegistry& registry);

    float matchDuration = 0.0f;
    uint32_t totalParticipants = 0;
    uint32_t eliminatedParticipants = 0;
    uint32_t totalKills = 0;
    float totalDamageDealt = 0.0f;
    float totalDamageReceived = 0.0f;
    uint32_t winnerParticipantId = 0;
    std::string winnerDisplayName;
};

} // namespace wildbound::match

