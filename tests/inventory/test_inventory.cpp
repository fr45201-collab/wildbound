#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "core/Logger.hpp"
#include "items/ItemData.hpp"
#include "items/ItemManager.hpp"
#include "items/Inventory.hpp"
#include "items/ConsumableSystem.hpp"
#include "world/World.hpp"
#include "world/WorldItem.hpp"
#include "world/LootContainer.hpp"
#include "entities/Player.hpp"
#include "combat/CombatSystem.hpp"
#include "weapons/WeaponManager.hpp"
#include "weapons/WeaponData.hpp"
#include "audio/AudioSystem.hpp"

using namespace wildbound;
using core::Logger;

static int g_passedTests = 0;
static int g_failedTests = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] ASSERTION FAILED: " << (msg) << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            g_failedTests++; \
            return false; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "[RUNNING] " << #fn << "...\n"; \
        if (fn()) { \
            std::cout << "[PASS] " << #fn << "\n"; \
            g_passedTests++; \
        } else { \
            std::cerr << "[FAIL] " << #fn << "\n"; \
        } \
    } while (0)

// ----------------------------------------------------------------------------
// Test 1: Item Definition & Ammunition Registry
// ----------------------------------------------------------------------------
bool TestItemDefinitionAndAmmunition() {
    items::ItemManager itemMgr;
    bool init = itemMgr.Initialize("data/items/item_catalog.json");
    TEST_ASSERT(init, "ItemManager should successfully initialize from catalog");
    itemMgr.RegisterDefaultAmmunition();

    // Verify arrow_iron_001
    const auto* arrow = itemMgr.GetItem("arrow_iron_001");
    TEST_ASSERT(arrow != nullptr, "arrow_iron_001 should be registered");
    TEST_ASSERT(arrow->category == items::ItemCategory::Ammunition, "Arrow category must be Ammunition");
    TEST_ASSERT(arrow->stackable == true, "Arrow must be stackable");
    TEST_ASSERT(arrow->maxStackSize == 30, "Arrow max stack size must be 30");
    TEST_ASSERT(arrow->ammunition.compatibleWeaponType == "bow", "Arrow compatible weapon must be bow");

    // Verify health potion
    const auto* pot = itemMgr.GetItem("con_health_potion_small_001");
    TEST_ASSERT(pot != nullptr, "Small health potion must be registered");
    TEST_ASSERT(pot->category == items::ItemCategory::Consumable, "Potion category must be Consumable");
    TEST_ASSERT(pot->consumable.healAmount == 50.0f, "Small health potion heals 50 HP");
    TEST_ASSERT(pot->maxStackSize == 10, "Potion max stack size is 10");

    return true;
}

// ----------------------------------------------------------------------------
// Test 2: Fixed 24-Slot Inventory Capacity Enforcement
// ----------------------------------------------------------------------------
bool TestInventoryCapacityEnforcement() {
    items::Inventory inv(24);
    TEST_ASSERT(inv.GetCapacity() == 24, "Inventory capacity must be 24 slots");
    TEST_ASSERT(inv.GetFilledSlotCount() == 0, "Initial inventory must have 0 filled slots");
    TEST_ASSERT(inv.GetFreeSlotCount() == 24, "Initial inventory must have 24 free slots");
    TEST_ASSERT(!inv.IsFull(), "New inventory must not be full");

    for (size_t i = 0; i < 24; ++i) {
        TEST_ASSERT(inv.IsSlotEmpty(i), "Slot must be empty");
    }

    // Fill 24 slots with non-stackable items
    for (size_t i = 0; i < 24; ++i) {
        items::ItemInstance wpn;
        wpn.definitionId = "sword_iron_001";
        wpn.quantity = 1;
        int rem = inv.AddItem(wpn);
        TEST_ASSERT(rem == 0, "Adding item to non-full inventory must succeed with 0 remainder");
    }

    TEST_ASSERT(inv.GetFilledSlotCount() == 24, "Inventory must now have 24 filled slots");
    TEST_ASSERT(inv.GetFreeSlotCount() == 0, "Inventory must have 0 free slots");
    TEST_ASSERT(inv.IsFull(), "Inventory must report IsFull == true");

    // Attempting to add 25th non-stackable item
    items::ItemInstance overflowWpn;
    overflowWpn.definitionId = "sword_iron_001";
    overflowWpn.quantity = 1;
    int rem = inv.AddItem(overflowWpn);
    TEST_ASSERT(rem == 1, "Adding item to full inventory must return full quantity as remainder");

    return true;
}

