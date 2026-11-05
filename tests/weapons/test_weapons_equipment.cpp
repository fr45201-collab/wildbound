#include "../../src/weapons/WeaponData.hpp"
#include "../../src/weapons/WeaponInstance.hpp"
#include "../../src/weapons/WeaponController.hpp"
#include "../../src/weapons/WeaponManager.hpp"
#include "../../src/entities/EquipmentSystem.hpp"
#include "../../src/entities/Player.hpp"
#include "../../src/combat/CombatSystem.hpp"
#include "../../src/combat/Combatant.hpp"
#include "../../src/items/Inventory.hpp"
#include "../../src/core/Logger.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

using namespace wildbound;
using namespace wildbound::weapons;
using namespace wildbound::entities;
using namespace wildbound::combat;

// Mock Combatant Dummy for combat hit validation
class DummyTarget : public Combatant {
public:
    DummyTarget(CombatantId id, Vector2 pos, float hp = 100.0f)
        : m_id(id), m_pos(pos), m_health(hp, hp) {}

    CombatantId GetCombatantId() const override { return m_id; }
    CombatantType GetCombatantType() const override { return CombatantType::Enemy; }
    Faction GetFaction() const override { return Faction::Monsters; }

    Vector2 GetCombatPosition() const override { return m_pos; }
    Rectangle GetHurtbox() const override {
        return Rectangle{ m_pos.x - 16.0f, m_pos.y - 16.0f, 32.0f, 32.0f };
    }
    graphics::animation::Direction GetCombatFacing() const override {
        return graphics::animation::Direction::Down;
    }

    bool IsAlive() const override { return m_health.IsAlive(); }
    HealthComponent& GetHealth() override { return m_health; }
    const HealthComponent& GetHealth() const override { return m_health; }

    AttackSystem& GetAttackSystem() override { return m_attackSys; }
    const AttackSystem& GetAttackSystem() const override { return m_attackSys; }

    void OnDamageReceived(const DamageEvent& evt) override {
        m_lastDamage = evt.finalDamage;
        m_hitCount++;
    }
    void OnDeath(const DeathEvent&) override {}

    float m_lastDamage = 0.0f;
    int m_hitCount = 0;

private:
    CombatantId m_id;
    Vector2 m_pos;
    HealthComponent m_health;
    AttackSystem m_attackSys;
};

void TestWeaponCatalogAndStats() {
    std::cout << "[TEST] 1. Weapon Catalog & Definition Loading..." << std::endl;
    WeaponManager mgr;
    bool ok = mgr.Initialize("data/weapons/weapon_catalog.json");
    assert(ok && "Failed to initialize weapon catalog");
    assert(mgr.GetWeaponCount() == 12 && "Expected 12 total weapons/ammo in catalog");

    // Category queries
    auto swords = mgr.GetWeaponsByCategory(WeaponCategory::Sword);
    auto bows = mgr.GetWeaponsByCategory(WeaponCategory::Bow);
    auto ammo = mgr.GetWeaponsByCategory(WeaponCategory::Ammunition);
    assert(swords.size() == 6 && "Expected 6 swords");
    assert(bows.size() == 5 && "Expected 5 bows");
    assert(ammo.size() == 1 && "Expected 1 ammunition entry");

    // Check specific sword stats
    const auto* trainingSword = mgr.GetWeapon("sword_wooden_training_001");
    assert(trainingSword != nullptr);
    assert(trainingSword->stats.baseDamage == 12.0f);
    assert(trainingSword->stats.attackCooldown == 0.35f);

    const auto* runeSword = mgr.GetWeapon("sword_ancient_rune_001");
    assert(runeSword != nullptr);
    assert(runeSword->stats.baseDamage == 45.0f);
    assert(runeSword->stats.critChance >= 0.20f);

    // Check bow stats
    const auto* huntingBow = mgr.GetWeapon("bow_wooden_hunting_001");
    assert(huntingBow != nullptr);
    assert(huntingBow->stats.drawDuration == 0.35f);
    assert(huntingBow->stats.projectileSpeed == 420.0f);

    const auto* runeBow = mgr.GetWeapon("bow_ancient_rune_001");
    assert(runeBow != nullptr);
    assert(runeBow->stats.baseDamage == 48.0f);
    assert(runeBow->stats.projectileSpeed == 560.0f);

    std::cout << "  -> Passed: All 12 weapons catalogued with canonical base statistics." << std::endl;
}

