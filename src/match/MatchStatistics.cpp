#include "MatchStatistics.hpp"
#include "MatchParticipantRegistry.hpp"

namespace wildbound::match {

void MatchStatistics::Reset() {
    matchDuration = 0.0f;
    totalParticipants = 0;
    eliminatedParticipants = 0;
    totalKills = 0;
    totalDamageDealt = 0.0f;
    totalDamageReceived = 0.0f;
    winnerParticipantId = 0;
    winnerDisplayName.clear();
}

void MatchStatistics::RecordDamage(uint32_t attackerId, uint32_t victimId, float amount, MatchParticipantRegistry& registry, bool isStorm) {
    if (amount <= 0.0f) return;

    totalDamageDealt += amount;

    if (attackerId != 0) {
        if (auto* attacker = registry.FindParticipant(attackerId)) {
            attacker->damageDealt += amount;
        }
    }

    if (victimId != 0) {
        if (auto* victim = registry.FindParticipant(victimId)) {
            victim->damageReceived += amount;
            if (isStorm) {
                victim->stormDamageReceived += amount;
            } else {
                victim->combatDamageReceived += amount;
            }
        }
    }
}

void MatchStatistics::RecordElimination(uint32_t victimId, uint32_t killerId, MatchParticipantRegistry& registry) {
    (void)victimId;
    ++eliminatedParticipants;

    if (killerId != 0) {
        ++totalKills;
        if (auto* killer = registry.FindParticipant(killerId)) {
            killer->kills += 1;
        }
    }
}

} // namespace wildbound::match