// ----------------------------------------------------------------------------
// Test 3: Stacking Logic & Max Stack Limits
// ----------------------------------------------------------------------------
bool TestItemStackingLogic() {
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");
    itemMgr.RegisterDefaultAmmunition();

    items::Inventory inv(24);

    // Add 50 gold coins (max stack 99)
    int rem1 = inv.AddItem("col_gold_coin_001", 50, &itemMgr);
    TEST_ASSERT(rem1 == 0, "Adding 50 coins should fit in slot 0");
    TEST_ASSERT(inv.GetSlot(0)->quantity == 50, "Slot 0 should contain 50 coins");
    TEST_ASSERT(inv.GetFilledSlotCount() == 1, "Only 1 slot filled");

    // Add 60 more gold coins: 49 merge into slot 0 (reaching 99), 11 go to slot 1
    int rem2 = inv.AddItem("col_gold_coin_001", 60, &itemMgr);
    TEST_ASSERT(rem2 == 0, "Adding 60 coins should completely fit");
    TEST_ASSERT(inv.GetSlot(0)->quantity == 99, "Slot 0 must be capped at max stack 99");
    TEST_ASSERT(inv.GetSlot(1)->quantity == 11, "Slot 1 must contain remaining 11 coins");
    TEST_ASSERT(inv.GetFilledSlotCount() == 2, "2 slots filled");
    TEST_ASSERT(inv.GetItemCount("col_gold_coin_001") == 110, "Total count must be 110 coins");

    // Add 35 iron arrows (max stack 30): 30 in slot 2, 5 in slot 3
    int rem3 = inv.AddItem("arrow_iron_001", 35, &itemMgr);
    TEST_ASSERT(rem3 == 0, "Adding 35 arrows should completely fit");
    TEST_ASSERT(inv.GetSlot(2)->quantity == 30, "Slot 2 capped at max stack 30");
    TEST_ASSERT(inv.GetSlot(3)->quantity == 5, "Slot 3 has remaining 5 arrows");
    TEST_ASSERT(inv.GetItemCount("arrow_iron_001") == 35, "Total arrows count must be 35");

    return true;
}

// ----------------------------------------------------------------------------
// Test 4: Stack Splitting & Merging
// ----------------------------------------------------------------------------
bool TestStackSplittingAndMerging() {
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    items::Inventory inv(24);
    inv.AddItem("col_gold_coin_001", 80, &itemMgr);
    TEST_ASSERT(inv.GetSlot(0)->quantity == 80, "Slot 0 has 80 coins");

    // Split 30 coins from Slot 0 to Slot 1
    bool splitOk = inv.SplitStack(0, 1, 30);
    TEST_ASSERT(splitOk, "SplitStack of 30 coins should succeed");
    TEST_ASSERT(inv.GetSlot(0)->quantity == 50, "Slot 0 must now have 50 coins");
    TEST_ASSERT(inv.GetSlot(1)->quantity == 30, "Slot 1 must now have 30 coins");

    // Merge Slot 1 back into Slot 0
    int merged = inv.MergeStacks(1, 0, &itemMgr);
    TEST_ASSERT(merged == 30, "MergeStacks should move all 30 coins back to Slot 0");
    TEST_ASSERT(inv.GetSlot(0)->quantity == 80, "Slot 0 restored to 80 coins");
    TEST_ASSERT(inv.IsSlotEmpty(1), "Slot 1 should now be empty");

    return true;
}

// ----------------------------------------------------------------------------
// Test 5: Ground Loot Spawning & Representation
// ----------------------------------------------------------------------------
bool TestGroundLootSpawning() {
    world::World world;
    Vector2 spawnPos{ 150.0f, 200.0f };

    world::WorldItem* wi = world.SpawnWorldItem("con_health_potion_small_001", spawnPos, 2);
    TEST_ASSERT(wi != nullptr, "SpawnWorldItem must return a valid pointer");
    TEST_ASSERT(wi->GetPosition().x == 150.0f && wi->GetPosition().y == 200.0f, "Position matches spawn coordinate");
    TEST_ASSERT(wi->GetItem().definitionId == "con_health_potion_small_001", "Item definition matches");
    TEST_ASSERT(wi->GetItem().quantity == 2, "Quantity matches 2");
    TEST_ASSERT(!wi->IsCollected(), "New ground item is not collected");
    TEST_ASSERT(world.GetWorldItems().size() == 1, "World has 1 active ground item");

    return true;
}

