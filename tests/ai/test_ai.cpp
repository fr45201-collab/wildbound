#include <cassert>
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <cmath>

#include "ai/core/AITypes.hpp"
#include "ai/core/AIConfiguration.hpp"
#include "ai/core/AIController.hpp"
#include "ai/navigation/NavigationQuery.hpp"
#include "ai/navigation/NavigationGrid.hpp"
#include "ai/navigation/Pathfinding.hpp"
#include "ai/movement/AIMovementController.hpp"
#include "ai/movement/ObstacleAvoidance.hpp"
#include "ai/movement/StuckDetector.hpp"
#include "ai/targeting/PerceptionSystem.hpp"
#include "ai/targeting/TargetMemory.hpp"
#include "ai/targeting/TargetSelector.hpp"
#include "ai/behaviours/PatrolBehaviour.hpp"
#include "ai/behaviours/InvestigateBehaviour.hpp"
#include "ai/AIManager.hpp"
#include "entities/Enemy.hpp"
#include "entities/EnemyConfig.hpp"
#include "world/World.hpp"
#include "combat/CombatSystem.hpp"
#include "scripting/LuaManager.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include "core/Logger.hpp"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: " << (msg) << " (" << #cond << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while(0)

using namespace wildbound;

void TestNavigationGrid() {
    std::cout << "[TEST] 1. Navigation Grid & Coordinate Transformations..." << std::endl;

    ai::NavigationGrid grid;
    bool initOk = grid.Initialize(320.0f, 320.0f, 16.0f);
    TEST_ASSERT(initOk, "NavigationGrid should initialize with valid dimensions");
    TEST_ASSERT(grid.GetWidthInCells() == 20, "Expected 20 cells horizontally");
    TEST_ASSERT(grid.GetHeightInCells() == 20, "Expected 20 cells vertically");
    TEST_ASSERT(grid.GetTotalCellCount() == 400, "Expected 400 total cells");
    TEST_ASSERT(grid.GetBlockedCellCount() == 0, "Initial grid should have 0 blocked cells");

    // Coordinate conversion
    ai::GridCoord c = grid.WorldToGrid(Vector2{ 24.0f, 40.0f });
    TEST_ASSERT(c.x == 1 && c.y == 2, "WorldToGrid(24, 40) should map to cell (1, 2)");

    Vector2 worldPos = grid.GridToWorld(1, 2);
    TEST_ASSERT(worldPos.x == 24.0f && worldPos.y == 40.0f, "GridToWorld(1, 2) should return cell center (24, 40)");

    // Walkable vs Blocked
    TEST_ASSERT(grid.IsCellWalkable(1, 2), "Cell (1, 2) should be walkable initially");
    grid.SetCellBlocked(1, 2, true);
    TEST_ASSERT(!grid.IsCellWalkable(1, 2), "Cell (1, 2) should be blocked after SetCellBlocked");
    TEST_ASSERT(grid.GetBlockedCellCount() == 1, "Blocked cell count should be 1");
    TEST_ASSERT(!grid.IsWalkable(Vector2{ 24.0f, 40.0f }), "World position inside blocked cell should not be walkable");

    // Bounds checking
    TEST_ASSERT(!grid.IsInBounds(-1, 0), "Negative coordinates should be out of bounds");
    TEST_ASSERT(!grid.IsInBounds(20, 20), "Coordinates >= width/height should be out of bounds");
    TEST_ASSERT(!grid.IsCellWalkable(-5, 10), "Out of bounds cells should not be walkable");

    // Nearest walkable resolution
    Vector2 nearest = grid.GetNearestWalkablePosition(Vector2{ 24.0f, 40.0f }, 32.0f);
    TEST_ASSERT(grid.IsWalkable(nearest), "Nearest walkable position must be walkable");
    TEST_ASSERT(nearest.x != 24.0f || nearest.y != 40.0f, "Nearest walkable position should differ from blocked point");

    std::cout << "  -> Navigation Grid tests passed successfully." << std::endl;
}

