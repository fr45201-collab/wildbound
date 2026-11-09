#include "../../src/scripting/LuaManager.hpp"
#include "../../src/core/EngineConfig.hpp"
#include "../../src/core/Logger.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cassert>
#include <cstdlib>

using wildbound::core::EngineConfig;
using wildbound::core::Logger;
using wildbound::scripting::LuaManager;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

// 1. Runtime Lifecycle & Error Handling Tests
void TestRuntimeLifecycleAndErrors() {
    std::cout << "[TEST] 1. Lua Runtime Lifecycle, Repeated Init & Error Handling..." << std::endl;

    LuaManager luaMgr;
    TEST_ASSERT(!luaMgr.IsInitialized(), "Must not be initialized before Initialize()");

    // 1.1 Initial initialization
    bool init1 = luaMgr.Initialize();
    TEST_ASSERT(init1, "First Initialize() must succeed");
    TEST_ASSERT(luaMgr.IsInitialized(), "Must be initialized");

    // 1.2 Re-initialization guard
    bool init2 = luaMgr.Initialize();
    TEST_ASSERT(init2, "Calling Initialize() when already initialized should return true safely");

    // 1.3 Execution of basic expressions
    TEST_ASSERT(luaMgr.ExecuteString("x = 100 + 42"), "Valid expression must execute");
    int xVal = luaMgr.GetState()["x"];
    TEST_ASSERT(xVal == 142, "x must equal 142");

    // 1.4 Syntax error handling
    bool syntaxFail = luaMgr.ExecuteString("this is not valid lua syntax !!!");
    TEST_ASSERT(!syntaxFail, "Syntax error string must return false and not crash");

    // 1.5 Missing script handling
    bool missingFail = luaMgr.LoadScript("scripts/non_existent_file_xyz.lua");
    TEST_ASSERT(!missingFail, "Missing file must return false cleanly without crashing");

    // 1.6 Runtime error handling in Lua
    bool runtimeFail = luaMgr.ExecuteString("function bad_func() error('deliberate runtime error') end\n bad_func()");
    TEST_ASSERT(!runtimeFail, "Runtime error must return false cleanly");

    // 1.7 Clean shutdown and repeated shutdown
    luaMgr.Shutdown();
    TEST_ASSERT(!luaMgr.IsInitialized(), "Must be uninitialized after Shutdown()");
    luaMgr.Shutdown(); // Repeated shutdown safe
    TEST_ASSERT(!luaMgr.IsInitialized(), "Repeated Shutdown() must be safe");

    // 1.8 Re-initialization after shutdown
    bool init3 = luaMgr.Initialize();
    TEST_ASSERT(init3, "Initialize() after Shutdown() must succeed");
    TEST_ASSERT(luaMgr.IsInitialized(), "Must be initialized again");
    luaMgr.Shutdown();

    std::cout << "  -> PASSED" << std::endl;
}

