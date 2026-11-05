#include "Mechanism.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::interaction {

Mechanism::Mechanism(std::string id, std::string name, Vector2 position, MechanismType type,
                     bool isOneTime, float cooldown, bool isTriggerBased)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_position(position)
    , m_type(type)
    , m_isOneTime(isOneTime)
    , m_cooldown(cooldown)
    , m_isTriggerBased(isTriggerBased)
{
    float w = (type == MechanismType::PressurePlate) ? 32.0f : 20.0f;
    float h = (type == MechanismType::PressurePlate) ? 32.0f : 20.0f;
    m_bounds = Rectangle{ m_position.x - w * 0.5f, m_position.y - h * 0.5f, w, h };
}

bool Mechanism::CanActivate() const {
    if (m_isOneTime && m_hasBeenActivated) return false;
    return m_currentCooldown <= 0.0f;
}

bool Mechanism::Activate(audio::AudioSystem* audioSys) {
    if (!CanActivate() || m_state == MechanismState::Active) {
        return false;
    }

    m_state = MechanismState::Active;
    m_hasBeenActivated = true;
    m_currentCooldown = m_cooldown;

    if (audioSys) {
        audioSys->DispatchEvent("Interact.MechanismActivate");
    }

    Logger::Info("MECHANISM", "Mechanism [" + m_id + "] ACTIVATED.");
    if (m_onStateChanged) {
        m_onStateChanged(*this, true);
    }
    return true;
}

bool Mechanism::Deactivate(audio::AudioSystem* audioSys) {
    if (!CanActivate() || m_state == MechanismState::Inactive || m_isOneTime) {
        return false;
    }

    m_state = MechanismState::Inactive;
    m_currentCooldown = m_cooldown;

    if (audioSys) {
        audioSys->DispatchEvent("Interact.MechanismDeactivate");
    }

    Logger::Info("MECHANISM", "Mechanism [" + m_id + "] DEACTIVATED.");
    if (m_onStateChanged) {
        m_onStateChanged(*this, false);
    }
    return true;
}

bool Mechanism::Toggle(audio::AudioSystem* audioSys) {
    if (m_state == MechanismState::Active) {
        return Deactivate(audioSys);
    } else {
        return Activate(audioSys);
    }
}

void Mechanism::Update(float dt) {
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown -= dt;
        if (m_currentCooldown < 0.0f) m_currentCooldown = 0.0f;
    }
}

void Mechanism::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    (void)assetMgr;

    if (m_type == MechanismType::PressurePlate) {
        Color baseCol = (m_state == MechanismState::Active) ? Color{ 190, 70, 70, 220 } : Color{ 120, 125, 135, 220 };
        ::DrawRectangleRec(m_bounds, baseCol);
        ::DrawRectangleLinesEx(m_bounds, 1.5f, (m_state == MechanismState::Active) ? RED : Color{ 80, 85, 95, 255 });
    } else if (m_type == MechanismType::Lever) {
        // Lever base
        ::DrawRectangle(static_cast<int>(m_bounds.x + 2), static_cast<int>(m_bounds.y + 12), 16, 6, Color{ 70, 75, 85, 255 });
        // Lever stick
        if (m_state == MechanismState::Active) {
            ::DrawLineEx(Vector2{ m_bounds.x + 10, m_bounds.y + 14 },
                         Vector2{ m_bounds.x + 16, m_bounds.y + 2 }, 3.0f, Color{ 80, 210, 100, 255 });
        } else {
            ::DrawLineEx(Vector2{ m_bounds.x + 10, m_bounds.y + 14 },
                         Vector2{ m_bounds.x + 4, m_bounds.y + 2 }, 3.0f, Color{ 220, 175, 60, 255 });
        }
    } else {
        // Switch / Button / Generic mechanism
        Color c = (m_state == MechanismState::Active) ? Color{ 60, 220, 90, 255 } : Color{ 200, 60, 60, 255 };
        ::DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), 8.0f, Color{ 50, 55, 65, 255 });
        ::DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), 5.0f, c);
    }
}

} // namespace wildbound::interaction
