#include "WorldManager.hpp"
#include "../core/Logger.hpp"
#include "../graphics/Camera2D.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cmath>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::world {

WorldManager::WorldManager() = default;
WorldManager::~WorldManager() = default;

bool WorldManager::Initialize(const std::string& worldDataDir) {
    m_biomeRegions.clear();
    m_settlements.clear();
    m_settlementIndexMap.clear();
    m_pois.clear();
    m_poiIndexMap.clear();
    m_spawnRegions.clear();
    m_spawnIndexMap.clear();
    m_lootLocations.clear();
    m_primaryRoutes.clear();
    m_chokepoints.clear();
    m_landmarks.clear();

    if (!fs::exists(worldDataDir)) {
        Logger::Error("WORLD_MGR", "World data directory not found: " + worldDataDir);
        return false;
    }

    try {
        // 1. Load World Definition
        std::string worldDefPath = worldDataDir + "/battle_royale_island.json";
        if (fs::exists(worldDefPath)) {
            std::ifstream file(worldDefPath);
            nlohmann::json root;
            file >> root;

            m_worldDef.worldId = root.value("world_id", "battle_royale_island");
            m_worldDef.name = root.value("name", "Isle of Wildbound");
            m_worldDef.description = root.value("description", "");
            m_worldDef.mapPath = root.value("map_path", "maps/world/battle_royale_island.json");
            
            if (root.contains("dimensions_tiles")) {
                m_worldDef.widthTiles = root["dimensions_tiles"].value("width", 320);
                m_worldDef.heightTiles = root["dimensions_tiles"].value("height", 320);
            }
            if (root.contains("dimensions_world")) {
                m_worldDef.worldWidth = root["dimensions_world"].value("width", 10240.0f);
                m_worldDef.worldHeight = root["dimensions_world"].value("height", 10240.0f);
            }
            m_worldDef.tileSize = root.value("tile_size", 32);

            if (root.contains("playable_bounds")) {
                const auto& pb = root["playable_bounds"];
                m_worldDef.playableBounds = Rectangle{
                    pb.value("x", 960.0f),
                    pb.value("y", 960.0f),
                    pb.value("width", 8320.0f),
                    pb.value("height", 8320.0f)
                };
            }

            if (root.contains("default_spawn")) {
                m_worldDef.defaultSpawn = Vector2{
                    root["default_spawn"].value("x", 4160.0f),
                    root["default_spawn"].value("y", 5920.0f)
                };
            }

            if (root.contains("biomes") && root["biomes"].is_array()) {
                m_worldDef.biomes = root["biomes"].get<std::vector<std::string>>();
            }
            m_worldDef.settlementsCount = root.value("settlements_count", 5);
            m_worldDef.poiCount = root.value("poi_count", 14);
            m_worldDef.spawnRegionsCount = root.value("spawn_regions_count", 12);
            m_worldDef.lootMarkersCount = root.value("loot_markers_count", 45);
        }

        // 2. Load Biome Regions
        std::string biomesPath = worldDataDir + "/biome_regions.json";
        if (fs::exists(biomesPath)) {
            std::ifstream file(biomesPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("regions") && root["regions"].is_array()) {
                for (const auto& item : root["regions"]) {
                    BiomeRegion reg;
                    reg.biomeId = item.value("biome_id", "");
                    reg.name = item.value("name", "");
                    if (item.contains("bounds")) {
                        const auto& b = item["bounds"];
                        reg.bounds = Rectangle{
                            b.value("x", 0.0f),
                            b.value("y", 0.0f),
                            b.value("width", 0.0f),
                            b.value("height", 0.0f)
                        };
                    }
                    reg.tileset = item.value("tileset", "");
                    reg.audioProfile = item.value("audio_profile", "");
                    reg.settlementId = item.value("settlement_id", "");
                    m_biomeRegions.push_back(reg);
                }
            }
        }

        // 3. Load Settlements
        std::string settlementsPath = worldDataDir + "/settlements.json";
        if (fs::exists(settlementsPath)) {
            std::ifstream file(settlementsPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("settlements") && root["settlements"].is_array()) {
                for (const auto& item : root["settlements"]) {
                    SettlementDef set;
                    set.id = item.value("id", "");
                    set.name = item.value("name", "");
                    set.biome = item.value("biome", "");
                    if (item.contains("center") && item["center"].is_array() && item["center"].size() >= 2) {
                        set.center = Vector2{ item["center"][0].get<float>(), item["center"][1].get<float>() };
                    }
                    set.radius = item.value("radius", 400.0f);

                    m_settlementIndexMap[set.id] = m_settlements.size();
                    m_settlements.push_back(set);
                }
            }
        }

        // 4. Load Points of Interest
        std::string poisPath = worldDataDir + "/points_of_interest.json";
        if (fs::exists(poisPath)) {
            std::ifstream file(poisPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("points_of_interest") && root["points_of_interest"].is_array()) {
                for (const auto& item : root["points_of_interest"]) {
                    PointOfInterest poi;
                    poi.id = item.value("id", "");
                    poi.name = item.value("name", "");
                    if (item.contains("position") && item["position"].is_array() && item["position"].size() >= 2) {
                        poi.position = Vector2{ item["position"][0].get<float>(), item["position"][1].get<float>() };
                    }
                    poi.type = item.value("type", "poi");
                    poi.biome = item.value("biome", "forest");

                    m_poiIndexMap[poi.id] = m_pois.size();
                    m_pois.push_back(poi);
                }
            }
        }

        // 5. Load Spawn Regions
        std::string spawnPath = worldDataDir + "/spawn_regions.json";
        if (fs::exists(spawnPath)) {
            std::ifstream file(spawnPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("spawn_regions") && root["spawn_regions"].is_array()) {
                for (const auto& item : root["spawn_regions"]) {
                    SpawnRegionDef sr;
                    sr.regionId = item.value("region_id", "");
                    sr.biome = item.value("biome", "forest");
                    if (item.contains("center") && item["center"].is_array() && item["center"].size() >= 2) {
                        sr.center = Vector2{ item["center"][0].get<float>(), item["center"][1].get<float>() };
                    }
                    if (item.contains("spawn_points") && item["spawn_points"].is_array()) {
                        for (const auto& sp : item["spawn_points"]) {
                            if (sp.is_array() && sp.size() >= 2) {
                                sr.spawnPoints.push_back(Vector2{ sp[0].get<float>(), sp[1].get<float>() });
                            }
                        }
                    }

                    m_spawnIndexMap[sr.regionId] = m_spawnRegions.size();
                    m_spawnRegions.push_back(sr);
                }
            }
        }

        // 6. Load Loot Locations
        std::string lootPath = worldDataDir + "/loot_locations.json";
        if (fs::exists(lootPath)) {
            std::ifstream file(lootPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("loot_locations") && root["loot_locations"].is_array()) {
                for (const auto& item : root["loot_locations"]) {
                    LootLocationDef l;
                    l.id = item.value("id", "");
                    if (item.contains("position") && item["position"].is_array() && item["position"].size() >= 2) {
                        l.position = Vector2{ item["position"][0].get<float>(), item["position"][1].get<float>() };
                    }
                    l.containerType = item.value("container_type", "crate");
                    l.parentPoi = item.value("parent_poi", "");
                    m_lootLocations.push_back(l);
                }
            }
        }

        // 7. Load Traversal Routes & Chokepoints
        std::string routesPath = worldDataDir + "/traversal_routes.json";
        if (fs::exists(routesPath)) {
            std::ifstream file(routesPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("primary_routes") && root["primary_routes"].is_array()) {
                for (const auto& item : root["primary_routes"]) {
                    TraversalRoute r;
                    r.id = item.value("id", "");
                    r.name = item.value("name", "");
                    r.startPoi = item.value("start_poi", "");
                    r.endPoi = item.value("end_poi", "");
                    r.distanceUnits = item.value("distance_units", 0.0f);
                    r.type = item.value("type", "highway");
                    m_primaryRoutes.push_back(r);
                }
            }
            if (root.contains("chokepoints") && root["chokepoints"].is_array()) {
                for (const auto& item : root["chokepoints"]) {
                    Chokepoint cp;
                    cp.id = item.value("id", "");
                    cp.name = item.value("name", "");
                    if (item.contains("position") && item["position"].is_array() && item["position"].size() >= 2) {
                        cp.position = Vector2{ item["position"][0].get<float>(), item["position"][1].get<float>() };
                    }
                    cp.widthUnits = item.value("width_units", 64.0f);
                    cp.type = item.value("type", "crossing");
                    m_chokepoints.push_back(cp);
                }
            }
        }

        // 8. Load World Landmarks
        std::string landmarksPath = worldDataDir + "/world_landmarks.json";
        if (fs::exists(landmarksPath)) {
            std::ifstream file(landmarksPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("landmarks") && root["landmarks"].is_array()) {
                for (const auto& item : root["landmarks"]) {
                    WorldLandmark lm;
                    lm.id = item.value("id", "");
                    lm.name = item.value("name", "");
                    lm.type = item.value("type", "monument");
                    lm.biome = item.value("biome", "forest");
                    if (item.contains("position") && item["position"].is_array() && item["position"].size() >= 2) {
                        lm.position = Vector2{ item["position"][0].get<float>(), item["position"][1].get<float>() };
                    }
                    m_landmarks.push_back(lm);
                }
            }
        }

        m_isInitialized = true;
        Logger::Info("WORLD_MGR", "WorldManager initialized: [" + m_worldDef.name + "] with " +
                     std::to_string(m_biomeRegions.size()) + " biomes, " +
                     std::to_string(m_settlements.size()) + " settlements, " +
                     std::to_string(m_pois.size()) + " POIs, " +
                     std::to_string(m_spawnRegions.size()) + " spawn regions, " +
                     std::to_string(m_lootLocations.size()) + " loot locations.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("WORLD_MGR", "Failed loading world data: " + std::string(ex.what()));
        return false;
    }
}

const BiomeRegion* WorldManager::GetBiomeRegionAt(const Vector2& worldPos) const {
    for (const auto& reg : m_biomeRegions) {
        if (CheckCollisionPointRec(worldPos, reg.bounds)) {
            return &reg;
        }
    }
    return nullptr;
}

const SettlementDef* WorldManager::GetSettlement(const std::string& id) const {
    auto it = m_settlementIndexMap.find(id);
    if (it != m_settlementIndexMap.end() && it->second < m_settlements.size()) {
        return &m_settlements[it->second];
    }
    return nullptr;
}

const PointOfInterest* WorldManager::GetPointOfInterest(const std::string& id) const {
    auto it = m_poiIndexMap.find(id);
    if (it != m_poiIndexMap.end() && it->second < m_pois.size()) {
        return &m_pois[it->second];
    }
    return nullptr;
}

const SpawnRegionDef* WorldManager::GetSpawnRegion(const std::string& regionId) const {
    auto it = m_spawnIndexMap.find(regionId);
    if (it != m_spawnIndexMap.end() && it->second < m_spawnRegions.size()) {
        return &m_spawnRegions[it->second];
    }
    return nullptr;
}

bool WorldManager::ValidateWorldData(std::vector<std::string>& outErrors) const {
    bool valid = true;

    if (!m_isInitialized) {
        outErrors.push_back("WorldManager is not initialized.");
        return false;
    }

    if (m_worldDef.widthTiles <= 0 || m_worldDef.heightTiles <= 0) {
        outErrors.push_back("Invalid world dimensions: " + std::to_string(m_worldDef.widthTiles) + "x" + std::to_string(m_worldDef.heightTiles));
        valid = false;
    }

    if (m_biomeRegions.empty()) {
        outErrors.push_back("No biome regions loaded.");
        valid = false;
    }

    if (m_settlements.size() < 5) {
        outErrors.push_back("Expected at least 5 settlements, found: " + std::to_string(m_settlements.size()));
        valid = false;
    }

    if (m_pois.size() < 10) {
        outErrors.push_back("Expected at least 10 POIs, found: " + std::to_string(m_pois.size()));
        valid = false;
    }

    if (m_spawnRegions.empty()) {
        outErrors.push_back("No spawn regions loaded.");
        valid = false;
    }

    // Check each spawn point is within playable bounds
    for (const auto& sr : m_spawnRegions) {
        for (const auto& sp : sr.spawnPoints) {
            if (!CheckCollisionPointRec(sp, m_worldDef.playableBounds)) {
                outErrors.push_back("Spawn point (" + std::to_string(sp.x) + ", " + std::to_string(sp.y) +
                                    ") in region " + sr.regionId + " is outside playable bounds.");
                valid = false;
            }
        }
    }

    // Check each loot location is within playable bounds
    for (const auto& ll : m_lootLocations) {
        if (!CheckCollisionPointRec(ll.position, m_worldDef.playableBounds)) {
            outErrors.push_back("Loot location " + ll.id + " at (" + std::to_string(ll.position.x) + ", " +
                                std::to_string(ll.position.y) + ") is outside playable bounds.");
            valid = false;
        }
    }

    return valid;
}

void WorldManager::RenderDebugOverlays(const graphics::Camera2DSystem& /*camera*/) const {
    if (!m_isInitialized) return;

    // 1. Playable Bounds
    if (m_showPlayableBounds) {
        DrawRectangleLinesEx(m_worldDef.playableBounds, 4.0f, RED);
    }

    // 2. Biome Boundaries
    if (m_showBiomeBounds) {
        for (const auto& br : m_biomeRegions) {
            DrawRectangleLinesEx(br.bounds, 2.0f, SKYBLUE);
        }
    }

    // 3. Settlement Boundaries
    if (m_showSettlementBounds) {
        for (const auto& st : m_settlements) {
            DrawCircleLines(static_cast<int>(st.center.x), static_cast<int>(st.center.y), st.radius, GOLD);
            DrawCircle(static_cast<int>(st.center.x), static_cast<int>(st.center.y), 6.0f, GOLD);
        }
    }

    // 4. Strategic POI Markers
    if (m_showPois) {
        for (const auto& poi : m_pois) {
            DrawCircle(static_cast<int>(poi.position.x), static_cast<int>(poi.position.y), 8.0f, PURPLE);
            DrawCircleLines(static_cast<int>(poi.position.x), static_cast<int>(poi.position.y), 16.0f, MAGENTA);
        }
    }

    // 5. Spawn Regions & Markers
    if (m_showSpawnRegions) {
        for (const auto& sr : m_spawnRegions) {
            DrawCircle(static_cast<int>(sr.center.x), static_cast<int>(sr.center.y), 10.0f, GREEN);
            for (const auto& sp : sr.spawnPoints) {
                DrawCircle(static_cast<int>(sp.x), static_cast<int>(sp.y), 4.0f, LIME);
                DrawCircleLines(static_cast<int>(sp.x), static_cast<int>(sp.y), 8.0f, DARKGREEN);
            }
        }
    }

    // 6. Loot Placement Markers
    if (m_showLootMarkers) {
        for (const auto& ll : m_lootLocations) {
            DrawCircle(static_cast<int>(ll.position.x), static_cast<int>(ll.position.y), 4.0f, YELLOW);
            DrawCircleLines(static_cast<int>(ll.position.x), static_cast<int>(ll.position.y), 7.0f, ORANGE);
        }
    }

    // 7. Traversal Routes
    if (m_showRoutes) {
        for (const auto& rt : m_primaryRoutes) {
            const auto* start = GetPointOfInterest(rt.startPoi);
            const auto* end = GetPointOfInterest(rt.endPoi);
            if (start && end) {
                DrawLineEx(start->position, end->position, 3.0f, Fade(YELLOW, 0.6f));
            }
        }
    }

    // 8. Chokepoints
    if (m_showChokepoints) {
        for (const auto& cp : m_chokepoints) {
            DrawCircle(static_cast<int>(cp.position.x), static_cast<int>(cp.position.y), 12.0f, RED);
            DrawCircleLines(static_cast<int>(cp.position.x), static_cast<int>(cp.position.y), cp.widthUnits * 0.5f, MAROON);
        }
    }
}

} // namespace wildbound::world