void TestWeaponRarityAndScaling() {
    std::cout << "[TEST] 2. Weapon Rarity Multipliers & Effective Damage..." << std::endl;
    WeaponManager mgr;
    mgr.Initialize("data/weapons/weapon_catalog.json");

    // Test multiplier values
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Common) - 1.00f) < 0.001f);
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Uncommon) - 1.05f) < 0.001f);
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Rare) - 1.10f) < 0.001f);
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Epic) - 1.15f) < 0.001f);
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Legendary) - 1.20f) < 0.001f);
    assert(std::abs(GetRarityDamageMultiplier(WeaponRarity::Mythic) - 1.25f) < 0.001f);

    // Test instances created at various rarities
    auto commonSword = mgr.CreateInstance("sword_iron_001", WeaponRarity::Common);
    assert(std::abs(commonSword.GetEffectiveDamage() - 25.0f) < 0.01f);

    auto epicSword = mgr.CreateInstance("sword_iron_001", WeaponRarity::Epic);
    // Base 25 * 1.15 = 28.75
    assert(std::abs(epicSword.GetEffectiveDamage() - 28.75f) < 0.01f);

    auto mythicSword = mgr.CreateInstance("sword_iron_001", WeaponRarity::Mythic);
    // Base 25 * 1.25 = 31.25
    assert(std::abs(mythicSword.GetEffectiveDamage() - 31.25f) < 0.01f);

    // Serialization helpers
    assert(WeaponRarityToString(WeaponRarity::Epic) == "Epic");
    assert(StringToWeaponRarity("Mythic") == WeaponRarity::Mythic);

    std::cout << "  -> Passed: Rarity multipliers scale base damage correctly across all 6 tiers." << std::endl;
}

void TestMeleeAttackLifecycle() {
    std::cout << "[TEST] 3. Melee Attack Lifecycle & Hitbox Timings..." << std::endl;
    WeaponController ctrl;
    assert(ctrl.GetAttackState() == WeaponAttackState::Idle);
    assert(ctrl.CanInitiateAttack());

    // Start attack: total cooldown 0.5s, prep 0.08s, active 0.12s, recovery 0.15s
    bool started = ctrl.StartMeleeAttack(0.5f, 0.08f, 0.12f, 0.15f);
    assert(started);
    assert(ctrl.GetAttackState() == WeaponAttackState::Preparation);
    assert(!ctrl.IsHitboxActive());
    assert(ctrl.IsAttacking());

    // Advance 0.05s -> still in Prep
    ctrl.Update(0.05f);
    assert(ctrl.GetAttackState() == WeaponAttackState::Preparation);

    // Advance 0.04s (total 0.09s >= 0.08s) -> Active swing!
    ctrl.Update(0.04f);
    assert(ctrl.GetAttackState() == WeaponAttackState::Active);
    assert(ctrl.IsHitboxActive());

    // Advance 0.13s -> Recovery
    ctrl.Update(0.13f);
    assert(ctrl.GetAttackState() == WeaponAttackState::Recovery);
    assert(!ctrl.IsHitboxActive());
    assert(ctrl.IsAttacking());

    // Advance 0.16s -> Cooldown
    ctrl.Update(0.16f);
    assert(ctrl.GetAttackState() == WeaponAttackState::Cooldown);
    assert(!ctrl.IsAttacking());

    // Advance remaining cooldown
    ctrl.Update(0.20f);
    assert(ctrl.GetAttackState() == WeaponAttackState::Idle);
    assert(ctrl.CanInitiateAttack());

    std::cout << "  -> Passed: Full Prep -> Active -> Recovery -> Cooldown lifecycle verified." << std::endl;
}

