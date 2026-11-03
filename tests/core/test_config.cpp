#include "../../src/core/EngineConfig.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>

using wildbound::core::EngineConfig;
using wildbound::core::Logger;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

int main() {
    Logger::Init();
    std::cout << "=== Running Core Configuration Tests ===" << std::endl;

    // Test 1: Core configuration initialization (defaults)
    {
        std::cout << "[TEST] 1. Core configuration default initialization..." << std::endl;
        EngineConfig config = EngineConfig::CreateDefault();

        TEST_ASSERT(config.gameTitle == "Wildbound", "Default title must be 'Wildbound'");
        TEST_ASSERT(config.windowWidth == 1280, "Default window width must be 1280");
        TEST_ASSERT(config.windowHeight == 720, "Default window height must be 720");
        TEST_ASSERT(config.internalWidth == 640, "Default internal width must be 640");
        TEST_ASSERT(config.internalHeight == 360, "Default internal height must be 360");
        TEST_ASSERT(config.targetFps == 60, "Default target FPS must be 60");
        TEST_ASSERT(config.debugMode == true, "Default debug mode should be true");
        TEST_ASSERT(config.Validate() == true, "Default config must be valid");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 2: Engine configuration validation and boundaries
    {
        std::cout << "[TEST] 2. Engine configuration validation..." << std::endl;
        EngineConfig invalidConfig;
        invalidConfig.windowWidth = -100;
        TEST_ASSERT(!invalidConfig.Validate(), "Negative window width should fail validation");

        EngineConfig invalidInternal;
        invalidInternal.internalWidth = 0;
        TEST_ASSERT(!invalidInternal.Validate(), "Zero internal width should fail validation");

        EngineConfig customConfig;
        customConfig.windowWidth = 1920;
        customConfig.windowHeight = 1080;
        customConfig.internalWidth = 640;
        customConfig.internalHeight = 360;
        TEST_ASSERT(customConfig.Validate(), "Valid custom resolution should pass validation");
        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "All Core Configuration tests passed successfully!" << std::endl;
    Logger::Shutdown();
    return 0;
}