// ----------------------------------------------------------------------------
// Test 6: Two-Phase Transactional Item Dropping
// ----------------------------------------------------------------------------
bool TestTransactionalItemDropping() {
    world::World world;
    items::Inventory inv(24);
    inv.AddItem("arrow_iron_001", 20);

    Vector2 playerPos{ 100.0f, 100.0f };
    Vector2 playerFacing{ 1.0f, 0.0f }; // Facing East

    // Drop 5 arrows
    world::WorldItem* dropped = inv.DropItem(0, 5, world, playerPos, playerFacing);
    TEST_ASSERT(dropped != nullptr, "DropItem should spawn WorldItem");
    TEST_ASSERT(dropped->GetItem().definitionId == "arrow_iron_001", "Dropped item is arrow_iron_001");
    TEST_ASSERT(dropped->GetItem().quantity == 5, "Dropped quantity is 5");
    TEST_ASSERT(dropped->GetPosition().x == 140.0f, "Dropped 40 units in front of player");
    TEST_ASSERT(inv.GetSlot(0)->quantity == 15, "Inventory quantity deducted to 15");

    // Drop remaining 15 arrows via DropAll
    world::WorldItem* droppedAll = inv.DropAll(0, world, playerPos, playerFacing);
    TEST_ASSERT(droppedAll != nullptr, "DropAll should spawn WorldItem");
    TEST_ASSERT(droppedAll->GetItem().quantity == 15, "Dropped all 15 remaining arrows");
    TEST_ASSERT(inv.IsSlotEmpty(0), "Slot 0 must now be empty");
    TEST_ASSERT(world.GetWorldItems().size() == 2, "World now contains 2 dropped items");

    return true;
}

// ----------------------------------------------------------------------------
// Test 7: Proximity Ground Loot Pickup
// ----------------------------------------------------------------------------
bool TestProximityPickup() {
    world::World world;
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    entities::Player player(1, "TestPlayer");
    player.SetPosition(Vector2{ 100.0f, 100.0f });

    // Spawn an item within pickup radius (e.g. at 110, 100; distance 10 < 36)
    world.SpawnWorldItem("col_gold_coin_001", Vector2{ 110.0f, 100.0f }, 25);
    // Spawn another item far away (distance 300 > 36)
    world.SpawnWorldItem("col_gold_coin_001", Vector2{ 400.0f, 400.0f }, 50);

    int pickedUp = world.ProcessPickups(player, &itemMgr);
    TEST_ASSERT(pickedUp == 25, "Player should pick up only the nearby 25 coins");
    TEST_ASSERT(player.GetInventory().GetItemCount("col_gold_coin_001") == 25, "Player inventory contains 25 coins");

    // Process world cleanup
    world.UpdateWorldItems(0.016f);
    TEST_ASSERT(world.GetWorldItems().size() == 1, "Collected item was purged, leaving only the distant item");

    return true;
}

// ----------------------------------------------------------------------------
// Test 8: Consumables & Healing Validation
// ----------------------------------------------------------------------------
bool TestConsumablesAndHealing() {
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    entities::Player player(1, "TestPlayer");
    auto& health = player.GetHealth();
    health.SetMaxHealth(100.0f);
    health.SetCurrentHealth(50.0f); // 50 HP damaged

    // Add health potion
    player.GetInventory().AddItem("con_health_potion_small_001", 2, &itemMgr);
    TEST_ASSERT(player.GetInventory().GetSlot(0)->quantity == 2, "Player has 2 health potions");

    // Use consumable
    bool used = player.UseConsumable(0, &itemMgr);
    TEST_ASSERT(used, "Using health potion when damaged should succeed");
    TEST_ASSERT(health.GetCurrentHealth() == 100.0f, "Health restored to 100 HP (+50)");
    TEST_ASSERT(player.GetInventory().GetSlot(0)->quantity == 1, "Potion quantity decremented to 1");

    // Attempt to use potion at full health (Over-healing prevention)
    items::ItemUseResult res = items::ConsumableSystem::UseConsumable(player, 0, &itemMgr);
    TEST_ASSERT(res == items::ItemUseResult::FullHealth, "Result must be FullHealth");
    TEST_ASSERT(player.GetInventory().GetSlot(0)->quantity == 1, "Potion must NOT be consumed at full health");

    return true;
}

// ----------------------------------------------------------------------------
// Test 9: Stamina Consumable
// ----------------------------------------------------------------------------
bool TestStaminaConsumable() {
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");

    entities::Player player(1, "TestPlayer");
    player.SetStamina(40.0f); // Stamina at 40 / 100

    player.GetInventory().AddItem("con_stamina_potion_001", 1, &itemMgr);
    bool used = player.UseConsumable(0, &itemMgr);
    TEST_ASSERT(used, "Using stamina potion should succeed");
    TEST_ASSERT(player.GetStamina() == 90.0f, "Stamina increased from 40 to 90 (+50)");
    TEST_ASSERT(player.GetInventory().IsSlotEmpty(0), "Stamina potion consumed");

    return true;
}

