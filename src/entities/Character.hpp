#pragma once

#include "Entity.hpp"
#include "CharacterStats.hpp"
#include "CharacterConfig.hpp"
#include "../graphics/animation/AnimationController.hpp"
#include "../graphics/animation/AnimationStateMachine.hpp"
#include "../graphics/animation/Direction.hpp"

namespace wildbound::entities {

class Character : public Entity {
public:
    Character(uint32_t id, const std::string& name, const std::string& tag = "character");
    ~Character() override = default;

    void ApplyConfig(const CharacterConfig& cfg);

    void Update(float dt) override;
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) override;

    Rectangle GetWorldBounds() const override;
    Rectangle GetCollisionBounds() const override;

    // Movement & Orientation
    Vector2 GetVelocity() const { return m_velocity; }
    void SetVelocity(Vector2 vel) { m_velocity = vel; }

    graphics::animation::Direction GetFacing() const { return m_facing; }
    void SetFacing(graphics::animation::Direction dir) {
        m_facing = dir;
        m_animStateMachine.SetFacing(dir);
    }

    graphics::animation::MovementState GetMovementState() const { return m_movementState; }

    const CharacterStats& GetStats() const { return m_stats; }
    CharacterStats& GetStats() { return m_stats; }

    const CharacterCollisionConfig& GetCollisionConfig() const { return m_collisionConfig; }
    void SetCollisionConfig(const CharacterCollisionConfig& col) { m_collisionConfig = col; }

    const std::string& GetSpritesheetId() const { return m_spritesheetId; }
    void SetSpritesheetId(const std::string& sheetId) { m_spritesheetId = sheetId; }

    // Animation subcomponents
    const graphics::animation::AnimationController& GetAnimController() const { return m_animController; }
    graphics::animation::AnimationController& GetAnimController() { return m_animController; }

    const graphics::animation::AnimationStateMachine& GetAnimStateMachine() const { return m_animStateMachine; }
    graphics::animation::AnimationStateMachine& GetAnimStateMachine() { return m_animStateMachine; }

protected:
    CharacterStats m_stats;
    CharacterCollisionConfig m_collisionConfig;
    std::string m_spritesheetId = "player_spritesheet";

    Vector2 m_velocity{ 0.0f, 0.0f };
    graphics::animation::Direction m_facing = graphics::animation::Direction::Down;
    graphics::animation::MovementState m_movementState = graphics::animation::MovementState::Idle;

    graphics::animation::AnimationController m_animController;
    graphics::animation::AnimationStateMachine m_animStateMachine;
};

} // namespace wildbound::entities

