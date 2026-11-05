#include "Door.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../physics/CollisionManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../entities/Player.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::interaction {

Door::Door(std::string id, std::string name, Vector2 position, Rectangle bounds,
           DoorState initialState, std::string requiredKeyId)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_position(position)
    , m_bounds(bounds)
    , m_state(initialState)
    , m_requiredKeyId(std::move(requiredKeyId))
{
    if (m_bounds.width <= 0.0f) m_bounds.width = 32.0f;
    if (m_bounds.height <= 0.0f) m_bounds.height = 48.0f;
    m_bounds.x = m_position.x - m_bounds.width * 0.5f;
    m_bounds.y = m_position.y - m_bounds.height * 0.5f;
}

bool Door::Open(audio::AudioSystem* audioSys, physics::CollisionManager* collisionMgr) {
    if (m_state == DoorState::Open || m_state == DoorState::Locked) {
        return false;
    }

    m_state = DoorState::Open;

    // Synchronize collision: disable solid physical blockage
    if (collisionMgr && m_solidColliderId != 0) {
        collisionMgr->SetColliderEnabled(m_solidColliderId, false);
    }

    if (audioSys) {
        audioSys->DispatchEvent("Environment.DoorOpen");
    }

    Logger::Info("DOOR", "Door [" + m_id + "] opened.");
    return true;
}

bool Door::Close(audio::AudioSystem* audioSys, physics::CollisionManager* collisionMgr) {
    if (m_state == DoorState::Closed) {
        return false;
    }

    m_state = DoorState::Closed;

    // Synchronize collision: re-enable solid physical blockage
    if (collisionMgr && m_solidColliderId != 0) {
        collisionMgr->SetColliderEnabled(m_solidColliderId, true);
    }

    if (audioSys) {
        audioSys->DispatchEvent("Environment.DoorClose");
    }

    Logger::Info("DOOR", "Door [" + m_id + "] closed.");
    return true;
}

bool Door::Toggle(audio::AudioSystem* audioSys, physics::CollisionManager* collisionMgr) {
    if (m_state == DoorState::Open) {
        return Close(audioSys, collisionMgr);
    } else if (m_state == DoorState::Closed) {
        return Open(audioSys, collisionMgr);
    }
    return false;
}

bool Door::Unlock(entities::Player& player, audio::AudioSystem* audioSys) {
    if (m_state != DoorState::Locked) {
        return true;
    }

    if (m_requiredKeyId.empty()) {
        m_state = DoorState::Closed;
        return true;
    }

    // Check if player has the required key
    const auto& inv = player.GetInventory();
    if (inv.HasItem(m_requiredKeyId)) {
        m_state = DoorState::Closed;
        if (audioSys) {
            audioSys->DispatchEvent("Interact.KeyUnlock");
        }
        Logger::Info("DOOR", "Door [" + m_id + "] unlocked with key [" + m_requiredKeyId + "].");
        return true;
    }

    Logger::Warn("DOOR", "Cannot unlock door [" + m_id + "]: missing required key [" + m_requiredKeyId + "].");
    return false;
}

InteractionResult Door::TryInteract(entities::Player& player, audio::AudioSystem* audioSys,
                                    physics::CollisionManager* collisionMgr) {
    if (m_state == DoorState::Locked) {
        if (Unlock(player, audioSys)) {
            // Automatically open upon unlocking
            Open(audioSys, collisionMgr);
            return InteractionResult::Success;
        }
        return InteractionResult::MissingRequirement;
    }

    if (Toggle(audioSys, collisionMgr)) {
        return InteractionResult::Success;
    }

    return InteractionResult::Failed;
}

void Door::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;

    // Render door visually based on state
    if (m_state == DoorState::Open) {
        // Open door: thin side frame
        ::DrawRectangleLines(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y),
                             static_cast<int>(m_bounds.width), static_cast<int>(m_bounds.height),
                             Color{ 100, 160, 220, 180 });
        ::DrawRectangle(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y), 6,
                        static_cast<int>(m_bounds.height), Color{ 120, 85, 50, 255 });
    } else if (m_state == DoorState::Locked) {
        // Locked door: solid brown with golden padlock badge
        ::DrawRectangle(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y),
                        static_cast<int>(m_bounds.width), static_cast<int>(m_bounds.height),
                        Color{ 95, 60, 35, 255 });
        ::DrawRectangleLines(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y),
                             static_cast<int>(m_bounds.width), static_cast<int>(m_bounds.height),
                             Color{ 190, 60, 60, 255 });
        // Golden lock center
        ::DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), 4.0f, Color{ 235, 195, 60, 255 });
    } else {
        // Closed door: solid wood texture/rect with brass handle
        ::DrawRectangle(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y),
                        static_cast<int>(m_bounds.width), static_cast<int>(m_bounds.height),
                        Color{ 115, 75, 45, 255 });
        ::DrawRectangleLines(static_cast<int>(m_bounds.x), static_cast<int>(m_bounds.y),
                             static_cast<int>(m_bounds.width), static_cast<int>(m_bounds.height),
                             Color{ 60, 40, 25, 255 });
        // Door handle
        ::DrawCircle(static_cast<int>(m_position.x + 8.0f), static_cast<int>(m_position.y), 3.0f, Color{ 230, 210, 100, 255 });
    }
}

} // namespace wildbound::interaction
