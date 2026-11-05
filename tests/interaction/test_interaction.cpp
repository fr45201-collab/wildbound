#include "interaction/InteractionTypes.hpp"
#include "interaction/InteractionDefinition.hpp"
#include "interaction/InteractableComponent.hpp"
#include "interaction/InteractionPrompt.hpp"
#include "interaction/InteractionContext.hpp"
#include "interaction/InteractionEvent.hpp"
#include "interaction/InteractionValidator.hpp"
#include "interaction/InteractionExecutor.hpp"
#include "interaction/Door.hpp"
#include "interaction/Mechanism.hpp"
#include "interaction/InteractionManager.hpp"
#include "entities/Player.hpp"
#include "items/ItemManager.hpp"
#include "world/World.hpp"
#include "world/BuildingManager.hpp"
#include "world/LootContainer.hpp"
#include "world/WorldItem.hpp"
#include "physics/CollisionManager.hpp"
#include "input/InputManager.hpp"
#include "core/Logger.hpp"

#include <iostream>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <memory>
#include <cmath>

using namespace wildbound;
using namespace wildbound::interaction;

void TestInteractableRegistration() {
    std::cout << "[TEST] 1. Interactable Registration & Registry..." << std::endl;

    InteractionManager mgr;
    assert(mgr.GetInteractableCount() == 0);

    InteractableComponent comp1;
    comp1.position = Vector2{ 100.0f, 150.0f };
    comp1.definition.id = "test_chest_01";
    comp1.definition.displayName = "Ancient Oak Chest";
    comp1.definition.category = InteractionCategory::Container;
    comp1.definition.range = 48.0f;
    comp1.definition.priority = 10;

    InteractableId id1 = mgr.RegisterInteractable(comp1);
    assert(id1 != INVALID_INTERACTABLE_ID);
    assert(mgr.GetInteractableCount() == 1);

    auto* fetched = mgr.GetInteractable(id1);
    assert(fetched != nullptr);
    assert(fetched->definition.displayName == "Ancient Oak Chest");
    assert(fetched->CanInteract());

    // Register second interactable
    InteractableComponent comp2;
    comp2.position = Vector2{ 200.0f, 250.0f };
    comp2.definition.id = "test_lever_01";
    comp2.definition.category = InteractionCategory::Mechanism;
    InteractableId id2 = mgr.RegisterInteractable(comp2);
    assert(id2 != id1);
    assert(mgr.GetInteractableCount() == 2);

    // Unregister
    mgr.UnregisterInteractable(id1);
    assert(mgr.GetInteractableCount() == 1);
    assert(mgr.GetInteractable(id1) == nullptr);
    assert(mgr.GetInteractable(id2) != nullptr);

    // Clear
    mgr.ClearInteractables();
    assert(mgr.GetInteractableCount() == 0);

    std::cout << "  -> Registration & Registry PASSED." << std::endl;
}

void TestDiscoveryAndDeterministicRanking() {
    std::cout << "[TEST] 2. Proximity Discovery & Deterministic Candidate Selection..." << std::endl;

    InteractionManager mgr;
    Vector2 playerPos{ 100.0f, 100.0f };
    Vector2 playerFacing{ 1.0f, 0.0f }; // Facing East (+X)

    // Candidate A: 30 px away to the East (Directly in front, facing dot = +1.0)
    InteractableComponent compA;
    compA.id = 101;
    compA.position = Vector2{ 130.0f, 100.0f };
    compA.definition.id = "target_front";
    compA.definition.displayName = "Front Object";
    compA.definition.range = 50.0f;
    compA.definition.priority = 10;
    mgr.RegisterInteractable(compA);

    // Candidate B: 20 px away to the West (Directly behind, facing dot = -1.0)
    InteractableComponent compB;
    compB.id = 102;
    compB.position = Vector2{ 80.0f, 100.0f };
    compB.definition.id = "target_behind";
    compB.definition.displayName = "Behind Object";
    compB.definition.range = 50.0f;
    compB.definition.priority = 10;
    mgr.RegisterInteractable(compB);

    // Candidate C: 120 px away (Out of range!)
    InteractableComponent compC;
    compC.id = 103;
    compC.position = Vector2{ 220.0f, 100.0f };
    compC.definition.id = "target_far";
    compC.definition.displayName = "Far Object";
    compC.definition.range = 50.0f;
    compC.definition.priority = 10;
    mgr.RegisterInteractable(compC);

    auto player = std::make_unique<entities::Player>(1, "Tester");
    player->Spawn(playerPos);
    player->SetFacingDirection(playerFacing);

    InteractionContext ctx;
    ctx.player = player.get();

    mgr.Update(0.016f, ctx, nullptr);

    const auto& candidates = mgr.GetCurrentCandidates();
    assert(candidates.size() == 2); // compC rejected as out of range

    // Candidate in front must be ranked #1 despite Candidate B being 10px closer behind
    const auto* target = mgr.GetCurrentTarget();
    assert(target != nullptr);
    assert(target->id == 101); // Candidate A selected!
    assert(mgr.GetCurrentPrompt().visible);
    assert(mgr.GetCurrentPrompt().targetName == "Front Object");

    // Turn player around (facing West: -1.0, 0.0)
    player->SetFacingDirection(Vector2{ -1.0f, 0.0f });
    mgr.Update(0.016f, ctx, nullptr);

    assert(mgr.GetCurrentTarget() != nullptr);
    assert(mgr.GetCurrentTarget()->id == 102); // Candidate B selected now!
    assert(mgr.GetCurrentPrompt().targetName == "Behind Object");

    // Move player far away
    player->SetPosition(Vector2{ 1000.0f, 1000.0f });
    mgr.Update(0.016f, ctx, nullptr);
    assert(mgr.GetCurrentCandidates().empty());
    assert(mgr.GetCurrentTarget() == nullptr);
    assert(!mgr.GetCurrentPrompt().visible);

    std::cout << "  -> Discovery & Selection PASSED." << std::endl;
}

