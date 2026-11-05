#include "physics/CollisionTypes.hpp"
#include "physics/CollisionFilter.hpp"
#include "physics/OverlapDetector.hpp"
#include "physics/Collider.hpp"
#include "physics/CollisionEvent.hpp"
#include "physics/TriggerVolume.hpp"
#include "physics/CollisionManager.hpp"
#include "core/Logger.hpp"

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <chrono>

using namespace wildbound::physics;

void TestCollisionLayersAndMasks() {
    std::cout << "[TEST] 1. Collision Layers & Bilateral Masks..." << std::endl;

    // 1.1 Layer bitflag values & operators
    assert(static_cast<uint32_t>(CollisionLayer::World) == 1u);
    assert(static_cast<uint32_t>(CollisionLayer::Player) == 2u);
    assert(static_cast<uint32_t>(CollisionLayer::Enemy) == 4u);
    assert(static_cast<uint32_t>(CollisionLayer::Wildlife) == 8u);
    assert(static_cast<uint32_t>(CollisionLayer::Projectile) == 16u);
    assert(static_cast<uint32_t>(CollisionLayer::Pickup) == 32u);
    assert(static_cast<uint32_t>(CollisionLayer::Trigger) == 64u);
    assert(static_cast<uint32_t>(CollisionLayer::Building) == 128u);
    assert(static_cast<uint32_t>(CollisionLayer::Hazard) == 256u);
    assert(static_cast<uint32_t>(CollisionLayer::Interactable) == 512u);

    CollisionLayer combined = CollisionLayer::Player | CollisionLayer::Enemy;
    (void)combined;
    assert((combined & CollisionLayer::Player) == CollisionLayer::Player);
    assert((combined & CollisionLayer::Enemy) == CollisionLayer::Enemy);
    assert((combined & CollisionLayer::World) == CollisionLayer::None);

    // 1.2 String conversions
    assert(CollisionLayerToString(CollisionLayer::Player) == "Player");
    assert(CollisionLayerToString(CollisionLayer::World) == "World");
    assert(StringToCollisionLayer("Enemy") == CollisionLayer::Enemy);
    assert(StringToCollisionLayer("Invalid") == CollisionLayer::None);

    // 1.3 Default mask configuration
    uint32_t playerMask = GetDefaultCollisionMask(CollisionLayer::Player);
    uint32_t worldMask = GetDefaultCollisionMask(CollisionLayer::World);
    uint32_t enemyMask = GetDefaultCollisionMask(CollisionLayer::Enemy);
    uint32_t pickupMask = GetDefaultCollisionMask(CollisionLayer::Pickup);
    uint32_t projMask = GetDefaultCollisionMask(CollisionLayer::Projectile);

    // Player vs World: Bilateral check
    assert(CollisionFilter::ShouldCollide(CollisionLayer::Player, playerMask, CollisionLayer::World, worldMask));

    // Player vs Enemy: Bilateral check
    assert(CollisionFilter::ShouldCollide(CollisionLayer::Player, playerMask, CollisionLayer::Enemy, enemyMask));

    // Projectile vs Enemy: Bilateral check
    assert(CollisionFilter::ShouldCollide(CollisionLayer::Projectile, projMask, CollisionLayer::Enemy, enemyMask));

    // Pickup vs Player: Bilateral check
    assert(CollisionFilter::ShouldCollide(CollisionLayer::Pickup, pickupMask, CollisionLayer::Player, playerMask));

    // Pickup vs Enemy: Should NOT collide (Enemy doesn't pick up player ground loot)
    assert(!CollisionFilter::ShouldCollide(CollisionLayer::Pickup, pickupMask, CollisionLayer::Enemy, enemyMask));
    (void)worldMask;
    (void)enemyMask;
    (void)pickupMask;
    (void)projMask;

    // 1.4 Bilateral failure test (One side accepts, other side does NOT)
    // A accepts B, but B mask = 0 (rejects A)
    uint32_t rejectAllMask = 0u;
    assert(!CollisionFilter::ShouldCollide(CollisionLayer::Player, playerMask, CollisionLayer::Enemy, rejectAllMask));
    assert(!CollisionFilter::ShouldCollide(CollisionLayer::Player, rejectAllMask, CollisionLayer::Enemy, enemyMask));
    (void)rejectAllMask;

    // 1.5 Self-collision exclusion
    EntityId ent1 = 10;
    EntityId ent2 = 20;
    assert(!CollisionFilter::ShouldTestPair(true, ent1, CollisionLayer::Player, playerMask,
                                           true, ent1, CollisionLayer::Player, playerMask, false)); // same entity
    assert(CollisionFilter::ShouldTestPair(true, ent1, CollisionLayer::Player, playerMask,
                                          true, ent2, CollisionLayer::Enemy, enemyMask, false));  // distinct entities
    (void)ent1;
    (void)ent2;

    // 1.6 Runtime mask mutation
    uint32_t mutableMask = playerMask;
    mutableMask &= ~static_cast<uint32_t>(CollisionLayer::Enemy); // Ignore enemies
    assert(!CollisionFilter::ShouldCollide(CollisionLayer::Player, mutableMask, CollisionLayer::Enemy, enemyMask));

    std::cout << "  -> Layer & Bilateral Mask validation PASSED." << std::endl;
}

