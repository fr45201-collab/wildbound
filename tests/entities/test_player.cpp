#include "../../src/entities/Player.hpp"
#include "../../src/entities/CharacterConfig.hpp"
#include "../../src/graphics/animation/Direction.hpp"
#include "../../src/graphics/animation/AnimationState.hpp"
#include "../../src/graphics/animation/AnimationController.hpp"
#include "../../src/graphics/animation/AnimationStateMachine.hpp"
#include "../../src/world/World.hpp"
#include "../../src/graphics/Camera2D.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>

using wildbound::core::Logger;
using namespace wildbound::entities;
using namespace wildbound::graphics::animation;
using namespace wildbound::world;
using namespace wildbound::resources;
using wildbound::graphics::Camera2DSystem;
using wildbound::graphics::CameraMode;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

static bool ApproxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) <= epsilon;
}

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Player & Animation System Tests ===" << std::endl;

    // Headless raylib context for texture/window operations
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundPlayerTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");
    assetMgr.LoadSpriteSheetMetadata("characters/player/player_spritesheet.json");
    assetMgr.LoadAnimationMetadata("characters/player/player_animations.json");

    // ========================================================================
    // Test 1: Player Initialization & Configuration Loading
    // ========================================================================
    {
        std::cout << "[TEST] 1. Player initialization & configuration loading..." << std::endl;
        CharacterConfig cfg = CharacterConfig::LoadFromFile("data/characters/player.json");
        TEST_ASSERT(cfg.id == "player", "Config ID should be 'player'");
        TEST_ASSERT(ApproxEqual(cfg.stats.walkSpeed, 100.0f), "Default walk speed should be 100 px/s");
        TEST_ASSERT(ApproxEqual(cfg.stats.runSpeed, 165.0f), "Default run speed should be 165 px/s");
        TEST_ASSERT(cfg.collision.enabled, "Collision should be enabled");
        TEST_ASSERT(ApproxEqual(cfg.collision.width, 16.0f), "Collision width should be 16 px");
        TEST_ASSERT(ApproxEqual(cfg.collision.height, 12.0f), "Collision height should be 12 px");

        Player player(1, "Explorer");
        player.ApplyConfig(cfg);
        TEST_ASSERT(player.GetName() == "Explorer", "Player name should match config");
        TEST_ASSERT(player.IsActive(), "Player should be active by default");
        TEST_ASSERT(player.GetMovementState() == MovementState::Idle, "Initial state should be Idle");
        TEST_ASSERT(player.GetFacing() == Direction::Down, "Initial facing should be Down");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 2: Movement Velocity & Delta-Time Integration
    // ========================================================================
    {
        std::cout << "[TEST] 2. Movement velocity calculations..." << std::endl;
        Player player(1, "Explorer");
        player.SetPosition(Vector2{ 100.0f, 100.0f });

        // Inject rightward movement
        player.GetController().SetManualInput(Vector2{ 1.0f, 0.0f }, false);
        player.Update(0.1f); // 100 ms

        Vector2 vel = player.GetVelocity();
        TEST_ASSERT(ApproxEqual(vel.x, 100.0f) && ApproxEqual(vel.y, 0.0f), "Velocity should be (100, 0)");
        TEST_ASSERT(ApproxEqual(player.GetPosition().x, 110.0f), "Position X should advance by 10 px in 0.1s");
        TEST_ASSERT(ApproxEqual(player.GetPosition().y, 100.0f), "Position Y should remain unchanged");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 3: Diagonal Normalization (No Diagonal Speedup)
    // ========================================================================
    {
        std::cout << "[TEST] 3. Diagonal movement normalization..." << std::endl;
        Player player(1, "Explorer");
        player.GetController().SetNormalizeDiagonal(true);

        // Inject diagonal input (1, 1)
        player.GetController().SetManualInput(Vector2{ 1.0f, 1.0f }, false);
        player.Update(0.1f);

        Vector2 dir = player.GetController().GetMovementDirection();
        float dirMag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        TEST_ASSERT(ApproxEqual(dirMag, 1.0f), "Normalized diagonal input direction must have length 1.0");

        Vector2 vel = player.GetVelocity();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        TEST_ASSERT(ApproxEqual(speed, 100.0f), "Diagonal walk speed must strictly equal 100.0 px/s (not 141.4)");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 4: Walking Speed
    // ========================================================================
    {
        std::cout << "[TEST] 4. Walking speed enforcement..." << std::endl;
        Player player(1, "Explorer");
        player.GetController().SetManualInput(Vector2{ 0.0f, -1.0f }, false); // Walk Up
        player.Update(1.0f);

        Vector2 vel = player.GetVelocity();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        TEST_ASSERT(ApproxEqual(speed, 100.0f), "Walking speed must be 100 px/s");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 5: Running Speed (Shift Modifier)
    // ========================================================================
    {
        std::cout << "[TEST] 5. Running speed (Shift modifier)..." << std::endl;
        Player player(1, "Explorer");
        player.GetController().SetManualInput(Vector2{ 0.0f, 1.0f }, true); // Run Down
        player.Update(1.0f);

        Vector2 vel = player.GetVelocity();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        TEST_ASSERT(ApproxEqual(speed, 165.0f), "Running speed must be 165 px/s");
        TEST_ASSERT(ApproxEqual(speed / 100.0f, 1.65f), "Run speed ratio must be exactly 1.65x walk speed");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 6: 4-Direction Orientation Detection
    // ========================================================================
    {
        std::cout << "[TEST] 6. Direction detection and bias..." << std::endl;
        TEST_ASSERT(VectorToDirection(Vector2{ 0.0f, 1.0f }) == Direction::Down, "(0, 1) should be Down");
        TEST_ASSERT(VectorToDirection(Vector2{ 0.0f, -1.0f }) == Direction::Up, "(0, -1) should be Up");
        TEST_ASSERT(VectorToDirection(Vector2{ -1.0f, 0.0f }) == Direction::Left, "(-1, 0) should be Left");
        TEST_ASSERT(VectorToDirection(Vector2{ 1.0f, 0.0f }) == Direction::Right, "(1, 0) should be Right");

        // Zero velocity should preserve facing
        TEST_ASSERT(VectorToDirection(Vector2{ 0.0f, 0.0f }, Direction::Left) == Direction::Left, "Zero vector preserves Left");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 7: Animation Frame Calculation (FPS-Independent)
    // ========================================================================
    {
        std::cout << "[TEST] 7. Animation frame elapsed-time calculation..." << std::endl;
        AnimationController animCtrl;
        animCtrl.Play("player_walk_down", true, 1.0f);

        // Frame duration is 0.14s:
        // t = 0.00s -> Frame 0
        // t = 0.15s -> Frame 1
        // t = 0.30s -> Frame 2
        // t = 0.45s -> Frame 3
        animCtrl.Update(0.00f, assetMgr);
        TEST_ASSERT(animCtrl.GetCurrentFrameIndex() == 0, "At t=0s, frame should be 0");

        animCtrl.Update(0.15f, assetMgr);
        TEST_ASSERT(animCtrl.GetCurrentFrameIndex() == 1, "At t=0.15s, frame should be 1");

        animCtrl.Update(0.15f, assetMgr); // t = 0.30s
        TEST_ASSERT(animCtrl.GetCurrentFrameIndex() == 2, "At t=0.30s, frame should be 2");

        animCtrl.Update(0.15f, assetMgr); // t = 0.45s
        TEST_ASSERT(animCtrl.GetCurrentFrameIndex() == 3, "At t=0.45s, frame should be 3");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 8: Animation State Machine Transitions
    // ========================================================================
    {
        std::cout << "[TEST] 8. Animation state machine transitions..." << std::endl;
        AnimationStateMachine sm("player");
        AnimationController ctrl;

        // 1. Idle initially
        sm.Update(Vector2{ 0.0f, 0.0f }, false, ctrl, assetMgr);
        TEST_ASSERT(sm.GetCurrentMovementState() == MovementState::Idle, "Zero velocity must yield Idle");
        TEST_ASSERT(ctrl.GetCurrentClipName() == "player_idle_down", "Clip must be player_idle_down");

        // 2. Transition: Idle -> Walking
        sm.Update(Vector2{ 100.0f, 0.0f }, false, ctrl, assetMgr);
        TEST_ASSERT(sm.GetCurrentMovementState() == MovementState::Walking, "Moving without shift yields Walking");
        TEST_ASSERT(sm.GetCurrentDirection() == Direction::Right, "Facing should update to Right");
        TEST_ASSERT(ctrl.GetCurrentClipName() == "player_walk_right", "Clip must transition to player_walk_right");

        // 3. Transition: Walking -> Running
        sm.Update(Vector2{ 165.0f, 0.0f }, true, ctrl, assetMgr);
        TEST_ASSERT(sm.GetCurrentMovementState() == MovementState::Running, "Shift modifier yields Running");
        TEST_ASSERT(ctrl.GetCurrentClipName() == "player_run_right", "Clip must transition to player_run_right");

        // 4. Transition: Running -> Idle
        sm.Update(Vector2{ 0.0f, 0.0f }, false, ctrl, assetMgr);
        TEST_ASSERT(sm.GetCurrentMovementState() == MovementState::Idle, "Stopping from Run yields Idle");
        TEST_ASSERT(sm.GetCurrentDirection() == Direction::Right, "Facing Right must be preserved in Idle");
        TEST_ASSERT(ctrl.GetCurrentClipName() == "player_idle_right", "Clip must transition to player_idle_right");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 9: Animation Looping
    // ========================================================================
    {
        std::cout << "[TEST] 9. Animation looping cycle..." << std::endl;
        AnimationController animCtrl;
        animCtrl.Play("player_walk_down", true, 1.0f);

        // 4 frames * 0.14s = 0.56s loop duration
        // Advancing by 0.60s should wrap smoothly back to frame 0
        animCtrl.Update(0.60f, assetMgr);
        TEST_ASSERT(animCtrl.GetCurrentFrameIndex() == 0, "After total duration, loop must wrap to frame 0");
        TEST_ASSERT(!animCtrl.IsFinished(), "Looping animation must never set IsFinished to true");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 10: Collision Rectangle Geometry
    // ========================================================================
    {
        std::cout << "[TEST] 10. Collision rectangle geometry & feet alignment..." << std::endl;
        Player player(1, "Explorer");
        CharacterCollisionConfig colCfg;
        colCfg.enabled = true;
        colCfg.width = 16.0f;
        colCfg.height = 12.0f;
        colCfg.offsetX = 0.0f;
        colCfg.offsetY = 0.0f;
        player.SetCollisionConfig(colCfg);

        player.SetPosition(Vector2{ 200.0f, 300.0f });
        Rectangle col = player.GetCollisionBounds();

        // Width 16 centered at x=200 -> [192, 208]
        TEST_ASSERT(ApproxEqual(col.x, 192.0f), "Collision X should be 192");
        TEST_ASSERT(ApproxEqual(col.width, 16.0f), "Collision width should be 16");

        // Height 12 grounded at y=300 -> [288, 300]
        TEST_ASSERT(ApproxEqual(col.y, 288.0f), "Collision Y should be 288");
        TEST_ASSERT(ApproxEqual(col.height, 12.0f), "Collision height should be 12");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 11: AABB Collision Detection
    // ========================================================================
    {
        std::cout << "[TEST] 11. AABB collision intersection detection..." << std::endl;
        CollisionBox obstacle;
        obstacle.bounds = Rectangle{ 100.0f, 100.0f, 32.0f, 32.0f };
        obstacle.type = CollisionType::Solid;

        // Player intersecting obstacle
        Rectangle playerCol1{ 110.0f, 110.0f, 16.0f, 12.0f };
        TEST_ASSERT(obstacle.Intersects(playerCol1), "Player inside obstacle should register collision");

        // Player outside obstacle
        Rectangle playerCol2{ 200.0f, 200.0f, 16.0f, 12.0f };
        TEST_ASSERT(!obstacle.Intersects(playerCol2), "Player outside obstacle must not register collision");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 12: Horizontal Collision Resolution
    // ========================================================================
    {
        std::cout << "[TEST] 12. Horizontal collision resolution..." << std::endl;
        World world;
        world.LoadMap("maps/forest/forest_01.json", assetMgr);

        Player player(1, "Explorer");
        CharacterConfig cfg = CharacterConfig::CreateDefault();
        player.ApplyConfig(cfg);

        // Cabin collision box is centered at X: [1865, 1975], Y: [393, 448]
        // Position player immediately to the left of the cabin wall (X = 1850, Y = 420)
        player.SetPosition(Vector2{ 1850.0f, 420.0f });

        // Move right into obstacle (+40 px)
        player.ResolveMovement(Vector2{ 40.0f, 0.0f }, world);

        // Player should be blocked and pushed out to the left of obstacle (X = 1857 < 1865)
        TEST_ASSERT(player.GetPosition().x < 1865.0f, "Player must not penetrate solid obstacle horizontally");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 13: Vertical Collision Resolution & Wall Sliding
    // ========================================================================
    {
        std::cout << "[TEST] 13. Vertical collision resolution & wall sliding..." << std::endl;
        World world;
        world.LoadMap("maps/forest/forest_01.json", assetMgr);

        Player player(1, "Explorer");
        CharacterConfig cfg = CharacterConfig::CreateDefault();
        player.ApplyConfig(cfg);

        // Place player adjacent to cabin wall (X = 1850, Y = 410)
        player.SetPosition(Vector2{ 1850.0f, 410.0f });

        // Move diagonally into the wall (Right +20 and Down +30)
        // Horizontal (X) should be blocked, while Vertical (Y) should slide freely!
        player.ResolveMovement(Vector2{ 20.0f, 30.0f }, world);

        TEST_ASSERT(player.GetPosition().x < 1865.0f, "X movement should be stopped at the wall");
        TEST_ASSERT(ApproxEqual(player.GetPosition().y, 440.0f), "Y movement must slide freely downwards without sticking!");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 14: Player Spawn Marker Loading from Map
    // ========================================================================
    {
        std::cout << "[TEST] 14. Player spawn marker loading from map..." << std::endl;
        World world;
        bool loaded = world.LoadMap("maps/forest/forest_01.json", assetMgr);
        TEST_ASSERT(loaded, "Failed to load forest_01 map");

        Vector2 spawn = world.GetSpawnPosition("player_spawn");
        TEST_ASSERT(ApproxEqual(spawn.x, 256.0f), "Spawn X coordinate should be 256.0");
        TEST_ASSERT(ApproxEqual(spawn.y, 960.0f), "Spawn Y coordinate should be 960.0");

        Player player(1, "Explorer");
        player.Spawn(spawn);
        TEST_ASSERT(ApproxEqual(player.GetPosition().x, 256.0f), "Player spawned at X=256");
        TEST_ASSERT(ApproxEqual(player.GetPosition().y, 960.0f), "Player spawned at Y=960");
        std::cout << "  -> PASSED" << std::endl;
    }

    // ========================================================================
    // Test 15: Camera Follow Integration
    // ========================================================================
    {
        std::cout << "[TEST] 15. Camera follow integration..." << std::endl;
        Camera2DSystem camera(640, 360);
        camera.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 2560.0f, 1920.0f });
        camera.SetFollowSpeed(10.0f);
        camera.SetMode(CameraMode::Follow);

        Player player(1, "Explorer");
        player.SetPosition(Vector2{ 500.0f, 500.0f });

        camera.SetTarget(Vector2{ 400.0f, 400.0f });
        camera.SetFollowTarget(player.GetPosition());

        // Update camera by 0.5 seconds
        camera.Update(0.5f);

        Vector2 camPos = camera.GetTarget();
        TEST_ASSERT(camPos.x > 400.0f && camPos.x <= 500.0f, "Camera target X should smoothly approach player");
        TEST_ASSERT(camPos.y > 400.0f && camPos.y <= 500.0f, "Camera target Y should smoothly approach player");
        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "\n========================================================" << std::endl;
    std::cout << "All Wildbound Player & Animation tests passed! (15/15)" << std::endl;
    std::cout << "========================================================" << std::endl;

    CloseWindow();
    return 0;
}

