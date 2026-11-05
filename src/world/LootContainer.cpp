#include "LootContainer.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../entities/Player.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::world {

LootContainer::LootContainer(std::string id, std::string name, Vector2 position, bool isLocked, std::string requiredKeyId)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_position(position)
    , m_isLocked(isLocked)
    , m_requiredKeyId(std::move(requiredKeyId))
{
}

bool LootContainer::CanInteract(Vector2 playerPos, float interactRadius) const {
    float dx = playerPos.x - m_position.x;
    float dy = playerPos.y - m_position.y;
    return (dx * dx + dy * dy) <= (interactRadius * interactRadius);
}

bool LootContainer::Open(audio::AudioSystem* audioSys) {
    m_isOpen = true;
    m_isLocked = false;
    Logger::Info("LOOT", "Opened loot container: " + m_name);
    if (audioSys) {
        audioSys->DispatchEvent("Environment.ChestOpen");
    }
    return true;
}

bool LootContainer::TryOpen(entities::Player& player, audio::AudioSystem* audioSys) {
    if (m_isOpen) return true;

    // Check lock requirements
    if (m_isLocked) {
        if (!m_requiredKeyId.empty()) {
            if (player.GetInventory().HasItem(m_requiredKeyId)) {
                Logger::Info("LOOT", "Unlocked container [" + m_id + "] with key [" + m_requiredKeyId + "]");
                m_isLocked = false;
            } else {
                Logger::Warn("LOOT", "Container [" + m_id + "] is locked! Requires: " + m_requiredKeyId);
                if (audioSys) audioSys->DispatchEvent("UI.Error");
                return false;
            }
        } else {
            m_isLocked = false;
        }
    }

    m_isOpen = true;
    Logger::Info("LOOT", "Opened loot container: " + m_name);

    if (audioSys) {
        audioSys->DispatchEvent("Environment.ChestOpen");
    }

    return true;
}

int LootContainer::LootAll(entities::Player& player, const items::ItemManager* itemMgr, audio::AudioSystem* audioSys) {
    return LootAll(player.GetInventory(), itemMgr, audioSys);
}

int LootContainer::LootAll(items::Inventory& targetInventory, const items::ItemManager* itemMgr, audio::AudioSystem* audioSys) {
    if (!m_isOpen) return 0;

    int totalTransferred = 0;
    for (size_t i = 0; i < m_inventory.GetCapacity(); ++i) {
        auto& slot = m_inventory.GetSlot(i);
        if (slot.has_value() && slot->quantity > 0) {
            int remaining = targetInventory.AddItem(*slot, itemMgr);
            int transferred = slot->quantity - remaining;
            totalTransferred += transferred;

            if (remaining <= 0) {
                m_inventory.RemoveItem(i, slot->quantity);
            } else {
                slot->quantity = remaining;
                break; // Target inventory is full
            }
        }
    }

    if (totalTransferred > 0 && audioSys) {
        audioSys->DispatchEvent("Item.Pickup");
    }

    return totalTransferred;
}

void LootContainer::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    // Base contact shadow
    ::DrawEllipse(static_cast<int>(m_position.x), static_cast<int>(m_position.y + 12), 16.0f, 6.0f, Color{ 0, 0, 0, 120 });

    // Chest box body
    Color woodColor = m_isOpen ? Color{ 145, 95, 45, 255 } : Color{ 120, 75, 35, 255 };
    Color trimColor = m_isLocked ? Color{ 220, 45, 45, 255 } : Color{ 235, 195, 65, 255 };

    Rectangle boxRect = { m_position.x - 16.0f, m_position.y - 14.0f, 32.0f, 26.0f };
    ::DrawRectangleRec(boxRect, woodColor);
    ::DrawRectangleLinesEx(boxRect, 2.0f, trimColor);

    // Latch
    ::DrawRectangle(static_cast<int>(m_position.x - 3), static_cast<int>(m_position.y - 4), 6, 8, trimColor);

    // Open lid indicator
    if (m_isOpen) {
        ::DrawRectangle(static_cast<int>(m_position.x - 14), static_cast<int>(m_position.y - 20), 28, 6, Color{ 165, 115, 60, 255 });
    }
}

} // namespace wildbound::world

