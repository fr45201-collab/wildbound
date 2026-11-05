#pragma once

#include "core/AITypes.hpp"
#include "core/AIConfiguration.hpp"
#include "core/AIController.hpp"
#include "navigation/NavigationGrid.hpp"
#include <raylib.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace wildbound::world {
    class World;
    class EnemySpawner;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::entities {
    class Player;
    class Enemy;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::scripting {
    class LuaManager;
}

namespace wildbound::ai {

struct GlobalAIConfig {
    bool aiEnabled = true;
    bool debugVisualization = false;
    bool showNavGrid = false;
    bool showPaths = true;
    bool showFOV = true;
    bool showAvoidance = false;
    float maxPerceptionRange = 350.0f;
    float globalDecisionInterval = 0.2f;
};

class AIManager {
public:
    AIManager();
    ~AIManager() = default;

    bool Initialize(const std::string& configPath = "data/ai/global_ai_config.json");
    void InitializeFromWorld(const world::World& world);

    // Entity lifecycle registration
    AIController* RegisterEntity(uint32_t entityId, const std::string& archetype, Vector2 spawnPos);
    void UnregisterEntity(uint32_t entityId);
    void ClearEntities();

    AIController* GetController(uint32_t entityId);
    const AIController* GetController(uint32_t entityId) const;

    // Simulation tick
    void Update(float dt,
                const world::World& world,
                world::EnemySpawner& spawner,
                combat::CombatSystem& combatSys,
                entities::Player* player = nullptr,
                audio::AudioSystem* audioSys = nullptr,
                scripting::LuaManager* luaMgr = nullptr);

    // Queries
    const NavigationGrid& GetNavigationGrid() const noexcept { return m_navGrid; }
    NavigationGrid& GetNavigationGrid() noexcept { return m_navGrid; }

    const GlobalAIConfig& GetConfig() const noexcept { return m_config; }
    GlobalAIConfig& GetConfig() noexcept { return m_config; }

    size_t GetActiveControllerCount() const noexcept { return m_controllers.size(); }
    const std::unordered_map<uint32_t, std::unique_ptr<AIController>>& GetControllers() const noexcept { return m_controllers; }

    // Debugging & Developer Controls
    bool IsPaused() const noexcept { return m_isPaused; }
    void SetPaused(bool paused) { m_isPaused = paused; }
    void StepSingleFrame() { m_stepSingleFrame = true; }
    void ReloadConfigurations();

    uint32_t GetSelectedEntityId() const noexcept { return m_selectedEntityId; }
    void SetSelectedEntityId(uint32_t id) { m_selectedEntityId = id; }

private:
    GlobalAIConfig m_config;
    NavigationGrid m_navGrid;
    std::unordered_map<uint32_t, std::unique_ptr<AIController>> m_controllers;

    bool m_isPaused = false;
    bool m_stepSingleFrame = false;
    uint32_t m_selectedEntityId = 0;
};

} // namespace wildbound::ai

