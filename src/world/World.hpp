#pragma once

#include "Tilemap.hpp"
#include "CollisionMetadata.hpp"
#include <memory>
#include <string>
#include <vector>

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::world {

/**
 * Top-level World subsystem managing active map data, boundaries,
 * spatial entity queries, and environmental collision detection.
 */
class World {
public:
    World();
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    bool LoadMap(const std::string& mapFilePath, resources::AssetManager& assetMgr);
    void UnloadMap();

    bool IsLoaded() const { return m_tilemap != nullptr; }

    const Tilemap* GetTilemap() const { return m_tilemap.get(); }
    Tilemap* GetTilemap() { return m_tilemap.get(); }

    Rectangle GetWorldBounds() const;
    const std::string& GetMapName() const;

    // Collision queries
    std::vector<CollisionBox> GetCollisionBoxes(const Rectangle& queryArea) const;
    bool CheckCollision(const Rectangle& queryBox) const;

    // Spatial object queries
    std::vector<const EnvironmentObject*> GetVisibleObjects(const Rectangle& visibleArea) const;

private:
    void RebuildCollisionCache();

    std::unique_ptr<Tilemap> m_tilemap;
    std::vector<CollisionBox> m_staticCollisionCache;
};

} // namespace wildbound::world

