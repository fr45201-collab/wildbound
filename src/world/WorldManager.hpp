#pragma once

#include "WorldData.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::world {

class World;

/**
 * WorldManager coordinates the Battle Royale world metadata,
 * geographical biomes, settlements, strategic POIs, spawn regions,
 * loot placement markers, traversal routes, and debug visualization overlays.
 */
class WorldManager {
public:
    WorldManager();
    ~WorldManager();

    bool Initialize(const std::string& worldDataDir = "data/world");

    // Metadata Queries
    [[nodiscard]] const BattleRoyaleWorldDef& GetWorldDef() const { return m_worldDef; }
    [[nodiscard]] Rectangle GetPlayableBoundary() const noexcept { return m_worldDef.playableBounds; }
    [[nodiscard]] const std::vector<BiomeRegion>& GetBiomeRegions() const { return m_biomeRegions; }
    [[nodiscard]] const BiomeRegion* GetBiomeRegionAt(const Vector2& worldPos) const;
    [[nodiscard]] const std::vector<SettlementDef>& GetSettlements() const { return m_settlements; }
    [[nodiscard]] const SettlementDef* GetSettlement(const std::string& id) const;
    [[nodiscard]] const std::vector<PointOfInterest>& GetPointsOfInterest() const { return m_pois; }
    [[nodiscard]] const PointOfInterest* GetPointOfInterest(const std::string& id) const;
    [[nodiscard]] const std::vector<SpawnRegionDef>& GetSpawnRegions() const { return m_spawnRegions; }
    [[nodiscard]] const SpawnRegionDef* GetSpawnRegion(const std::string& regionId) const;
    [[nodiscard]] const std::vector<LootLocationDef>& GetLootLocations() const { return m_lootLocations; }
    [[nodiscard]] const std::vector<TraversalRoute>& GetPrimaryRoutes() const { return m_primaryRoutes; }
    [[nodiscard]] const std::vector<Chokepoint>& GetChokepoints() const { return m_chokepoints; }
    [[nodiscard]] const std::vector<WorldLandmark>& GetLandmarks() const { return m_landmarks; }

    // Validation
    [[nodiscard]] bool ValidateWorldData(std::vector<std::string>& outErrors) const;

    // Debug Visualization Overlay Toggles
    [[nodiscard]] bool GetShowBiomeBounds() const { return m_showBiomeBounds; }
    void SetShowBiomeBounds(bool show) { m_showBiomeBounds = show; }

    [[nodiscard]] bool GetShowPois() const { return m_showPois; }
    void SetShowPois(bool show) { m_showPois = show; }

    [[nodiscard]] bool GetShowSettlementBounds() const { return m_showSettlementBounds; }
    void SetShowSettlementBounds(bool show) { m_showSettlementBounds = show; }

    [[nodiscard]] bool GetShowSpawnRegions() const { return m_showSpawnRegions; }
    void SetShowSpawnRegions(bool show) { m_showSpawnRegions = show; }

    [[nodiscard]] bool GetShowLootMarkers() const { return m_showLootMarkers; }
    void SetShowLootMarkers(bool show) { m_showLootMarkers = show; }

    [[nodiscard]] bool GetShowRoutes() const { return m_showRoutes; }
    void SetShowRoutes(bool show) { m_showRoutes = show; }

    [[nodiscard]] bool GetShowChokepoints() const { return m_showChokepoints; }
    void SetShowChokepoints(bool show) { m_showChokepoints = show; }

    [[nodiscard]] bool GetShowPlayableBounds() const { return m_showPlayableBounds; }
    void SetShowPlayableBounds(bool show) { m_showPlayableBounds = show; }

    // Render debug overlays
    void RenderDebugOverlays(const graphics::Camera2DSystem& camera) const;

private:
    BattleRoyaleWorldDef m_worldDef;
    std::vector<BiomeRegion> m_biomeRegions;
    std::vector<SettlementDef> m_settlements;
    std::unordered_map<std::string, size_t> m_settlementIndexMap;
    std::vector<PointOfInterest> m_pois;
    std::unordered_map<std::string, size_t> m_poiIndexMap;
    std::vector<SpawnRegionDef> m_spawnRegions;
    std::unordered_map<std::string, size_t> m_spawnIndexMap;
    std::vector<LootLocationDef> m_lootLocations;
    std::vector<TraversalRoute> m_primaryRoutes;
    std::vector<Chokepoint> m_chokepoints;
    std::vector<WorldLandmark> m_landmarks;

    bool m_isInitialized = false;

    // Overlay flags
    bool m_showBiomeBounds = false;
    bool m_showPois = true;
    bool m_showSettlementBounds = true;
    bool m_showSpawnRegions = false;
    bool m_showLootMarkers = false;
    bool m_showRoutes = true;
    bool m_showChokepoints = true;
    bool m_showPlayableBounds = true;
};

} // namespace wildbound::world

