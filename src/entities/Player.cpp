#include "Player.hpp"
#include "../world/World.hpp"
#include "../combat/CombatSystem.hpp"
#include "../items/ItemManager.hpp"
#include "../items/ConsumableSystem.hpp"
#include "../weapons/WeaponData.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::entities {

Player::Player(uint32_t id, const std::string& name)
    : Character(id, name, "player")
{
    // Register default primary attack
    combat::AttackDefinition slash;
    slash.id = "slash";
    slash.name = "Sword Slash";
    slash.damageType = combat::DamageType::Physical;
    slash.baseDamage = 25.0f;
    slash.range = 32.0f;
    slash.cooldown = 0.45f;
    slash.hitboxWidth = 36.0f;
    slash.hitboxHeight = 28.0f;
    slash.forwardOffset = 20.0f;
    slash.animationPrefix = "attack";
    m_attackSystem.RegisterAttack(slash);

    combat::AttackDefinition shoot;
    shoot.id = "shoot";
    shoot.name = "Bow Shot";
    shoot.damageType = combat::DamageType::Physical;
    shoot.baseDamage = 35.0f;
    shoot.range = 240.0f;
    shoot.cooldown = 0.5f;
    shoot.hitboxWidth = 32.0f;
    shoot.hitboxHeight = 32.0f;
    shoot.forwardOffset = 16.0f;
    shoot.animationPrefix = "attack";
    m_attackSystem.RegisterAttack(shoot);

    // Initialize default modular character appearance (Phase 7)
    m_modularAppearance.ApplyDefaultAppearance();
}

void Player::Spawn(Vector2 spawnPos) {
    m_position = spawnPos;
    m_velocity = Vector2{ 0.0f, 0.0f };
    m_movementState = graphics::animation::MovementState::Idle;
    m_health.Reset();
    m_attackSystem.ResetCooldown();
    Logger::Info("PLAYER", "Player spawned at world coordinates (" +
                 std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ")");
}

void Player::ResolveMovement(Vector2 delta, const world::World& world) {
    if (!m_health.IsAlive()) return;

    // 1. Horizontal Movement & Collision Resolution (X-Axis)
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

    // 2. Vertical Movement & Collision Resolution (Y-Axis) - enables smooth wall sliding!
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

    // 3. Map World Boundary Clamping
    Rectangle worldBounds = world.GetWorldBounds();
    if (worldBounds.width > 0.0f && worldBounds.height > 0.0f) {
        float halfW = m_collisionConfig.width * 0.5f;
        m_position.x = std::clamp(m_position.x, worldBounds.x + halfW, worldBounds.x + worldBounds.width - halfW);
        m_position.y = std::clamp(m_position.y, worldBounds.y + m_collisionConfig.height, worldBounds.y + worldBounds.height);
    }
}

void Player::Update(float dt) {
    if (!m_health.IsAlive()) {
        m_velocity = Vector2{ 0.0f, 0.0f };
        return;
    }

    m_attackSystem.Update(dt);
    m_controller.Update();

    Vector2 dir = m_controller.GetMovementDirection();
    bool isRunning = m_controller.IsRunning();

    float speed = m_stats.GetCurrentSpeed(isRunning);
    m_velocity = Vector2{ dir.x * speed, dir.y * speed };
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;
}

