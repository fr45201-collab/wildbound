#include "../../src/graphics/Viewport.hpp"
#include "../../src/graphics/Camera2D.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

using wildbound::core::Logger;
using wildbound::graphics::Viewport;
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

static bool ApproxEqualVec2(Vector2 a, Vector2 b, float epsilon = 0.01f) {
    return ApproxEqual(a.x, b.x, epsilon) && ApproxEqual(a.y, b.y, epsilon);
}

int main() {
    Logger::Init();
    std::cout << "=== Running Rendering, Viewport & Camera Tests ===" << std::endl;

    // Test 1: Virtual resolution calculations
    {
        std::cout << "[TEST] 1. Virtual resolution & default scaling..." << std::endl;
        Viewport vp(640, 360, 1280, 720);
        TEST_ASSERT(vp.GetVirtualWidth() == 640, "Virtual width must be 640");
        TEST_ASSERT(vp.GetVirtualHeight() == 360, "Virtual height must be 360");
        TEST_ASSERT(vp.GetWindowWidth() == 1280, "Window width must be 1280");
        TEST_ASSERT(vp.GetWindowHeight() == 720, "Window height must be 720");
        TEST_ASSERT(ApproxEqual(vp.GetScale(), 2.0f), "1280x720 / 640x360 must yield scale 2.0x");
        TEST_ASSERT(!vp.IsLetterboxed(), "Exact 16:9 2x scale should not be letterboxed");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 2: Aspect-ratio preservation
    {
        std::cout << "[TEST] 2. Aspect ratio preservation..." << std::endl;
        Viewport vp(640, 360, 1280, 720);
        TEST_ASSERT(ApproxEqual(vp.GetAspectRatio(), 16.0f / 9.0f), "Aspect ratio must be 16:9");

        // Resize to wider aspect (e.g. 21:9 - 2560x1080)
        vp.Update(2560, 1080);
        float scale = vp.GetScale();
        // Scale should be bounded by height: 1080 / 360 = 3.0
        TEST_ASSERT(ApproxEqual(scale, 3.0f), "2560x1080 uniform scale must be 3.0x bounded by height");
        Rectangle dst = vp.GetDestRect();
        TEST_ASSERT(ApproxEqual(dst.width, 640.0f * 3.0f), "Scaled width must be 1920");
        TEST_ASSERT(ApproxEqual(dst.height, 360.0f * 3.0f), "Scaled height must be 1080");
        TEST_ASSERT(vp.IsLetterboxed(), "Wider window must be pillarboxed");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 3: Viewport centering
    {
        std::cout << "[TEST] 3. Viewport centering..." << std::endl;
        // Window 1920x1200 (16:10) with 640x360 canvas (16:9)
        Viewport vp(640, 360, 1920, 1200);
        // Uniform scale: min(1920/640, 1200/360) = min(3.0, 3.333) = 3.0
        Rectangle dst = vp.GetDestRect();
        TEST_ASSERT(ApproxEqual(dst.x, 0.0f), "X offset should be 0 for top/bottom letterbox");
        TEST_ASSERT(ApproxEqual(dst.y, (1200.0f - 1080.0f) * 0.5f), "Y offset must be 60.0 (vertically centered)");

        // Pillarboxed window: 1600x720
        vp.Update(1600, 720);
        dst = vp.GetDestRect();
        TEST_ASSERT(ApproxEqual(dst.x, (1600.0f - 1280.0f) * 0.5f), "X offset must be 160.0 (horizontally centered)");
        TEST_ASSERT(ApproxEqual(dst.y, 0.0f), "Y offset should be 0");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 4: Screen-to-virtual coordinate conversion
    {
        std::cout << "[TEST] 4. Screen-to-Virtual coordinate conversion..." << std::endl;
        Viewport vp(640, 360, 1280, 720);
        Vector2 screenCenter{ 640.0f, 360.0f };
        Vector2 virtCenter = vp.ScreenToVirtual(screenCenter);
        TEST_ASSERT(ApproxEqualVec2(virtCenter, Vector2{ 320.0f, 180.0f }), "Screen center must map to virtual center (320, 180)");

        // Roundtrip conversion
        Vector2 screenRoundtrip = vp.VirtualToScreen(virtCenter);
        TEST_ASSERT(ApproxEqualVec2(screenRoundtrip, screenCenter), "VirtualToScreen roundtrip must match original screen pos");

        // Test with pillarbox offset (1600x720)
        vp.Update(1600, 720);
        // Viewport rect is [160, 0, 1280, 720]
        Vector2 offsetScreenPoint{ 160.0f, 0.0f };
        Vector2 virtOrigin = vp.ScreenToVirtual(offsetScreenPoint);
        TEST_ASSERT(ApproxEqualVec2(virtOrigin, Vector2{ 0.0f, 0.0f }), "Left margin of viewport must map to virtual (0, 0)");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 5: World-to-screen and Screen-to-world conversion
    {
        std::cout << "[TEST] 5. World-to-Screen coordinate conversion..." << std::endl;
        Viewport vp(640, 360, 1280, 720);
        Camera2DSystem camera(640, 360);
        camera.SetTarget(Vector2{ 800.0f, 600.0f });
        camera.SetZoom(1.0f);

        // World target should project directly to virtual center (320, 180)
        Vector2 virtPos = camera.WorldToVirtual(Vector2{ 800.0f, 600.0f });
        TEST_ASSERT(ApproxEqualVec2(virtPos, Vector2{ 320.0f, 180.0f }), "World target must project to virtual canvas center");

        // And from virtual center to window screen center (640, 360)
        Vector2 screenPos = camera.WorldToScreen(Vector2{ 800.0f, 600.0f }, vp);
        TEST_ASSERT(ApproxEqualVec2(screenPos, Vector2{ 640.0f, 360.0f }), "World target must project to screen center");

        // Inverse ScreenToWorld
        Vector2 unprojectedWorld = camera.ScreenToWorld(screenPos, vp);
        TEST_ASSERT(ApproxEqualVec2(unprojectedWorld, Vector2{ 800.0f, 600.0f }), "ScreenToWorld must recover original world position");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 6: Camera zoom constraints
    {
        std::cout << "[TEST] 6. Camera zoom constraints..." << std::endl;
        Camera2DSystem camera(640, 360);
        camera.SetZoomLimits(0.5f, 4.0f);

        camera.SetZoom(0.1f);
        TEST_ASSERT(ApproxEqual(camera.GetZoom(), 0.5f), "Zoom below minimum must clamp to minZoom (0.5)");

        camera.SetZoom(10.0f);
        TEST_ASSERT(ApproxEqual(camera.GetZoom(), 4.0f), "Zoom above maximum must clamp to maxZoom (4.0)");

        camera.SetZoom(2.0f);
        TEST_ASSERT(ApproxEqual(camera.GetZoom(), 2.0f), "Valid zoom within range must be set accurately");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 7: Camera boundary calculations
    {
        std::cout << "[TEST] 7. Camera boundary clamping..." << std::endl;
        Camera2DSystem camera(640, 360);
        camera.SetZoom(1.0f);
        camera.SetWorldBounds(Rectangle{ 0.0f, 0.0f, 1600.0f, 1200.0f });

        // Visible extent at zoom 1.0 is 640x360.
        // Allowed target x: [320, 1600 - 320] = [320, 1280]
        // Allowed target y: [180, 1200 - 180] = [180, 1020]

        // Try moving outside top-left boundary
        camera.SetTarget(Vector2{ -500.0f, -500.0f });
        Vector2 clampedMin = camera.GetTarget();
        TEST_ASSERT(ApproxEqual(clampedMin.x, 320.0f), "Camera target.x must clamp to min visible boundary (320)");
        TEST_ASSERT(ApproxEqual(clampedMin.y, 180.0f), "Camera target.y must clamp to min visible boundary (180)");

        // Try moving outside bottom-right boundary
        camera.SetTarget(Vector2{ 5000.0f, 5000.0f });
        Vector2 clampedMax = camera.GetTarget();
        TEST_ASSERT(ApproxEqual(clampedMax.x, 1280.0f), "Camera target.x must clamp to max visible boundary (1280)");
        TEST_ASSERT(ApproxEqual(clampedMax.y, 1020.0f), "Camera target.y must clamp to max visible boundary (1020)");

        // Test boundary behavior when world is smaller than viewport (Centering behavior)
        camera.SetWorldBounds(Rectangle{ 100.0f, 50.0f, 400.0f, 200.0f });
        camera.SetTarget(Vector2{ 0.0f, 0.0f });
        Vector2 smallWorldTarget = camera.GetTarget();
        TEST_ASSERT(ApproxEqual(smallWorldTarget.x, 300.0f), "Small world must center horizontally (100 + 400*0.5 = 300)");
        TEST_ASSERT(ApproxEqual(smallWorldTarget.y, 150.0f), "Small world must center vertically (50 + 200*0.5 = 150)");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 8: Delta-time calculation and clamping
    {
        std::cout << "[TEST] 8. Frame delta-time calculations..." << std::endl;
        float normalDt = 1.0f / 60.0f;
        TEST_ASSERT(normalDt > 0.01f && normalDt < 0.02f, "60 FPS delta time must be approx 0.0167s");

        // Test spiral of death clamp logic
        auto ClampDeltaTime = [](float dt) {
            constexpr float maxDt = 0.25f;
            return (dt > maxDt) ? maxDt : dt;
        };

        float lagSpike = 2.0f; // 2-second freeze
        float clampedLag = ClampDeltaTime(lagSpike);
        TEST_ASSERT(ApproxEqual(clampedLag, 0.25f), "Extreme frame time must be clamped to 0.25s maximum");
        TEST_ASSERT(ApproxEqual(ClampDeltaTime(normalDt), normalDt), "Normal frame time must remain unchanged");
        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "All Rendering, Viewport & Camera tests passed successfully!" << std::endl;
    Logger::Shutdown();
    return 0;
}