void TestColliderBehavior() {
    std::cout << "[TEST] 2. Collider Representation & Overlap Detector..." << std::endl;

    // 2.1 Pure AABB overlap testing
    Rectangle boxA{ 0.0f, 0.0f, 32.0f, 32.0f };
    Rectangle boxB{ 16.0f, 16.0f, 32.0f, 32.0f };
    Rectangle boxC{ 100.0f, 100.0f, 32.0f, 32.0f };

    assert(OverlapDetector::Intersects(boxA, boxB));
    assert(!OverlapDetector::Intersects(boxA, boxC));

    // Intersection geometry
    Rectangle inter = OverlapDetector::ComputeIntersection(boxA, boxB);
    assert(inter.x == 16.0f && inter.y == 16.0f);
    assert(inter.width == 16.0f && inter.height == 16.0f);

    Rectangle noInter = OverlapDetector::ComputeIntersection(boxA, boxC);
    assert(noInter.width == 0.0f && noInter.height == 0.0f);

    // Point containment
    assert(OverlapDetector::ContainsPoint(boxA, Vector2{ 10.0f, 10.0f }));
    assert(!OverlapDetector::ContainsPoint(boxA, Vector2{ 50.0f, 50.0f }));

    // 2.2 Collider construction & offset mapping
    Collider col1(1, 100, CollisionLayer::Player, GetDefaultCollisionMask(CollisionLayer::Player),
                  Vector2{ 16.0f, 12.0f }, Vector2{ 2.0f, -4.0f });

    // Entity anchor at (100, 200)
    col1.UpdateWorldBounds(Vector2{ 100.0f, 200.0f });
    Rectangle bounds1 = col1.GetWorldBounds();
    // expected: x = 100 - (16/2) + 2 = 94
    // expected: y = 200 - 12 - 4 = 184
    assert(bounds1.x == 94.0f);
    assert(bounds1.y == 184.0f);
    assert(bounds1.width == 16.0f);
    assert(bounds1.height == 12.0f);

    // 2.3 Disabled collider exclusion
    Collider col2(2, 200, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy),
                  Vector2{ 16.0f, 12.0f });
    col2.UpdateWorldBounds(Vector2{ 100.0f, 200.0f });
    assert(col1.Intersects(col2));

    col2.SetEnabled(false);
    assert(!col2.IsEnabled());
    assert(!col1.Intersects(col2));

    // 2.4 Negative dimension clamp validation
    Collider invalidCol(3, 300, CollisionLayer::World, 0xFFFFFFFFu, Vector2{ -10.0f, -20.0f });
    assert(invalidCol.GetSize().x == 0.0f);
    assert(invalidCol.GetSize().y == 0.0f);

    std::cout << "  -> Collider behavior & spatial bounds PASSED." << std::endl;
}

