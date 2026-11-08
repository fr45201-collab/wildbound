#pragma once

#include "Tilemap.hpp"
#include "CollisionMetadata.hpp"
#include "WorldItem.hpp"
#include "LootContainer.hpp"
#include <memory>
#include <string>
#include <vector>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::physics {
    class CollisionManager;
}

namespace wildbound::world {

/**
 * Top-level World subsystem managing active map data, boundaries,
 * spatial entity queries, environmental collision detection,
 * world item pickups, and loot containers.
 */
class World {
public:
    World();
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    bool LoadMap(const std::string& mapFilePath, resources::AssetManager* assetMgr = nullptr);
    bool LoadMap(const std::string& mapFilePath, resources::AssetManager& assetMgr) { return LoadMap(mapFilePath, &assetMgr); }
    void UnloadMap();

    bool IsLoaded() const { return m_tilemap != nullptr; }

    const Tilemap* GetTilemap() const { return m_tilemap.get(); }
    Tilemap* GetTilemap() { return m_tilemap.get(); }

    Rectangle GetWorldBounds() const;
    const std::string& GetMapName() const;
    Vector2 GetSpawnPosition(const std::string& spawnName = "player_spawn") const;

    // Collision queries
    std::vector<CollisionBox> GetCollisionBoxes(const Rectangle& queryArea) const;
    bool CheckCollision(const Rectangle& queryBox) const;
    void PopulateCollisionManager(physics::CollisionManager& colMgr) const;

    // Spatial object queries
    std::vector<const EnvironmentObject*> GetVisibleObjects(const Rectangle& visibleArea) const;
    void GetVisibleObjects(const Rectangle& visibleArea, std::vector<const EnvironmentObject*>& outObjects) const;

    // World Items & Pickups (Phase 19)
    WorldItem* SpawnWorldItem(Vector2 position, items::ItemInstance item);
    WorldItem* SpawnWorldItem(const std::string& definitionId, Vector2 position, int quantity = 1);
    void UpdateWorldItems(float dt);
    int ProcessPickups(entities::Player& player, const items::ItemManager* itemMgr = nullptr, audio::AudioSystem* audioSys = nullptr);
    void RenderWorldItems(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr = nullptr);
    const std::vector<std::unique_ptr<WorldItem>>& GetWorldItems() const { return m_worldItems; }
    WorldItem* GetWorldItem(uint64_t id);
    WorldItem* GetWorldItem(const std::string& idStr);
    void ClearWorldItems();

    // Loot Containers (Phase 19)
    LootContainer* SpawnLootContainer(std::string id, std::string name, Vector2 position, bool isLocked = false, std::string requiredKeyId = "");
    LootContainer* SpawnLootContainer(std::string id, Vector2 position, std::string name = "Chest", bool isLocked = false, std::string requiredKeyId = "");
    void RenderLootContainers(graphics::Renderer& renderer, resources::AssetManager& assetMgr);
    const std::vector<std::unique_ptr<LootContainer>>& GetLootContainers() const { return m_lootContainers; }
    LootContainer* GetLootContainer(const std::string& id);
    LootContainer* GetLootContainerNear(Vector2 position, float radius = 48.0f);
    void ClearLootContainers();

private:
    void RebuildCollisionCache();

    std::unique_ptr<Tilemap> m_tilemap;
    std::vector<CollisionBox> m_staticCollisionCache;

    std::vector<std::unique_ptr<WorldItem>> m_worldItems;
    std::vector<std::unique_ptr<LootContainer>> m_lootContainers;
    uint64_t m_nextWorldItemId = 1;
};

} // namespace wildbound::world

