#include "EnemyConfig.hpp"
#include "../core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using wildbound::core::Logger;

namespace wildbound::entities {

EnemyConfig EnemyConfig::CreateDefaultGoblin() {
    EnemyConfig cfg;
    cfg.id = "goblin";
    cfg.name = "Forest Goblin";
    cfg.type = "humanoid";
    cfg.faction = combat::Faction::Monsters;
    cfg.spritesheet = "goblin_spritesheet";

    cfg.stats.walkSpeed = 65.0f;
    cfg.stats.runSpeed = 110.0f;
    cfg.stats.normalizeDiagonal = true;

    cfg.health.maxHealth = 50.0f;
    cfg.health.initialHealth = 50.0f;
    cfg.health.regenerationEnabled = false;

    cfg.collision.enabled = true;
    cfg.collision.width = 16.0f;
    cfg.collision.height = 12.0f;

    cfg.combat.defaultAttack = "goblin_club";
    cfg.combat.attackCooldown = 1.0f;
    cfg.combat.attackRange = 28.0f;

    cfg.ai.behaviorScript = "scripts/gameplay/ai/enemy_base.lua";
    cfg.ai.detectionRadius = 140.0f;
    cfg.ai.attackRadius = 32.0f;
    cfg.ai.retreatHealthPercent = 0.2f;
    cfg.ai.decisionInterval = 0.2f;
    cfg.ai.patrolRadius = 80.0f;

    return cfg;
}

EnemyConfig EnemyConfig::LoadFromFile(const std::string& filepath) {
    EnemyConfig cfg = CreateDefaultGoblin();

    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Warn("CONFIG", "Could not open enemy config file: " + filepath + ". Using default goblin.");
            return cfg;
        }

        nlohmann::json j;
        file >> j;

        cfg.id = j.value("id", "enemy");
        cfg.name = j.value("name", "Enemy");
        cfg.type = j.value("type", "monster");
        cfg.faction = combat::StringToFaction(j.value("faction", "monsters"));
        cfg.spritesheet = j.value("spritesheet", "goblin_spritesheet");

        // Stats
        if (j.contains("stats")) {
            const auto& s = j["stats"];
            cfg.stats.walkSpeed = s.value("walk_speed", 65.0f);
            cfg.stats.runSpeed = s.value("run_speed", 110.0f);
            cfg.stats.normalizeDiagonal = s.value("normalize_diagonal", true);
        }

        // Health
        if (j.contains("health")) {
            cfg.health = combat::HealthConfig::FromJson(j);
        }

        // Collision
        if (j.contains("collision")) {
            const auto& c = j["collision"];
            cfg.collision.enabled = c.value("enabled", true);
            cfg.collision.width = c.value("width", 16.0f);
            cfg.collision.height = c.value("height", 12.0f);
            cfg.collision.offsetX = c.value("offset_x", 0.0f);
            cfg.collision.offsetY = c.value("offset_y", 0.0f);
        }

        // Combat
        if (j.contains("combat")) {
            const auto& cb = j["combat"];
            cfg.combat.defaultAttack = cb.value("default_attack", "goblin_club");
            cfg.combat.attackCooldown = cb.value("attack_cooldown", 1.0f);
            cfg.combat.attackRange = cb.value("attack_range", 28.0f);
        }

        // AI
        if (j.contains("ai")) {
            const auto& a = j["ai"];
            cfg.ai.behaviorScript = a.value("behavior_script", "scripts/gameplay/ai/enemy_base.lua");
            cfg.ai.detectionRadius = a.value("detection_radius", 140.0f);
            cfg.ai.attackRadius = a.value("attack_radius", 32.0f);
            cfg.ai.retreatHealthPercent = a.value("retreat_health_percent", 0.2f);
            cfg.ai.decisionInterval = a.value("decision_interval", 0.2f);
            cfg.ai.patrolRadius = a.value("patrol_radius", 80.0f);
        }

        // Animations
        if (j.contains("animations") && j["animations"].is_object()) {
            for (auto& [key, val] : j["animations"].items()) {
                if (val.is_string()) {
                    cfg.animationClips[key] = val.get<std::string>();
                }
            }
        }

        Logger::Info("CONFIG", "Loaded enemy configuration for [" + cfg.id + "] from " + filepath);
    }
    catch (const std::exception& ex) {
        Logger::Error("CONFIG", "Error parsing enemy config JSON from " + filepath + ": " + ex.what());
    }

    return cfg;
}

} // namespace wildbound::entities

