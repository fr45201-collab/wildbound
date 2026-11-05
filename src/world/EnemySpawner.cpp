#include "EnemySpawner.hpp"
#include "World.hpp"
#include "../combat/CombatSystem.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::world {

SpawnerConfig SpawnerConfig::LoadFromFile(const std::string& filepath) {
    SpawnerConfig cfg;
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Warn("SPAWNER", "Could not open spawner config: " + filepath + ". Using defaults.");
            return cfg;
        }

        nlohmann::json j;
        file >> j;

        if (j.contains("spawning")) {
            const auto& s = j["spawning"];
            cfg.enabled = s.value("enabled", true);
            cfg.maxActiveEnemies = s.value("max_active_enemies", 6);
            cfg.spawnInterval = s.value("spawn_interval", 6.0f);
            cfg.initialSpawnCount = s.value("initial_spawn_count", 3);
            cfg.minDistanceFromPlayer = s.value("min_distance_from_player", 180.0f);
            cfg.maxDistanceFromPlayer = s.value("max_distance_from_player", 450.0f);

            if (s.contains("enemy_types") && s["enemy_types"].is_array()) {
                cfg.enemyTypes.clear();
                for (const auto& item : s["enemy_types"]) {
                    EnemySpawnType et;
                    et.type = item.value("type", "goblin");
                    et.weight = item.value("weight", 50);
                    et.configFile = item.value("config_file", "data/characters/enemies/goblin.json");
                    cfg.enemyTypes.push_back(et);
                }
            }

            if (s.contains("spawn_zones") && s["spawn_zones"].is_array()) {
                cfg.spawnZones.clear();
                for (const auto& item : s["spawn_zones"]) {
                    SpawnZone sz;
                    sz.name = item.value("name", "Zone");
                    sz.x = item.value("x", 0.0f);
                    sz.y = item.value("y", 0.0f);
                    sz.radius = item.value("radius", 100.0f);
                    cfg.spawnZones.push_back(sz);
                }
            }
        }
        Logger::Info("SPAWNER", "Loaded enemy spawner config from " + filepath);
    }
    catch (const std::exception& ex) {
        Logger::Error("SPAWNER", "Failed loading spawner config JSON: " + std::string(ex.what()));
    }
    return cfg;
}

EnemySpawner::EnemySpawner()
    : m_rng(1337) // Deterministic default seed
{
}

bool EnemySpawner::Initialize(const std::string& configPath, resources::AssetManager& assetMgr) {
    m_config = SpawnerConfig::LoadFromFile(configPath);
    m_spawnTimer = 1.0f; // Initial small delay before spawning starts

    // Preload enemy spritesheets and animations into AMS
    assetMgr.LoadSpriteSheetMetadata("characters/enemies/goblin/goblin_spritesheet.json");
    assetMgr.LoadAnimationMetadata("characters/enemies/goblin/goblin_animations.json");
    assetMgr.LoadSpriteSheetMetadata("characters/enemies/skeleton/skeleton_spritesheet.json");
    assetMgr.LoadAnimationMetadata("characters/enemies/skeleton/skeleton_animations.json");

    Logger::Info("SPAWNER", "EnemySpawner initialized. Max active enemies: " +
                 std::to_string(m_config.maxActiveEnemies));
    return true;
}

size_t EnemySpawner::GetActiveEnemyCount() const {
    size_t count = 0;
    for (const auto& e : m_enemies) {
        if (e && e->IsAlive()) {
            count++;
        }
    }
    return count;
}

bool EnemySpawner::IsValidSpawnLocation(Vector2 pos, const World& world) const {
    // 1. World boundary check
    Rectangle bounds = world.GetWorldBounds();
    if (bounds.width > 0.0f && bounds.height > 0.0f) {
        if (pos.x < bounds.x + 32.0f || pos.x > bounds.x + bounds.width - 32.0f ||
            pos.y < bounds.y + 48.0f || pos.y > bounds.y + bounds.height - 32.0f) {
            return false;
        }
    }

    // 2. Collision check (feet footprint 16x12 px)
    Rectangle feetBox{ pos.x - 8.0f, pos.y - 12.0f, 16.0f, 12.0f };
    if (world.CheckCollision(feetBox)) {
        return false;
    }

    return true;
}