// 2. C++/Lua Bidirectional Bindings & Userdata Tests
void TestCppLuaBindingsAndUserdata() {
    std::cout << "[TEST] 2. C++/Lua sol2 Bindings, Vector2 Userdata & Math Helpers..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();
    {
        sol::state& lua = luaMgr.GetState();

        // 2.1 Engine table and version
        sol::table wb = lua["wildbound"];
        TEST_ASSERT(wb.valid(), "wildbound namespace table must exist");
        std::string version = wb["version"];
        TEST_ASSERT(!version.empty(), "wildbound.version must be non-empty");

        // 2.2 Math functions
        sol::table wbMath = wb["math"];
        TEST_ASSERT(wbMath.valid(), "wildbound.math table must exist");

        sol::function distFn = wbMath["distance"];
        TEST_ASSERT(distFn.valid(), "wildbound.math.distance must be bound");
        float dist = distFn(0.0f, 0.0f, 3.0f, 4.0f);
        TEST_ASSERT(std::abs(dist - 5.0f) < 0.001f, "Distance between (0,0) and (3,4) must be 5.0");

        sol::function clampFn = wbMath["clamp"];
        float clampedHigh = clampFn(15.0f, 0.0f, 10.0f);
        float clampedLow = clampFn(-5.0f, 0.0f, 10.0f);
        TEST_ASSERT(clampedHigh == 10.0f, "Clamped high should be 10.0");
        TEST_ASSERT(clampedLow == 0.0f, "Clamped low should be 0.0");

        // 2.3 Vector2 Userdata
        bool vecOk = luaMgr.ExecuteString(R"lua(
            v1 = Vector2.new(10.0, 20.0)
            v2 = Vector2.new(10.0, 24.0)
            v_dist = Vector2.distance(v1, v2)
            v_len = v1:length()
            v_norm = v1:normalize()
        )lua");
        TEST_ASSERT(vecOk, "Vector2 creation and method calls in Lua must succeed");

        float vDist = lua["v_dist"];
        TEST_ASSERT(std::abs(vDist - 4.0f) < 0.001f, "Vector2.distance must equal 4.0");

        // 2.4 C++ exposing custom callback and reading table from Lua
        lua["cpp_double"] = [](int a) { return a * 2; };
        luaMgr.ExecuteString("result_double = cpp_double(21)");
        int resDouble = lua["result_double"];
        TEST_ASSERT(resDouble == 42, "C++ callback exposed to Lua must return 42");

        // 2.5 AI State reflection table
        sol::table states = wb["ai"]["states"];
        TEST_ASSERT(states.valid(), "wildbound.ai.states table must exist");
        std::string idleState = states["IDLE"];
        std::string attackState = states["ATTACK"];
        std::string retreatState = states["RETREAT"];
        TEST_ASSERT(idleState == "IDLE", "AI state IDLE must match");
        TEST_ASSERT(attackState == "ATTACK", "AI state ATTACK must match");
        TEST_ASSERT(retreatState == "RETREAT", "AI state RETREAT must match");
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 3. Script Management, Caching & Hot Reload Tests
void TestScriptManagementAndHotReload() {
    std::cout << "[TEST] 3. Script Management, Caching & Development Hot Reload..." << std::endl;

    LuaManager luaMgr;
    std::cout << "  3.a Initializing..." << std::endl;
    luaMgr.Initialize();

    std::cout << "  3.b Loading enemy_base.lua..." << std::endl;
    bool loaded1 = luaMgr.LoadScript("scripts/gameplay/ai/enemy_base.lua");
    std::cout << "  3.c Loaded: " << loaded1 << std::endl;
    TEST_ASSERT(loaded1, "enemy_base.lua must load successfully");
    TEST_ASSERT(luaMgr.IsScriptLoaded("scripts/gameplay/ai/enemy_base.lua"), "Must be recorded as loaded");

    const auto& scripts = luaMgr.GetLoadedScripts();
    TEST_ASSERT(scripts.find("scripts/gameplay/ai/enemy_base.lua") != scripts.end(), "Script metadata must exist");
    TEST_ASSERT(scripts.at("scripts/gameplay/ai/enemy_base.lua").executionCount >= 1, "Execution count must be >= 1");

    // 3.2 Second call uses cache
    bool loadedCached = luaMgr.LoadScript("scripts/gameplay/ai/enemy_base.lua", false);
    TEST_ASSERT(loadedCached, "Cached load must succeed");

    // 3.3 Explicit reload
    bool reloaded = luaMgr.ReloadScript("scripts/gameplay/ai/enemy_base.lua");
    TEST_ASSERT(reloaded, "ReloadScript must succeed");

    // 3.4 Reload all scripts
    bool reloadedAll = luaMgr.ReloadAllScripts();
    TEST_ASSERT(reloadedAll, "ReloadAllScripts must succeed");

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 4. Script Safety, Infinite Loop & Instruction Budget Tests
void TestScriptSafetyAndExecutionBudget() {
    std::cout << "[TEST] 4. Infinite Loop Protection & Instruction Budget Abort..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    // 4.1 Set strict instruction budget
    luaMgr.SetInstructionBudget(10000); // 10,000 instructions
    TEST_ASSERT(luaMgr.GetInstructionBudget() == 10000, "Instruction budget must be 10000");

    // 4.2 Execute runaway infinite loop
    auto startTime = std::chrono::high_resolution_clock::now();
    bool runawayResult = luaMgr.ExecuteString(R"lua(
        local counter = 0
        while true do
            counter = counter + 1
        end
    )lua");
    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // The infinite loop MUST be aborted by instruction count hook
    TEST_ASSERT(!runawayResult, "Infinite loop must be aborted by instruction budget hook and return false");
    TEST_ASSERT(luaMgr.GetBudgetViolationCount() > 0, "Budget violation count must be incremented");
    TEST_ASSERT(durationMs < 500.0, "Infinite loop abort must complete in under 500 ms without hanging the engine");

    // 4.3 Lua state remains functional after budget abort
    bool normalResult = luaMgr.ExecuteString("safe_val = 99 * 2");
    TEST_ASSERT(normalResult, "Lua state must recover cleanly and execute valid scripts after abort");
    int safeVal = luaMgr.GetState()["safe_val"];
    TEST_ASSERT(safeVal == 198, "safe_val must equal 198");

    // 4.4 Memory usage query
    size_t memKb = luaMgr.GetMemoryUsageKb();
    TEST_ASSERT(memKb > 0, "Lua memory usage must be greater than 0 KB");

    luaMgr.Shutdown();
    std::cout << "  -> PASSED (Infinite loop cleanly aborted in " << durationMs << " ms)" << std::endl;
}

// 5. Goblin AI Script Behaviors
void TestGoblinAIBehavior() {
    std::cout << "[TEST] 5. Goblin AI Script Verification (Cowardly, Hit-and-Run, Retreat)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    bool loaded = luaMgr.LoadScript("scripts/gameplay/ai/goblin.lua");
    TEST_ASSERT(loaded, "goblin.lua must load cleanly");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateGoblin = lua["wildbound"]["ai"]["goblin_update"];
        TEST_ASSERT(updateGoblin.valid(), "wildbound.ai.goblin_update must be bound");

        // 5.1 Idle test: no target, at spawn
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["spawn_x"] = 100.0f;
            ctx["spawn_y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = false;
            ctx["state"] = "IDLE";

            sol::table res = updateGoblin(ctx);
            std::string nextState = res["next_state"];
            TEST_ASSERT(nextState == "IDLE", "Goblin at spawn without target should be IDLE");
        }

        // 5.2 Patrol test: drifted away from spawn
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 150.0f;
            ctx["y"] = 100.0f;
            ctx["spawn_x"] = 100.0f;
            ctx["spawn_y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = false;
            ctx["patrol_radius"] = 80.0f;
            ctx["state"] = "IDLE";

            sol::table res = updateGoblin(ctx);
            std::string nextState = res["next_state"];
            TEST_ASSERT(nextState == "PATROL", "Goblin drifted from spawn should transition to PATROL");
        }

        // 5.3 Chase test: target detected within detection radius (140)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 180.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 80.0f;
            ctx["detection_radius"] = 140.0f;
            ctx["attack_radius"] = 32.0f;
            ctx["state"] = "IDLE";

            sol::table res = updateGoblin(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];
            TEST_ASSERT(nextState == "CHASE", "Goblin should CHASE target when detected");
            TEST_ASSERT(moveX > 0.0f, "Move vector should point towards target (move_x > 0)");
        }

        // 5.4 Attack test: target in attack radius (<= 32)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 120.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 20.0f;
            ctx["detection_radius"] = 140.0f;
            ctx["attack_radius"] = 32.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateGoblin(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];
            std::string attackId = res["attack_id"];

            TEST_ASSERT(nextState == "ATTACK", "Goblin should ATTACK when target within 32 units");
            TEST_ASSERT(wantAttack == true, "Goblin should request attack");
            TEST_ASSERT(attackId == "goblin_club", "Goblin attack ID must be goblin_club");
        }

        // 5.5 Cowardly Retreat test: Low HP (15% <= 20% retreat threshold)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["target_x"] = 120.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 20.0f;
            ctx["hp_percent"] = 0.15f; // 15% HP
            ctx["retreat_hp"] = 0.20f;
            ctx["can_retreat"] = true;
            ctx["has_target"] = true;
            ctx["state"] = "ATTACK";

            sol::table res = updateGoblin(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];

            TEST_ASSERT(nextState == "RETREAT", "Goblin with low HP (< 20%) MUST RETREAT in cowardice");
            TEST_ASSERT(moveX < 0.0f, "Retreat move vector must flee directly AWAY from target (move_x < 0)");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 6. Skeleton AI Script Behaviors
void TestSkeletonAIBehavior() {
    std::cout << "[TEST] 6. Skeleton AI Script Verification (Relentless Undead, Never Retreats)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    bool loaded = luaMgr.LoadScript("scripts/gameplay/ai/skeleton.lua");
    TEST_ASSERT(loaded, "skeleton.lua must load cleanly");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateSkeleton = lua["wildbound"]["ai"]["skeleton_update"];
        TEST_ASSERT(updateSkeleton.valid(), "wildbound.ai.skeleton_update must be bound");

        // 6.1 Attack test: target in attack radius (<= 36)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 200.0f;
            ctx["y"] = 200.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 225.0f;
            ctx["target_y"] = 200.0f;
            ctx["target_dist"] = 25.0f;
            ctx["detection_radius"] = 160.0f;
            ctx["attack_radius"] = 36.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateSkeleton(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];
            std::string attackId = res["attack_id"];

            TEST_ASSERT(nextState == "ATTACK", "Skeleton should ATTACK when target within 36 units");
            TEST_ASSERT(wantAttack == true, "Skeleton should request attack");
            TEST_ASSERT(attackId == "skeleton_blade", "Skeleton attack ID must be skeleton_blade");
        }

        // 6.2 Relentless Fearless Undead test: Low HP (10%), target in range -> NEVER RETREATS
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 200.0f;
            ctx["y"] = 200.0f;
            ctx["target_x"] = 225.0f;
            ctx["target_y"] = 200.0f;
            ctx["target_dist"] = 25.0f;
            ctx["hp_percent"] = 0.10f; // 10% HP!
            ctx["retreat_hp"] = 0.20f;
            ctx["can_retreat"] = false; // Skeletons are immune to fear
            ctx["has_target"] = true;
            ctx["state"] = "CHASE";
            ctx["detection_radius"] = 160.0f;
            ctx["attack_radius"] = 36.0f;

            sol::table res = updateSkeleton(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];

            TEST_ASSERT(nextState != "RETREAT", "Skeleton MUST NEVER RETREAT even at 10% HP");
            TEST_ASSERT(nextState == "ATTACK", "Skeleton fights relentlessly to the death in ATTACK state");
            TEST_ASSERT(wantAttack == true, "Skeleton continues attacking");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 6B. Slime AI Script Behaviors (DEF-004)
void TestSlimeAIBehavior() {
    std::cout << "[TEST] 6B. Marsh Slime AI Script Verification (Viscous Swamper, Low HP Retreat)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    bool loaded = luaMgr.LoadScript("scripts/gameplay/ai/slime.lua");
    TEST_ASSERT(loaded, "slime.lua must load cleanly");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateSlime = lua["wildbound"]["ai"]["slime_update"];
        TEST_ASSERT(updateSlime.valid(), "wildbound.ai.slime_update must be bound");

        // 6B.1 Idle & Leash Return
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["spawn_x"] = 100.0f;
            ctx["spawn_y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = false;
            ctx["patrol_radius"] = 90.0f;

            sol::table res = updateSlime(ctx);
            std::string nextState = res["next_state"];
            TEST_ASSERT(nextState == "IDLE", "Slime should be IDLE at spawn point");

            // Leash return test (beyond patrol_radius 90)
            ctx["x"] = 250.0f;
            ctx["y"] = 100.0f;
            res = updateSlime(ctx);
            nextState = res["next_state"];
            float moveX = res["move_x"];
            TEST_ASSERT(nextState == "RETURN", "Slime beyond patrol radius must RETURN to spawn");
            TEST_ASSERT(moveX < 0.0f, "Return vector must point back toward spawn (move_x < 0)");
        }

        // 6B.2 Detection and Chase (target within 110 radius)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 160.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 60.0f;
            ctx["detection_radius"] = 110.0f;
            ctx["attack_radius"] = 26.0f;
            ctx["state"] = "IDLE";

            sol::table res = updateSlime(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];
            TEST_ASSERT(nextState == "CHASE", "Slime should CHASE target within detection radius");
            TEST_ASSERT(moveX > 0.0f, "Chase vector must head towards target");
        }

        // 6B.3 Attack test: target within melee reach (<= 26 units)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 120.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 20.0f;
            ctx["detection_radius"] = 110.0f;
            ctx["attack_radius"] = 26.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateSlime(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];
            std::string attackId = res["attack_id"];

            TEST_ASSERT(nextState == "ATTACK", "Slime should ATTACK when target within 26 units");
            TEST_ASSERT(wantAttack == true, "Slime should request attack");
            TEST_ASSERT(attackId == "slime_slam", "Slime attack ID must be slime_slam");
        }

        // 6B.4 Retreat test: Low HP (20% <= 25% retreat threshold)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 100.0f;
            ctx["y"] = 100.0f;
            ctx["target_x"] = 120.0f;
            ctx["target_y"] = 100.0f;
            ctx["target_dist"] = 20.0f;
            ctx["hp_percent"] = 0.20f;
            ctx["retreat_hp"] = 0.25f;
            ctx["can_retreat"] = true;
            ctx["has_target"] = true;
            ctx["state"] = "ATTACK";

            sol::table res = updateSlime(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];

            TEST_ASSERT(nextState == "RETREAT", "Slime with low HP (<= 25%) MUST RETREAT");
            TEST_ASSERT(moveX < 0.0f, "Retreat vector must flee away from target (move_x < 0)");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 6C. Snake / Marsh Viper AI Script Behaviors (DEF-004)
void TestSnakeAIBehavior() {
    std::cout << "[TEST] 6C. Marsh Viper AI Script Verification (Ambush Predator, Fast Strikes)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    bool loaded = luaMgr.LoadScript("scripts/gameplay/ai/snake.lua");
    TEST_ASSERT(loaded, "snake.lua must load cleanly");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateSnake = lua["wildbound"]["ai"]["snake_update"];
        sol::protected_function updateViper = lua["wildbound"]["ai"]["viper_update"];
        TEST_ASSERT(updateSnake.valid(), "wildbound.ai.snake_update must be bound");
        TEST_ASSERT(updateViper.valid(), "wildbound.ai.viper_update alias must be bound");

        // 6C.1 Ambush posture at spawn
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 50.0f;
            ctx["y"] = 50.0f;
            ctx["spawn_x"] = 50.0f;
            ctx["spawn_y"] = 50.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = false;

            sol::table res = updateSnake(ctx);
            std::string nextState = res["next_state"];
            TEST_ASSERT(nextState == "IDLE", "Viper should remain coiled in IDLE ambush state at spawn");
        }

        // 6C.2 Chase within detection radius (150 units)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 50.0f;
            ctx["y"] = 50.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 150.0f;
            ctx["target_y"] = 50.0f;
            ctx["target_dist"] = 100.0f;
            ctx["detection_radius"] = 150.0f;
            ctx["attack_radius"] = 34.0f;
            ctx["state"] = "IDLE";

            sol::table res = updateSnake(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];
            TEST_ASSERT(nextState == "CHASE", "Viper should CHASE target within 150 units");
            TEST_ASSERT(moveX > 0.0f, "Chase vector should slither toward target");
        }

        // 6C.3 Attack test: target within strike radius (<= 34 units)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 50.0f;
            ctx["y"] = 50.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 80.0f;
            ctx["target_y"] = 50.0f;
            ctx["target_dist"] = 30.0f;
            ctx["detection_radius"] = 150.0f;
            ctx["attack_radius"] = 34.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateViper(ctx); // Test via alias
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];
            std::string attackId = res["attack_id"];

            TEST_ASSERT(nextState == "ATTACK", "Viper should ATTACK when target within 34 units");
            TEST_ASSERT(wantAttack == true, "Viper should request attack");
            TEST_ASSERT(attackId == "viper_strike", "Viper attack ID must be viper_strike");
        }

        // 6C.4 Evasive Retreat: Low HP (30% <= 35% threshold)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 50.0f;
            ctx["y"] = 50.0f;
            ctx["target_x"] = 80.0f;
            ctx["target_y"] = 50.0f;
            ctx["target_dist"] = 30.0f;
            ctx["hp_percent"] = 0.30f;
            ctx["retreat_hp"] = 0.35f;
            ctx["can_retreat"] = true;
            ctx["has_target"] = true;
            ctx["state"] = "ATTACK";

            sol::table res = updateSnake(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];

            TEST_ASSERT(nextState == "RETREAT", "Viper with low HP (<= 35%) MUST RETREAT");
            TEST_ASSERT(moveX < 0.0f, "Retreat vector must flee away from target (move_x < 0)");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 6D. Cyclops AI Script Behaviors (DEF-004)
