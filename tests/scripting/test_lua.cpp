#include "../../src/scripting/LuaManager.hpp"
#include "../../src/core/EngineConfig.hpp"
#include "../../src/core/Logger.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <iostream>
#include <cassert>
#include <cstdlib>

using wildbound::core::EngineConfig;
using wildbound::core::Logger;
using wildbound::scripting::LuaManager;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

int main() {
    Logger::Init();
    std::cout << "=== Running Lua Scripting & Config Tests ===" << std::endl;

    LuaManager luaManager;

    // Test 3: Lua runtime initialization
    {
        std::cout << "[TEST] 3. Lua runtime initialization..." << std::endl;
        bool initSuccess = luaManager.Initialize();
        TEST_ASSERT(initSuccess, "Lua runtime must initialize successfully");
        TEST_ASSERT(luaManager.IsInitialized(), "LuaManager state must be marked initialized");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 4: Lua script execution
    {
        std::cout << "[TEST] 4. Lua script execution..." << std::endl;
        bool execSuccess = luaManager.ExecuteString(
            "test_calculation = 10 * 4 + 2\n"
            "test_message = 'Wildbound Phase 0'\n"
        );
        TEST_ASSERT(execSuccess, "Execution of basic Lua statements should succeed");

        sol::state& lua = luaManager.GetState();
        int calc = lua["test_calculation"];
        std::string msg = lua["test_message"];

        TEST_ASSERT(calc == 42, "Lua calculation variable should equal 42");
        TEST_ASSERT(msg == "Wildbound Phase 0", "Lua string variable should match");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 5: C++ reading Lua configuration values and table structure
    {
        std::cout << "[TEST] 5. C++ reading Lua configuration values..." << std::endl;
        const char* luaConfigCode = R"lua(
            return {
                game = {
                    title = "Wildbound Test Realm",
                    version = "0.9.9",
                    author = "Lead Architect"
                },
                display = {
                    window_width = 1920,
                    window_height = 1080,
                    internal_width = 640,
                    internal_height = 360,
                    target_fps = 144,
                    vsync = false,
                    resizable = true,
                    fullscreen = false
                },
                debug = {
                    enabled = true,
                    show_metrics_overlay = false,
                    log_level = "DEBUG"
                }
            }
        )lua";

        sol::state& lua = luaManager.GetState();
        auto result = lua.safe_script(luaConfigCode);
        TEST_ASSERT(result.valid(), "Lua configuration snippet should evaluate without error");

        sol::table root = result;
        sol::table gameTable = root["game"];
        sol::table displayTable = root["display"];

        std::string title = gameTable["title"];
        int winWidth = displayTable["window_width"];
        int winHeight = displayTable["window_height"];
        int targetFps = displayTable["target_fps"];
        bool vsync = displayTable["vsync"];

        TEST_ASSERT(title == "Wildbound Test Realm", "C++ must read Lua table game.title correctly");
        TEST_ASSERT(winWidth == 1920, "C++ must read Lua table display.window_width correctly");
        TEST_ASSERT(winHeight == 1080, "C++ must read Lua table display.window_height correctly");
        TEST_ASSERT(targetFps == 144, "C++ must read Lua table display.target_fps correctly");
        TEST_ASSERT(vsync == false, "C++ must read Lua table display.vsync correctly");

        // Also test LoadEngineConfig from actual project file
        EngineConfig parsedConfig;
        bool loaded = luaManager.LoadEngineConfig("scripts/config/engine.lua", parsedConfig);
        TEST_ASSERT(loaded, "LoadEngineConfig must successfully parse scripts/config/engine.lua");
        TEST_ASSERT(parsedConfig.gameTitle == "Wildbound", "Parsed title must be Wildbound");
        TEST_ASSERT(parsedConfig.windowWidth == 1280, "Parsed window width must be 1280");
        TEST_ASSERT(parsedConfig.windowHeight == 720, "Parsed window height must be 720");
        TEST_ASSERT(parsedConfig.internalWidth == 640, "Parsed internal width must be 640");
        TEST_ASSERT(parsedConfig.internalHeight == 360, "Parsed internal height must be 360");
        TEST_ASSERT(parsedConfig.targetFps == 60, "Parsed target FPS must be 60");
        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "All Lua Scripting tests passed successfully!" << std::endl;
    luaManager.Shutdown();
    Logger::Shutdown();
    return 0;
}