void TestAStarPathfinding() {
    std::cout << "[TEST] 2. A* Pathfinding, Obstacle Routing & Corner Cutting Prevention..." << std::endl;

    ai::NavigationGrid grid;
    grid.Initialize(320.0f, 320.0f, 16.0f);

    // Setup a vertical wall between x=5 from y=0 to y=14
    for (int y = 0; y < 15; ++y) {
        grid.SetCellBlocked(5, y, true);
    }

    ai::PathfindingConfig config;
    config.mode = ai::MovementMode::EightDirectional;
    config.preventCornerCutting = true;
    config.enableSmoothing = true;

    ai::Pathfinding pathfinder(grid, config);

    // 1. Direct path around the wall
    Vector2 startPos{ 32.0f, 64.0f };   // (2, 4) left of wall
    Vector2 goalPos{ 144.0f, 64.0f };   // (9, 4) right of wall

    ai::PathResult res = pathfinder.FindPath(startPos, goalPos);
    TEST_ASSERT(res.IsValid(), "Pathfinder should find a valid path around the wall");
    TEST_ASSERT(res.status == ai::PathStatus::Found, "Path status must be Found");
    TEST_ASSERT(!res.waypoints.empty(), "Path waypoints must not be empty");

    // Verify no waypoint lies on a blocked cell
    for (const auto& wp : res.waypoints) {
        TEST_ASSERT(grid.IsWalkable(wp), "Every waypoint along the path must be on a walkable cell");
    }

    // 2. Corner cutting test
    // Create an L-shaped corner at (10, 10) and (11, 9)
    grid.Clear();
    grid.SetCellBlocked(10, 9, true);
    grid.SetCellBlocked(9, 10, true);
    // Attempting to move diagonally from (9, 9) to (10, 10) should be blocked by corner-cutting prevention
    ai::Pathfinding strictPathfinder(grid, config);
    ai::PathResult diagRes = strictPathfinder.FindPath(grid.GridToWorld(9, 9), grid.GridToWorld(10, 10));
    TEST_ASSERT(diagRes.IsValid(), "Path around diagonal corner should be found");
    // Verify that path does not cut directly through the corner
    for (size_t i = 1; i < diagRes.waypoints.size(); ++i) {
        ai::GridCoord from = grid.WorldToGrid(diagRes.waypoints[i - 1]);
        ai::GridCoord to = grid.WorldToGrid(diagRes.waypoints[i]);
        if (std::abs(from.x - to.x) == 1 && std::abs(from.y - to.y) == 1) {
            // Diagonal step must have walkable orthogonal neighbours
            TEST_ASSERT(grid.IsCellWalkable(from.x, to.y) && grid.IsCellWalkable(to.x, from.y),
                        "Diagonal step must not cut through blocked corners");
        }
    }

    // 3. Unreachable destination test
    // Surround cell (15, 15) with walls
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx != 0 || dy != 0) {
                grid.SetCellBlocked(15 + dx, 15 + dy, true);
            }
        }
    }
    ai::PathResult unreachableRes = pathfinder.FindPath(grid.GridToWorld(2, 2), grid.GridToWorld(15, 15));
    // Since goal is completely enclosed, pathfinder should report GoalBlocked or Unreachable
    TEST_ASSERT(unreachableRes.status == ai::PathStatus::GoalBlocked || unreachableRes.status == ai::PathStatus::Unreachable,
                "Enclosed destination must report GoalBlocked or Unreachable");

    std::cout << "  -> A* Pathfinding tests passed successfully." << std::endl;
}

