#include "../../src/storm/StormTypes.hpp"
#include "../../src/storm/SafeZone.hpp"
#include "../../src/storm/StormDamageSystem.hpp"
#include "../../src/storm/StormBoundaryRenderer.hpp"
#include "../../src/storm/StormManager.hpp"
#include "../../src/combat/HealthComponent.hpp"
#include "../../src/ui/hud/HUDManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <cmath>
#include <vector>

using wildbound::core::Logger;
using namespace wildbound::storm;
using namespace wildbound::combat;
using namespace wildbound::ui;

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
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 31: CORRUPTION STORM TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Headless raylib context for texture & rendering in tests
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundStormTest");

    Rectangle playableBounds{ 768.0f, 768.0f, 8704.0f, 8704.0f };

    // ========================================================================
    // Test 1: Safe Zone Geometry & Containment Queries
    // ========================================================================
    {
        std::cout << "[TEST] 1. Safe Zone Geometry & Containment Queries..." << std::endl;
        SafeZone zone{ Vector2{ 5000.0f, 5000.0f }, 1000.0f };

        // Inside
        Vector2 insidePoint{ 5200.0f, 5200.0f };
        TEST_ASSERT(zone.Contains(insidePoint), "Point (5200, 5200) must be inside safe zone");
        TEST_ASSERT(zone.SignedDistanceToBoundary(insidePoint) < 0.0f, "Signed distance inside must be negative");

        // Outside
        Vector2 outsidePoint{ 6500.0f, 5000.0f };
        TEST_ASSERT(!zone.Contains(outsidePoint), "Point (6500, 5000) must be outside safe zone");
        TEST_ASSERT(zone.SignedDistanceToBoundary(outsidePoint) == 500.0f, "Signed distance outside must be +500");

        // Boundary
        Vector2 boundaryPoint{ 6000.0f, 5000.0f };
        TEST_ASSERT(zone.Contains(boundaryPoint), "Point on boundary must be contained");
        TEST_ASSERT(ApproxEqual(zone.SignedDistanceToBoundary(boundaryPoint), 0.0f), "Signed distance on boundary must be 0");

        // Closest safe point projection
        Vector2 projected = zone.GetClosestSafePoint(outsidePoint);
        TEST_ASSERT(ApproxEqual(projected.x, 6000.0f) && ApproxEqual(projected.y, 5000.0f), "Projected point must be on circumference at (6000, 5000)");

        std::cout << "  -> Inside, outside, boundary, and vector projection verified." << std::endl;
    }

    // ========================================================================
    // Test 2: SafeZoneHelper Bounding & Containment Rules
    // ========================================================================
    {
        std::cout << "[TEST] 2. SafeZoneHelper Bounding & Containment Rules..." << std::endl;
        Vector2 center{ 5120.0f, 5120.0f };
        float radius = 4000.0f;

        TEST_ASSERT(SafeZoneHelper::IsCircleContainedInRect(center, radius, playableBounds),
                    "Default initial zone must be fully inside playable bounds");

        // Circle extending past right edge
        Vector2 badCenter{ 8500.0f, 5120.0f };
        TEST_ASSERT(!SafeZoneHelper::IsCircleContainedInRect(badCenter, radius, playableBounds),
                    "Circle extending outside bounds must return false");

        Vector2 clamped = SafeZoneHelper::ClampCircleCenterToRect(badCenter, radius, playableBounds);
        TEST_ASSERT(SafeZoneHelper::IsCircleContainedInRect(clamped, radius, playableBounds),
                    "Clamped center must be fully inside playable bounds");

        // Next center generation with drift
        SafeZone curZone{ center, radius };
        for (uint32_t seed = 1; seed <= 20; ++seed) {
            Vector2 nextC = SafeZoneHelper::CalculateNextCenter(curZone, 2500.0f, playableBounds, 0.4f, seed);
            TEST_ASSERT(SafeZoneHelper::IsCircleContainedInRect(nextC, 2500.0f, playableBounds),
                        "Generated next center must keep circle inside playable bounds");
            TEST_ASSERT(curZone.DistanceTo(nextC) <= (radius - 2500.0f) + 1.0f,
                        "Generated next circle must be contained within current circle");
        }
        std::cout << "  -> 20/20 deterministic center calculations strictly bounded." << std::endl;
    }

    // ========================================================================
    // Test 3: Storm Config Ingestion & Validation
    // ========================================================================
    {
        std::cout << "[TEST] 3. Storm Configuration Ingestion & Validation..." << std::endl;
        StormConfig cfg = StormConfig::LoadFromJson("data/storm/storm_config.json", playableBounds);

        TEST_ASSERT(!cfg.phases.empty(), "Phases array must not be empty");
        TEST_ASSERT(cfg.phases.size() == 6, "Expected 6 storm phases configured");
        TEST_ASSERT(cfg.initialRadius > 1000.0f, "Initial radius must be substantial");
        TEST_ASSERT(cfg.phases[0].warningDuration > 0.0f, "Phase 1 warning duration must be positive");
        TEST_ASSERT(cfg.phases[0].shrinkDuration > 0.0f, "Phase 1 shrink duration must be positive");
        TEST_ASSERT(cfg.phases[0].damagePerSecond > 0.0f, "Phase 1 DPS must be positive");
        TEST_ASSERT(cfg.phases.back().damagePerSecond >= 10.0f, "Final phase DPS must be high");

        std::cout << "  -> Config verified: " << cfg.phases.size() << " phases loaded cleanly." << std::endl;
    }

    // ========================================================================
    // Test 4: Storm State Machine & Phase Transitions
    // ========================================================================
    {
        std::cout << "[TEST] 4. Storm State Machine & Phase Progression..." << std::endl;
        StormManager stormMgr;
        stormMgr.Initialize("data/storm/storm_config.json", playableBounds);

        TEST_ASSERT(stormMgr.GetState() == StormState::Idle, "Storm must start in Idle state");

        stormMgr.StartStorm();
        TEST_ASSERT(stormMgr.GetState() == StormState::Warning, "Storm must enter Warning state on start");
        TEST_ASSERT(stormMgr.GetCurrentPhaseIndex() == 0, "Current phase index must be 0 (Phase 1)");
        TEST_ASSERT(stormMgr.GetPhaseTimer() > 0.0f, "Warning timer must be initialized");

        // Advance phase manually
        stormMgr.AdvancePhase();
        TEST_ASSERT(stormMgr.GetState() == StormState::Shrinking, "AdvancePhase must transition Warning -> Shrinking");

        // Pause/Resume
        stormMgr.PauseStorm();
        TEST_ASSERT(stormMgr.IsPaused(), "Storm must report paused");
        float timerBefore = stormMgr.GetPhaseTimer();
        stormMgr.Update(1.0f);
        TEST_ASSERT(stormMgr.GetPhaseTimer() == timerBefore, "Timer must not progress while paused");

        stormMgr.ResumeStorm();
        TEST_ASSERT(!stormMgr.IsPaused(), "Storm must report active after resume");
        stormMgr.Update(1.0f);
        TEST_ASSERT(stormMgr.GetPhaseTimer() < timerBefore, "Timer must progress after resume");

        std::cout << "  -> State machine, pause/resume, and phase advance verified." << std::endl;
    }

    // ========================================================================
    // Test 5: Smooth Shrinking & Moving Center Interpolation
    // ========================================================================
    {
        std::cout << "[TEST] 5. Smooth Shrinking & Moving Center Interpolation..." << std::endl;
        SafeZone zoneA{ Vector2{ 5000.0f, 5000.0f }, 4000.0f };
        SafeZone zoneB{ Vector2{ 5200.0f, 4800.0f }, 2000.0f };

        // Test t=0.0
        SafeZone lerp0 = SafeZone::Lerp(zoneA, zoneB, 0.0f);
        TEST_ASSERT(lerp0.center.x == 5000.0f && lerp0.radius == 4000.0f, "Lerp t=0 must equal zone A");

        // Test t=0.5
        SafeZone lerp5 = SafeZone::Lerp(zoneA, zoneB, 0.5f);
        TEST_ASSERT(lerp5.center.x == 5100.0f && lerp5.center.y == 4900.0f, "Lerp t=0.5 center mismatch");
        TEST_ASSERT(lerp5.radius == 3000.0f, "Lerp t=0.5 radius mismatch");

        // Test t=1.0
        SafeZone lerp1 = SafeZone::Lerp(zoneA, zoneB, 1.0f);
        TEST_ASSERT(lerp1.center.x == 5200.0f && lerp1.radius == 2000.0f, "Lerp t=1 must equal zone B");

        std::cout << "  -> Continuous mathematical interpolation confirmed." << std::endl;
    }

    // ========================================================================
    // Test 6: Frame-Rate Independent Storm Damage & Fractional Accumulator
    // ========================================================================
    {
        std::cout << "[TEST] 6. Frame-Rate Independent Storm Damage..." << std::endl;
        SafeZone zone{ Vector2{ 5000.0f, 5000.0f }, 1000.0f };
        Vector2 outsidePos{ 7000.0f, 5000.0f };
        float dps = 10.0f; // 10 HP per second

        // Simulate 1 second at 30 FPS (30 steps of dt = 1/30)
        HealthComponent health30(100.0f);
        StormDamageSystem dmgSys30;
        for (int i = 0; i < 30; ++i) {
            dmgSys30.Update(1.0f / 30.0f, zone, dps, outsidePos, health30);
        }

        // Simulate 1 second at 120 FPS (120 steps of dt = 1/120)
        HealthComponent health120(100.0f);
        StormDamageSystem dmgSys120;
        for (int i = 0; i < 120; ++i) {
            dmgSys120.Update(1.0f / 120.0f, zone, dps, outsidePos, health120);
        }

        TEST_ASSERT(ApproxEqual(dmgSys30.GetTotalDamageDealt(), 10.0f, 0.1f), "30 FPS total damage must be ~10 HP");
        TEST_ASSERT(ApproxEqual(dmgSys120.GetTotalDamageDealt(), 10.0f, 0.1f), "120 FPS total damage must be ~10 HP");
        TEST_ASSERT(ApproxEqual(health30.GetCurrentHealth(), 90.0f, 0.1f), "Health after 1s at 30 FPS must be ~90 HP");
        TEST_ASSERT(ApproxEqual(health120.GetCurrentHealth(), 90.0f, 0.1f), "Health after 1s at 120 FPS must be ~90 HP");

        std::cout << "  -> 30 FPS (" << health30.GetCurrentHealth() << " HP) matches 120 FPS ("
                  << health120.GetCurrentHealth() << " HP) exactly." << std::endl;
    }

    // ========================================================================
    // Test 7: Damage Cessation & Safety Guarantees
    // ========================================================================
    {
        std::cout << "[TEST] 7. Damage Cessation & Safety Guarantees..." << std::endl;
        SafeZone zone{ Vector2{ 5000.0f, 5000.0f }, 1000.0f };
        HealthComponent health(100.0f);
        StormDamageSystem dmgSys;

        // Player inside safe zone
        Vector2 insidePos{ 5050.0f, 5050.0f };
        bool dealt = dmgSys.Update(1.0f, zone, 20.0f, insidePos, health);
        TEST_ASSERT(!dealt, "No damage must be dealt inside safe zone");
        TEST_ASSERT(health.GetCurrentHealth() == 100.0f, "Health must remain 100% inside safe zone");
        TEST_ASSERT(dmgSys.GetDamageAccumulator() == 0.0f, "Accumulator must stay 0 inside safe zone");

        // Player dies -> damage stops
        health.SetCurrentHealth(0.0f);
        health.MarkDead();
        Vector2 outsidePos{ 7000.0f, 5000.0f };
        bool dealtDead = dmgSys.Update(1.0f, zone, 20.0f, outsidePos, health);
        TEST_ASSERT(!dealtDead, "Dead player must not receive storm damage");

        std::cout << "  -> Zero damage inside safe zone, dead state respected." << std::endl;
    }

    // ========================================================================
    // Test 8: HUD Integration & Warning States
    // ========================================================================
    {
        std::cout << "[TEST] 8. HUD Integration & Warning States..." << std::endl;
        StormManager stormMgr;
        stormMgr.Initialize("data/storm/storm_config.json", playableBounds);
        stormMgr.StartStorm();

        HUDManager hudMgr;
        stormMgr.Update(0.1f, nullptr, nullptr, &hudMgr);

        const auto& match = hudMgr.GetMatchInfo();
        TEST_ASSERT(match.stormTimer > 0.0f, "HUD matchInfo stormTimer must sync with StormManager");
        TEST_ASSERT(!match.phaseName.empty(), "HUD phaseName must be populated");

        std::cout << "  -> HUD synced: PhaseName='" << match.phaseName
                  << "', Timer=" << match.stormTimer << "s." << std::endl;
    }

    // ========================================================================
    // Test 9: Minimap Radar Scaling & Projection
    // ========================================================================
    {
        std::cout << "[TEST] 9. Minimap Radar Scaling & Projection..." << std::endl;
        StormBoundaryRenderer renderer;
        SafeZone currentZone{ Vector2{ 5120.0f, 5120.0f }, 3000.0f };
        SafeZone targetZone{ Vector2{ 5300.0f, 5000.0f }, 2000.0f };

        // Test at zoom 1.0x, 1.5x, 2.0x
        for (float zoom : { 1.0f, 1.5f, 2.0f }) {
            renderer.RenderMinimap(currentZone, targetZone, true,
                                   Vector2{ 574.0f, 64.0f }, 54.0f,
                                   Vector2{ 5120.0f, 5120.0f }, zoom, playableBounds);
        }
        std::cout << "  -> Minimap projection tested at multiple radar zoom levels." << std::endl;
    }

    // ========================================================================
    // Test 10: Complete Match Lifecycle Integration Scenario
    // ========================================================================
    {
        std::cout << "[TEST] 10. Complete Match Lifecycle Integration Scenario..." << std::endl;
        StormManager stormMgr;
        stormMgr.Initialize("data/storm/storm_config.json", playableBounds);
        stormMgr.StartStorm();

        HealthComponent playerHealth(100.0f);
        Vector2 playerPos = stormMgr.GetCurrentSafeZone().center;

        // Phase 1 Warning countdown
        float timerInitial = stormMgr.GetPhaseTimer();
        stormMgr.Update(5.0f);
        TEST_ASSERT(stormMgr.GetPhaseTimer() < timerInitial, "Warning countdown must tick down");

        // Fast forward through warning into shrink
        stormMgr.SetPhaseTimer(0.01f);
        stormMgr.Update(0.05f);
        TEST_ASSERT(stormMgr.GetState() == StormState::Shrinking, "Must enter Shrinking state");

        // Player steps outside safe zone
        playerPos.x += stormMgr.GetCurrentSafeZone().radius + 200.0f;
        stormMgr.GetDamageSystem().Update(1.0f, stormMgr.GetCurrentSafeZone(), stormMgr.GetCurrentDPS(), playerPos, playerHealth);
        TEST_ASSERT(playerHealth.GetCurrentHealth() < 100.0f, "Player must take storm damage when outside");

        // Player steps back inside safe zone
        playerPos = stormMgr.GetCurrentSafeZone().center;
        float hpBefore = playerHealth.GetCurrentHealth();
        stormMgr.GetDamageSystem().Update(1.0f, stormMgr.GetCurrentSafeZone(), stormMgr.GetCurrentDPS(), playerPos, playerHealth);
        TEST_ASSERT(playerHealth.GetCurrentHealth() == hpBefore, "Player must stop taking damage when back inside");

        // Advance to final phase
        while (stormMgr.GetCurrentPhaseIndex() < stormMgr.GetTotalPhases() - 1) {
            stormMgr.AdvancePhase();
        }
        TEST_ASSERT(stormMgr.GetCurrentPhaseIndex() == stormMgr.GetTotalPhases() - 1, "Must reach final phase");
        TEST_ASSERT(stormMgr.GetCurrentPhaseConfig()->targetRadiusRatio <= 0.05f, "Final phase must have collapsed radius");

        std::cout << "  -> Complete match lifecycle: Start -> Warning -> Shrink -> Outside Damage -> Safe Recovery -> Final Phase OK!" << std::endl;
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL 10 CORRUPTION STORM TEST SUITES PASSED (100% OK)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    CloseWindow();
    return 0;
}