void TestValidationRules() {
    std::cout << "[TEST] 3. Precondition Validation Rules..." << std::endl;

    auto player = std::make_unique<entities::Player>(1, "Tester");
    player->Spawn(Vector2{ 50.0f, 50.0f });

    InteractionContext ctx;
    ctx.player = player.get();

    // 1. Invalid target
    assert(InteractionValidator::Validate(nullptr, player->GetPosition(), ctx) == InteractionResult::InvalidTarget);

    // 2. Disabled target
    InteractableComponent comp;
    comp.position = Vector2{ 50.0f, 60.0f };
    comp.definition.range = 40.0f;
    comp.definition.enabled = false;
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::TargetUnavailable);
    comp.definition.enabled = true;

    // 3. Out of range target
    comp.position = Vector2{ 50.0f, 150.0f }; // 100px away, range=40
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::OutOfRange);
    comp.position = Vector2{ 50.0f, 60.0f }; // 10px away

    // 4. Missing required item (Key)
    comp.definition.requiredItemId = "spe_celestial_key_001";
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::MissingRequirement);

    // Give key to player
    player->GetInventory().AddItem("spe_celestial_key_001", 1);
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::Success);

    // 5. Cooldown active
    comp.currentCooldown = 1.5f;
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::CooldownActive);
    comp.currentCooldown = 0.0f;

    // 6. Already executing
    comp.isExecuting = true;
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::AlreadyExecuting);
    comp.isExecuting = false;

    // 7. Dead player blocked
    player->GetHealth().ApplyDamage(player->GetHealth().GetMaxHealth());
    assert(player->GetHealth().IsDead());
    assert(InteractionValidator::Validate(&comp, player->GetPosition(), ctx) == InteractionResult::Blocked);

    std::cout << "  -> Validation Rules PASSED." << std::endl;
}

void TestDoorInteractions() {
    std::cout << "[TEST] 4. Door Interactions & Collision Synchronization..." << std::endl;

    physics::CollisionManager colMgr;
    Door door("door_01", "Oak Fort Door", Vector2{ 200.0f, 200.0f }, Rectangle{ 184.0f, 176.0f, 32.0f, 48.0f });

    // Register solid collider for closed door
    physics::Collider solidCol = physics::Collider::CreateStatic(0, physics::CollisionLayer::Interactable,
                                                                 door.GetBounds(), "door_solid");
    uint64_t colId = colMgr.RegisterCollider(solidCol);
    door.SetSolidColliderId(colId);

    assert(!door.IsOpen());
    assert(colMgr.GetCollider(colId)->IsEnabled() == true); // Solid collider active when closed

    // Open Door
    bool opened = door.Open(nullptr, &colMgr);
    assert(opened);
    assert(door.IsOpen());
    assert(colMgr.GetCollider(colId)->IsEnabled() == false); // Solid collider DISABLED when open!

    // Toggle Door (should close)
    bool closed = door.Toggle(nullptr, &colMgr);
    assert(closed);
    assert(!door.IsOpen());
    assert(colMgr.GetCollider(colId)->IsEnabled() == true); // Solid collider RE-ENABLED when closed!

    // Locked Door with Key Requirement
    Door lockedDoor("door_locked_01", "Vault Door", Vector2{ 300.0f, 300.0f }, Rectangle{ 284.0f, 276.0f, 32.0f, 48.0f },
                    DoorState::Locked, "spe_rusty_key_001");
    auto player = std::make_unique<entities::Player>(1, "Thief");
    player->Spawn(Vector2{ 300.0f, 320.0f });

    // Try without key
    assert(lockedDoor.IsLocked());
    assert(lockedDoor.TryInteract(*player, nullptr, &colMgr) == InteractionResult::MissingRequirement);
    assert(lockedDoor.IsLocked());

    // Give key to player
    player->GetInventory().AddItem("spe_rusty_key_001", 1);
    assert(lockedDoor.TryInteract(*player, nullptr, &colMgr) == InteractionResult::Success);
    assert(!lockedDoor.IsLocked());
    assert(lockedDoor.IsOpen()); // Unlocked and opened!

    std::cout << "  -> Door Interactions PASSED." << std::endl;
}