void TestPathFollowingAndMovement() {
    std::cout << "[TEST] 3. Path Following & Movement Controller..." << std::endl;

    ai::MovementControllerConfig cfg;
    cfg.waypointArrivalRadius = 10.0f;
    cfg.goalArrivalRadius = 15.0f;
    cfg.enableAvoidance = false;

    ai::AIMovementController movement(cfg);

    ai::PathResult path;
    path.status = ai::PathStatus::Found;
    path.waypoints = {
        Vector2{ 10.0f, 10.0f },
        Vector2{ 50.0f, 10.0f },
        Vector2{ 50.0f, 60.0f }
    };

    movement.SetPath(path);
    TEST_ASSERT(movement.HasActivePath(), "Movement controller should have an active path");
    TEST_ASSERT(!movement.HasReachedDestination(), "Should not have reached destination at start");

    // Entity is at (10, 10), heading towards waypoint 1 at (50, 10)
    Vector2 vel = movement.Update(Vector2{ 10.0f, 10.0f }, 100.0f, 0.1f);
    TEST_ASSERT(vel.x > 0.0f && std::abs(vel.y) < 0.001f, "Velocity should point right towards waypoint 1");
    TEST_ASSERT(movement.GetDesiredFacing() == graphics::animation::Direction::Right, "Facing should be Right");

    // Arrive at waypoint 1 (50, 10) -> advances to waypoint 2 (50, 60)
    vel = movement.Update(Vector2{ 49.0f, 10.0f }, 100.0f, 0.1f);
    vel = movement.Update(Vector2{ 50.0f, 15.0f }, 100.0f, 0.1f);
    TEST_ASSERT(vel.y > 0.0f && std::abs(vel.x) < 0.001f, "Velocity should point down towards waypoint 2");
    TEST_ASSERT(movement.GetDesiredFacing() == graphics::animation::Direction::Down, "Facing should be Down");

    // Arrive at goal (50, 60)
    vel = movement.Update(Vector2{ 50.0f, 55.0f }, 100.0f, 0.1f);
    TEST_ASSERT(movement.HasReachedDestination(), "Movement controller should report reached destination");
    TEST_ASSERT(vel.x == 0.0f && vel.y == 0.0f, "Velocity should be zero after arrival");

    std::cout << "  -> Path Following tests passed successfully." << std::endl;
}

void TestObstacleAvoidanceAndStuckDetection() {
    std::cout << "[TEST] 4. Obstacle Avoidance & Stuck Detection..." << std::endl;

    // 1. Obstacle Avoidance
    ai::AvoidanceConfig aCfg;
    aCfg.separationRadius = 30.0f;
    aCfg.maxForce = 0.5f;

    ai::ObstacleAvoidance avoidance(aCfg);

    std::vector<ai::AvoidanceEntity> nearby = {
        { 2, Vector2{ 110.0f, 100.0f }, 10.0f } // Neighbor 10px to the right
    };

    Vector2 selfPos{ 100.0f, 100.0f };
    Vector2 force = avoidance.ComputeAvoidanceForce(1, selfPos, nearby);
    TEST_ASSERT(force.x < 0.0f, "Avoidance force should push away to the left (negative x)");

    // 2. Stuck Detection
    ai::StuckDetectorConfig sCfg;
    sCfg.checkInterval = 0.2f;
    sCfg.minProgressDistance = 4.0f;
    sCfg.stuckTimeThreshold = 0.5f;
    sCfg.maxRetryCount = 3;

    ai::StuckDetector stuckDetector(sCfg);
    stuckDetector.Reset(selfPos);

    TEST_ASSERT(!stuckDetector.IsStuck(), "Initially should not be stuck");

    // Simulate zero progress while having non-zero velocity
    Vector2 activeVel{ 50.0f, 0.0f };
    stuckDetector.Update(selfPos, activeVel, 0.25f);
    stuckDetector.Update(selfPos, activeVel, 0.25f);
    stuckDetector.Update(selfPos, activeVel, 0.25f);

    TEST_ASSERT(stuckDetector.IsStuck(), "Stuck detector should trigger after threshold with zero displacement");

    stuckDetector.OnPathReplanned();
    TEST_ASSERT(!stuckDetector.IsStuck(), "Stuck state should reset after replanning");
    TEST_ASSERT(stuckDetector.GetRetryCount() == 1, "Retry count should increment to 1");

    std::cout << "  -> Obstacle Avoidance & Stuck Detection tests passed successfully." << std::endl;
}

