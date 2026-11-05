#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include "items/ItemData.hpp"
#include "items/ItemInstance.hpp"
#include "items/Inventory.hpp"
#include "items/ItemManager.hpp"
#include "weapons/WeaponData.hpp"
#include "weapons/WeaponManager.hpp"
#include "entities/Player.hpp"
#include "entities/EquipmentSystem.hpp"
#include "combat/CombatSystem.hpp"
#include "combat/DamageSystem.hpp"
#include "combat/ProjectileSystem.hpp"
#include "world/World.hpp"
#include "world/WorldItem.hpp"
#include "world/LootContainer.hpp"
#include "audio/AudioSystem.hpp"

using namespace wildbound;

void TestItemDefinitionsAndInstances() {
    std::cout << "[TEST] 1. Item Definitions & Runtime Instances..." << std::endl;

    items::ItemManager itemMgr;
    bool loaded = itemMgr.Initialize("data/items/item_catalog.json");
    assert(loaded);
    assert(itemMgr.GetItemCount() >= 40);

    const auto* healthPot = itemMgr.GetItem("con_health_potion_small_001");
    assert(healthPot != nullptr);
    assert(healthPot->category == items::ItemCategory::Consumable);
    assert(healthPot->stackable);
    assert(healthPot->maxStackSize == 10);
    (void)loaded;
    (void)healthPot;

    // Create runtime instance
    items::ItemInstance inst1(101, "con_health_potion_small_001", 5);
    assert(inst1.IsValid());
    assert(inst1.instanceId == 101);
    assert(inst1.definitionId == "con_health_potion_small_001");
    assert(inst1.quantity == 5);

    std::cout << "  -> Passed: Item definitions and instances verified." << std::endl;
}

void TestInventoryOperations() {
    std::cout << "[TEST] 2. Authoritative Inventory Management..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    items::Inventory inv(4); // 4-slot small test container
    assert(inv.GetCapacity() == 4);
    assert(inv.GetFreeSlotCount() == 4);
    assert(!inv.IsFull());

    // Add stackable items
    int overflow = inv.AddItem("col_gold_coin_001", 50, itemMgr);
    assert(overflow == 0);
    assert(inv.GetFilledSlotCount() == 1);
    assert(inv.GetItemCount("col_gold_coin_001") == 50);

    // Add more of same stackable item -> should auto-stack into slot 0
    overflow = inv.AddItem("col_gold_coin_001", 30, itemMgr);
    assert(overflow == 0);
    assert(inv.GetFilledSlotCount() == 1);
    assert(inv.GetItemCount("col_gold_coin_001") == 80);

    // Fill remaining slots
    inv.AddItem("con_health_potion_small_001", 10, itemMgr); // slot 1
    inv.AddItem("con_bread_001", 5, itemMgr);                // slot 2
    inv.AddItem("mat_iron_ore_001", 15, itemMgr);            // slot 3
    assert(inv.IsFull());
    assert(inv.GetFreeSlotCount() == 0);

    // Attempting to add a new distinct item when full should be rejected
    overflow = inv.AddItem("col_silver_coin_001", 1, itemMgr);
    assert(overflow == 1);
    assert(!inv.HasItem("col_silver_coin_001"));

    // Stack splitting
    // Clear slot 3 to make room
    inv.RemoveItem(3, 15);
    assert(!inv.IsSlotEmpty(0));
    assert(inv.IsSlotEmpty(3));
    bool splitSuccess = inv.SplitStack(0, 3, 30);
    assert(splitSuccess);
    assert(inv.GetSlot(0)->quantity == 50);
    assert(inv.GetSlot(3)->quantity == 30);
    assert(inv.GetItemCount("col_gold_coin_001") == 80);

    // Slot swapping
    bool swapSuccess = inv.SwapSlots(1, 2);
    assert(swapSuccess);
    assert(inv.GetSlot(1)->definitionId == "con_bread_001");
    assert(inv.GetSlot(2)->definitionId == "con_health_potion_small_001");

    // Partial removal
    inv.RemoveItem(1, 2);
    assert(inv.GetSlot(1)->quantity == 3);

    // Complete removal
    inv.RemoveItem(1, 3);
    assert(inv.IsSlotEmpty(1));

    (void)splitSuccess;
    (void)swapSuccess;

    std::cout << "  -> Passed: Authoritative inventory operations verified." << std::endl;
}

