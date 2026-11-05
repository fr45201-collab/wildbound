#pragma once

#include <raylib.h>
#include <vector>
#include <cstdint>

namespace wildbound::ai {

struct TargetCandidate {
    uint32_t id = 0;
    Vector2 position{ 0.0f, 0.0f };
    float currentHealth = 100.0f;
    float maxHealth = 100.0f;
    bool isRecentAttacker = false;
    bool isReachable = true;
};

struct TargetScoringWeights {
    float distanceWeight = 1.0f;
    float recentAttackerBonus = 2.0f;
    float lowHealthBonus = 0.5f;
    float unreachablePenalty = 10.0f;
};

class TargetSelector {
public:
    explicit TargetSelector(TargetScoringWeights weights = {});
    ~TargetSelector() = default;

    uint32_t SelectBestTarget(Vector2 observerPos,
                              const std::vector<TargetCandidate>& candidates,
                              float maxRange,
                              uint32_t currentTargetId = 0) const;

    float EvaluateScore(Vector2 observerPos,
                        const TargetCandidate& candidate,
                        float maxRange,
                        bool isCurrentTarget) const;

    void SetWeights(const TargetScoringWeights& weights) { m_weights = weights; }
    const TargetScoringWeights& GetWeights() const noexcept { return m_weights; }

private:
    TargetScoringWeights m_weights;
};

} // namespace wildbound::ai

