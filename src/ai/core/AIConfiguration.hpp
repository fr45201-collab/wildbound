#pragma once

#include "AITypes.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::ai {

struct PatrolPoint {
    Vector2 position{ 0.0f, 0.0f };
    float waitDuration = 2.0f;
};

struct PatrolRoute {
    std::string id = "patrol_default";
    std::vector<PatrolPoint> points;
    bool isLoop = true;
};

struct CreatureAIProfile {
    std::string archetype = "goblin";
    float detectionRadius = 140.0f;
    float fieldOfView = 120.0f;
    float attackRadius = 32.0f;
    float preferredDistance = 28.0f;
    float retreatHealthPercent = 0.2f;
    float decisionInterval = 0.2f;
    float pathfindingInterval = 0.4f;
    float pursuitDuration = 6.0f;
    float investigateDuration = 3.0f;
    float aggressionLevel = 1.0f;
    bool canRetreat = true;
    bool omnidirectionalWhenAlert = true;
    PatrolRoute defaultPatrol;

    static CreatureAIProfile LoadFromFile(const std::string& filepath);
    static CreatureAIProfile CreateDefault(const std::string& archetype);
};

} // namespace wildbound::ai

