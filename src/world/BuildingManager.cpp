#include "BuildingManager.hpp"
#include "World.hpp"
#include "../entities/Player.hpp"
#include "../graphics/Camera2D.hpp"
#include "../resources/AssetManager.hpp"
#include "../core/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cmath>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::world {

BuildingManager::BuildingManager() = default;
BuildingManager::~BuildingManager() = default;

bool BuildingManager::Initialize(const std::string& buildingCatalogPath, const std::string& defaultExteriorMap) {
    m_buildings.clear();
    m_isIndoors = false;
    m_currentInteriorId.clear();
    m_exteriorMapPath = defaultExteriorMap;

    if (!fs::exists(buildingCatalogPath)) {
        Logger::Error("BUILDING", "Catalog file not found: " + buildingCatalogPath);
        return false;
    }

    try {
        std::ifstream file(buildingCatalogPath);
        if (!file.is_open()) {
            Logger::Error("BUILDING", "Failed to open catalog file: " + buildingCatalogPath);
            return false;
        }

        nlohmann::json root;
        file >> root;

        m_villageName = root.value("village_name", "Greenwood Village");
        m_region = root.value("region", "Greenwood Hamlet & Commons");
        m_exteriorMapPath = root.value("default_exterior_map", defaultExteriorMap);

        if (root.contains("buildings") && root["buildings"].is_array()) {
            for (const auto& bJson : root["buildings"]) {
                Building b;
                b.id = bJson.value("id", "");
                b.name = bJson.value("name", "Unnamed Building");
                b.buildingType = bJson.value("building_type", "house_cottage_001");
                b.district = bJson.value("district", "General");

                if (bJson.contains("exterior_bounds")) {
                    b.exteriorBounds.x = bJson["exterior_bounds"].value("x", 0.0f);
                    b.exteriorBounds.y = bJson["exterior_bounds"].value("y", 0.0f);
                    b.exteriorBounds.width = bJson["exterior_bounds"].value("width", 32.0f);
                    b.exteriorBounds.height = bJson["exterior_bounds"].value("height", 32.0f);
                }

                if (bJson.contains("entrance_position")) {
                    b.entrancePosition.x = bJson["entrance_position"].value("x", 0.0f);
                    b.entrancePosition.y = bJson["entrance_position"].value("y", 0.0f);
                }
                b.entranceRadius = bJson.value("entrance_radius", 32.0f);

                b.isEnterable = bJson.value("is_enterable", false);
                b.interiorMapPath = bJson.value("interior_map_path", "");

                if (bJson.contains("interior_spawn_position")) {
                    b.interiorSpawnPosition.x = bJson["interior_spawn_position"].value("x", 0.0f);
                    b.interiorSpawnPosition.y = bJson["interior_spawn_position"].value("y", 0.0f);
                }

                if (bJson.contains("interior_exit_position")) {
                    b.interiorExitPosition.x = bJson["interior_exit_position"].value("x", 0.0f);
                    b.interiorExitPosition.y = bJson["interior_exit_position"].value("y", 0.0f);
                }

                if (bJson.contains("exterior_return_position")) {
                    b.exteriorReturnPosition.x = bJson["exterior_return_position"].value("x", 0.0f);
                    b.exteriorReturnPosition.y = bJson["exterior_return_position"].value("y", 0.0f);
                }

                b.description = bJson.value("description", "");

                m_buildings.push_back(std::move(b));
            }
        }

        Logger::Info("BUILDING", "Loaded " + std::to_string(m_buildings.size()) +
                     " buildings for settlement '" + m_villageName + "' (" +
                     std::to_string(GetEnterableBuildingCount()) + " enterable interiors).");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("BUILDING", "Exception loading building catalog: " + std::string(e.what()));
        return false;
    }
}

const Building* BuildingManager::GetBuilding(const std::string& id) const {
    for (const auto& b : m_buildings) {
        if (b.id == id) {
            return &b;
        }
    }
    return nullptr;
}

const Building* BuildingManager::GetCurrentInteriorBuilding() const {
    if (!m_isIndoors || m_currentInteriorId.empty()) {
        return nullptr;
    }
    return GetBuilding(m_currentInteriorId);
}