void TestTriggerVolumesAndLifecycle() {
    std::cout << "[TEST] 3. Trigger Volumes (Enter, Stay, Exit)..." << std::endl;

    CollisionManager mgr;

    // Register a static trigger volume at (50, 50) of size (40, 40)
    Collider triggerCol = Collider::CreateTrigger(
        0, INVALID_ENTITY_ID, CollisionLayer::Trigger,
        GetDefaultCollisionMask(CollisionLayer::Trigger),
        Vector2{ 40.0f, 40.0f }, Vector2{ 0.0f, 0.0f }, "entrance_trigger", true
    );
    triggerCol.SetWorldBounds(Rectangle{ 50.0f, 50.0f, 40.0f, 40.0f });
    ColliderId triggerId = mgr.RegisterCollider(triggerCol);

    // Register a dynamic player collider starting outside at (0, 0)
    Collider playerCol(
        0, 1, CollisionLayer::Player,
        GetDefaultCollisionMask(CollisionLayer::Player),
        Vector2{ 20.0f, 20.0f }, Vector2{ 0.0f, 0.0f },
        ColliderType::Physical, "player"
    );
    playerCol.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 20.0f, 20.0f });
    ColliderId playerId = mgr.RegisterCollider(playerCol);

    std::vector<CollisionEventType> capturedEvents;
    mgr.RegisterCallback([&](const CollisionEvent& ev) {
        if (ev.isTrigger) {
            capturedEvents.push_back(ev.type);
        }
    });

    // Tick 1: Player is outside trigger -> No events
    mgr.Update(0.016f);
    assert(capturedEvents.empty());
    assert(!mgr.IsOverlapping(triggerId, playerId));

    // Tick 2: Move player inside trigger -> TriggerEnter
    mgr.SetColliderTransform(playerId, Vector2{ 60.0f, 70.0f });
    mgr.Update(0.016f);
    assert(capturedEvents.size() == 1);
    assert(capturedEvents[0] == CollisionEventType::TriggerEnter);
    assert(mgr.IsOverlapping(triggerId, playerId));

    // Tick 3: Player remains inside trigger -> TriggerStay
    capturedEvents.clear();
    mgr.Update(0.016f);
    assert(capturedEvents.size() == 1);
    assert(capturedEvents[0] == CollisionEventType::TriggerStay);
    assert(mgr.IsOverlapping(triggerId, playerId));

    // Tick 4: Move player outside trigger -> TriggerExit
    capturedEvents.clear();
    auto* pCol = mgr.GetColliderMutable(playerId);
    pCol->SetWorldBounds(Rectangle{ 200.0f, 200.0f, 20.0f, 20.0f });
    mgr.Update(0.016f);
    assert(capturedEvents.size() == 1);
    assert(capturedEvents[0] == CollisionEventType::TriggerExit);
    assert(!mgr.IsOverlapping(triggerId, playerId));

    // Tick 5: Player stays outside -> No events
    capturedEvents.clear();
    mgr.Update(0.016f);
    assert(capturedEvents.empty());

    // 3.2 Multiple entities in one trigger
    TriggerVolume trigVol(triggerId, INVALID_ENTITY_ID, CollisionLayer::Trigger,
                          GetDefaultCollisionMask(CollisionLayer::Trigger), { 40.0f, 40.0f });
    trigVol.OnEntityEnter(1);
    trigVol.OnEntityEnter(101);
    assert(trigVol.GetEntityCount() == 2);
    assert(trigVol.IsEntityInside(1));
    assert(trigVol.IsEntityInside(101));
    assert(!trigVol.IsEntityInside(999));

    trigVol.OnEntityExit(1);
    assert(trigVol.GetEntityCount() == 1);
    assert(!trigVol.IsEntityInside(1));
    assert(trigVol.IsEntityInside(101));

    std::cout << "  -> Trigger lifecycle (Enter -> Stay -> Exit) PASSED." << std::endl;
}

void TestDeferredMutationSafety() {
    std::cout << "[TEST] 4. Deferred Mutation Safety During Callbacks..." << std::endl;

    CollisionManager mgr;

    // Register two overlapping colliders
    Collider colA = Collider::CreateStatic(1, CollisionLayer::World, Rectangle{ 10.0f, 10.0f, 20.0f, 20.0f }, "boxA");
    Collider colB(2, 10, CollisionLayer::Player, GetDefaultCollisionMask(CollisionLayer::Player),
                  Vector2{ 20.0f, 20.0f }, Vector2{ 0.0f, 0.0f }, ColliderType::Physical, "player");
    colB.SetWorldBounds(Rectangle{ 15.0f, 15.0f, 20.0f, 20.0f });

    mgr.RegisterCollider(colA);
    mgr.RegisterCollider(colB);

    bool callbackExecuted = false;

    // During event callback, mutate the collision manager (Unregister, Register, SetEnabled)
    mgr.RegisterCallback([&](const CollisionEvent& ev) {
        callbackExecuted = true;
        // Unregister collider B while iterating events
        mgr.UnregisterCollider(ev.colliderIdB);

        // Register a new collider C while iterating events
        Collider colC = Collider::CreateStatic(99, CollisionLayer::World, Rectangle{ 50.0f, 50.0f, 20.0f, 20.0f });
        mgr.RegisterCollider(colC);

        // Disable collider A
        mgr.SetColliderEnabled(ev.colliderIdA, false);
    });

    // Execute update: should NOT crash, throw, or invalidate iterators
    mgr.Update(0.016f);
    assert(callbackExecuted);

    // Verify mutations were applied cleanly after the tick
    assert(mgr.GetCollider(2) == nullptr); // Collider B was unregistered
    assert(mgr.GetCollider(99) != nullptr); // Collider C was added
    assert(mgr.GetCollider(1) != nullptr && !mgr.GetCollider(1)->IsEnabled()); // Collider A was disabled

    // Entity cleanup test
    Collider colD(101, 555, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy), { 16.0f, 16.0f });
    Collider colE(102, 555, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy), { 16.0f, 16.0f });
    mgr.RegisterCollider(colD);
    mgr.RegisterCollider(colE);
    assert(mgr.GetCollidersForEntity(555).size() == 2);

    mgr.UnregisterEntityColliders(555);
    assert(mgr.GetCollidersForEntity(555).empty());
    assert(mgr.GetCollider(101) == nullptr);
    assert(mgr.GetCollider(102) == nullptr);

    std::cout << "  -> Deferred mutation queue & safe entity destruction PASSED." << std::endl;
}