void TestContainerInteractions() {
    std::cout << "[TEST] 5. Chest & Container Transactions..." << std::endl;

    world::World world;
    auto* chest = world.SpawnLootContainer("test_chest", Vector2{ 150.0f, 150.0f }, "Treasury Chest");
    assert(chest != nullptr);
    assert(!chest->IsOpen());

    // Add items into chest
    chest->GetInventory().AddItem("col_gold_coin_001", 100);
    chest->GetInventory().AddItem("sword_iron_broadsword_001", 1);
    assert(chest->GetInventory().GetItemCount("col_gold_coin_001") == 100);
    assert(chest->GetInventory().GetItemCount("sword_iron_broadsword_001") == 1);

    auto player = std::make_unique<entities::Player>(1, "Looter");
    player->Spawn(Vector2{ 150.0f, 160.0f });

    InteractionManager mgr;
    InteractionContext ctx;
    ctx.player = player.get();
    ctx.world = &world;

    mgr.SyncWithWorld(ctx);
    mgr.Update(0.016f, ctx, nullptr);

    assert(mgr.GetCurrentTarget() != nullptr);
    assert(mgr.GetCurrentTarget()->definition.category == InteractionCategory::Container);

    // Execute interaction
    InteractionResult res = mgr.TryInteract(ctx);
    assert(res == InteractionResult::Success);

    // Verify chest opened and items transferred to player
    assert(chest->IsOpen());
    assert(chest->GetInventory().GetFilledSlotCount() == 0); // Chest emptied
    assert(player->GetInventory().HasItem("col_gold_coin_001"));
    assert(player->GetInventory().HasItem("sword_iron_broadsword_001"));

    // Attempt second interaction on empty chest: advance cooldown and verify no duplicate items
    mgr.Update(0.3f, ctx, nullptr);
    size_t prevPlayerItemCount = player->GetInventory().GetFilledSlotCount();
    InteractionResult res2 = mgr.TryInteract(ctx);
    assert(res2 == InteractionResult::Success);
    assert(player->GetInventory().GetFilledSlotCount() == prevPlayerItemCount); // No duplicate items generated

    std::cout << "  -> Container Transactions PASSED." << std::endl;
}

void TestCollectibleTransactions() {
    std::cout << "[TEST] 6. Collectible World Items & Inventory Capacity..." << std::endl;

    world::World world;
    auto* item = world.SpawnWorldItem("con_health_potion_small_001", Vector2{ 80.0f, 80.0f }, 3);
    assert(item != nullptr);
    assert(!item->IsCollected());

    auto player = std::make_unique<entities::Player>(1, "Collector");
    player->Spawn(Vector2{ 80.0f, 85.0f });

    InteractionManager mgr;
    mgr.SetPickupPolicy(PickupPolicy::Manual);

    InteractionContext ctx;
    ctx.player = player.get();
    ctx.world = &world;

    mgr.SyncWithWorld(ctx);
    mgr.Update(0.016f, ctx, nullptr);

    assert(mgr.GetCurrentTarget() != nullptr);
    assert(mgr.GetCurrentTarget()->definition.category == InteractionCategory::Collectible);

    // Successful collection
    InteractionResult res = mgr.TryInteract(ctx);
    assert(res == InteractionResult::Success);
    assert(item->IsCollected());
    assert(player->GetInventory().HasItem("con_health_potion_small_001"));

    // Test inventory full case
    auto* item2 = world.SpawnWorldItem("mat_iron_ore_001", Vector2{ 80.0f, 80.0f }, 5);
    // Fill player inventory capacity completely
    for (size_t s = 0; s < player->GetInventory().GetCapacity(); ++s) {
        if (!player->GetInventory().GetSlot(s).has_value()) {
            player->GetInventory().AddItem("junk_dummy_item_" + std::to_string(s), 1);
        }
    }
    assert(player->GetInventory().IsFull());

    mgr.ClearInteractables();
    mgr.SyncWithWorld(ctx);
    mgr.Update(0.016f, ctx, nullptr);

    InteractionResult resFull = mgr.TryInteract(ctx);
    assert(resFull == InteractionResult::Blocked);
    assert(!item2->IsCollected()); // Item MUST NOT be destroyed or marked collected if inventory was full!

    std::cout << "  -> Collectible Transactions PASSED." << std::endl;
}