// ----------------------------------------------------------------------------
// Test 10: Bow Ammunition Consumption in Combat
// ----------------------------------------------------------------------------
bool TestBowAmmunitionConsumption() {
    items::ItemManager itemMgr;
    itemMgr.Initialize("data/items/item_catalog.json");
    itemMgr.RegisterDefaultAmmunition();
    weapons::WeaponManager weaponMgr;
    weaponMgr.Initialize("data/weapons/weapon_catalog.json");

    entities::Player player(1, "ArcherPlayer");
    combat::CombatSystem combatSys;

    // Equip Bow
    player.GetEquipmentSystem().Equip("bow_wooden_hunting_001", itemMgr, &player, &weaponMgr);

    // 1. Attempt to attack without arrows
    bool attackNoAmmo = player.TryAttack(combatSys, "shoot");
    TEST_ASSERT(!attackNoAmmo, "Bow attack must fail when player has 0 arrows in inventory");

    // 2. Add 3 arrows to inventory
    player.GetInventory().AddItem("arrow_iron_001", 3);
    TEST_ASSERT(player.GetInventory().GetItemCount("arrow_iron_001") == 3, "Inventory has 3 arrows");

    // 3. Attack with arrows present
    bool attackWithAmmo = player.TryAttack(combatSys, "shoot");
    TEST_ASSERT(attackWithAmmo, "Bow attack must succeed when arrows are available");
    TEST_ASSERT(player.GetInventory().GetItemCount("arrow_iron_001") == 2, "Arrow count decremented to 2");
    TEST_ASSERT(combatSys.GetProjectileSystem().GetActiveProjectileCount() == 1, "Projectile spawned in projectile system");

    return true;
}

// ----------------------------------------------------------------------------
// Test 11: Loot Container Transfer (Chest LootAll)
// ----------------------------------------------------------------------------
bool TestLootContainerTransfer() {
    world::LootContainer chest("chest_test_001", "Ancient Chest", Vector2{ 200.0f, 200.0f });
    chest.GetInventory().AddItem("col_gold_coin_001", 75);
    chest.GetInventory().AddItem("arrow_iron_001", 15);
    chest.GetInventory().AddItem("con_health_potion_small_001", 2);

    entities::Player player(1, "LootingPlayer");
    TEST_ASSERT(player.GetInventory().GetFilledSlotCount() == 0, "Player starts empty");

    // Loot all items from chest into player inventory
    chest.Open();
    int transferred = chest.LootAll(player.GetInventory());
    TEST_ASSERT(transferred == 92, "All 92 items transferred (75 coins + 15 arrows + 2 potions)");
    TEST_ASSERT(chest.GetInventory().GetFilledSlotCount() == 0, "Chest is now empty");
    TEST_ASSERT(player.GetInventory().GetItemCount("col_gold_coin_001") == 75, "Player received 75 coins");
    TEST_ASSERT(player.GetInventory().GetItemCount("arrow_iron_001") == 15, "Player received 15 arrows");
    TEST_ASSERT(player.GetInventory().GetItemCount("con_health_potion_small_001") == 2, "Player received 2 potions");

    return true;
}

// ----------------------------------------------------------------------------
// Main Entry Point
// ----------------------------------------------------------------------------
int main() {
    std::cout << "============================================================\n";
    std::cout << " WILDBOUND PHASE 23: LOOT & INVENTORY SYSTEM TEST SUITE\n";
    std::cout << "============================================================\n";

    RUN_TEST(TestItemDefinitionAndAmmunition);
    RUN_TEST(TestInventoryCapacityEnforcement);
    RUN_TEST(TestItemStackingLogic);
    RUN_TEST(TestStackSplittingAndMerging);
    RUN_TEST(TestGroundLootSpawning);
    RUN_TEST(TestTransactionalItemDropping);
    RUN_TEST(TestProximityPickup);
    RUN_TEST(TestConsumablesAndHealing);
    RUN_TEST(TestStaminaConsumable);
    RUN_TEST(TestBowAmmunitionConsumption);
    RUN_TEST(TestLootContainerTransfer);

    std::cout << "============================================================\n";
    std::cout << " SUMMARY: " << g_passedTests << " Passed, " << g_failedTests << " Failed\n";
    std::cout << "============================================================\n";

    return (g_failedTests == 0) ? 0 : 1;
}