void TestPerceptionAndLineOfSight() {
    std::cout << "[TEST] 5. Perception, Field of View & Line of Sight..." << std::endl;

    ai::NavigationGrid grid;
    grid.Initialize(320.0f, 320.0f, 16.0f);

    ai::PerceptionConfig pCfg;
    pCfg.viewDistance = 150.0f;
    pCfg.fieldOfViewDegrees = 120.0f;
    pCfg.omnidirectionalWhenAlert = true;
    pCfg.requireLineOfSight = true;

    ai::PerceptionSystem perception(pCfg);

    Vector2 observerPos{ 100.0f, 100.0f };
    graphics::animation::Direction facing = graphics::animation::Direction::Down; // Facing positive Y

    // Target directly in front: (100, 160)
    Vector2 targetInFront{ 100.0f, 160.0f };
    TEST_ASSERT(perception.CanDetectTarget(observerPos, facing, targetInFront, false, grid),
                "Target directly in front within view distance should be detected");

    // Target behind observer: (100, 40)
    Vector2 targetBehind{ 100.0f, 40.0f };
    TEST_ASSERT(!perception.CanDetectTarget(observerPos, facing, targetBehind, false, grid),
                "Target behind observer should NOT be detected when not alert (FOV=120 deg)");

    // Target behind observer when alert (omnidirectional):
    TEST_ASSERT(perception.CanDetectTarget(observerPos, facing, targetBehind, true, grid),
                "Target behind observer SHOULD be detected when alert");

    // Target beyond view distance: (100, 300)
    Vector2 targetFar{ 100.0f, 300.0f };
    TEST_ASSERT(!perception.CanDetectTarget(observerPos, facing, targetFar, true, grid),
                "Target beyond view distance should NOT be detected");

    // Line of Sight blocked by wall
    for (int x = 4; x <= 8; ++x) {
        grid.SetCellBlocked(x, 8, true); // Wall at y=128
    }
    TEST_ASSERT(!perception.CanDetectTarget(observerPos, facing, targetInFront, false, grid),
                "Target blocked by wall should NOT be detected (Line of Sight blocked)");

    std::cout << "  -> Perception & Line of Sight tests passed successfully." << std::endl;
}

void TestTargetSelectionAndMemory() {
    std::cout << "[TEST] 6. Target Selection & Target Memory..." << std::endl;

    // 1. Target Memory
    ai::TargetMemoryConfig tmCfg;
    tmCfg.memoryDuration = 2.0f;

    ai::TargetMemory memory(tmCfg);
    memory.RecordTarget(42, Vector2{ 200.0f, 150.0f }, 1.5f);

    TEST_ASSERT(memory.HasActiveMemory(), "Target memory should be active immediately after recording");
    TEST_ASSERT(memory.GetTargetId() == 42, "Stored target ID should be 42");
    TEST_ASSERT(memory.GetLastKnownPosition().x == 200.0f, "Stored position X should be 200");

    memory.Update(1.0f);
    TEST_ASSERT(memory.HasActiveMemory(), "Memory should remain active after 1.0s (< 2.0s duration)");

    memory.Update(1.5f); // 2.5s total
    TEST_ASSERT(!memory.HasActiveMemory(), "Memory should expire after exceeding duration");
    TEST_ASSERT(memory.IsExpired(), "IsExpired() should return true");

    // 2. Target Selector Scoring
    ai::TargetSelector selector;
    std::vector<ai::TargetCandidate> candidates = {
        { 1, Vector2{ 150.0f, 100.0f }, 80.0f, 100.0f, false, true },  // 50px away
        { 2, Vector2{ 120.0f, 100.0f }, 30.0f, 100.0f, true, true },   // 20px away, recent attacker, low HP
        { 3, Vector2{ 110.0f, 100.0f }, 10.0f, 100.0f, false, false }  // 10px away, but unreachable
    };

    Vector2 obsPos{ 100.0f, 100.0f };
    uint32_t bestId = selector.SelectBestTarget(obsPos, candidates, 200.0f);
    TEST_ASSERT(bestId == 2, "Candidate 2 should be selected due to proximity, attacker bonus, and reachability");

    std::cout << "  -> Target Selection & Memory tests passed successfully." << std::endl;
}

