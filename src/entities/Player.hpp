#pragma once

#include "Character.hpp"
#include "CharacterController.hpp"

namespace wildbound::world {
    class World;
}

namespace wildbound::entities {

/**
 * Playable Character Entity.
 * Integrates input controller, swept AABB collision resolution with wall sliding,
 * camera follow targeting, and animation state machine updates.
 */
class Player : public Character {
public:
    explicit Player(uint32_t id = 1, const std::string& name = "Explorer");
    ~Player() override = default;

    void Update(float dt) override;
    void UpdateWithWorld(float dt, const world::World& world, const resources::AssetManager& assetMgr);

    // Controls
    CharacterController& GetController() { return m_controller; }
    const CharacterController& GetController() const { return m_controller; }

    // Spawn
    void Spawn(Vector2 spawnPos);

    // Collision Resolution Helper (exposed for unit testing)
    void ResolveMovement(Vector2 delta, const world::World& world);

private:
    CharacterController m_controller;
};

} // namespace wildbound::entities

