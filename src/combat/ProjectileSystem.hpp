#pragma once

#include "CombatTypes.hpp"
#include "../graphics/animation/Direction.hpp"
#include <raylib.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

namespace wildbound::graphics {
    class Renderer;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::combat {

class Combatant;
class CombatRules;
class DamageSystem;
class CombatEventDispatcher;

struct Projectile {
    uint64_t id = 0;
    Vector2 position{ 0.0f, 0.0f };
    Vector2 velocity{ 0.0f, 0.0f };
    float rotation = 0.0f; // in degrees
    float baseDamage = 25.0f;
    DamageType damageType = DamageType::Physical;
    CombatantId sourceId = 0;
    Faction faction = Faction::Players;
    float lifetime = 0.0f;
    float maxLifetime = 2.0f;
    float hitRadius = 14.0f;
    std::string textureId = "tex_ammo_arrow_broadhead_001_held";
    bool active = true;
};

/**
 * High-Performance Ranged Projectile Management Subsystem.
 * Simulates arrows and physical projectiles, performs hurtbox intersection detection,
 * applies damage via DamageSystem, and plays impact audio.
 */
class ProjectileSystem {
public:
    ProjectileSystem();
    ~ProjectileSystem() = default;

    void SpawnArrow(Vector2 origin,
                    graphics::animation::Direction facing,
                    float speed = 450.0f,
                    float damage = 30.0f,
                    CombatantId sourceId = 1,
                    Faction faction = Faction::Players);

    void Update(float dt,
                const std::unordered_map<CombatantId, Combatant*>& combatants,
                const CombatRules& rules,
                DamageSystem& damageSys,
                CombatEventDispatcher* dispatcher = nullptr,
                audio::AudioSystem* audioSys = nullptr);

    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr);

    [[nodiscard]] size_t GetActiveProjectileCount() const;
    [[nodiscard]] const std::vector<Projectile>& GetProjectiles() const { return m_projectiles; }
    void Clear();

private:
    std::vector<Projectile> m_projectiles;
    uint64_t m_nextId = 1;
};

} // namespace wildbound::combat