void TestDeterministicEventOrdering() {
    std::cout << "[TEST] 5. Deterministic Event & Pair Ordering..." << std::endl;

    auto runScenario = []() -> std::vector<std::string> {
        CollisionManager mgr;

        // Register 4 overlapping colliders in random/arbitrary order
        Collider c3(3, 30, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy), { 30.0f, 30.0f });
        c3.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 30.0f, 30.0f });
        Collider c1(1, 10, CollisionLayer::Player, GetDefaultCollisionMask(CollisionLayer::Player), { 30.0f, 30.0f });
        c1.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 30.0f, 30.0f });
        Collider c4(4, 40, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy), { 30.0f, 30.0f });
        c4.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 30.0f, 30.0f });
        Collider c2(2, 20, CollisionLayer::Player, GetDefaultCollisionMask(CollisionLayer::Player), { 30.0f, 30.0f });
        c2.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 30.0f, 30.0f });

        // Insert out-of-order
        mgr.RegisterCollider(c3);
        mgr.RegisterCollider(c1);
        mgr.RegisterCollider(c4);
        mgr.RegisterCollider(c2);

        std::vector<std::string> eventLog;
        mgr.RegisterCallback([&](const CollisionEvent& ev) {
            std::string s = std::string(CollisionEventTypeToString(ev.type)) + "_" +
                            std::to_string(ev.colliderIdA) + "_" + std::to_string(ev.colliderIdB);
            eventLog.push_back(s);
        });

        mgr.Update(0.016f);
        return eventLog;
    };

    // Run identical scenario three times
    std::vector<std::string> run1 = runScenario();
    std::vector<std::string> run2 = runScenario();
    std::vector<std::string> run3 = runScenario();

    assert(!run1.empty());
    assert(run1.size() == run2.size());
    assert(run1.size() == run3.size());

    for (size_t i = 0; i < run1.size(); ++i) {
        assert(run1[i] == run2[i]);
        assert(run1[i] == run3[i]);
    }

    std::cout << "  -> Deterministic update order verified across 3 runs (" << run1.size() << " events). PASSED." << std::endl;
}

void TestSpatialQueriesAndBenchmark() {
    std::cout << "[TEST] 6. Spatial Queries & Performance Benchmark..." << std::endl;

    CollisionManager mgr;

    // Spawn a grid of 100 static obstacles (10x10)
    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 10; ++c) {
            mgr.RegisterStaticBox(CollisionLayer::World,
                                  Rectangle{ static_cast<float>(c * 32), static_cast<float>(r * 32), 32.0f, 32.0f },
                                  "tile_" + std::to_string(c) + "_" + std::to_string(r));
        }
    }
    assert(mgr.GetTotalColliderCount() == 100);

    // Spawn 20 dynamic entities
    for (int i = 0; i < 20; ++i) {
        Collider dyn(1000 + i, 200 + i, CollisionLayer::Enemy, GetDefaultCollisionMask(CollisionLayer::Enemy),
                     Vector2{ 16.0f, 16.0f });
        dyn.SetWorldBounds(Rectangle{ static_cast<float>(i * 14), static_cast<float>(i * 14), 16.0f, 16.0f });
        mgr.RegisterCollider(dyn);
    }
    assert(mgr.GetTotalColliderCount() == 120);

    // Query AABB test
    Rectangle searchBox{ 0.0f, 0.0f, 64.0f, 64.0f };
    auto found = mgr.QueryAABB(searchBox, static_cast<uint32_t>(CollisionLayer::World));
    assert(!found.empty());
    assert(mgr.HasOverlap(searchBox, static_cast<uint32_t>(CollisionLayer::World)));

    // Benchmark 100 simulation ticks
    auto start = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < 100; ++t) {
        mgr.Update(0.016f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "  -> 100 simulation ticks with 120 colliders executed in "
              << elapsedMs << " ms (" << (elapsedMs / 100.0) << " ms/tick). PASSED." << std::endl;
    assert(elapsedMs < 1000.0); // Comfortably within 16.6ms frame budget even in unoptimized Debug builds
}

int main() {
    std::cout << "\n=== Running Wildbound Physics & Collision System Tests (Phase 20) ===\n" << std::endl;

    TestCollisionLayersAndMasks();
    TestColliderBehavior();
    TestTriggerVolumesAndLifecycle();
    TestDeferredMutationSafety();
    TestDeterministicEventOrdering();
    TestSpatialQueriesAndBenchmark();

    std::cout << "\n>>> ALL PHASE 20 PHYSICS & COLLISION TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