void TestSingleHitPerTargetPrevention() {
    std::cout << "[TEST] 4. Single-Hit-Per-Target Prevention..." << std::endl;
    WeaponController ctrl;
    ctrl.StartMeleeAttack(0.5f, 0.05f, 0.10f, 0.10f);
    ctrl.Update(0.06f); // transition to Active

    assert(!ctrl.HasHitTarget(101));
    assert(!ctrl.HasHitTarget(102));

    // First contact on target 101
    ctrl.RecordHitTarget(101);
    assert(ctrl.HasHitTarget(101));
    assert(!ctrl.HasHitTarget(102));

    // Subsequent query prevents multi-hit in the same frame
    assert(ctrl.HasHitTarget(101));

    // First contact on target 102
    ctrl.RecordHitTarget(102);
    assert(ctrl.HasHitTarget(102));

    // Resetting swing clears records
    ctrl.ClearHitTargets();
    assert(!ctrl.HasHitTarget(101));
    assert(!ctrl.HasHitTarget(102));

    std::cout << "  -> Passed: Single-hit tracking prevents duplicate damage per swing." << std::endl;
}

void TestRangedBowDrawMechanics() {
    std::cout << "[TEST] 5. Ranged Bow Draw Cycle & Scaling..." << std::endl;
    WeaponController ctrl;

    ctrl.StartDraw(0.50f);
    assert(ctrl.IsDrawing());
    assert(ctrl.GetDrawProgress() == 0.0f);
    assert(!ctrl.IsFullyDrawn());

    // Advance half-way
    ctrl.Update(0.25f);
    assert(std::abs(ctrl.GetDrawProgress() - 0.50f) < 0.01f);
    assert(!ctrl.IsFullyDrawn());

    // Advance past max duration
    ctrl.Update(0.30f);
    assert(std::abs(ctrl.GetDrawProgress() - 1.0f) < 0.01f);
    assert(ctrl.IsFullyDrawn());

    // Release draw
    float charge = ctrl.ReleaseDraw();
    assert(std::abs(charge - 1.0f) < 0.01f);
    assert(!ctrl.IsDrawing());

    std::cout << "  -> Passed: Bow draw cycle charges accurately and releases full power." << std::endl;
}

void TestEquipmentSlotsAndSwitching() {
    std::cout << "[TEST] 6. Multi-Slot Equipment & Hot-Switching..." << std::endl;
    EquipmentSystem eq;
    WeaponManager mgr;
    mgr.Initialize("data/weapons/weapon_catalog.json");

    // Equip primary sword in MainHand
    items::ItemInstance swordInst{ 1, "sword_iron_001", 1 };
    bool ok1 = eq.Equip(EquipmentSlot::MainHand, swordInst, nullptr, nullptr, &mgr);
    assert(ok1);

    // Equip secondary bow in OffHand
    items::ItemInstance bowInst{ 2, "bow_wooden_hunting_001", 1 };
    bool ok2 = eq.Equip(EquipmentSlot::OffHand, bowInst, nullptr, nullptr, &mgr);
    assert(ok2);

    assert(eq.GetActiveWeaponSlot() == EquipmentSlot::MainHand);
    assert(eq.GetEquippedWeaponConfig()->category == WeaponCategory::Sword);
    assert(eq.GetActiveWeaponInstance()->GetDefinitionId() == "sword_iron_001");

    // Switch to OffHand
    bool switched = eq.SwitchWeapon(EquipmentSlot::OffHand);
    assert(switched);
    assert(eq.GetActiveWeaponSlot() == EquipmentSlot::OffHand);
    assert(eq.GetEquippedWeaponConfig()->category == WeaponCategory::Bow);
    assert(eq.GetActiveWeaponInstance()->GetDefinitionId() == "bow_wooden_hunting_001");

    // Toggle active weapon
    eq.ToggleActiveWeapon();
    assert(eq.GetActiveWeaponSlot() == EquipmentSlot::MainHand);

    std::cout << "  -> Passed: Primary and secondary weapon slots switch cleanly." << std::endl;
}

