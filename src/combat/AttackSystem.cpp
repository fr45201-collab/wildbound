#include "AttackSystem.hpp"
#include "../core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using wildbound::core::Logger;

namespace wildbound::combat {

void AttackSystem::RegisterAttack(const AttackDefinition& def) {
    m_attacks[def.id] = def;
}

bool AttackSystem::LoadAttacksFromJson(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("ATTACK_SYS", "Failed to open attack definition file: " + filepath);
            return false;
        }

        nlohmann::json j;
        file >> j;

        if (j.contains("attacks") && j["attacks"].is_array()) {
            for (const auto& item : j["attacks"]) {
                AttackDefinition def;
                def.id = item.value("id", "slash");
                def.name = item.value("name", "Basic Attack");
                def.damageType = StringToDamageType(item.value("damage_type", "physical"));
                def.baseDamage = item.value("base_damage", 20.0f);
                def.range = item.value("range", 32.0f);
                def.cooldown = item.value("cooldown", 0.5f);
                def.hitboxWidth = item.value("hitbox_width", 32.0f);
                def.hitboxHeight = item.value("hitbox_height", 24.0f);
                def.forwardOffset = item.value("forward_offset", 20.0f);
                def.animationPrefix = item.value("animation_prefix", "attack");

                RegisterAttack(def);
            }
            Logger::Info("ATTACK_SYS", "Loaded " + std::to_string(m_attacks.size()) + " attack definitions.");
            return true;
        }
    }
    catch (const std::exception& ex) {
        Logger::Error("ATTACK_SYS", "Error parsing attacks JSON: " + std::string(ex.what()));
    }
    return false;
}

const AttackDefinition* AttackSystem::GetAttack(const std::string& attackId) const {
    auto it = m_attacks.find(attackId);
    if (it != m_attacks.end()) {
        return &it->second;
    }
    return nullptr;
}

void AttackSystem::Update(float dt) {
    if (m_cooldownRemaining > 0.0f) {
        m_cooldownRemaining -= dt;
        if (m_cooldownRemaining <= 0.0f) {
            m_cooldownRemaining = 0.0f;
            m_activeAttackId.clear();
        }
    }
}

bool AttackSystem::CanAttack(const std::string& attackId) const {
    if (m_cooldownRemaining > 0.0f) {
        return false;
    }
    return (m_attacks.find(attackId) != m_attacks.end());
}

bool AttackSystem::ExecuteAttack(const std::string& attackId) {
    const AttackDefinition* def = GetAttack(attackId);
    if (!def) {
        return false;
    }

    if (m_cooldownRemaining > 0.0f) {
        return false;
    }

    m_cooldownRemaining = def->cooldown;
    m_lastCooldownTotal = def->cooldown;
    m_activeAttackId = attackId;
    return true;
}

} // namespace wildbound::combat