void TestTacticalBehaviors() {
    std::cout << "[TEST] 7. Tactical Behaviors (Patrol, Investigate, Return)..." << std::endl;

    // 1. Patrol Behaviour
    ai::PatrolRoute route;
    route.id = "test_route";
    route.isLoop = true;
    route.points = {
        { Vector2{ 50.0f, 50.0f }, 0.5f },
        { Vector2{ 100.0f, 50.0f }, 0.5f }
    };

    ai::PatrolBehaviour patrol;
    patrol.SetRoute(route);

    TEST_ASSERT(patrol.GetCurrentTargetPosition().x == 50.0f, "Initial patrol point should be (50, 50)");

    // Arrive at waypoint 0
    patrol.Update(Vector2{ 50.0f, 50.0f }, 0.1f);
    TEST_ASSERT(patrol.IsWaiting(), "Patrol should enter waiting state upon arrival");

    // Wait timer expires
    bool targetChanged = patrol.Update(Vector2{ 50.0f, 50.0f }, 0.6f);
    TEST_ASSERT(targetChanged, "Patrol should advance to next waypoint after wait timer");
    TEST_ASSERT(patrol.GetCurrentTargetPosition().x == 100.0f, "Next patrol point should be (100, 50)");

    // 2. Investigate Behaviour
    ai::InvestigateBehaviour investigate;
    investigate.StartInvestigation(Vector2{ 150.0f, 150.0f }, 1.0f);
    TEST_ASSERT(!investigate.HasArrived(), "Should not have arrived at start of investigation");

    investigate.Update(Vector2{ 150.0f, 150.0f }, 0.1f);
    TEST_ASSERT(investigate.HasArrived(), "Should report arrived at target position");

    investigate.Update(Vector2{ 150.0f, 150.0f }, 1.1f);
    TEST_ASSERT(investigate.IsComplete(), "Investigation should complete after search duration");

    std::cout << "  -> Tactical Behaviors tests passed successfully." << std::endl;
}

void TestCreatureProfiles() {
    std::cout << "[TEST] 8. Creature-Specific Behavior Profiles..." << std::endl;

    auto goblin = ai::CreatureAIProfile::CreateDefault("goblin");
    TEST_ASSERT(goblin.canRetreat == true, "Goblin should retreat on low HP");
    TEST_ASSERT(goblin.retreatHealthPercent == 0.2f, "Goblin retreat threshold should be 20%");

    auto skeleton = ai::CreatureAIProfile::CreateDefault("skeleton");
    TEST_ASSERT(skeleton.canRetreat == false, "Skeleton should never retreat");
    TEST_ASSERT(skeleton.pursuitDuration > goblin.pursuitDuration, "Skeleton pursuit duration should exceed Goblin");

    auto wolf = ai::CreatureAIProfile::CreateDefault("wolf");
    TEST_ASSERT(wolf.aggressionLevel > goblin.aggressionLevel, "Wolf aggression level should exceed Goblin");
    TEST_ASSERT(wolf.detectionRadius >= 180.0f, "Wolf detection radius should be at least 180px");

    auto boar = ai::CreatureAIProfile::CreateDefault("wild_boar");
    TEST_ASSERT(boar.archetype == "wild_boar", "Wild Boar archetype verified");

    auto bat = ai::CreatureAIProfile::CreateDefault("bat");
    TEST_ASSERT(bat.fieldOfView >= 180.0f, "Bat should have wide field of view");

    auto wildlife = ai::CreatureAIProfile::CreateDefault("small_bird");
    TEST_ASSERT(wildlife.aggressionLevel == 0.0f, "Ambient wildlife should have 0 aggression");

    std::cout << "  -> Creature Profiles tests passed successfully." << std::endl;
}

