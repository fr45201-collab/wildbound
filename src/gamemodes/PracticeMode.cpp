#include "PracticeMode.hpp"
#include "entities/Player.hpp"
#include "combat/CombatSystem.hpp"
#include "core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::gamemodes {

PracticeMode::PracticeMode() {
    Reset();
}

void PracticeMode::Initialize(Vector2 defaultSpawnPos) {
    m_isActive = true;
    m_godMode = false;
    m_dummySpawning = true;
    m_spawnPosition = defaultSpawnPos;
    m_spawnedDummyIds.clear();
    Logger::Info("PRACTICE", "Practice Mode initialized at spawn (" + std::to_string(defaultSpawnPos.x) + ", " + std::to_string(defaultSpawnPos.y) + ").");
}

void PracticeMode::RestorePlayerHealth(entities::Player* player) {
    if (!player) return;
    auto& health = player->GetHealth();
    health.Heal(health.GetMaxHealth());
    Logger::Info("PRACTICE", "Restored player health to maximum.");
}

void PracticeMode::ResetPlayerPosition(entities::Player* player) {
    if (!player) return;
    player->SetPosition(m_spawnPosition);
    Logger::Info("PRACTICE", "Reset player position to practice spawn.");
}

void PracticeMode::SpawnCombatDummy(Vector2 pos, world::EnemySpawner* /*spawner*/, combat::CombatSystem* /*combatSys*/) {
    if (!m_dummySpawning) return;

    if (m_dummySpawner) {
        uint32_t id = m_dummySpawner(pos);
        if (id > 0) {
            m_spawnedDummyIds.push_back(id);
            Logger::Info("PRACTICE", "Spawned practice combat dummy " + std::to_string(id) + " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ").");
        }
    } else {
        uint32_t fallbackId = 9000 + static_cast<uint32_t>(m_spawnedDummyIds.size() + 1);
        m_spawnedDummyIds.push_back(fallbackId);
        Logger::Info("PRACTICE", "Spawned practice dummy (simulated " + std::to_string(fallbackId) + ") at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ").");
    }
}

void PracticeMode::ClearCombatDummies(world::EnemySpawner* /*spawner*/, combat::CombatSystem* /*combatSys*/) {
    if (m_dummyClearer) {
        m_dummyClearer();
    }
    m_spawnedDummyIds.clear();
    Logger::Info("PRACTICE", "Cleared all practice combat dummies.");
}

void PracticeMode::Reset() {
    m_isActive = false;
    m_godMode = false;
    m_dummySpawning = true;
    m_spawnPosition = { 100.0f, 100.0f };
    m_spawnedDummyIds.clear();
}

} // namespace wildbound::gamemodes
