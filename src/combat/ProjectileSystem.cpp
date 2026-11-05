#include "ProjectileSystem.hpp"
#include "Combatant.hpp"
#include "CombatRules.hpp"
#include "DamageSystem.hpp"
#include "HitDetection.hpp"
#include "../graphics/Renderer.hpp"
#include "../resources/AssetManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <cmath>

using wildbound::core::Logger;

namespace wildbound::combat {

ProjectileSystem::ProjectileSystem() = default;

void ProjectileSystem::SpawnArrow(Vector2 origin,
                                  graphics::animation::Direction facing,
                                  float speed,
                                  float damage,
                                  CombatantId sourceId,
                                  Faction faction)
{
    Projectile p;
    p.id = m_nextId++;
    p.position = origin;
    p.baseDamage = damage;
    p.sourceId = sourceId;
    p.faction = faction;
    p.lifetime = 0.0f;
    p.maxLifetime = 1.8f;
    p.active = true;
    p.textureId = "tex_ammo_arrow_broadhead_001_held";

    Vector2 dirVec = DirectionToVector(facing);
    p.velocity = Vector2{ dirVec.x * speed, dirVec.y * speed };

    switch (facing) {
        case graphics::animation::Direction::Right: p.rotation = 0.0f; break;
        case graphics::animation::Direction::Down:  p.rotation = 90.0f; break;
        case graphics::animation::Direction::Left:  p.rotation = 180.0f; break;
        case graphics::animation::Direction::Up:    p.rotation = 270.0f; break;
        default: p.rotation = 0.0f; break;
    }

    m_projectiles.push_back(p);
    Logger::Debug("PROJECTILE", "Spawned arrow projectile #" + std::to_string(p.id) + " at (" +
                  std::to_string(origin.x) + ", " + std::to_string(origin.y) + ")");
}

void ProjectileSystem::Update(float dt,
                              const std::unordered_map<CombatantId, Combatant*>& combatants,
                              const CombatRules& rules,
                              DamageSystem& damageSys,
                              CombatEventDispatcher* dispatcher,
                              audio::AudioSystem* audioSys)
{
    for (auto& p : m_projectiles) {
        if (!p.active) continue;

        p.lifetime += dt;
        if (p.lifetime >= p.maxLifetime) {
            p.active = false;
            continue;
        }

        // Advance position
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;

        Rectangle arrowHitbox = { p.position.x - 6.0f, p.position.y - 6.0f, 12.0f, 12.0f };

        // Test collisions against all registered hostile combatants
        for (const auto& [targetId, target] : combatants) {
            if (!target || !target->IsAlive() || target->GetCombatantId() == p.sourceId) {
                continue;
            }

            if (!rules.CanTarget(p.faction, target->GetFaction())) {
                continue;
            }

            Rectangle targetHurtbox = target->GetHurtbox();
            if (HitDetection::Intersects(arrowHitbox, targetHurtbox)) {
                Vector2 impactPoint = HitDetection::ComputeImpactPoint(arrowHitbox, targetHurtbox);

                DamageRequest req;
                req.sourceId = p.sourceId;
                req.targetId = targetId;
                req.damageType = p.damageType;
                req.baseDamage = p.baseDamage;
                req.damageMultiplier = 1.0f;
                req.impactPoint = impactPoint;

                damageSys.ApplyDamage(req, target, dispatcher);

                if (audioSys) {
                    audioSys->DispatchEvent("Combat.SwordImpact");
                }

                p.active = false;
                Logger::Info("PROJECTILE", "Arrow #" + std::to_string(p.id) + " hit target #" +
                             std::to_string(targetId) + " for " + std::to_string(p.baseDamage) + " damage.");
                break;
            }
        }
    }

    // Erase inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(), [](const Projectile& p) {
            return !p.active;
        }),
        m_projectiles.end()
    );
}

void ProjectileSystem::Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) {
    (void)renderer;
    for (const auto& p : m_projectiles) {
        if (!p.active) continue;

        Texture2D tex = assetMgr.GetTexture(p.textureId);
        if (tex.id > 0) {
            Rectangle src = { 0, 0, 32, 32 };
            Rectangle dest = { p.position.x, p.position.y, 24, 24 };
            Vector2 origin = { 12.0f, 12.0f };
            ::DrawTexturePro(tex, src, dest, origin, p.rotation, WHITE);
        } else {
            // Crisp fallback rendering: directional line + arrowhead
            float rad = p.rotation * (PI / 180.0f);
            Vector2 tip = { p.position.x + std::cos(rad) * 12.0f, p.position.y + std::sin(rad) * 12.0f };
            Vector2 tail = { p.position.x - std::cos(rad) * 12.0f, p.position.y - std::sin(rad) * 12.0f };
            ::DrawLineEx(tail, tip, 2.5f, Color{ 210, 185, 120, 255 });
            ::DrawCircleV(tip, 3.0f, Color{ 180, 50, 50, 255 });
        }
    }
}

size_t ProjectileSystem::GetActiveProjectileCount() const {
    size_t count = 0;
    for (const auto& p : m_projectiles) {
        if (p.active) count++;
    }
    return count;
}

void ProjectileSystem::Clear() {
    m_projectiles.clear();
}

} // namespace wildbound::combat