void TestWorldItemsAndPickups() {
    std::cout << "[TEST] 3. World Items & Pickups..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    world::World testWorld;
    entities::Player player(1, "TestExplorer");
    player.Spawn(Vector2{ 100.0f, 100.0f });

    // Spawn world item within pickup radius (dist <= 36.0f)
    world::WorldItem* wi1 = testWorld.SpawnWorldItem("con_health_potion_small_001", Vector2{ 110.0f, 105.0f }, 3);
    assert(wi1 != nullptr);
    assert(!wi1->IsCollected());
    assert(wi1->CanPickup(player.GetPosition()));

    // Spawn world item outside pickup radius
    world::WorldItem* wi2 = testWorld.SpawnWorldItem("col_gold_coin_001", Vector2{ 300.0f, 300.0f }, 25);
    assert(wi2 != nullptr);
    assert(!wi2->CanPickup(player.GetPosition()));

    // Process pickups
    int pickedUp = testWorld.ProcessPickups(player, &itemMgr, nullptr);
    assert(pickedUp == 3);
    assert(wi1->IsCollected());
    assert(!wi2->IsCollected());
    assert(player.GetInventory().GetItemCount("con_health_potion_small_001") == 3);

    // Update world items to trigger collected item garbage collection
    testWorld.UpdateWorldItems(0.016f);
    assert(testWorld.GetWorldItems().size() == 1);
    assert(testWorld.GetWorldItems()[0]->GetItem().definitionId == "col_gold_coin_001");

    // Full inventory rejection test
    player.GetInventory().Clear();
    player.GetInventory().SetCapacity(1);
    player.GetInventory().AddItem("mat_iron_ore_001", 1, itemMgr);
    assert(player.GetInventory().IsFull());

    world::WorldItem* wi3 = testWorld.SpawnWorldItem("col_ruby_001", Vector2{ 100.0f, 100.0f }, 1);
    assert(wi3->CanPickup(player.GetPosition()));
    int rejectedPickup = testWorld.ProcessPickups(player, &itemMgr, nullptr);
    assert(rejectedPickup == 0);
    assert(!wi3->IsCollected()); // Remained on ground!

    (void)wi1;
    (void)wi2;
    (void)wi3;
    (void)pickedUp;
    (void)rejectedPickup;

    std::cout << "  -> Passed: World pickups and transactional safety verified." << std::endl;
}

