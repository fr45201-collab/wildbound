#pragma once

#include "CharacterStats.hpp"
#include "CharacterConfig.hpp"
#include "../combat/HealthComponent.hpp"
#include "../combat/CombatTypes.hpp"
#include <string>
#include <unordered_map>

namespace wildbound::entities {

struct EnemyCombatConfig {
    std::string defaultAttack = "slash";
    float attackCooldown = 1.0f;
    float attackRange = 32.0f;
};

struct EnemyAIConfig {
    std::string behaviorScript = "scripts/gameplay/ai/enemy_base.lua";
    float detectionRadius = 150.0f;
    float attackRadius = 32.0f;
    float retreatHealthPercent = 0.2f;
    float decisionInterval = 0.2f;
    float patrolRadius = 80.0f;
};

struct EnemyConfig {
    std::string id = "enemy";
    std::string name = "Enemy";
    std::string type = "monster";
    combat::Faction faction = combat::Faction::Monsters;
    std::string spritesheet = "goblin_spritesheet";

    CharacterStats stats;
    combat::HealthConfig health;
    CharacterCollisionConfig collision;
    EnemyCombatConfig combat;
    EnemyAIConfig ai;

    std::unordered_map<std::string, std::string> animationClips;

    static EnemyConfig LoadFromFile(const std::string& filepath);
    static EnemyConfig CreateDefaultGoblin();
};

} // namespace wildbound::entities

