#pragma once

#include "InteractionTypes.hpp"
#include <raylib.h>
#include <string>
#include <functional>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::interaction {

class Mechanism;
using MechanismStateCallback = std::function<void(Mechanism& mechanism, bool active)>;

/**
 * World Mechanism representation (Levers, Switches, Pressure Plates, Gate Controls).
 * Supports Active/Inactive state toggling, one-time vs repeatable activation,
 * cooldown timers, automatic trigger vs manual interaction mode, and event callbacks.
 */
class Mechanism {
public:
    Mechanism(std::string id, std::string name, Vector2 position, MechanismType type,
              bool isOneTime = false, float cooldown = 0.5f, bool isTriggerBased = false);
    ~Mechanism() = default;

    [[nodiscard]] const std::string& GetId() const { return m_id; }
    [[nodiscard]] const std::string& GetName() const { return m_name; }
    [[nodiscard]] Vector2 GetPosition() const { return m_position; }
    [[nodiscard]] Rectangle GetBounds() const { return m_bounds; }
    [[nodiscard]] MechanismType GetType() const { return m_type; }
    [[nodiscard]] MechanismState GetState() const { return m_state; }
    [[nodiscard]] bool IsActive() const { return m_state == MechanismState::Active; }
    [[nodiscard]] bool IsOneTime() const { return m_isOneTime; }
    [[nodiscard]] bool IsTriggerBased() const { return m_isTriggerBased; }
    [[nodiscard]] bool CanActivate() const;

    void SetStateCallback(MechanismStateCallback callback) { m_onStateChanged = std::move(callback); }

    bool Activate(audio::AudioSystem* audioSys = nullptr);
    bool Deactivate(audio::AudioSystem* audioSys = nullptr);
    bool Toggle(audio::AudioSystem* audioSys = nullptr);

    void Update(float dt);
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr);

private:
    std::string m_id;
    std::string m_name;
    Vector2 m_position{ 0.0f, 0.0f };
    Rectangle m_bounds{ 0.0f, 0.0f, 24.0f, 24.0f };
    MechanismType m_type = MechanismType::Lever;
    MechanismState m_state = MechanismState::Inactive;
    bool m_isOneTime = false;
    float m_cooldown = 0.5f;
    float m_currentCooldown = 0.0f;
    bool m_isTriggerBased = false;
    bool m_hasBeenActivated = false;

    MechanismStateCallback m_onStateChanged = nullptr;
};

} // namespace wildbound::interaction