void TestLootContainers() {
    std::cout << "[TEST] 4. Loot Containers (Chests & Caches)..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    world::World testWorld;
    entities::Player player(1, "TestExplorer");
    player.Spawn(Vector2{ 200.0f, 200.0f });

    // Spawn an unlocked chest
    world::LootContainer* chest1 = testWorld.SpawnLootContainer("chest_01", Vector2{ 210.0f, 210.0f }, "Wood Chest");
    assert(chest1 != nullptr);
    assert(!chest1->IsOpen());
    assert(!chest1->IsLocked());

    chest1->GetInventory().AddItem("col_gold_coin_001", 50, itemMgr);
    chest1->GetInventory().AddItem("con_health_potion_small_001", 2, itemMgr);

    // Open and loot
    bool opened = chest1->TryOpen(player);
    assert(opened);
    assert(chest1->IsOpen());

    int looted = chest1->LootAll(player, &itemMgr, nullptr);
    assert(looted == 52);
    assert(player.GetInventory().GetItemCount("col_gold_coin_001") == 50);
    assert(player.GetInventory().GetItemCount("con_health_potion_small_001") == 2);
    assert(chest1->GetInventory().GetFilledSlotCount() == 0);

    // Spawn locked chest requiring "key_dungeon_001"
    world::LootContainer* lockedChest = testWorld.SpawnLootContainer("chest_locked", "Iron Strongbox", Vector2{ 200.0f, 200.0f }, true, "key_dungeon_001");
    assert(lockedChest->IsLocked());

    // Player does not have key -> open fails
    bool failedOpen = lockedChest->TryOpen(player);
    assert(!failedOpen);
    assert(!lockedChest->IsOpen());

    // Give player the required key
    player.GetInventory().AddItem("key_dungeon_001", 1, itemMgr);
    bool successOpen = lockedChest->TryOpen(player);
    assert(successOpen);
    assert(lockedChest->IsOpen());
    assert(!lockedChest->IsLocked());

    (void)opened;
    (void)failedOpen;
    (void)successOpen;
    (void)looted;

    std::cout << "  -> Passed: Loot container locking, unlocking, and transfers verified." << std::endl;
}

void TestEquipmentAndDamageMitigation() {
    std::cout << "[TEST] 5. Equipment & Armor Damage Mitigation..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    entities::Player player(1, "ArmoredKnight");
    player.GetHealth().SetMaxHealth(100.0f);
    player.GetHealth().SetCurrentHealth(100.0f);

    auto& equip = player.GetEquipmentSystem();
    assert(equip.GetTotalDefense() == 0.0f);
    assert(equip.GetDamageReduction() == 0.0f);

    // Equip Iron Cuirass (Chest) and Iron Helmet (Head)
    bool eq1 = equip.Equip("armor_iron_cuirass", itemMgr, &player);
    assert(eq1);
    assert(equip.HasEquipped(entities::EquipmentSlot::Chest));

    bool eq2 = equip.Equip("helm_iron_001", itemMgr, &player);
    assert(eq2);
    assert(equip.HasEquipped(entities::EquipmentSlot::Head));

    float totalDef = equip.GetTotalDefense();
    assert(totalDef > 0.0f); // Iron cuirass (25) + iron helm (10) = 35

    float expectedReduction = totalDef / (totalDef + 100.0f);
    float actualReduction = equip.GetDamageReduction();
    assert(std::abs(actualReduction - expectedReduction) < 0.001f);

    // Test DamageSystem mitigation against Combatant
    combat::DamageRequest req;
    req.baseDamage = 50.0f;
    req.damageType = combat::DamageType::Physical;

    combat::DamageSystem dmgSys;
    combat::DamageCalculationResult res = dmgSys.CalculateDamage(req, &player);
    float expectedDmg = 50.0f * (1.0f - actualReduction);
    assert(std::abs(res.finalDamage - expectedDmg) < 0.05f);

    // Unequip helmet
    equip.Unequip(entities::EquipmentSlot::Head, &player.GetInventory());
    assert(!equip.HasEquipped(entities::EquipmentSlot::Head));
    assert(equip.GetTotalDefense() < totalDef);

    (void)eq1;
    (void)eq2;

    std::cout << "  -> Passed: Equipment defense calculation and damage reduction verified." << std::endl;
}