void TestLuaAIIntegration() {
    std::cout << "[TEST] 9. Lua AI Script Execution & C++ Fallback..." << std::endl;

    scripting::LuaManager luaMgr;
    bool initOk = luaMgr.Initialize();
    TEST_ASSERT(initOk, "LuaManager should initialize");

    bool scriptLoaded = luaMgr.LoadScriptFile("scripts/gameplay/ai/enemy_base.lua");
    TEST_ASSERT(scriptLoaded, "enemy_base.lua should load cleanly");

    sol::state& lua = luaMgr.GetState();
    sol::function updateAi = lua["wildbound"]["ai"]["update_enemy"];
    TEST_ASSERT(updateAi.valid(), "wildbound.ai.update_enemy Lua function must be valid");

    // Test context invocation
    sol::table ctx = lua.create_table();
    ctx["state"] = "IDLE";
    ctx["hp_percent"] = 1.0f;
    ctx["has_target"] = true;
    ctx["target_dist"] = 20.0f;
    ctx["attack_radius"] = 32.0f;
    ctx["detection_radius"] = 140.0f;

    sol::table result = updateAi(ctx);
    TEST_ASSERT(result.valid(), "Lua AI result must be a valid table");
    std::string nextState = result["next_state"];
    bool wantAtk = result["request_attack"];
    TEST_ASSERT(nextState == "ATTACK", "Enemy should transition to ATTACK when target within attack_radius");
    TEST_ASSERT(wantAtk == true, "Enemy should request attack");

    // Test Low HP Retreat
    ctx["hp_percent"] = 0.1f;
    ctx["retreat_hp"] = 0.2f;
    ctx["target_x"] = 100.0f;
    ctx["target_y"] = 100.0f;
    ctx["x"] = 80.0f;
    ctx["y"] = 80.0f;

    sol::table retreatRes = updateAi(ctx);
    std::string retState = retreatRes["next_state"];
    TEST_ASSERT(retState == "RETREAT", "Enemy should transition to RETREAT when HP <= retreat_hp");

    std::cout << "  -> Lua AI Integration tests passed successfully." << std::endl;
}

void TestScalabilityAndPerformance() {
    std::cout << "[TEST] 10. Scalability & Performance Benchmarks (5, 10, 25, 50 Entities)..." << std::endl;

    ai::NavigationGrid grid;
    grid.Initialize(640.0f, 640.0f, 16.0f); // 40x40 grid = 1600 cells

    // Scatter obstacles across grid
    for (int y = 5; y < 35; y += 4) {
        for (int x = 5; x < 35; x += 4) {
            grid.SetCellBlocked(x, y, true);
            grid.SetCellBlocked(x + 1, y, true);
        }
    }

    ai::PathfindingConfig pfCfg;
    pfCfg.mode = ai::MovementMode::EightDirectional;
    pfCfg.preventCornerCutting = true;
    pfCfg.enableSmoothing = true;

    ai::Pathfinding pathfinder(grid, pfCfg);

    const int populations[] = { 5, 10, 25, 50 };

    for (int pop : populations) {
        auto startTime = std::chrono::high_resolution_clock::now();

        size_t successfulPaths = 0;
        for (int i = 0; i < pop; ++i) {
            float startX = 20.0f + static_cast<float>((i * 11) % 500);
            float startY = 20.0f + static_cast<float>((i * 13) % 500);
            float goalX = 300.0f;
            float goalY = 300.0f;

            ai::PathResult res = pathfinder.FindPath(Vector2{ startX, startY }, Vector2{ goalX, goalY });
            if (res.IsValid()) {
                successfulPaths++;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        double avgPerEntityMs = elapsedMs / pop;

        std::cout << "  -> Pop: " << pop << " entities | Time: " << elapsedMs
                  << " ms (Avg: " << avgPerEntityMs << " ms/entity) | Success: "
                  << successfulPaths << "/" << pop << std::endl;

        TEST_ASSERT(avgPerEntityMs < 5.0, "Average pathfinding per entity must be under 5.0 ms");
    }

    std::cout << "  -> Scalability & Performance benchmarks passed successfully." << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 29: AI & NAVIGATION EXPANSION TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestNavigationGrid();
    TestAStarPathfinding();
    TestPathFollowingAndMovement();
    TestObstacleAvoidanceAndStuckDetection();
    TestPerceptionAndLineOfSight();
    TestTargetSelectionAndMemory();
    TestTacticalBehaviors();
    TestCreatureProfiles();
    TestLuaAIIntegration();
    TestScalabilityAndPerformance();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 AI & NAVIGATION TEST GROUPS PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
