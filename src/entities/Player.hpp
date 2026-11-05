#pragma once

#include "Character.hpp"
#include "CharacterController.hpp"
#include "ModularCharacter.hpp"
#include "EquipmentSystem.hpp"
#include "../items/Inventory.hpp"
#include "../combat/Combatant.hpp"
#include "../combat/HealthComponent.hpp"
#include "../combat/AttackSystem.hpp"
#include "../weapons/WeaponController.hpp"

namespace wildbound::world {
    class World;
}

namespace wildbound::combat {
    class CombatSystem;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::entities {

/**
 * Playable Character Entity.
 * Integrates input controller, swept AABB collision resolution with wall sliding,
 * camera follow targeting, animation state machine updates, inventory, equipment, and Combatant interface.
 */
class Player : public Character, public combat::Combatant {
public:
    explicit Player(uint32_t id = 1, const std::string& name = "Explorer");
    ~Player() override = default;

    void Update(float dt) override;
    void UpdateWithWorld(float dt, const world::World& world, const resources::AssetManager& assetMgr, combat::CombatSystem* combatSys = nullptr, audio::AudioSystem* audioSys = nullptr);
    void Render(graphics::Renderer& renderer, resources::AssetManager& assetMgr) override;

    // Surface & Footstep Detection
    [[nodiscard]] std::string DetectSurfaceUnderFoot(const world::World& world) const;

    // Inventory & Equipment (Phase 19/24)
    items::Inventory& GetInventory() { return m_inventory; }
    const items::Inventory& GetInventory() const { return m_inventory; }

    EquipmentSystem& GetEquipment() { return m_equipmentSystem; }
    const EquipmentSystem& GetEquipment() const { return m_equipmentSystem; }
    EquipmentSystem& GetEquipmentSystem() { return m_equipmentSystem; }
    const EquipmentSystem& GetEquipmentSystem() const { return m_equipmentSystem; }

    // Consumable Usage
    bool UseConsumable(size_t inventorySlot, const items::ItemManager* itemMgr = nullptr, audio::AudioSystem* audioSys = nullptr);

    // Stamina
    float GetStamina() const { return m_stamina; }
    float GetMaxStamina() const { return m_maxStamina; }
    void SetStamina(float val) { m_stamina = std::clamp(val, 0.0f, m_maxStamina); }

    // Modular Appearance
    ModularCharacter& GetModularAppearance() { return m_modularAppearance; }
    const ModularCharacter& GetModularAppearance() const { return m_modularAppearance; }

    // Orientation & Facing
    Vector2 GetFacingDirection() const {
        return graphics::animation::DirectionToVector(GetFacing());
    }
    void SetFacingDirection(Vector2 dir) {
        SetFacing(graphics::animation::VectorToDirection(dir, GetFacing()));
    }
    void SetFacingDirection(graphics::animation::Direction dir) {
        SetFacing(dir);
    }

    // Collider linkage (Phase 20)
    uint64_t GetColliderId() const { return m_colliderId; }
    void SetColliderId(uint64_t id) { m_colliderId = id; }

    // Controls
    CharacterController& GetController() { return m_controller; }
    const CharacterController& GetController() const { return m_controller; }
    void SetInputManager(const input::InputManager* inputMgr) { m_controller.SetInputManager(inputMgr); }

    // Spawn
    void Spawn(Vector2 spawnPos);

    // Collision Resolution Helper (exposed for unit testing)
    void ResolveMovement(Vector2 delta, const world::World& world);

    // Combatant Implementation
    combat::CombatantId GetCombatantId() const override { return GetId(); }
    combat::CombatantType GetCombatantType() const override { return combat::CombatantType::Player; }
    combat::Faction GetFaction() const override { return m_faction; }
    void SetFaction(combat::Faction faction) { m_faction = faction; }

    Vector2 GetCombatPosition() const override { return GetPosition(); }
    Rectangle GetHurtbox() const override { return GetWorldBounds(); }
    graphics::animation::Direction GetCombatFacing() const override { return GetFacing(); }

    bool IsAlive() const override { return m_health.IsAlive(); }
    combat::HealthComponent& GetHealth() override { return m_health; }
    const combat::HealthComponent& GetHealth() const override { return m_health; }

    combat::AttackSystem& GetAttackSystem() override { return m_attackSystem; }
    const combat::AttackSystem& GetAttackSystem() const override { return m_attackSystem; }

    float GetDefenseRating() const override { return m_equipmentSystem.GetTotalDefense(); }
    float GetDamageReduction() const override { return m_equipmentSystem.GetDamageReduction(); }

    // Weapon Combat Scaling & Single-Hit Target Tracking (Phase 24)
    float GetEquippedWeaponDamage() const override;
    float GetEquippedCritChance() const override;
    float GetEquippedCritMultiplier() const override;
    bool HasHitTargetThisAttack(uint64_t targetId) const override;
    void RecordHitTargetThisAttack(uint64_t targetId) override;

    weapons::WeaponController& GetWeaponController() { return m_weaponController; }
    const weapons::WeaponController& GetWeaponController() const { return m_weaponController; }

    bool SwitchWeapon(EquipmentSlot targetSlot, const weapons::WeaponManager* weaponMgr = nullptr);
    bool ToggleWeapon(const weapons::WeaponManager* weaponMgr = nullptr);

    void OnDamageReceived(const combat::DamageEvent& evt) override;
    void OnDeath(const combat::DeathEvent& evt) override;

    // Combat Action
    bool TryAttack(combat::CombatSystem& combatSys, const std::string& attackId = "slash");

private:
    CharacterController m_controller;
    combat::HealthComponent m_health{ 100.0f, 100.0f };
    combat::AttackSystem m_attackSystem;
    weapons::WeaponController m_weaponController;
    combat::Faction m_faction = combat::Faction::Players;
    ModularCharacter m_modularAppearance;
    items::Inventory m_inventory{ 24 };
    EquipmentSystem m_equipmentSystem;
    float m_stamina = 100.0f;
    float m_maxStamina = 100.0f;
    uint64_t m_colliderId = 0;
    float m_footstepTimer = 0.0f;
};

} // namespace wildbound::entities
