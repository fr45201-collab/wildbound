#include "World.hpp"
#include "MapLoader.hpp"
#include "../entities/Player.hpp"
#include "../items/ItemManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../physics/CollisionManager.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::world {

World::World() = default;

World::~World() {
    UnloadMap();
}

bool World::LoadMap(const std::string& mapFilePath, resources::AssetManager* assetMgr) {
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
        ClearWorldItems();
        ClearLootContainers();
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
        if (layer.GetName() == "Collision Metadata" || layer.GetName() == "Collision" || layer.GetName() == "Collision Layer") {
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

void World::PopulateCollisionManager(physics::CollisionManager& colMgr) const {
    for (const auto& box : m_staticCollisionCache) {
        if (box.type == CollisionType::Trigger || box.isTrigger) {
            colMgr.RegisterCollider(physics::Collider::CreateTrigger(
                0, physics::INVALID_ENTITY_ID,
                physics::CollisionLayer::Trigger,
                physics::GetDefaultCollisionMask(physics::CollisionLayer::Trigger),
                Vector2{ box.bounds.width, box.bounds.height },
                Vector2{ 0.0f, 0.0f },
                box.tag.empty() ? "world_trigger" : box.tag,
                true
            ));
        } else if (box.type == CollisionType::Solid || box.type == CollisionType::Impassable || box.type == CollisionType::Water) {
            colMgr.RegisterStaticBox(
                physics::CollisionLayer::World,
                box.bounds,
                box.tag.empty() ? "static_world" : box.tag
            );
        }
    }
    Logger::Info("WORLD", "Registered " + std::to_string(m_staticCollisionCache.size()) +
                 " static colliders with CollisionManager.");
}

std::vector<const EnvironmentObject*> World::GetVisibleObjects(const Rectangle& visibleArea) const {
    std::vector<const EnvironmentObject*> visible;
    GetVisibleObjects(visibleArea, visible);
    return visible;
}

void World::GetVisibleObjects(const Rectangle& visibleArea, std::vector<const EnvironmentObject*>& outObjects) const {
    outObjects.clear();
    if (!m_tilemap) return;

    for (const auto& obj : m_tilemap->GetObjects()) {
        Rectangle objBounds = obj.GetWorldRenderBounds();
        if (CheckCollisionRecs(objBounds, visibleArea)) {
            outObjects.push_back(&obj);
        }
    }
}

// World Items & Pickups (Phase 19)
WorldItem* World::SpawnWorldItem(Vector2 position, items::ItemInstance item) {
    auto wi = std::make_unique<WorldItem>(m_nextWorldItemId++, position, std::move(item));
    WorldItem* ptr = wi.get();
    m_worldItems.push_back(std::move(wi));
    Logger::Debug("WORLD", "Spawned WorldItem #" + std::to_string(ptr->GetId()) +
                  " [" + ptr->GetItem().definitionId + " x" + std::to_string(ptr->GetItem().quantity) +
                  "] at (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    return ptr;
}

WorldItem* World::SpawnWorldItem(const std::string& definitionId, Vector2 position, int quantity) {
    return SpawnWorldItem(position, items::ItemInstance{ 0, definitionId, quantity });
}

void World::UpdateWorldItems(float dt) {
    for (auto& wi : m_worldItems) {
        if (wi && !wi->IsCollected()) {
            wi->Update(dt);
        }
    }

    // Clean up collected items
    m_worldItems.erase(
        std::remove_if(m_worldItems.begin(), m_worldItems.end(), [](const std::unique_ptr<WorldItem>& wi) {
            return !wi || wi->IsCollected();
        }),
        m_worldItems.end()
    );
}

int World::ProcessPickups(entities::Player& player, const items::ItemManager* itemMgr, audio::AudioSystem* audioSys) {
    int totalPickedUp = 0;
    Vector2 playerPos = player.GetPosition();

    for (auto& wi : m_worldItems) {
        if (!wi || wi->IsCollected()) continue;

        if (wi->CanPickup(playerPos)) {
            // Transactional transfer: Add to inventory first
            int remaining = player.GetInventory().AddItem(wi->GetItem(), itemMgr);
            int pickedUpCount = wi->GetItem().quantity - remaining;

            if (pickedUpCount > 0) {
                totalPickedUp += pickedUpCount;
                if (remaining <= 0) {
                    wi->MarkCollected();
                } else {
                    wi->GetItem().quantity = remaining;
                }

                Logger::Info("PICKUP", "Player picked up " + std::to_string(pickedUpCount) +
                             "x [" + wi->GetItem().definitionId + "]");

                if (audioSys) {
                    audioSys->DispatchEvent("Item.Pickup");
                }
            } else {
                // Inventory is full
                Logger::Warn("PICKUP", "Cannot pick up [" + wi->GetItem().definitionId + "]: Inventory is full!");
            }
        }
    }

    return totalPickedUp;
}

void World::RenderWorldItems(graphics::Renderer& renderer, resources::AssetManager& assetMgr, const items::ItemManager* itemMgr) {
    for (const auto& wi : m_worldItems) {
        if (wi && !wi->IsCollected()) {
            wi->Render(renderer, assetMgr, itemMgr);
        }
    }
}

void World::ClearWorldItems() {
    m_worldItems.clear();
}

WorldItem* World::GetWorldItem(uint64_t id) {
    for (auto& wi : m_worldItems) {
        if (wi && wi->GetId() == id) {
            return wi.get();
        }
    }
    return nullptr;
}

WorldItem* World::GetWorldItem(const std::string& idStr) {
    if (idStr.empty()) return nullptr;
    try {
        uint64_t id = std::stoull(idStr);
        return GetWorldItem(id);
    } catch (...) {
        return nullptr;
    }
}

// Loot Containers (Phase 19)
LootContainer* World::SpawnLootContainer(std::string id, std::string name, Vector2 position, bool isLocked, std::string requiredKeyId) {
    auto lc = std::make_unique<LootContainer>(std::move(id), std::move(name), position, isLocked, std::move(requiredKeyId));
    LootContainer* ptr = lc.get();
    m_lootContainers.push_back(std::move(lc));
    Logger::Debug("WORLD", "Spawned LootContainer: " + ptr->GetName() + " at (" +
                  std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    return ptr;
}

LootContainer* World::SpawnLootContainer(std::string id, Vector2 position, std::string name, bool isLocked, std::string requiredKeyId) {
    return SpawnLootContainer(std::move(id), std::move(name), position, isLocked, std::move(requiredKeyId));
}

void World::RenderLootContainers(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    for (const auto& lc : m_lootContainers) {
        if (lc) {
            lc->Render(renderer, assetMgr);
        }
    }
}

LootContainer* World::GetLootContainer(const std::string& id) {
    for (auto& lc : m_lootContainers) {
        if (lc && lc->GetId() == id) {
            return lc.get();
        }
    }
    return nullptr;
}

LootContainer* World::GetLootContainerNear(Vector2 position, float radius) {
    float r2 = radius * radius;
    for (auto& lc : m_lootContainers) {
        if (!lc) continue;
        Vector2 cPos = lc->GetPosition();
        float dx = cPos.x - position.x;
        float dy = cPos.y - position.y;
        if (dx * dx + dy * dy <= r2) {
            return lc.get();
        }
    }
    return nullptr;
}

void World::ClearLootContainers() {
    m_lootContainers.clear();
}

} // namespace wildbound::world

