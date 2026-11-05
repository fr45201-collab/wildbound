#pragma once

#include "../entities/Enemy.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <random>

namespace wildbound::world {
    class World;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::scripting {
    class LuaManager;
}

namespace wildbound::world {

struct EnemySpawnType {
    std::string type;
    int weight = 50;
    std::string configFile;
};

struct SpawnZone {
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
    float radius = 100.0f;
};

struct SpawnerConfig {
    bool enabled = true;
    size_t maxActiveEnemies = 6;
    float spawnInterval = 6.0f;
    int initialSpawnCount = 3;
    float minDistanceFromPlayer = 180.0f;
    float maxDistanceFromPlayer = 450.0f;
    std::vector<EnemySpawnType> enemyTypes;
    std::vector<SpawnZone> spawnZones;

    static SpawnerConfig LoadFromFile(const std::string& filepath);
};

class EnemySpawner {
public:
    EnemySpawner();
    ~EnemySpawner() = default;

    bool Initialize(const std::string& configPath, resources::AssetManager& assetMgr);
    void Update(float dt, const World& world, combat::CombatSystem& combatSys,
                const resources::AssetManager& assetMgr, scripting::LuaManager* luaMgr = nullptr,
                Vector2 playerPos = { 0.0f, 0.0f });

    // Explicit Spawning API
    entities::Enemy* SpawnEnemy(const std::string& type, Vector2 pos, combat::CombatSystem& combatSys);
    void ClearAllEnemies(combat::CombatSystem& combatSys);

    // Position Validation (collision and world bounds aware)
    bool IsValidSpawnLocation(Vector2 pos, const World& world) const;

    // Queries
    size_t GetActiveEnemyCount() const;
    const std::vector<std::unique_ptr<entities::Enemy>>& GetEnemies() const { return m_enemies; }
    std::vector<std::unique_ptr<entities::Enemy>>& GetEnemies() { return m_enemies; }

    bool IsEnabled() const { return m_config.enabled; }
    void SetEnabled(bool enabled) { m_config.enabled = enabled; }

    float GetSpawnTimer() const { return m_spawnTimer; }
    const SpawnerConfig& GetConfig() const { return m_config; }

    void SetRngSeed(unsigned int seed) { m_rng.seed(seed); }

private:
    Vector2 FindValidSpawnPosition(const World& world, Vector2 playerPos);
    std::string SelectRandomEnemyType();

    SpawnerConfig m_config;
    std::vector<std::unique_ptr<entities::Enemy>> m_enemies;
    float m_spawnTimer = 0.0f;
    uint32_t m_nextEnemyId = 1000;
    std::mt19937 m_rng;
};

} // namespace wildbound::world

