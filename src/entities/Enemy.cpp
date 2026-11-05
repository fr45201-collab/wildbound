#include "Enemy.hpp"
#include "../world/World.hpp"
#include "../combat/CombatSystem.hpp"
#include "../scripting/LuaManager.hpp"
#include "../core/Logger.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::entities {

Enemy::Enemy(uint32_t id, const std::string& name, const std::string& type)
    : Character(id, name, "enemy")
{
    m_config.id = type;
    m_config.name = name;
    m_animStateMachine.SetCharacterPrefix(type);
}

void Enemy::ApplyConfig(const EnemyConfig& cfg) {
    m_config = cfg;
    m_name = cfg.name;
    m_spritesheetId = cfg.spritesheet;
    m_stats = cfg.stats;
    m_collisionConfig = cfg.collision;
    m_health.Initialize(cfg.health);

    m_animStateMachine.SetCharacterPrefix(cfg.id);

    // Register default attack in AttackSystem
    combat::AttackDefinition atkDef;
    atkDef.id = cfg.combat.defaultAttack;
    atkDef.name = cfg.name + " Attack";
    atkDef.damageType = combat::DamageType::Physical;
    atkDef.baseDamage = (cfg.id == "goblin") ? 12.0f : 18.0f;
    atkDef.range = cfg.combat.attackRange;
    atkDef.cooldown = cfg.combat.attackCooldown;
    atkDef.hitboxWidth = 30.0f;
    atkDef.hitboxHeight = 24.0f;
    atkDef.forwardOffset = 18.0f;
    atkDef.animationPrefix = "attack";
    m_attackSystem.RegisterAttack(atkDef);

    // Apply animation clips
    for (const auto& [animKey, clipName] : cfg.animationClips) {
        if (animKey == "idle_down")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Down, clipName);
        if (animKey == "idle_up")    m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Up, clipName);
        if (animKey == "idle_left")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Left, clipName);
        if (animKey == "idle_right") m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Idle, graphics::animation::Direction::Right, clipName);
        if (animKey == "walk_down")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Down, clipName);
        if (animKey == "walk_up")    m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Up, clipName);
        if (animKey == "walk_left")  m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Left, clipName);
        if (animKey == "walk_right") m_animStateMachine.SetCustomClip(graphics::animation::MovementState::Walking, graphics::animation::Direction::Right, clipName);
    }
}

void Enemy::Spawn(Vector2 spawnPos) {
    m_position = spawnPos;
    m_spawnOrigin = spawnPos;
    m_patrolTarget = spawnPos;
    m_velocity = Vector2{ 0.0f, 0.0f };
    m_aiState = AIState::Idle;
    m_targetId = 0;
    m_health.Reset();
    m_attackSystem.ResetCooldown();
    m_deathTimer = 0.0f;
    m_pendingDestruction = false;
    m_active = true;
}