Vector2 EnemySpawner::FindValidSpawnPosition(const World& world, Vector2 playerPos) {
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> radDist(m_config.minDistanceFromPlayer, m_config.maxDistanceFromPlayer);

    // If spawn zones are configured, pick a random zone
    if (!m_config.spawnZones.empty()) {
        std::uniform_int_distribution<size_t> zoneDist(0, m_config.spawnZones.size() - 1);
        const auto& zone = m_config.spawnZones[zoneDist(m_rng)];
        std::uniform_real_distribution<float> offsetDist(-zone.radius, zone.radius);

        for (int attempts = 0; attempts < 15; ++attempts) {
            Vector2 candidate{ zone.x + offsetDist(m_rng), zone.y + offsetDist(m_rng) };
            if (IsValidSpawnLocation(candidate, world)) {
                return candidate;
            }
        }
    }

    // Fallback: spawn around player at safe distance
    for (int attempts = 0; attempts < 20; ++attempts) {
        float angle = angleDist(m_rng);
        float dist = radDist(m_rng);
        Vector2 candidate{ playerPos.x + std::cos(angle) * dist, playerPos.y + std::sin(angle) * dist };
        if (IsValidSpawnLocation(candidate, world)) {
            return candidate;
        }
    }

    return Vector2{ 0.0f, 0.0f };
}

std::string EnemySpawner::SelectRandomEnemyType() {
    if (m_config.enemyTypes.empty()) {
        return "goblin";
    }

    int totalWeight = 0;
    for (const auto& et : m_config.enemyTypes) {
        totalWeight += et.weight;
    }

    if (totalWeight <= 0) return m_config.enemyTypes[0].type;

    std::uniform_int_distribution<int> dist(1, totalWeight);
    int roll = dist(m_rng);
    int current = 0;

    for (const auto& et : m_config.enemyTypes) {
        current += et.weight;
        if (roll <= current) {
            return et.type;
        }
    }

    return m_config.enemyTypes[0].type;
}

entities::Enemy* EnemySpawner::SpawnEnemy(const std::string& type, Vector2 pos, combat::CombatSystem& combatSys) {
    uint32_t id = m_nextEnemyId++;
    std::string name = (type == "skeleton") ? "Skeleton Warrior" : "Forest Goblin";

    auto enemy = std::make_unique<entities::Enemy>(id, name, type);

    std::string cfgFile = "data/characters/enemies/" + type + ".json";
    entities::EnemyConfig cfg = entities::EnemyConfig::LoadFromFile(cfgFile);
    enemy->ApplyConfig(cfg);
    enemy->Spawn(pos);

    // Register into unified CombatSystem
    combatSys.RegisterCombatant(enemy.get());

    entities::Enemy* ptr = enemy.get();
    m_enemies.push_back(std::move(enemy));

    Logger::Info("SPAWNER", "Spawned [" + type + "] ID=" + std::to_string(id) +
                 " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
    return ptr;
}

void EnemySpawner::ClearAllEnemies(combat::CombatSystem& combatSys) {
    for (auto& e : m_enemies) {
        if (e) {
            combatSys.UnregisterCombatant(e->GetCombatantId());
        }
    }
    m_enemies.clear();
}

void EnemySpawner::Update(float dt, const World& world, combat::CombatSystem& combatSys,
                          const resources::AssetManager& assetMgr, scripting::LuaManager* luaMgr,
                          Vector2 playerPos)
{
    // 1. Spawning interval countdown
    if (m_config.enabled) {
        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.0f) {
            m_spawnTimer = m_config.spawnInterval;

            if (GetActiveEnemyCount() < m_config.maxActiveEnemies) {
                Vector2 spawnPos = FindValidSpawnPosition(world, playerPos);
                if (spawnPos.x > 0.0f && spawnPos.y > 0.0f) {
                    std::string type = SelectRandomEnemyType();
                    SpawnEnemy(type, spawnPos, combatSys);
                }
            }
        }
    }

    // 2. Update active enemies & remove dead/destroyed entities cleanly
    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        auto& enemy = *it;
        if (!enemy) {
            it = m_enemies.erase(it);
            continue;
        }

        enemy->UpdateWithWorld(dt, world, combatSys, assetMgr, luaMgr);

        // Deferred removal: clean up when death animation completes
        if (enemy->IsPendingDestruction()) {
            combatSys.UnregisterCombatant(enemy->GetCombatantId());
            it = m_enemies.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace wildbound::world