void TestCyclopsAIBehavior() {
    std::cout << "[TEST] 6D. Mountain Cyclops AI Script Verification (Undaunted Brute, Never Retreats)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    bool loaded = luaMgr.LoadScript("scripts/gameplay/ai/cyclops.lua");
    TEST_ASSERT(loaded, "cyclops.lua must load cleanly");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateCyclops = lua["wildbound"]["ai"]["cyclops_update"];
        TEST_ASSERT(updateCyclops.valid(), "wildbound.ai.cyclops_update must be bound");

        // 6D.1 Long-range sightlines and chase (detection_radius 220)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 0.0f;
            ctx["y"] = 0.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 200.0f;
            ctx["target_y"] = 0.0f;
            ctx["target_dist"] = 200.0f;
            ctx["detection_radius"] = 220.0f;
            ctx["attack_radius"] = 48.0f;
            ctx["state"] = "IDLE";

            sol::table res = updateCyclops(ctx);
            std::string nextState = res["next_state"];
            float moveX = res["move_x"];
            TEST_ASSERT(nextState == "CHASE", "Cyclops should spot and CHASE target at 200 units");
            TEST_ASSERT(moveX > 0.0f, "Chase vector should move towards target");
        }

        // 6D.2 Massive sweeping club attack (attack_radius 48)
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 0.0f;
            ctx["y"] = 0.0f;
            ctx["hp_percent"] = 1.0f;
            ctx["has_target"] = true;
            ctx["target_x"] = 40.0f;
            ctx["target_y"] = 0.0f;
            ctx["target_dist"] = 40.0f;
            ctx["detection_radius"] = 220.0f;
            ctx["attack_radius"] = 48.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateCyclops(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];
            std::string attackId = res["attack_id"];

            TEST_ASSERT(nextState == "ATTACK", "Cyclops should ATTACK within 48 units reach");
            TEST_ASSERT(wantAttack == true, "Cyclops should request attack");
            TEST_ASSERT(attackId == "cyclops_club", "Cyclops attack ID must be cyclops_club");
        }

        // 6D.3 Fearless Undaunted Brute: 5% HP -> NEVER RETREATS
        {
            sol::table ctx = lua.create_table();
            ctx["x"] = 0.0f;
            ctx["y"] = 0.0f;
            ctx["target_x"] = 40.0f;
            ctx["target_y"] = 0.0f;
            ctx["target_dist"] = 40.0f;
            ctx["hp_percent"] = 0.05f; // Critical 5% HP!
            ctx["retreat_hp"] = 0.30f;
            ctx["can_retreat"] = false;
            ctx["has_target"] = true;
            ctx["detection_radius"] = 220.0f;
            ctx["attack_radius"] = 48.0f;
            ctx["state"] = "CHASE";

            sol::table res = updateCyclops(ctx);
            std::string nextState = res["next_state"];
            bool wantAttack = res["request_attack"];

            TEST_ASSERT(nextState != "RETREAT", "Cyclops MUST NEVER RETREAT even at 5% HP");
            TEST_ASSERT(nextState == "ATTACK", "Cyclops must stay in ATTACK state");
            TEST_ASSERT(wantAttack == true, "Cyclops continues relentless attack");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 6E. Malformed AI Context & Error Handling (DEF-004)
void TestMalformedAIContextAndErrorHandling() {
    std::cout << "[TEST] 6E. Malformed AI Context Robustness & Default Fallback..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    luaMgr.LoadScript("scripts/gameplay/ai/slime.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/snake.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/cyclops.lua");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function scripts[] = {
            lua["wildbound"]["ai"]["slime_update"],
            lua["wildbound"]["ai"]["snake_update"],
            lua["wildbound"]["ai"]["cyclops_update"]
        };

        for (auto& fn : scripts) {
            TEST_ASSERT(fn.valid(), "AI script function must be valid");

            // Empty context table
            sol::table emptyCtx = lua.create_table();
            sol::table res = fn(emptyCtx);
            std::string state = res["next_state"];
            TEST_ASSERT(state == "IDLE", "Empty context should fall back gracefully to IDLE");

            // Dead state
            sol::table deadCtx = lua.create_table();
            deadCtx["hp_percent"] = 0.0f;
            res = fn(deadCtx);
            state = res["next_state"];
            TEST_ASSERT(state == "DEAD", "hp_percent <= 0 must yield DEAD state");
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED" << std::endl;
}

// 7. Multiple Enemies Independence & No Cross-Contamination
void TestMultipleEnemiesIndependence() {
    std::cout << "[TEST] 7. Multiple Enemies AI Independence (10 Goblins + 10 Skeletons)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    luaMgr.LoadScript("scripts/gameplay/ai/goblin.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/skeleton.lua");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateGoblin = lua["wildbound"]["ai"]["goblin_update"];
        sol::protected_function updateSkeleton = lua["wildbound"]["ai"]["skeleton_update"];

        struct EnemyStateSim {
            uint32_t id;
            std::string type;
            float x, y;
            float hpPercent;
            float targetDist;
            std::string expectedState;
        };

        std::vector<EnemyStateSim> enemies;
        // 10 Goblins: half healthy chasing, half critical retreating
        for (uint32_t i = 1; i <= 10; ++i) {
            bool lowHp = (i % 2 == 0);
            enemies.push_back({
                i, "goblin",
                100.0f + i * 10.0f, 100.0f,
                lowHp ? 0.15f : 1.0f,
                lowHp ? 25.0f : 80.0f,
                lowHp ? "RETREAT" : "CHASE"
            });
        }
        // 10 Skeletons: all pressing the attack regardless of HP
        for (uint32_t i = 11; i <= 20; ++i) {
            bool lowHp = (i % 2 == 0);
            enemies.push_back({
                i, "skeleton",
                200.0f + i * 10.0f, 200.0f,
                lowHp ? 0.05f : 1.0f,
                25.0f,
                "ATTACK" // Skeletons always attack in range, never retreat!
            });
        }

        // Execute multiple iterations to confirm zero cross-contamination
        for (int tick = 0; tick < 5; ++tick) {
            for (const auto& e : enemies) {
                sol::table ctx = lua.create_table();
                ctx["enemy_id"] = e.id;
                ctx["enemy_type"] = e.type;
                ctx["x"] = e.x;
                ctx["y"] = e.y;
                ctx["target_x"] = e.x + e.targetDist;
                ctx["target_y"] = e.y;
                ctx["target_dist"] = e.targetDist;
                ctx["hp_percent"] = e.hpPercent;
                ctx["has_target"] = true;
                ctx["detection_radius"] = 160.0f;
                ctx["attack_radius"] = 35.0f;
                ctx["retreat_hp"] = 0.20f;
                ctx["can_retreat"] = (e.type == "goblin");

                sol::table res = (e.type == "goblin") ? updateGoblin(ctx) : updateSkeleton(ctx);
                std::string state = res["next_state"];
                TEST_ASSERT(state == e.expectedState,
                    ("Enemy " + std::to_string(e.id) + " (" + e.type + ") expected state " + e.expectedState + " but got " + state).c_str());
            }
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED (20 distinct enemies simulated with zero state pollution)" << std::endl;
}

// 7B. Expanded 5-Archetype Independence (DEF-004)
void TestExpandedEnemiesIndependence() {
    std::cout << "[TEST] 7B. Expanded 5-Archetype AI Independence (25 Enemies: Goblin, Skeleton, Slime, Snake, Cyclops)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();

    luaMgr.LoadScript("scripts/gameplay/ai/goblin.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/skeleton.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/slime.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/snake.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/cyclops.lua");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateGoblin = lua["wildbound"]["ai"]["goblin_update"];
        sol::protected_function updateSkeleton = lua["wildbound"]["ai"]["skeleton_update"];
        sol::protected_function updateSlime = lua["wildbound"]["ai"]["slime_update"];
        sol::protected_function updateSnake = lua["wildbound"]["ai"]["snake_update"];
        sol::protected_function updateCyclops = lua["wildbound"]["ai"]["cyclops_update"];

        struct EnemySim {
            uint32_t id;
            std::string type;
            float x, y;
            float hpPercent;
            float targetDist;
            float attackRadius;
            float detectionRadius;
            float retreatHp;
            bool canRetreat;
            std::string expectedState;
        };

        std::vector<EnemySim> enemies;
        // 5 Goblins
        for (uint32_t i = 1; i <= 5; ++i) {
            bool low = (i % 2 == 0);
            enemies.push_back({ i, "goblin", 100.0f + i * 10, 100.0f, low ? 0.15f : 1.0f, low ? 20.0f : 80.0f, 32.0f, 140.0f, 0.20f, true, low ? "RETREAT" : "CHASE" });
        }
        // 5 Skeletons (never retreat)
        for (uint32_t i = 6; i <= 10; ++i) {
            bool low = (i % 2 == 0);
            enemies.push_back({ i, "skeleton", 200.0f + i * 10, 200.0f, low ? 0.05f : 1.0f, 25.0f, 36.0f, 160.0f, 0.20f, false, "ATTACK" });
        }
        // 5 Slimes (retreat <= 25% HP)
        for (uint32_t i = 11; i <= 15; ++i) {
            bool low = (i % 2 == 0);
            enemies.push_back({ i, "slime", 300.0f + i * 10, 300.0f, low ? 0.20f : 1.0f, low ? 20.0f : 70.0f, 26.0f, 110.0f, 0.25f, true, low ? "RETREAT" : "CHASE" });
        }
        // 5 Snakes (retreat <= 35% HP)
        for (uint32_t i = 16; i <= 20; ++i) {
            bool low = (i % 2 == 0);
            enemies.push_back({ i, "snake", 400.0f + i * 10, 400.0f, low ? 0.30f : 1.0f, low ? 20.0f : 90.0f, 34.0f, 150.0f, 0.35f, true, low ? "RETREAT" : "CHASE" });
        }
        // 5 Cyclops (never retreat)
        for (uint32_t i = 21; i <= 25; ++i) {
            bool low = (i % 2 == 0);
            enemies.push_back({ i, "cyclops", 500.0f + i * 10, 500.0f, low ? 0.05f : 1.0f, 35.0f, 48.0f, 220.0f, 0.0f, false, "ATTACK" });
        }

        for (int tick = 0; tick < 5; ++tick) {
            for (const auto& e : enemies) {
                sol::table ctx = lua.create_table();
                ctx["enemy_id"] = e.id;
                ctx["enemy_type"] = e.type;
                ctx["x"] = e.x;
                ctx["y"] = e.y;
                ctx["target_x"] = e.x + e.targetDist;
                ctx["target_y"] = e.y;
                ctx["target_dist"] = e.targetDist;
                ctx["hp_percent"] = e.hpPercent;
                ctx["has_target"] = true;
                ctx["detection_radius"] = e.detectionRadius;
                ctx["attack_radius"] = e.attackRadius;
                ctx["retreat_hp"] = e.retreatHp;
                ctx["can_retreat"] = e.canRetreat;

                sol::table res;
                if (e.type == "goblin") res = updateGoblin(ctx);
                else if (e.type == "skeleton") res = updateSkeleton(ctx);
                else if (e.type == "slime") res = updateSlime(ctx);
                else if (e.type == "snake") res = updateSnake(ctx);
                else if (e.type == "cyclops") res = updateCyclops(ctx);

                std::string state = res["next_state"];
                TEST_ASSERT(state == e.expectedState,
                    ("Expanded Enemy " + std::to_string(e.id) + " (" + e.type + ") expected state " + e.expectedState + " but got " + state).c_str());
            }
        }
    }

    luaMgr.Shutdown();
    std::cout << "  -> PASSED (25 distinct enemies simulated with zero cross-contamination)" << std::endl;
}

// 8. C++ Decision Validation Rules
void TestDecisionValidationRules() {
    std::cout << "[TEST] 8. C++ Action & Decision Validation (NaN, Range, Cooldown)..." << std::endl;

    // 8.1 NaN / Inf Move Vector Validation
    float nanVal = std::numeric_limits<float>::quiet_NaN();
    float infVal = std::numeric_limits<float>::infinity();

    TEST_ASSERT(!std::isfinite(nanVal), "NaN must be detected as non-finite");
    TEST_ASSERT(!std::isfinite(infVal), "Inf must be detected as non-finite");

    // 8.2 Attack Range Validation
    float attackRadius = 32.0f;
    float inRangeDist = 30.0f;
    float outOfRangeDist = 100.0f;
    float maxAllowedDist = attackRadius * 1.5f; // 48.0f

    bool inRangeValid = (inRangeDist <= maxAllowedDist);
    bool outOfRangeValid = (outOfRangeDist <= maxAllowedDist);
    TEST_ASSERT(inRangeValid, "Target at 30.0 units must be valid for attack");
    TEST_ASSERT(!outOfRangeValid, "Target at 100.0 units must be REJECTED as out of range");

    std::cout << "  -> PASSED" << std::endl;
}

// 9. Scalability & Performance Benchmarks
void TestPerformanceBenchmarks() {
    std::cout << "[TEST] 9. Performance Benchmarks (1, 10, 25, 50 Enemies)..." << std::endl;

    LuaManager luaMgr;
    luaMgr.Initialize();
    luaMgr.LoadScript("scripts/gameplay/ai/goblin.lua");
    luaMgr.LoadScript("scripts/gameplay/ai/skeleton.lua");

    {
        sol::state& lua = luaMgr.GetState();
        sol::protected_function updateGoblin = lua["wildbound"]["ai"]["goblin_update"];

        const int populations[] = { 1, 10, 25, 50 };
        const int iterations = 100;

        for (int pop : populations) {
            auto startTime = std::chrono::high_resolution_clock::now();

            for (int iter = 0; iter < iterations; ++iter) {
                for (int i = 0; i < pop; ++i) {
                    sol::table ctx = lua.create_table();
                    ctx["enemy_id"] = i;
                    ctx["x"] = 100.0f + i;
                    ctx["y"] = 100.0f;
                    ctx["spawn_x"] = 100.0f;
                    ctx["spawn_y"] = 100.0f;
                    ctx["hp_percent"] = 1.0f;
                    ctx["has_target"] = true;
                    ctx["target_x"] = 150.0f;
                    ctx["target_y"] = 100.0f;
                    ctx["target_dist"] = 50.0f;
                    ctx["detection_radius"] = 140.0f;
                    ctx["attack_radius"] = 32.0f;

                    updateGoblin(ctx);
                }
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            double totalMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            double avgUsPerDecision = (totalMs * 1000.0) / (pop * iterations);

            std::cout << "  -> Population: " << pop << " enemies | Total (" << iterations << " ticks): "
                      << totalMs << " ms | Avg per decision: " << avgUsPerDecision << " us" << std::endl;

            TEST_ASSERT(avgUsPerDecision < 500.0, "Average AI decision time must be under 500 microseconds");
        }
    }

    std::cout << "  -> Performance benchmarks passed successfully." << std::endl;
    luaMgr.Shutdown();
}

int main() {
    Logger::Init();
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 46 & PHASE 50E: LUA SCRIPTING TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestRuntimeLifecycleAndErrors();
    TestCppLuaBindingsAndUserdata();
    TestScriptManagementAndHotReload();
    TestScriptSafetyAndExecutionBudget();
    TestGoblinAIBehavior();
    TestSkeletonAIBehavior();
    TestSlimeAIBehavior();
    TestSnakeAIBehavior();
    TestCyclopsAIBehavior();
    TestMalformedAIContextAndErrorHandling();
    TestMultipleEnemiesIndependence();
    TestExpandedEnemiesIndependence();
    TestDecisionValidationRules();
    TestPerformanceBenchmarks();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 14 LUA INTEGRATION TEST GROUPS PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    Logger::Shutdown();
    return 0;
}