std::string Player::DetectSurfaceUnderFoot(const world::World& world) const {
    const auto* tilemap = world.GetTilemap();
    if (!tilemap) return "grass";

    Vector2 tileCoord = tilemap->WorldToTile(m_position);
    int col = static_cast<int>(tileCoord.x);
    int row = static_cast<int>(tileCoord.y);

    if (col < 0 || col >= tilemap->GetWidth() || row < 0 || row >= tilemap->GetHeight()) {
        return "grass";
    }

    const auto* groundLayer = tilemap->GetLayer("Ground");
    if (!groundLayer && tilemap->GetLayerCount() > 0) {
        groundLayer = tilemap->GetLayerAt(0);
    }
    if (!groundLayer) return "grass";

    int gid = groundLayer->GetTileGid(col, row);
    if (gid == 0 && tilemap->GetLayerCount() > 1) {
        const auto* layer1 = tilemap->GetLayerAt(1);
        if (layer1) gid = layer1->GetTileGid(col, row);
    }

    const auto* tileset = tilemap->FindTilesetForGid(gid);
    std::string tsName = tileset ? tileset->GetName() : "";
    std::transform(tsName.begin(), tsName.end(), tsName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::string mapName = tilemap->GetName();
    std::transform(mapName.begin(), mapName.end(), mapName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // 1. Direct tile property if present
    if (tileset) {
        int localId = tileset->GidToLocalId(gid);
        if (tileset->HasTileProperty(localId, "surface")) {
            return tileset->GetTileProperty(localId, "surface");
        }
    }

    // 2. Biome tileset heuristics
    if (tsName.find("desert") != std::string::npos || mapName.find("desert") != std::string::npos) {
        if (gid >= 33 && gid <= 48) return "stone";
        return "sand";
    }
    if (tsName.find("snow") != std::string::npos || mapName.find("snow") != std::string::npos) {
        if (gid >= 25 && gid <= 32) return "water";
        return "snow";
    }
    if (tsName.find("swamp") != std::string::npos || mapName.find("swamp") != std::string::npos) {
        if (gid >= 25 && gid <= 32) return "water";
        return "mud";
    }
    if (tsName.find("mountain") != std::string::npos || mapName.find("mountain") != std::string::npos) {
        if (gid >= 9 && gid <= 16) return "dirt";
        return "stone";
    }
    if (tsName.find("interior") != std::string::npos || mapName.find("interior") != std::string::npos || mapName.find("village") != std::string::npos) {
        if (gid >= 1 && gid <= 16) return "wood";
        return "stone";
    }

    // Forest default tileset mapping
    if (gid >= 9 && gid <= 16) return "dirt";
    if (gid >= 17 && gid <= 24) return "stone";
    if (gid >= 25 && gid <= 32) return "water";
    if (gid >= 33 && gid <= 40) return "wood";

    return "grass";
}

void Player::UpdateWithWorld(float dt, const world::World& world, const resources::AssetManager& assetMgr, combat::CombatSystem* combatSys, audio::AudioSystem* audioSys) {
    if (!m_active) return;

    m_attackSystem.Update(dt);

    if (!m_health.IsAlive()) {
        m_velocity = Vector2{ 0.0f, 0.0f };
        m_animController.Update(dt, assetMgr);
        return;
    }

    // 1. Process Input
    m_controller.Update();
    Vector2 dir = m_controller.GetMovementDirection();
    bool isRunning = m_controller.IsRunning();

    // Update Weapon Controller (Phase 24)
    m_weaponController.Update(dt);

    // 2. Process Attack Input Request
    if (m_controller.IsAttackRequested() && combatSys) {
        TryAttack(*combatSys, "slash");
    }

    // 3. Calculate Velocity (incorporating equipment speed modifiers)
    float currentSpeed = m_stats.GetCurrentSpeed(isRunning) * m_equipmentSystem.GetSpeedModifier();
    m_velocity = Vector2{ dir.x * currentSpeed, dir.y * currentSpeed };

    // 4. Perform Substepped / Axis-Decoupled Collision Movement
    Vector2 delta{ m_velocity.x * dt, m_velocity.y * dt };
    ResolveMovement(delta, world);

    // 5. Update Animation State Machine
    m_animStateMachine.Update(m_velocity, isRunning, m_animController, assetMgr);
    m_movementState = m_animStateMachine.GetCurrentMovementState();
    m_facing = m_animStateMachine.GetCurrentDirection();

    // 6. Footstep Cadence Audio Integration
    float moveSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
    if (moveSpeed > 5.0f && m_health.IsAlive()) {
        float stepInterval = isRunning ? 0.26f : 0.40f;
        m_footstepTimer += dt;
        if (m_footstepTimer >= stepInterval) {
            m_footstepTimer = 0.0f;
            if (audioSys) {
                std::string surface = DetectSurfaceUnderFoot(world);
                audioSys->DispatchEvent("player.footstep." + surface);
            }
        }
    } else {
        m_footstepTimer = 0.15f; // Ready to step when movement starts
    }

    // 7. Update Animation Playback Frame
    m_animController.Update(dt, assetMgr);
}

float Player::GetEquippedWeaponDamage() const {
    const auto* wpn = m_equipmentSystem.GetActiveWeaponInstance();
    return wpn ? wpn->GetEffectiveDamage() : 0.0f;
}

float Player::GetEquippedCritChance() const {
    const auto* wpn = m_equipmentSystem.GetActiveWeaponInstance();
    return wpn ? wpn->GetEffectiveCritChance() : 0.05f;
}

float Player::GetEquippedCritMultiplier() const {
    const auto* wpn = m_equipmentSystem.GetActiveWeaponInstance();
    return wpn ? wpn->GetEffectiveCritMultiplier() : 1.5f;
}

bool Player::HasHitTargetThisAttack(uint64_t targetId) const {
    return m_weaponController.HasHitTarget(targetId);
}

void Player::RecordHitTargetThisAttack(uint64_t targetId) {
    m_weaponController.RecordHitTarget(targetId);
}

bool Player::SwitchWeapon(EquipmentSlot targetSlot, const weapons::WeaponManager* weaponMgr) {
    if (!m_weaponController.CanSwitchWeapon()) {
        return false;
    }
    return m_equipmentSystem.SwitchWeapon(targetSlot, &m_modularAppearance, weaponMgr);
}

bool Player::ToggleWeapon(const weapons::WeaponManager* weaponMgr) {
    if (!m_weaponController.CanSwitchWeapon()) {
        return false;
    }
    return m_equipmentSystem.ToggleActiveWeapon(&m_modularAppearance, weaponMgr);
}

bool Player::TryAttack(combat::CombatSystem& combatSys, const std::string& attackId) {
    if (!m_health.IsAlive()) return false;

    const auto* equippedWpn = m_equipmentSystem.GetEquippedWeaponConfig();
    const auto* weaponInst = m_equipmentSystem.GetActiveWeaponInstance();

    // Phase 19/23/24: Check if a Bow is equipped
    if (equippedWpn && equippedWpn->category == weapons::WeaponCategory::Bow) {
        if (!m_attackSystem.CanAttack(attackId)) {
            if (m_weaponController.GetAttackState() == weapons::WeaponAttackState::Idle && m_weaponController.GetCooldownRemaining() <= 0.0f) {
                m_attackSystem.ResetCooldown();
            } else {
                return false;
            }
        }

        // Phase 23 Ammunition Verification: Bow attack requires arrows
        bool hasAmmo = false;
        if (m_inventory.RemoveItemById("arrow_iron_001", 1)) {
            hasAmmo = true;
        } else {
            // Check any slot with ammo
            for (size_t s = 0; s < m_inventory.GetCapacity(); ++s) {
                const auto& slot = m_inventory.GetSlot(s);
                if (slot.has_value() && slot->quantity > 0 &&
                    (slot->definitionId.find("arrow") != std::string::npos ||
                     slot->definitionId.find("ammo") != std::string::npos)) {
                    m_inventory.RemoveItem(s, 1);
                    hasAmmo = true;
                    break;
                }
            }
        }

        if (!hasAmmo) {
            Logger::Warn("COMBAT", "Cannot fire bow: Out of arrows!");
            return false;
        }

        float cooldown = weaponInst ? weaponInst->GetEffectiveAttackCooldown() : 0.45f;
        float speed = weaponInst ? weaponInst->GetEffectiveProjectileSpeed() : 480.0f;
        float damage = weaponInst ? weaponInst->GetEffectiveDamage() : 35.0f;

        m_attackSystem.ExecuteAttack(attackId);
        m_weaponController.Reset();
        m_weaponController.StartMeleeAttack(cooldown, 0.05f, 0.05f, 0.10f);

        Vector2 spawnPos = m_position;
        combatSys.GetProjectileSystem().SpawnArrow(
            spawnPos, m_facing, speed, damage, GetCombatantId(), GetFaction()
        );
        return true;
    }

    // Melee attack cycle
    if (!m_attackSystem.CanAttack(attackId)) {
        return false;
    }

    m_weaponController.Reset();
    float cooldown = weaponInst ? weaponInst->GetEffectiveAttackCooldown() : 0.45f;
    m_weaponController.StartMeleeAttack(cooldown, 0.08f, 0.12f, 0.15f);

    return combatSys.ProcessAttack(this, attackId, m_facing);
}

bool Player::UseConsumable(size_t inventorySlot, const items::ItemManager* itemMgr, audio::AudioSystem* audioSys) {
    if (!m_health.IsAlive()) return false;
    return items::ConsumableSystem::UseConsumable(*this, inventorySlot, itemMgr, audioSys) == items::ItemUseResult::Success;
}

void Player::OnDamageReceived(const combat::DamageEvent& evt) {
    Logger::Info("PLAYER", "Player received " + std::to_string(evt.finalDamage) +
                 " " + combat::DamageTypeToString(evt.damageType) +
                 " damage! Remaining HP: " + std::to_string(m_health.GetCurrentHealth()));
}

void Player::OnDeath(const combat::DeathEvent& evt) {
    (void)evt;
    Logger::Warn("PLAYER", "Player has fallen in battle!");
    m_velocity = Vector2{ 0.0f, 0.0f };
}

void Player::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    if (!m_active) return;
    m_modularAppearance.Render(renderer, assetMgr, m_position, m_facing, m_animController);
}

} // namespace wildbound::entities