const Building* BuildingManager::FindNearestBuilding(const Vector2& position, float maxDist) const {
    const Building* nearest = nullptr;
    float bestDistSq = maxDist * maxDist;

    for (const auto& b : m_buildings) {
        float dx = position.x - b.entrancePosition.x;
        float dy = position.y - b.entrancePosition.y;
        float distSq = dx * dx + dy * dy;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            nearest = &b;
        }
    }
    return nearest;
}

const Building* BuildingManager::CheckEntrancePrompt(const Vector2& playerPos) const {
    if (m_isIndoors) {
        return nullptr;
    }
    for (const auto& b : m_buildings) {
        if (b.isEnterable && b.IsInEntranceRange(playerPos)) {
            return &b;
        }
    }
    return nullptr;
}

bool BuildingManager::CheckExitPrompt(const Vector2& playerPos) const {
    if (!m_isIndoors) {
        return false;
    }
    const Building* current = GetCurrentInteriorBuilding();
    if (!current) {
        return false;
    }
    return current->IsInExitRange(playerPos);
}

bool BuildingManager::EnterBuilding(const std::string& buildingId, World& world, entities::Player& player, graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr) {
    if (m_isIndoors) {
        Logger::Warn("BUILDING", "Cannot enter building: already indoors.");
        return false;
    }

    const Building* b = GetBuilding(buildingId);
    if (!b) {
        Logger::Error("BUILDING", "Cannot enter: unknown building id '" + buildingId + "'.");
        return false;
    }

    if (!b->isEnterable || b->interiorMapPath.empty()) {
        Logger::Warn("BUILDING", "Building '" + b->name + "' does not have an enterable interior.");
        return false;
    }

    m_savedExteriorPlayerPos = player.GetPosition();

    Logger::Info("BUILDING", "Entering building '" + b->name + "' -> Loading interior: " + b->interiorMapPath);
    if (!world.LoadMap(b->interiorMapPath, assetMgr)) {
        Logger::Error("BUILDING", "Failed to load interior map: " + b->interiorMapPath);
        return false;
    }

    m_isIndoors = true;
    m_currentInteriorId = b->id;

    // Position player at interior spawn point
    player.Spawn(b->interiorSpawnPosition);

    // Update camera for interior bounds
    camera.SetWorldBounds(world.GetWorldBounds());
    camera.SetTarget(b->interiorSpawnPosition);
    camera.SetFollowTarget(b->interiorSpawnPosition);

    Logger::Info("BUILDING", "Player successfully entered '" + b->name + "'. Camera bounds updated.");
    return true;
}

bool BuildingManager::ExitBuilding(World& world, entities::Player& player, graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr) {
    if (!m_isIndoors) {
        Logger::Warn("BUILDING", "Cannot exit building: player is already outdoors.");
        return false;
    }

    const Building* b = GetCurrentInteriorBuilding();
    Vector2 returnPos = m_savedExteriorPlayerPos;
    if (b && (b->exteriorReturnPosition.x != 0.0f || b->exteriorReturnPosition.y != 0.0f)) {
        returnPos = b->exteriorReturnPosition;
    }

    Logger::Info("BUILDING", "Exiting interior -> Reloading exterior map: " + m_exteriorMapPath);
    if (!world.LoadMap(m_exteriorMapPath, assetMgr)) {
        Logger::Error("BUILDING", "Failed to reload exterior map: " + m_exteriorMapPath);
        return false;
    }

    m_isIndoors = false;
    m_currentInteriorId.clear();

    // Position player outside at exterior entrance
    player.Spawn(returnPos);

    // Update camera for exterior world bounds
    camera.SetWorldBounds(world.GetWorldBounds());
    camera.SetTarget(returnPos);
    camera.SetFollowTarget(returnPos);

    Logger::Info("BUILDING", "Player successfully exited to exterior. Camera bounds restored.");
    return true;
}

int BuildingManager::GetEnterableBuildingCount() const {
    int count = 0;
    for (const auto& b : m_buildings) {
        if (b.isEnterable) {
            count++;
        }
    }
    return count;
}

} // namespace wildbound::world