void TestWeaponCombatAndProjectiles() {
    std::cout << "[TEST] 6. Weapon Combat & Bow Projectiles..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    weapons::WeaponManager weaponMgr;
    weaponMgr.Initialize("data/weapons/weapon_catalog.json");

    combat::CombatSystem combatSys;
    combatSys.Initialize("data/combat/combat_rules.json", "data/combat/attacks.json");

    entities::Player player(1, "ArcherPlayer");
    combatSys.RegisterCombatant(&player);

    // 1. Equip Iron Sword -> Melee slash
    player.GetEquipmentSystem().Equip("sword_iron_broadsword_001", itemMgr, &player, &weaponMgr);
    assert(player.GetEquipmentSystem().HasEquipped(entities::EquipmentSlot::Weapon));

    bool meleeAttack = player.TryAttack(combatSys, "slash");
    assert(meleeAttack);
    assert(combatSys.GetProjectileSystem().GetProjectiles().empty());

    // Reset attack cooldown for next attack test
    player.GetAttackSystem().ResetCooldown();

    // 2. Equip Short Bow -> Fires Arrow Projectile
    player.GetEquipmentSystem().Equip("bow_wooden_hunting_001", itemMgr, &player, &weaponMgr);
    assert(player.GetEquipmentSystem().GetEquippedWeaponConfig() != nullptr);
    assert(player.GetEquipmentSystem().GetEquippedWeaponConfig()->category == weapons::WeaponCategory::Bow);

    // Provide arrows for the bow
    player.GetInventory().AddItem(items::ItemInstance{ 50, "arrow_iron_001", 10 });

    bool bowAttack = player.TryAttack(combatSys, "slash");
    assert(bowAttack);
    assert(combatSys.GetProjectileSystem().GetProjectiles().size() == 1);

    (void)meleeAttack;
    (void)bowAttack;

    const auto& arrow = combatSys.GetProjectileSystem().GetProjectiles()[0];
    assert(arrow.active);
    assert(arrow.baseDamage > 0.0f);
    assert(arrow.sourceId == player.GetCombatantId());

    // Update projectile flight
    Vector2 initialPos = arrow.position;
    combatSys.Update(0.1f, nullptr);
    const auto& arrowAfter = combatSys.GetProjectileSystem().GetProjectiles()[0];
    assert(arrowAfter.position.y > initialPos.y); // Downward facing flight

    std::cout << "  -> Passed: Sword melee and bow arrow projectile firing verified." << std::endl;
}

void TestConsumableUsage() {
    std::cout << "[TEST] 7. Consumables & Resource Usage..." << std::endl;

    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    entities::Player player(1, "InjuredPlayer");
    player.GetHealth().SetMaxHealth(100.0f);
    player.GetHealth().SetCurrentHealth(40.0f); // 40 / 100 HP

    player.GetInventory().AddItem("con_health_potion_small_001", 3, itemMgr);
    assert(player.GetInventory().GetItemCount("con_health_potion_small_001") == 3);

    // Use health potion (restores 50 HP)
    bool used = player.UseConsumable(0, &itemMgr, nullptr);
    assert(used);
    assert(player.GetHealth().GetCurrentHealth() == 90.0f);
    assert(player.GetInventory().GetItemCount("con_health_potion_small_001") == 2);

    // Use food bread (restores 15 HP -> caps at 100 max HP)
    player.GetInventory().AddItem("con_bread_001", 1, itemMgr);
    bool foodUsed = player.UseConsumable(1, &itemMgr, nullptr);
    assert(foodUsed);
    assert(player.GetHealth().GetCurrentHealth() == 100.0f);
    assert(player.GetInventory().GetItemCount("con_bread_001") == 0);

    (void)used;
    (void)foodUsed;

    std::cout << "  -> Passed: Consumable usage and health restoration verified." << std::endl;
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "  WILDBOUND PHASE 19 — GAMEPLAY INTEGRATION SUITE " << std::endl;
    std::cout << "==================================================" << std::endl;

    try {
        TestItemDefinitionsAndInstances();
        TestInventoryOperations();
        TestWorldItemsAndPickups();
        TestLootContainers();
        TestEquipmentAndDamageMitigation();
        TestWeaponCombatAndProjectiles();
        TestConsumableUsage();

        std::cout << "==================================================" << std::endl;
        std::cout << "  ALL PHASE 19 GAMEPLAY INTEGRATION TESTS PASSED! " << std::endl;
        std::cout << "==================================================" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}

