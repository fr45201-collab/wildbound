#include "AIConfiguration.hpp"
#include "../../core/Logger.hpp"
#include <fstream>

using wildbound::core::Logger;
using json = nlohmann::json;

namespace wildbound::ai {

CreatureAIProfile CreatureAIProfile::LoadFromFile(const std::string& filepath) {
    CreatureAIProfile profile;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::Warn("AI_CFG", "Failed to open AI config: " + filepath + ", using defaults");
        return profile;
    }

    try {
        json j;
        file >> j;

        if (j.contains("archetype")) profile.archetype = j["archetype"].get<std::string>();
        if (j.contains("detection_radius")) profile.detectionRadius = j["detection_radius"].get<float>();
        if (j.contains("field_of_view")) profile.fieldOfView = j["field_of_view"].get<float>();
        if (j.contains("attack_radius")) profile.attackRadius = j["attack_radius"].get<float>();
        if (j.contains("preferred_distance")) profile.preferredDistance = j["preferred_distance"].get<float>();
        if (j.contains("retreat_health_percent")) profile.retreatHealthPercent = j["retreat_health_percent"].get<float>();
        if (j.contains("decision_interval")) profile.decisionInterval = j["decision_interval"].get<float>();
        if (j.contains("pathfinding_interval")) profile.pathfindingInterval = j["pathfinding_interval"].get<float>();
        if (j.contains("pursuit_duration")) profile.pursuitDuration = j["pursuit_duration"].get<float>();
        if (j.contains("investigate_duration")) profile.investigateDuration = j["investigate_duration"].get<float>();
        if (j.contains("aggression_level")) profile.aggressionLevel = j["aggression_level"].get<float>();
        if (j.contains("can_retreat")) profile.canRetreat = j["can_retreat"].get<bool>();
        if (j.contains("omnidirectional_when_alert")) profile.omnidirectionalWhenAlert = j["omnidirectional_when_alert"].get<bool>();

        if (j.contains("patrol")) {
            const auto& pj = j["patrol"];
            if (pj.contains("id")) profile.defaultPatrol.id = pj["id"].get<std::string>();
            if (pj.contains("is_loop")) profile.defaultPatrol.isLoop = pj["is_loop"].get<bool>();
            if (pj.contains("points") && pj["points"].is_array()) {
                for (const auto& pt : pj["points"]) {
                    PatrolPoint p;
                    p.position = Vector2{ pt.value("x", 0.0f), pt.value("y", 0.0f) };
                    p.waitDuration = pt.value("wait_duration", 2.0f);
                    profile.defaultPatrol.points.push_back(p);
                }
            }
        }
    }
    catch (const std::exception& e) {
        Logger::Error("AI_CFG", "Error parsing AI config " + filepath + ": " + e.what());
    }

    return profile;
}

CreatureAIProfile CreatureAIProfile::CreateDefault(const std::string& archetype) {
    CreatureAIProfile profile;
    profile.archetype = archetype;

    if (archetype == "goblin") {
        // Opportunistic melee, short pursuit tolerance, retreats on low HP
        profile.detectionRadius = 140.0f;
        profile.fieldOfView = 120.0f;
        profile.attackRadius = 32.0f;
        profile.preferredDistance = 26.0f;
        profile.retreatHealthPercent = 0.2f;
        profile.decisionInterval = 0.2f;
        profile.pathfindingInterval = 0.35f;
        profile.pursuitDuration = 5.0f;
        profile.investigateDuration = 3.0f;
        profile.aggressionLevel = 1.0f;
        profile.canRetreat = true;
    } else if (archetype == "skeleton") {
        // Relentless melee, maintains pressure, never retreats
        profile.detectionRadius = 150.0f;
        profile.fieldOfView = 140.0f;
        profile.attackRadius = 34.0f;
        profile.preferredDistance = 28.0f;
        profile.retreatHealthPercent = 0.0f;
        profile.decisionInterval = 0.2f;
        profile.pathfindingInterval = 0.4f;
        profile.pursuitDuration = 8.0f;
        profile.investigateDuration = 4.0f;
        profile.aggressionLevel = 1.2f;
        profile.canRetreat = false;
    } else if (archetype == "wolf") {
        // Fast predatory beast, leaping chase, circles prey
        profile.detectionRadius = 180.0f;
        profile.fieldOfView = 160.0f;
        profile.attackRadius = 36.0f;
        profile.preferredDistance = 30.0f;
        profile.retreatHealthPercent = 0.15f;
        profile.decisionInterval = 0.15f;
        profile.pathfindingInterval = 0.25f;
        profile.pursuitDuration = 7.0f;
        profile.investigateDuration = 4.0f;
        profile.aggressionLevel = 1.3f;
        profile.canRetreat = true;
    } else if (archetype == "wild_boar") {
        // Territorial beast, alert warning, heavy charge
        profile.detectionRadius = 130.0f;
        profile.fieldOfView = 110.0f;
        profile.attackRadius = 34.0f;
        profile.preferredDistance = 30.0f;
        profile.retreatHealthPercent = 0.1f;
        profile.decisionInterval = 0.2f;
        profile.pathfindingInterval = 0.35f;
        profile.pursuitDuration = 4.0f;
        profile.investigateDuration = 2.5f;
        profile.aggressionLevel = 0.9f;
        profile.canRetreat = true;
    } else if (archetype == "bat") {
        // Evasive aerial-like flyer, erratic fluttering, hit-and-run
        profile.detectionRadius = 160.0f;
        profile.fieldOfView = 220.0f;
        profile.attackRadius = 28.0f;
        profile.preferredDistance = 36.0f;
        profile.retreatHealthPercent = 0.3f;
        profile.decisionInterval = 0.15f;
        profile.pathfindingInterval = 0.3f;
        profile.pursuitDuration = 4.0f;
        profile.investigateDuration = 2.0f;
        profile.aggressionLevel = 0.8f;
        profile.canRetreat = true;
    } else if (archetype == "small_bird" || archetype == "butterfly") {
        // Non-combat ambient wildlife: roaming and fleeing from proximity
        profile.detectionRadius = 60.0f;
        profile.fieldOfView = 360.0f;
        profile.attackRadius = 0.0f;
        profile.preferredDistance = 60.0f;
        profile.retreatHealthPercent = 1.0f; // Always flee
        profile.decisionInterval = 0.3f;
        profile.pathfindingInterval = 0.5f;
        profile.pursuitDuration = 0.0f;
        profile.investigateDuration = 0.0f;
        profile.aggressionLevel = 0.0f;
        profile.canRetreat = true;
    }

    return profile;
}

} // namespace wildbound::ai

