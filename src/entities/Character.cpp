#include "Character.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../resources/SpriteSheet.hpp"

namespace wildbound::entities {

Character::Character(uint32_t id, const std::string& name, const std::string& tag)
    : Entity(id, name, tag)
    , m_animStateMachine(name)
{
}

void Character::ApplyConfig(const CharacterConfig& cfg) {
    m_name = cfg.name;
    m_spritesheetId = cfg.spritesheet;
    m_stats = cfg.stats;
    m_collisionConfig = cfg.collision;

    m_animStateMachine.SetCharacterPrefix(cfg.id);

    // Apply animation clip overrides
    for (const auto& [animKey, clipName] : cfg.animationClips) {
        if (animKey == "idle_down")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Down, clipName);
        if (animKey == "idle_up")    m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Up, clipName);
        if (animKey == "idle_left")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Left, clipName);
        if (animKey == "idle_right") m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Right, clipName);
        if (animKey == "walk_down")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Down, clipName);
        if (animKey == "walk_up")    m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Up, clipName);
        if (animKey == "walk_left")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Left, clipName);
        if (animKey == "walk_right") m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Right, clipName);
        if (animKey == "run_down")   m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Running, graphics::animation::Direction::Down, clipName);
        if (animKey == "run_up")     m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Running, graphics::animation::Direction::Up, clipName);
        if (animKey == "run_left")   m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Running, graphics::animation::Direction::Left, clipName);
        if (animKey == "run_right")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Running, graphics::animation::Direction::Right, clipName);
    }
}

void Character::Update(float /*dt*/) {
}

void Character::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    if (!m_active) return;

    Texture2D tex = assetMgr.GetTexture(m_spritesheetId);
    if (tex.id == 0) return;

    Rectangle srcRect = m_animController.GetCurrentFrameRect(assetMgr);
    Vector2 origin = m_animController.GetCurrentFrameOrigin(assetMgr);

    // Logical position represents the bottom-center anchor of the feet
    Rectangle destRect{
        m_position.x - origin.x,
        m_position.y - origin.y,
        srcRect.width,
        srcRect.height
    };

    renderer.DrawSprite(tex, srcRect, destRect, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
}

Rectangle Character::GetWorldBounds() const {
    // Default 32x48 character sprite bounds anchored at bottom-center
    return Rectangle{
        m_position.x - 16.0f,
        m_position.y - 48.0f,
        32.0f,
        48.0f
    };
}

Rectangle Character::GetCollisionBounds() const {
    if (!m_collisionConfig.enabled) {
        return Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    return Rectangle{
        m_position.x - (m_collisionConfig.width * 0.5f) + m_collisionConfig.offsetX,
        m_position.y - m_collisionConfig.height + m_collisionConfig.offsetY,
        m_collisionConfig.width,
        m_collisionConfig.height
    };
}

} // namespace wildbound::entities

