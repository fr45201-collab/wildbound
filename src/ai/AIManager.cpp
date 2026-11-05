#include "AIManager.hpp"
#include "../world/World.hpp"
#include "../world/EnemySpawner.hpp"
#include "../combat/CombatSystem.hpp"
#include "../entities/Player.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using wildbound::core::Logger;
using json = nlohmann::json;

namespace wildbound::ai {

AIManager::AIManager() = default;

bool AIManager::Initialize(const std::string& configPath) {
    std::ifstream file(configPath);
    if (file.is_open()) {
        try {
            json j;
            file >> j;
            if (j.contains("ai_enabled")) m_config.aiEnabled = j["ai_enabled"].get<bool>();
            if (j.contains("debug_visualization")) m_config.debugVisualization = j["debug_visualization"].get<bool>();
            if (j.contains("show_nav_grid")) m_config.showNavGrid = j["show_nav_grid"].get<bool>();
            if (j.contains("show_paths")) m_config.showPaths = j["show_paths"].get<bool>();
            if (j.contains("show_fov")) m_config.showFOV = j["show_fov"].get<bool>();
            if (j.contains("show_avoidance")) m_config.showAvoidance = j["show_avoidance"].get<bool>();
            if (j.contains("max_perception_range")) m_config.maxPerceptionRange = j["max_perception_range"].get<float>();
            if (j.contains("global_decision_interval")) m_config.globalDecisionInterval = j["global_decision_interval"].get<float>();
        } catch (const std::exception& e) {
            Logger::Warn("AI_MGR", "Failed parsing " + configPath + ": " + e.what());
        }
    } else {
        Logger::Info("AI_MGR", "Using default global AI configuration");
    }

    Logger::Info("AI_MGR", "AIManager initialized successfully.");
    return true;
}

void AIManager::InitializeFromWorld(const world::World& world) {
    m_navGrid.BuildFromWorld(world);
    Logger::Info("AI_MGR", "NavigationGrid generated from active world.");
}

AIController* AIManager::RegisterEntity(uint32_t entityId, const std::string& archetype, Vector2 spawnPos) {
    CreatureAIProfile profile = CreatureAIProfile::LoadFromFile("data/ai/creatures/" + archetype + ".json");
    if (profile.archetype.empty()) {
        profile = CreatureAIProfile::CreateDefault(archetype);
    }

    auto controller = std::make_unique<AIController>();
    controller->Initialize(entityId, profile, spawnPos, &m_navGrid);

    AIController* ptr = controller.get();
    m_controllers[entityId] = std::move(controller);
    return ptr;
}

void AIManager::UnregisterEntity(uint32_t entityId) {
    m_controllers.erase(entityId);
    if (m_selectedEntityId == entityId) {
        m_selectedEntityId = 0;
    }
}

void AIManager::ClearEntities() {
    m_controllers.clear();
    m_selectedEntityId = 0;
}

AIController* AIManager::GetController(uint32_t entityId) {
    auto it = m_controllers.find(entityId);
    return (it != m_controllers.end()) ? it->second.get() : nullptr;
}

const AIController* AIManager::GetController(uint32_t entityId) const {
    auto it = m_controllers.find(entityId);
    return (it != m_controllers.end()) ? it->second.get() : nullptr;
}

void AIManager::Update(float dt,
                       const world::World& world,
                       world::EnemySpawner& spawner,
                       combat::CombatSystem& combatSys,
                       entities::Player* player,
                       audio::AudioSystem* audioSys,
                       scripting::LuaManager* luaMgr)
{
    (void)world;
    (void)luaMgr;

    if (!m_config.aiEnabled) return;

    if (m_isPaused && !m_stepSingleFrame) {
        return;
    }
    m_stepSingleFrame = false;

    // 1. Gather Target Candidates (e.g. Player)
    std::vector<TargetCandidate> candidates;
    if (player && player->IsAlive()) {
        TargetCandidate tc;
        tc.id = player->GetCombatantId();
        tc.position = player->GetPosition();
        tc.currentHealth = player->GetHealth().GetCurrentHealth();
        tc.maxHealth = player->GetHealth().GetMaxHealth();
        tc.isRecentAttacker = false;
        tc.isReachable = true;
        candidates.push_back(tc);
    }

    // 2. Gather Avoidance Entities (Enemies and Player)
    std::vector<AvoidanceEntity> avoidanceEntities;
    if (player && player->IsAlive()) {
        avoidanceEntities.push_back(AvoidanceEntity{ player->GetCombatantId(), player->GetPosition(), 18.0f });
    }

    for (const auto& enemy : spawner.GetEnemies()) {
        if (!enemy || !enemy->IsAlive()) continue;
        avoidanceEntities.push_back(AvoidanceEntity{ enemy->GetCombatantId(), enemy->GetPosition(), 16.0f });
    }

    // 3. Update all active AI Controllers and synchronize with Enemies
    for (const auto& enemy : spawner.GetEnemies()) {
        if (!enemy) continue;

        uint32_t eId = enemy->GetCombatantId();
        auto it = m_controllers.find(eId);

        if (it == m_controllers.end()) {
            // Lazy register controller for enemy
            std::string arch = enemy->GetConfig().id;
            RegisterEntity(eId, arch, enemy->GetSpawnOrigin());
            it = m_controllers.find(eId);
        }

        if (it != m_controllers.end()) {
            auto* ctrl = it->second.get();

            if (!enemy->IsAlive()) {
                if (ctrl->GetState() != AIState::Dead) {
                    ctrl->OnDeath();
                }
                continue;
            }

            ctrl->Update(dt,
                         enemy->GetPosition(),
                         enemy->GetHealth().GetCurrentHealth(),
                         enemy->GetHealth().GetMaxHealth(),
                         candidates,
                         avoidanceEntities,
                         m_navGrid,
                         combatSys,
                         enemy.get(),
                         audioSys);

            // Synchronize state and velocity back to Enemy
            enemy->SetAIState(ctrl->GetState());
            enemy->SetTargetId(ctrl->GetTargetId());
            enemy->SetVelocity(ctrl->GetDesiredVelocity());
            enemy->SetFacing(ctrl->GetDesiredFacing());
        }
    }

    // 4. Clean up controllers for destroyed enemies
    for (auto it = m_controllers.begin(); it != m_controllers.end();) {
        bool stillExists = false;
        for (const auto& enemy : spawner.GetEnemies()) {
            if (enemy && enemy->GetCombatantId() == it->first && !enemy->IsPendingDestruction()) {
                stillExists = true;
                break;
            }
        }
        if (!stillExists) {
            it = m_controllers.erase(it);
        } else {
            ++it;
        }
    }
}

void AIManager::ReloadConfigurations() {
    Logger::Info("AI_MGR", "Reloading AI configurations...");
    for (auto& [id, ctrl] : m_controllers) {
        if (ctrl) {
            CreatureAIProfile profile = CreatureAIProfile::LoadFromFile("data/ai/creatures/" + ctrl->GetProfile().archetype + ".json");
            if (!profile.archetype.empty()) {
                ctrl->Initialize(id, profile, ctrl->GetSpawnOrigin(), &m_navGrid);
            }
        }
    }
}

} // namespace wildbound::ai

