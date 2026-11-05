#include "BiomeManager.hpp"
#include "World.hpp"
#include "../graphics/Camera2D.hpp"
#include "../entities/Player.hpp"
#include "../resources/AssetManager.hpp"
#include "environment/EnvironmentalAnimationSystem.hpp"
#include "../core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using wildbound::core::Logger;

namespace wildbound::world {

BiomeManager::BiomeManager() = default;
BiomeManager::~BiomeManager() = default;

bool BiomeManager::Initialize(const std::string& biomesDirectory) {
    m_biomes.clear();
    m_biomeIndexMap.clear();

    Logger::Info("BIOME", "Initializing BiomeManager from directory: " + biomesDirectory);

    // Standard expected biome catalog order
    const std::vector<std::string> defaultBiomeFiles = {
        "forest.json",
        "desert.json",
        "snow.json",
        "swamp.json",
        "mountain.json"
    };

    std::filesystem::path dirPath(biomesDirectory);
    if (!std::filesystem::exists(dirPath)) {
        Logger::Error("BIOME", "Biomes directory does not exist: " + biomesDirectory);
        return false;
    }

    for (const auto& filename : defaultBiomeFiles) {
        std::filesystem::path filePath = dirPath / filename;
        if (!std::filesystem::exists(filePath)) {
            Logger::Warn("BIOME", "Biome definition not found: " + filePath.string());
            continue;
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            Logger::Error("BIOME", "Failed to open biome file: " + filePath.string());
            continue;
        }

        try {
            nlohmann::json root;
            file >> root;

            BiomeConfig cfg;
            cfg.id = root.value("id", "");
            if (cfg.id.empty()) {
                Logger::Warn("BIOME", "Skipping biome with missing id in " + filePath.string());
                continue;
            }

            cfg.displayName = root.value("display_name", cfg.id);
            cfg.description = root.value("description", "");
            cfg.tileset = root.value("tileset", "");
            cfg.tilesetImage = root.value("tileset_image", "");

            // Terrain types
            if (root.contains("terrain") && root["terrain"].is_array()) {
                for (const auto& tJson : root["terrain"]) {
                    TerrainType t;
                    t.name = tJson.value("name", "");
                    t.gid = tJson.value("gid", 0);
                    t.walkable = tJson.value("walkable", true);
                    cfg.terrain.push_back(t);
                }
            }

            // Vegetation
            if (root.contains("vegetation") && root["vegetation"].is_array()) {
                for (const auto& v : root["vegetation"]) {
                    cfg.vegetation.push_back(v.get<std::string>());
                }
            }

            // Environment Objects
            if (root.contains("environment_objects") && root["environment_objects"].is_array()) {
                for (const auto& eo : root["environment_objects"]) {
                    cfg.environmentObjects.push_back(eo.get<std::string>());
                }
            }

            // Animations
            if (root.contains("animations") && root["animations"].is_array()) {
                for (const auto& a : root["animations"]) {
                    cfg.animations.push_back(a.get<std::string>());
                }
            }

            // Atmosphere
            if (root.contains("atmosphere") && root["atmosphere"].is_object()) {
                const auto& atm = root["atmosphere"];
                cfg.atmosphere.weather = atm.value("weather", "temperate");
                cfg.atmosphere.fogDensity = atm.value("fog_density", 0.35f);
                cfg.atmosphere.cloudSpeed = atm.value("cloud_speed", 15.0f);
                cfg.atmosphere.cloudLayerEnabled = atm.value("cloud_layer_enabled", true);

                if (atm.contains("ambient_tint") && atm["ambient_tint"].is_object()) {
                    const auto& tint = atm["ambient_tint"];
                    cfg.atmosphere.ambientTint.r = static_cast<unsigned char>(tint.value("r", 255));
                    cfg.atmosphere.ambientTint.g = static_cast<unsigned char>(tint.value("g", 255));
                    cfg.atmosphere.ambientTint.b = static_cast<unsigned char>(tint.value("b", 255));
                    cfg.atmosphere.ambientTint.a = static_cast<unsigned char>(tint.value("a", 255));
                }
            }

            // Ambient entities
            if (root.contains("ambient_entities") && root["ambient_entities"].is_array()) {
                for (const auto& ae : root["ambient_entities"]) {
                    cfg.ambientEntities.push_back(ae.get<std::string>());
                }
            }

            // Maps
            if (root.contains("maps") && root["maps"].is_array()) {
                for (const auto& m : root["maps"]) {
                    cfg.maps.push_back(m.get<std::string>());
                }
            }

            // Default spawn
            if (root.contains("default_spawn") && root["default_spawn"].is_object()) {
                cfg.defaultSpawn.x = root["default_spawn"].value("x", 320.0f);
                cfg.defaultSpawn.y = root["default_spawn"].value("y", 320.0f);
            }

            size_t idx = m_biomes.size();
            m_biomeIndexMap[cfg.id] = idx;
            m_biomes.push_back(std::move(cfg));

            Logger::Info("BIOME", "Loaded biome [" + m_biomes[idx].id + "] '" + m_biomes[idx].displayName + "' (" +
                         std::to_string(m_biomes[idx].maps.size()) + " maps, " +
                         std::to_string(m_biomes[idx].terrain.size()) + " terrain tiles)");
        } catch (const std::exception& e) {
            Logger::Error("BIOME", "JSON parsing error in " + filePath.string() + ": " + e.what());
        }
    }

    Logger::Info("BIOME", "Total biomes loaded: " + std::to_string(m_biomes.size()));
    return !m_biomes.empty();
}

const BiomeConfig* BiomeManager::GetActiveBiome() const {
    return GetBiome(m_activeBiomeId);
}

const BiomeConfig* BiomeManager::GetBiome(const std::string& id) const {
    auto it = m_biomeIndexMap.find(id);
    if (it != m_biomeIndexMap.end() && it->second < m_biomes.size()) {
        return &m_biomes[it->second];
    }
    return nullptr;
}

bool BiomeManager::HasBiome(const std::string& id) const {
    return m_biomeIndexMap.find(id) != m_biomeIndexMap.end();
}

bool BiomeManager::SwitchBiome(const std::string& biomeId,
                              World& world,
                              entities::Player& player,
                              graphics::Camera2DSystem& camera,
                              resources::AssetManager& assetMgr,
                              environment::EnvironmentalAnimationSystem* envAnim,
                              size_t mapIndex) {
    const BiomeConfig* biome = GetBiome(biomeId);
    if (!biome) {
        Logger::Error("BIOME", "Cannot switch to unknown biome ID: '" + biomeId + "'");
        return false;
    }

    if (biome->maps.empty()) {
        Logger::Error("BIOME", "Biome '" + biomeId + "' has no registered maps.");
        return false;
    }

    if (mapIndex >= biome->maps.size()) {
        mapIndex = 0;
    }

    const std::string& mapPath = biome->maps[mapIndex];
    Logger::Info("BIOME", "Switching to biome: '" + biome->displayName + "' (" + biomeId +
                 ") -> Loading map: " + mapPath);

    if (!world.LoadMap(mapPath, assetMgr)) {
        Logger::Error("BIOME", "Failed to load map: " + mapPath);
        return false;
    }

    m_activeBiomeId = biomeId;

    // Determine spawn position
    Vector2 spawnPos = biome->defaultSpawn;
    Vector2 mapSpawn = world.GetSpawnPosition("player_spawn");
    if (mapSpawn.x > 0.0f || mapSpawn.y > 0.0f) {
        spawnPos = mapSpawn;
    }

    // Spawn player and reposition camera
    player.Spawn(spawnPos);
    camera.SetWorldBounds(world.GetWorldBounds());
    camera.SetTarget(spawnPos);
    camera.SetFollowTarget(spawnPos);

    // Configure atmospheric and environmental animations if available
    if (envAnim) {
        envAnim->PopulateFromWorld(world, assetMgr);
        envAnim->SetFogOpacity(biome->atmosphere.fogDensity);
        envAnim->SetCloudsEnabled(biome->atmosphere.cloudLayerEnabled);
        envAnim->SetFogEnabled(biome->atmosphere.fogDensity > 0.01f);
    }

    Logger::Info("BIOME", "Biome switch complete. Player spawned at (" +
                 std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ")");
    return true;
}

} // namespace wildbound::world