void TestMechanisms() {
    std::cout << "[TEST] 7. Mechanisms (Levers, Switches, Pressure Plates)..." << std::endl;

    Mechanism lever("lever_01", "Dungeon Lever", Vector2{ 50.0f, 50.0f }, MechanismType::Lever);
    assert(!lever.IsActive());

    bool callbackFired = false;
    bool callbackState = false;
    lever.SetStateCallback([&](Mechanism& /*m*/, bool active) {
        callbackFired = true;
        callbackState = active;
    });

    // Activate lever
    bool act = lever.Activate();
    assert(act);
    assert(lever.IsActive());
    assert(callbackFired);
    assert(callbackState == true);

    // Advance cooldown timer before deactivating
    lever.Update(0.6f);

    // Deactivate lever
    callbackFired = false;
    bool deact = lever.Deactivate();
    assert(deact);
    assert(!lever.IsActive());
    assert(callbackFired);
    assert(callbackState == false);

    // One-time mechanism
    Mechanism oneTime("switch_01", "Sacred Altar Switch", Vector2{ 60.0f, 60.0f }, MechanismType::Switch, true);
    assert(oneTime.CanActivate());
    assert(oneTime.Activate());
    assert(oneTime.IsActive());
    assert(!oneTime.CanActivate()); // Cannot activate again
    assert(!oneTime.Deactivate());  // Cannot deactivate one-time mechanism

    std::cout << "  -> Mechanisms PASSED." << std::endl;
}

void TestInputAndContextModality() {
    std::cout << "[TEST] 8. Input Integration & Modal Context Restrictions..." << std::endl;

    InteractionManager mgr;
    InteractableComponent comp;
    comp.position = Vector2{ 100.0f, 100.0f };
    comp.definition.id = "target_obj";
    comp.definition.displayName = "Campfire";
    comp.definition.verb = "Inspect";
    comp.definition.range = 50.0f;
    mgr.RegisterInteractable(comp);

    auto player = std::make_unique<entities::Player>(1, "Tester");
    player->Spawn(Vector2{ 100.0f, 110.0f });

    input::InputManager inputMgr;
    InteractionContext ctx;
    ctx.player = player.get();

    // 1. In Gameplay context: prompt glyph is 'E' (or Gamepad 'X')
    inputMgr.SetContext(input::InputContextType::Gameplay);
    mgr.Update(0.016f, ctx, &inputMgr);
    assert(mgr.GetCurrentPrompt().visible);
    assert(mgr.GetCurrentPrompt().keyGlyph == "E");

    // Simulate Interact action in Gameplay context
    inputMgr.GetBuffer().BufferAction(input::InputAction::Interact);
    assert(inputMgr.GetBuffer().HasBufferedAction(input::InputAction::Interact));

    mgr.Update(0.016f, ctx, &inputMgr);
    assert(!inputMgr.GetBuffer().HasBufferedAction(input::InputAction::Interact)); // Consumed!

    // 2. In Menu / Paused context: interaction MUST be blocked to prevent action bleed
    inputMgr.SetContext(input::InputContextType::Menu);
    inputMgr.GetBuffer().BufferAction(input::InputAction::Interact);

    mgr.Update(0.016f, ctx, &inputMgr);
    // Buffered action should NOT be consumed by gameplay interaction while in Menu context!
    assert(inputMgr.GetBuffer().HasBufferedAction(input::InputAction::Interact));

    std::cout << "  -> Input & Modality PASSED." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 22: WORLD INTERACTION SYSTEM TEST SUITE" << std::endl;
    std::cout << "============================================================" << std::endl;

    try {
        TestInteractableRegistration();
        TestDiscoveryAndDeterministicRanking();
        TestValidationRules();
        TestDoorInteractions();
        TestContainerInteractions();
        TestCollectibleTransactions();
        TestMechanisms();
        TestInputAndContextModality();

        std::cout << "============================================================" << std::endl;
        std::cout << "ALL 8 INTERACTION TEST SUITES PASSED SUCCESSFULLY (100%)" << std::endl;
        std::cout << "============================================================" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