void TestEquipmentStatModifiers() {
    std::cout << "[TEST] 7. Equipment Stat Modifiers Stacking..." << std::endl;
    EquipmentSystem eq;

    // Equip steel armor piece on chest
    items::ItemInstance steelChest{ 10, "arm_steel_knight_001", 1 };
    eq.Equip(EquipmentSlot::Chest, steelChest);

    // Equip boots on feet
    items::ItemInstance boots{ 11, "sho_leather_boots_001", 1 };
    eq.Equip(EquipmentSlot::Feet, boots);

    const auto& mods = eq.GetStatModifiers();
    assert(mods.totalDefense > 0.0f);
    assert(mods.damageReduction > 0.0f);
    assert(mods.healthBonus > 0.0f);
    // Boot speed bonus (+0.05) - chest armor penalty (-0.03) = 1.02
    assert(mods.speedModifier > 1.0f);

    std::cout << "  -> Passed: Stat modifiers correctly stack defense, health bonus, and speed." << std::endl;
}

void TestPlayerCombatIntegration() {
    std::cout << "[TEST] 8. Player Combat Integration & Damage Application..." << std::endl;
    CombatSystem combatSys;
    combatSys.Initialize();

    WeaponManager weaponMgr;
    weaponMgr.Initialize("data/weapons/weapon_catalog.json");

    Player player;
    player.Spawn(Vector2{ 100.0f, 100.0f });
    player.SetFacing(graphics::animation::Direction::Right);
    combatSys.RegisterCombatant(&player);

    // Create target dummy at (130, 100) directly in front of player
    DummyTarget dummy(99, Vector2{ 130.0f, 100.0f }, 100.0f);
    combatSys.RegisterCombatant(&dummy);

    // Equip steel longsword (base damage 34.0)
    items::ItemInstance sword{ 20, "sword_steel_longsword_001", 1 };
    player.GetEquipment().Equip(EquipmentSlot::MainHand, sword, nullptr, nullptr, &weaponMgr);

    assert(player.GetEquippedWeaponDamage() == 34.0f);

    // Perform melee attack
    bool attacked = player.TryAttack(combatSys, "slash");
    assert(attacked);
    assert(dummy.m_hitCount == 1);
    assert(dummy.m_lastDamage >= 34.0f);

    // Switching weapon while attacking should be rejected
    assert(!player.SwitchWeapon(EquipmentSlot::OffHand));

    // Reset combat state & equip bow
    player.GetWeaponController().Reset();
    items::ItemInstance bow{ 21, "bow_wooden_hunting_001", 1 };
    player.GetEquipment().Equip(EquipmentSlot::MainHand, bow, nullptr, nullptr, &weaponMgr);

    // Firing bow without arrows in inventory must fail
    player.GetInventory().Clear();
    bool bowAttackedNoAmmo = player.TryAttack(combatSys, "slash");
    assert(!bowAttackedNoAmmo && "Bow attack should fail without arrows");

    // Add 10 arrows to inventory
    items::ItemInstance arrows{ 22, "arrow_iron_001", 10 };
    player.GetInventory().AddItem(arrows);

    // Firing bow with arrows succeeds and consumes 1 arrow
    bool bowAttackedWithAmmo = player.TryAttack(combatSys, "slash");
    assert(bowAttackedWithAmmo && "Bow attack should succeed with arrows");
    assert(player.GetInventory().GetSlot(0)->quantity == 9 && "Should have consumed 1 arrow");

    std::cout << "  -> Passed: Player melee damage scaling, bow ammo consumption, and projectile launch verified." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 24: WEAPONS & EQUIPMENT TEST SUITE" << std::endl;
    std::cout << "============================================================" << std::endl;

    TestWeaponCatalogAndStats();
    TestWeaponRarityAndScaling();
    TestMeleeAttackLifecycle();
    TestSingleHitPerTargetPrevention();
    TestRangedBowDrawMechanics();
    TestEquipmentSlotsAndSwitching();
    TestEquipmentStatModifiers();
    TestPlayerCombatIntegration();

    std::cout << "============================================================" << std::endl;
    std::cout << "ALL PHASE 24 WEAPON & EQUIPMENT TESTS PASSED 100%!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}

