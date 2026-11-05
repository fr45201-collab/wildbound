#pragma once

#include "BuildingData.hpp"
#include <vector>
#include <string>
#include <memory>

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::world {

class World;

/**
 * Manages village building structures, metadata catalog,
 * proximity detection, and transitions between the exterior world and building interiors.
 */
class BuildingManager {
public:
    BuildingManager();
    ~BuildingManager();

    bool Initialize(const std::string& buildingCatalogPath, const std::string& defaultExteriorMap = "maps/forest/forest_01.json");

    const std::vector<Building>& GetBuildings() const { return m_buildings; }
    const Building* GetBuilding(const std::string& id) const;
    const Building* FindNearestBuilding(const Vector2& position, float maxDist = 300.0f) const;

    // Interior state queries
    [[nodiscard]] bool IsIndoors() const { return m_isIndoors; }
    [[nodiscard]] const std::string& GetCurrentInteriorId() const { return m_currentInteriorId; }
    [[nodiscard]] const Building* GetCurrentInteriorBuilding() const;
    [[nodiscard]] const std::string& GetExteriorMapPath() const { return m_exteriorMapPath; }
    [[nodiscard]] const std::string& GetVillageName() const { return m_villageName; }
    [[nodiscard]] const std::string& GetRegion() const { return m_region; }

    // Proximity checks
    [[nodiscard]] const Building* CheckEntrancePrompt(const Vector2& playerPos) const;
    [[nodiscard]] bool CheckExitPrompt(const Vector2& playerPos) const;

    // Transitions
    bool EnterBuilding(const std::string& buildingId, World& world, entities::Player& player, graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr);
    bool ExitBuilding(World& world, entities::Player& player, graphics::Camera2DSystem& camera, resources::AssetManager& assetMgr);

    // Helpers
    [[nodiscard]] int GetEnterableBuildingCount() const;
    [[nodiscard]] int GetTotalBuildingCount() const { return static_cast<int>(m_buildings.size()); }

private:
    std::vector<Building> m_buildings;
    std::string m_villageName = "Greenwood Village";
    std::string m_region = "Greenwood Hamlet & Commons";
    std::string m_exteriorMapPath = "maps/forest/forest_01.json";

    bool m_isIndoors = false;
    std::string m_currentInteriorId;
    Vector2 m_savedExteriorPlayerPos{ 0.0f, 0.0f };
};

} // namespace wildbound::world

