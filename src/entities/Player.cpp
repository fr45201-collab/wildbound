#include "Player.hpp"
#include "../world/World.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::entities {

Player::Player(uint32_t id, const std::string& name)
    : Character(id, name, "player")
{
}

void Player::Spawn(Vector2 spawnPos) {
    m_position = spawnPos;
    m_velocity = Vector2{ 0.0f, 0.0f };
    m_movementState = graphics::animation::MovementState::Idle;
    Logger::Info("PLAYER", "Player spawned at world coordinates (" +
                 std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ")");
}

void Player::ResolveMovement(Vector2 delta, const world::World& world) {
    // 1. Horizontal Movement & Collision Resolution (X-Axis)
    if (std::fabs(delta.x) > 0.0001f) {
        m_position.x += delta.x;

        if (m_collisionConfig.enabled) {
            Rectangle colBounds = GetCollisionBounds();
            auto collisions = world.GetCollisionBoxes(colBounds);

            for (const auto& box : collisions) {
                if (box.IsSolid() && box.Intersects(colBounds)) {
                    if (delta.x > 0.0f) {
                        // Moving Right: align right edge of player to left edge of obstacle
                        m_position.x = box.bounds.x - (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                    } else if (delta.x < 0.0f) {
                        // Moving Left: align left edge of player to right edge of obstacle
                        m_position.x = (box.bounds.x + box.bounds.width) + (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                    }
                    colBounds = GetCollisionBounds();
                }
            }
        }
    }

    // 2. Vertical Movement & Collision Resolution (Y-Axis) - enables smooth wall sliding!
    if (std::fabs(delta.y) > 0.0001f) {
        m_position.y += delta.y;

        if (m_collisionConfig.enabled) {
            Rectangle colBounds = GetCollisionBounds();
            auto collisions = world.GetCollisionBoxes(colBounds);

            for (const auto& box : collisions) {
                if (box.IsSolid() && box.Intersects(colBounds)) {
                    if (delta.y > 0.0f) {
                        // Moving Down: align bottom edge of player to top edge of obstacle
                        m_position.y = box.bounds.y - m_collisionConfig.offsetY;
                    } else if (delta.y < 0.0f) {
                        // Moving Up: align top edge of player to bottom edge of obstacle
                        m_position.y = (box.bounds.y + box.bounds.height) + m_collisionConfig.height - m_collisionConfig.offsetY;
                    }
                    colBounds = GetCollisionBounds();
                }
            }
        }
    }

    // 3. Map World Boundary Clamping
    Rectangle worldBounds = world.GetWorldBounds();
    if (worldBounds.width > 0.0f && worldBounds.height > 0.0f) {
        float halfW = m_collisionConfig.width * 0.5f;
        m_position.x = std::clamp(m_position.x, worldBounds.x + halfW, worldBounds.x + worldBounds.width - halfW);
        m_position.y = std::clamp(m_position.y, worldBounds.y + m_collisionConfig.height, worldBounds.y + worldBounds.height);
    }
}

void Player::Update(float dt) {
    // Fallback simple update when no world is supplied
    m_controller.Update();
    Vector2 dir = m_controller.GetMovementDirection();
    bool isRunning = m_controller.IsRunning();

    float speed = m_stats.GetCurrentSpeed(isRunning);
    m_velocity = Vector2{ dir.x * speed, dir.y * speed };
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;
}

void Player::UpdateWithWorld(float dt, const world::World& world, const resources::AssetManager& assetMgr) {
    if (!m_active) return;

    // 1. Process Input
    m_controller.Update();
    Vector2 dir = m_controller.GetMovementDirection();
    bool isRunning = m_controller.IsRunning();

    // 2. Calculate Velocity
    float currentSpeed = m_stats.GetCurrentSpeed(isRunning);
    m_velocity = Vector2{ dir.x * currentSpeed, dir.y * currentSpeed };

    // 3. Perform Substepped / Axis-Decoupled Collision Movement
    Vector2 delta{ m_velocity.x * dt, m_velocity.y * dt };
    ResolveMovement(delta, world);

    // 4. Update Animation State Machine
    m_animStateMachine.Update(m_velocity, isRunning, m_animController, assetMgr);
    m_movementState = m_animStateMachine.GetCurrentMovementState();
    m_facing = m_animStateMachine.GetCurrentDirection();

    // 5. Update Animation Playback Frame
    m_animController.Update(dt, assetMgr);
}

} // namespace wildbound::entities

