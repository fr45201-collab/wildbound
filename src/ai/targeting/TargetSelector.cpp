#include "TargetSelector.hpp"
#include <cmath>
#include <algorithm>

namespace wildbound::ai {

TargetSelector::TargetSelector(TargetScoringWeights weights)
    : m_weights(weights)
{
}

float TargetSelector::EvaluateScore(Vector2 observerPos,
                                    const TargetCandidate& candidate,
                                    float maxRange,
                                    bool isCurrentTarget) const
{
    float dx = candidate.position.x - observerPos.x;
    float dy = candidate.position.y - observerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > maxRange) return -1000.0f;

    // Base score from distance proximity
    float distNorm = std::clamp(dist / maxRange, 0.0f, 1.0f);
    float score = (1.0f - distNorm) * m_weights.distanceWeight * 5.0f;

    // Bonus for recent attacker
    if (candidate.isRecentAttacker) {
        score += m_weights.recentAttackerBonus * 3.0f;
    }

    // Bonus for low health target
    if (candidate.maxHealth > 0.0f) {
        float hpPct = candidate.currentHealth / candidate.maxHealth;
        score += (1.0f - hpPct) * m_weights.lowHealthBonus * 2.0f;
    }

    // Stickiness: small bonus to avoid rapid target fluttering
    if (isCurrentTarget) {
        score += 1.0f;
    }

    // Heavy penalty if target is not reachable on navmesh
    if (!candidate.isReachable) {
        score -= m_weights.unreachablePenalty;
    }

    return score;
}

uint32_t TargetSelector::SelectBestTarget(Vector2 observerPos,
                                         const std::vector<TargetCandidate>& candidates,
                                         float maxRange,
                                         uint32_t currentTargetId) const
{
    float bestScore = -999.0f;
    uint32_t bestId = 0;

    for (const auto& cand : candidates) {
        bool isCurrent = (cand.id == currentTargetId);
        float score = EvaluateScore(observerPos, cand, maxRange, isCurrent);
        if (score > bestScore) {
            bestScore = score;
            bestId = cand.id;
        }
    }

    return bestId;
}

} // namespace wildbound::ai

