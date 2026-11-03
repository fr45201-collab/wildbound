#include "World.hpp"
#include "MapLoader.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::world {

World::World() = default;

World::~World() {
    UnloadMap();
}

bool World::LoadMap(const std::string& mapFilePath, resources::AssetManager& assetMgr) {
    UnloadMap();

    Logger::Info("WORLD", "Loading map: " + mapFilePath);
    m_tilemap = MapLoader::LoadMap(mapFilePath, assetMgr);

    if (!m_tilemap) {
        Logger::Error("WORLD", "Failed loading world map: " + mapFilePath);
        return false;
    }

    RebuildCollisionCache();
    Logger::Info("WORLD", "Map '" + m_tilemap->GetName() + "' successfully initialized into World.");
    return true;
}

void World::UnloadMap() {
    if (m_tilemap) {
        Logger::Info("WORLD", "Unloading map: " + m_tilemap->GetName());
        m_tilemap.reset();
        m_staticCollisionCache.clear();
    }
}

Rectangle World::GetWorldBounds() const {
    if (m_tilemap) {
        return m_tilemap->GetWorldBounds();
    }
    return Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
}

const std::string& World::GetMapName() const {
    static const std::string emptyName = "None";
    if (m_tilemap) {
        return m_tilemap->GetName();
    }
    return emptyName;
}

Vector2 World::GetSpawnPosition(const std::string& spawnName) const {
    if (m_tilemap) {
        return m_tilemap->GetSpawnPosition(spawnName);
    }
    return Vector2{ 256.0f, 960.0f };
}

void World::RebuildCollisionCache() {
    m_staticCollisionCache.clear();
    if (!m_tilemap) return;

    // 1. Collect collision boxes from environmental objects
    for (const auto& obj : m_tilemap->GetObjects()) {
        if (obj.HasCollision()) {
            m_staticCollisionCache.push_back(obj.GetWorldCollision());
        }
    }

    // 2. Check for tile-level collision layers (e.g. "Collision Metadata", "Water", "Collision")
    for (const auto& layer : m_tilemap->GetLayers()) {
        if (layer.GetName() == "Collision Metadata" || layer.GetName() == "Collision") {
            int tw = m_tilemap->GetTileWidth();
            int th = m_tilemap->GetTileHeight();
            for (int r = 0; r < layer.GetHeight(); ++r) {
                for (int c = 0; c < layer.GetWidth(); ++c) {
                    int gid = layer.GetTileGid(c, r);
                    if (gid > 0) {
                        CollisionBox box;
                        box.bounds = Rectangle{
                            static_cast<float>(c * tw),
                            static_cast<float>(r * th),
                            static_cast<float>(tw),
                            static_cast<float>(th)
                        };
                        box.type = CollisionType::Solid;
                        box.tag = "tile_collision";
                        m_staticCollisionCache.push_back(box);
                    }
                }
            }
        }
    }

    Logger::Info("WORLD", "Built collision spatial index with " +
                 std::to_string(m_staticCollisionCache.size()) + " collision bounds.");
}

std::vector<CollisionBox> World::GetCollisionBoxes(const Rectangle& queryArea) const {
    std::vector<CollisionBox> result;
    for (const auto& box : m_staticCollisionCache) {
        if (box.Intersects(queryArea)) {
            result.push_back(box);
        }
    }
    return result;
}

bool World::CheckCollision(const Rectangle& queryBox) const {
    for (const auto& box : m_staticCollisionCache) {
        if (box.IsSolid() && box.Intersects(queryBox)) {
            return true;
        }
    }
    return false;
}

std::vector<const EnvironmentObject*> World::GetVisibleObjects(const Rectangle& visibleArea) const {
    std::vector<const EnvironmentObject*> visible;
    if (!m_tilemap) return visible;

    for (const auto& obj : m_tilemap->GetObjects()) {
        Rectangle objBounds = obj.GetWorldRenderBounds();
        if (CheckCollisionRecs(objBounds, visibleArea)) {
            visible.push_back(&obj);
        }
    }
    return visible;
}

} // namespace wildbound::world

