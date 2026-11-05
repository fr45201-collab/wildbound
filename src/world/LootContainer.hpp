#pragma once

#include "../items/Inventory.hpp"
#include <raylib.h>
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

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::world {

/**
 * World Loot Container (Chests, Crates, Caches).
 * Supports locked states with required keys, opening transitions,
 * transactional loot transfers into player inventory, and audio feedback.
 */
class LootContainer {
public:
    LootContainer(std::string id, std::string name, Vector2 position, bool isLocked = false, std::string requiredKeyId = "");
    ~LootContainer() = default;

    [[nodiscard]] const std::string& GetId() const { return m_id; }
    [[nodiscard]] const std::string& GetName() const { return m_name; }
    [[nodiscard]] Vector2 GetPosition() const { return m_position; }

    [[nodiscard]] bool IsOpen() const { return m_isOpen; }
    [[nodiscard]] bool IsLocked() const { return m_isLocked; }
    [[nodiscard]] const std::string& GetRequiredKeyId() const { return m_requiredKeyId; }

    items::Inventory& GetInventory() { return m_inventory; }
    const items::Inventory& GetInventory() const { return m_inventory; }

    // Interaction
    bool Open(audio::AudioSystem* audioSys = nullptr);
    bool TryOpen(entities::Player& player, audio::AudioSystem* audioSys = nullptr);
    int LootAll(entities::Player& player, const items::ItemManager* itemMgr = nullptr, audio::AudioSystem* audioSys = nullptr);
    int LootAll(items::Inventory& targetInventory, const items::ItemManager* itemMgr = nullptr, audio::AudioSystem* audioSys = nullptr);

    [[nodiscard]] bool CanInteract(Vector2 playerPos, float interactRadius = 45.0f) const;
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr);

private:
    std::string m_id;
    std::string m_name;
    Vector2 m_position{ 0.0f, 0.0f };
    items::Inventory m_inventory{ 12 };
    bool m_isOpen = false;
    bool m_isLocked = false;
    std::string m_requiredKeyId;
};

} // namespace wildbound::world

