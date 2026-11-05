#pragma once

#include "InteractionTypes.hpp"
#include <raylib.h>
#include <string>
#include <cstdint>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::physics {
    class CollisionManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::interaction {

/**
 * Authoritative World Door & Gate representation.
 * Supports Closed, Open, and Locked states, key requirements,
 * visual state updates, and solid collision synchronization with CollisionManager.
 */
class Door {
public:
    Door(std::string id, std::string name, Vector2 position, Rectangle bounds,
         DoorState initialState = DoorState::Closed, std::string requiredKeyId = "");
    ~Door() = default;

    [[nodiscard]] const std::string& GetId() const { return m_id; }
    [[nodiscard]] const std::string& GetName() const { return m_name; }
    [[nodiscard]] Vector2 GetPosition() const { return m_position; }
    [[nodiscard]] Rectangle GetBounds() const { return m_bounds; }
    [[nodiscard]] DoorState GetState() const { return m_state; }
    [[nodiscard]] bool IsOpen() const { return m_state == DoorState::Open; }
    [[nodiscard]] bool IsLocked() const { return m_state == DoorState::Locked; }
    [[nodiscard]] const std::string& GetRequiredKeyId() const { return m_requiredKeyId; }

    void SetSolidColliderId(uint64_t colliderId) { m_solidColliderId = colliderId; }
    [[nodiscard]] uint64_t GetSolidColliderId() const { return m_solidColliderId; }

    // State mutations & collision synchronization
    bool Open(audio::AudioSystem* audioSys = nullptr, physics::CollisionManager* collisionMgr = nullptr);
    bool Close(audio::AudioSystem* audioSys = nullptr, physics::CollisionManager* collisionMgr = nullptr);
    bool Toggle(audio::AudioSystem* audioSys = nullptr, physics::CollisionManager* collisionMgr = nullptr);
    bool Unlock(entities::Player& player, audio::AudioSystem* audioSys = nullptr);

    InteractionResult TryInteract(entities::Player& player, audio::AudioSystem* audioSys = nullptr,
                                  physics::CollisionManager* collisionMgr = nullptr);

    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr);

private:
    std::string m_id;
    std::string m_name;
    Vector2 m_position{ 0.0f, 0.0f };
    Rectangle m_bounds{ 0.0f, 0.0f, 32.0f, 48.0f };
    DoorState m_state = DoorState::Closed;
    std::string m_requiredKeyId;
    uint64_t m_solidColliderId = 0;
};

} // namespace wildbound::interaction