void Enemy::ResolveMovement(Vector2 delta, const world::World& world) {
    if (!m_health.IsAlive()) return;

    // 1. Horizontal Movement (X-Axis)
    if (std::fabs(delta.x) > 0.0001f) {
        m_position.x += delta.x;

        if (m_collisionConfig.enabled) {
            Rectangle colBounds = GetCollisionBounds();
            auto collisions = world.GetCollisionBoxes(colBounds);

            for (const auto& box : collisions) {
                if (box.IsSolid() && box.Intersects(colBounds)) {
                    if (delta.x > 0.0f) {
                        m_position.x = box.bounds.x - (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                    } else if (delta.x < 0.0f) {
                        m_position.x = (box.bounds.x + box.bounds.width) + (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                    }
                    colBounds = GetCollisionBounds();
                }
            }
        }
    }

    // 2. Vertical Movement (Y-Axis)
    if (std::fabs(delta.y) > 0.0001f) {
        m_position.y += delta.y;

        if (m_collisionConfig.enabled) {
            Rectangle colBounds = GetCollisionBounds();
            auto collisions = world.GetCollisionBoxes(colBounds);

            for (const auto& box : collisions) {
                if (box.IsSolid() && box.Intersects(colBounds)) {
                    if (delta.y > 0.0f) {
                        m_position.y = box.bounds.y - m_collisionConfig.offsetY;
                    } else if (delta.y < 0.0f) {
                        m_position.y = (box.bounds.y + box.bounds.height) + m_collisionConfig.height - m_collisionConfig.offsetY;
                    }
                    colBounds = GetCollisionBounds();
                }
            }
        }
    }

    // 3. World Boundary Clamping
    Rectangle worldBounds = world.GetWorldBounds();
    if (worldBounds.width > 0.0f && worldBounds.height > 0.0f) {
        float halfW = m_collisionConfig.width * 0.5f;
        m_position.x = std::clamp(m_position.x, worldBounds.x + halfW, worldBounds.x + worldBounds.width - halfW);
        m_position.y = std::clamp(m_position.y, worldBounds.y + m_collisionConfig.height, worldBounds.y + worldBounds.height);
    }
}

void Enemy::Update(float dt) {
    if (!m_health.IsAlive()) {
        m_velocity = Vector2{ 0.0f, 0.0f };
        m_aiState = AIState::Dead;
        m_deathTimer += dt;
        if (m_deathTimer >= 1.5f) {
            m_pendingDestruction = true;
        }
        return;
    }

    m_attackSystem.Update(dt);
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;
}

void Enemy::UpdateWithWorld(float dt, const world::World& world, combat::CombatSystem& combatSys,
                            const resources::AssetManager& assetMgr, scripting::LuaManager* luaMgr)
{
    if (m_pendingDestruction || !m_active) return;

    m_attackSystem.Update(dt);

    if (!m_health.IsAlive()) {
        m_velocity = Vector2{ 0.0f, 0.0f };
        m_aiState = AIState::Dead;
        m_deathTimer += dt;
        if (m_deathTimer >= 1.5f) {
            m_pendingDestruction = true;
            m_active = false;
        }
        m_animController.Update(dt, assetMgr);
        return;
    }

    // 1. AI Decision Update
    m_aiDecisionTimer -= dt;
    if (m_aiDecisionTimer <= 0.0f) {
        m_aiDecisionTimer = m_config.ai.decisionInterval;
        ExecuteAIDecision(combatSys, luaMgr);
    }

    // 2. Perform Movement & Collision Resolution
    Vector2 delta{ m_velocity.x * dt, m_velocity.y * dt };
    ResolveMovement(delta, world);

    // 3. Update Animation State Machine & Controller
    bool isRunning = (m_aiState == AIState::Chase || m_aiState == AIState::Retreat);
    m_animStateMachine.Update(m_velocity, isRunning, m_animController, assetMgr);
    m_movementState = m_animStateMachine.GetCurrentMovementState();
    m_facing = m_animStateMachine.GetCurrentDirection();

    m_animController.Update(dt, assetMgr);
}

void Enemy::ExecuteAIDecision(combat::CombatSystem& combatSys, scripting::LuaManager* luaMgr) {
    if (!m_health.IsAlive()) {
        m_aiState = AIState::Dead;
        m_velocity = Vector2{ 0.0f, 0.0f };
        return;
    }

    // Try Lua-Driven AI if initialized
    if (luaMgr && luaMgr->IsInitialized()) {
        try {
            sol::state& lua = luaMgr->GetState();
            sol::function updateAi = lua["wildbound"]["ai"]["update_enemy"];
            if (updateAi.valid()) {
                // Pass safe context table to Lua
                sol::table ctx = lua.create_table();
                ctx["enemy_id"] = GetId();
                ctx["x"] = m_position.x;
                ctx["y"] = m_position.y;
                ctx["spawn_x"] = m_spawnOrigin.x;
                ctx["spawn_y"] = m_spawnOrigin.y;
                ctx["hp_percent"] = m_health.GetHealthPercent();
                ctx["detection_radius"] = m_config.ai.detectionRadius;
                ctx["attack_radius"] = m_config.ai.attackRadius;
                ctx["retreat_hp"] = m_config.ai.retreatHealthPercent;
                ctx["state"] = AIStateToString(m_aiState);
                ctx["target_id"] = m_targetId;

                // Find closest target
                combat::Combatant* target = combatSys.FindClosestTarget(m_position, m_config.ai.detectionRadius, m_config.faction);
                if (target) {
                    Vector2 tPos = target->GetCombatPosition();
                    ctx["has_target"] = true;
                    ctx["target_x"] = tPos.x;
                    ctx["target_y"] = tPos.y;
                    ctx["target_id"] = target->GetCombatantId();
                    float dx = tPos.x - m_position.x;
                    float dy = tPos.y - m_position.y;
                    ctx["target_dist"] = std::sqrt(dx * dx + dy * dy);
                } else {
                    ctx["has_target"] = false;
                    ctx["target_dist"] = 99999.0f;
                }

                // Call Lua script function
                sol::table result = updateAi(ctx);
                if (result.valid()) {
                    std::string nextStateStr = result["next_state"].get_or(std::string("IDLE"));
                    m_aiState = StringToAIState(nextStateStr);
                    m_targetId = result["target_id"].get_or(combat::CombatantId(0));

                    float moveX = result["move_x"].get_or(0.0f);
                    float moveY = result["move_y"].get_or(0.0f);
                    bool wantAttack = result["request_attack"].get_or(false);
                    std::string atkId = result["attack_id"].get_or(m_config.combat.defaultAttack);

                    float speed = (m_aiState == AIState::Chase || m_aiState == AIState::Retreat)
                                      ? m_stats.runSpeed : m_stats.walkSpeed;

                    float lenSq = moveX * moveX + moveY * moveY;
                    if (lenSq > 0.0001f) {
                        float invLen = 1.0f / std::sqrt(lenSq);
                        m_velocity = Vector2{ moveX * invLen * speed, moveY * invLen * speed };
                    } else {
                        m_velocity = Vector2{ 0.0f, 0.0f };
                    }

                    if (wantAttack && target) {
                        combatSys.ProcessAttack(this, atkId, m_facing);
                    }
                    return;
                }
            }
        }
        catch (const std::exception& ex) {
            Logger::Error("AI", "Lua AI execution failed for enemy [" + std::to_string(GetId()) + "]: " + ex.what());
        }
    }

    // Robust C++ Fallback AI State Machine
    combat::Combatant* target = combatSys.FindClosestTarget(m_position, m_config.ai.detectionRadius, m_config.faction);

    // 1. Health check: Retreat if low HP
    if (m_health.GetHealthPercent() <= m_config.ai.retreatHealthPercent && target) {
        m_aiState = AIState::Retreat;
        m_targetId = target->GetCombatantId();

        // Flee away from target
        Vector2 tPos = target->GetCombatPosition();
        Vector2 fleeDir{ m_position.x - tPos.x, m_position.y - tPos.y };
        float len = std::sqrt(fleeDir.x * fleeDir.x + fleeDir.y * fleeDir.y);
        if (len > 0.0001f) {
            m_velocity = Vector2{ (fleeDir.x / len) * m_stats.runSpeed, (fleeDir.y / len) * m_stats.runSpeed };
        }
        return;
    }

    // 2. Target acquisition & Combat range check
    if (target) {
        m_targetId = target->GetCombatantId();
        Vector2 tPos = target->GetCombatPosition();
        float dx = tPos.x - m_position.x;
        float dy = tPos.y - m_position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= m_config.ai.attackRadius) {
            // In attack range!
            m_aiState = AIState::Attack;
            m_velocity = Vector2{ 0.0f, 0.0f };

            // Face target
            m_facing = VectorToDirection(Vector2{ dx, dy }, m_facing);

            // Execute attack through CombatSystem
            combatSys.ProcessAttack(this, m_config.combat.defaultAttack, m_facing);
        } else {
            // Chase target
            m_aiState = AIState::Chase;
            float invLen = 1.0f / dist;
            m_velocity = Vector2{ dx * invLen * m_stats.runSpeed, dy * invLen * m_stats.runSpeed };
        }
    } else {
        // No target: Patrol or Idle
        m_targetId = 0;
        float dSpawnX = m_position.x - m_spawnOrigin.x;
        float dSpawnY = m_position.y - m_spawnOrigin.y;
        float distToSpawn = std::sqrt(dSpawnX * dSpawnX + dSpawnY * dSpawnY);

        if (distToSpawn > m_config.ai.patrolRadius) {
            // Return to spawn origin
            m_aiState = AIState::Patrol;
            float dx = m_spawnOrigin.x - m_position.x;
            float dy = m_spawnOrigin.y - m_position.y;
            float len = std::sqrt(dx * dx + dy * dy);
            m_velocity = Vector2{ (dx / len) * m_stats.walkSpeed, (dy / len) * m_stats.walkSpeed };
        } else {
            m_aiState = AIState::Idle;
            m_velocity = Vector2{ 0.0f, 0.0f };
        }
    }
}

void Enemy::OnDamageReceived(const combat::DamageEvent& evt) {
    Logger::Info("ENEMY", m_name + " [" + std::to_string(GetId()) + "] took " +
                 std::to_string(evt.finalDamage) + " damage! HP: " +
                 std::to_string(m_health.GetCurrentHealth()) + "/" +
                 std::to_string(m_health.GetMaxHealth()));

    // Automatically acquire attacker as target if idle
    if (m_targetId == 0 && evt.attackerId != 0) {
        m_targetId = evt.attackerId;
        m_aiState = AIState::Chase;
    }
}

void Enemy::OnDeath(const combat::DeathEvent& evt) {
    (void)evt;
    Logger::Info("ENEMY", m_name + " [" + std::to_string(GetId()) + "] died in combat.");
    m_aiState = AIState::Dead;
    m_velocity = Vector2{ 0.0f, 0.0f };
    m_deathTimer = 0.0f;
}

} // namespace wildbound::entities

