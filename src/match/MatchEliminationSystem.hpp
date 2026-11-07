#pragma once

#include "MatchParticipant.hpp"
#include <functional>
#include <vector>

namespace wildbound::match {

class MatchParticipantRegistry;
class MatchStatistics;

/**
 * Authoritative Player Elimination Pipeline.
 * Enforces idempotent elimination processing, killer attribution,
 * statistics updates, and elimination notifications.
 */
class MatchEliminationSystem {
public:
    using EliminationCallback = std::function<void(const MatchParticipant& victim, const EliminationInfo& info)>;

    MatchEliminationSystem() = default;
    ~MatchEliminationSystem() = default;

    void SubscribeElimination(EliminationCallback cb) {
        m_callbacks.push_back(std::move(cb));
    }

    void ClearCallbacks() {
        m_callbacks.clear();
    }

    /**
     * Authoritative elimination processing.
     * @return True if elimination was successfully processed, false if victim was invalid or already eliminated.
     */
    bool ProcessElimination(uint32_t victimParticipantId,
                            EliminationCause cause,
                            uint32_t killerParticipantId,
                            float matchTime,
                            MatchParticipantRegistry& registry,
                            MatchStatistics& stats);

private:
    std::vector<EliminationCallback> m_callbacks;
};

} // namespace wildbound::match

