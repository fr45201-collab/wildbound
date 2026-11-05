#pragma once

#include "../../entities/Enemy.hpp"
#include <string>
#include <raylib.h>

namespace wildbound::ai {

using AIState = wildbound::entities::AIState;
using wildbound::entities::AIStateToString;
using wildbound::entities::StringToAIState;

struct AIContext {
    uint32_t entityId = 0;
    Vector2 position{ 0.0f, 0.0f };
    Vector2 spawnOrigin{ 0.0f, 0.0f };
    float currentHealth = 50.0f;
    float maxHealth = 50.0f;
    float healthPercent = 1.0f;
    AIState currentState = AIState::Idle;
    uint32_t targetId = 0;
    Vector2 targetPos{ 0.0f, 0.0f };
    float targetDistance = 99999.0f;
    bool hasTarget = false;
    bool targetInLoS = false;
    bool targetInAttackRange = false;
};

} // namespace wildbound::ai
