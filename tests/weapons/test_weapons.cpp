#include "../../src/weapons/WeaponData.hpp"
#include "../../src/weapons/WeaponManager.hpp"
#include "../../src/resources/AssetManager.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
using namespace wildbound::weapons;
using namespace wildbound::resources;
using namespace wildbound::graphics::animation;
namespace fs = std::filesystem;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

int main() {
    Logger::Init();
    std::cout << "=== Running Wildbound Weapon Asset Collection Tests (Phase 15) ===" << std::endl;

    // Headless raylib context
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "WildboundWeaponTest");

    AssetManager assetMgr;
    assetMgr.Initialize("assets");

    WeaponManager mgr;
    bool ok = mgr.Initialize("data/weapons/weapon_catalog.json");
    TEST_ASSERT(ok, "WeaponManager::Initialize should succeed");

    // ========================================================================
    // Test 1: Weapon Catalog Loading & Metadata Integrity
    // ========================================================================
    {
        std::cout << "[TEST] 1. Weapon catalog loading & metadata verification..." << std::endl;
        TEST_ASSERT(mgr.GetWeaponCount() == 12, "Must contain exactly 12 weapon items (6 swords, 5 bows, 1 arrow)");

        const std::vector<std::string> expectedSwords = {
            "sword_wooden_training_001",
            "sword_rusty_short_001",
            "sword_iron_001",
            "sword_steel_longsword_001",
            "sword_curved_001",
            "sword_ancient_rune_001"
        };

        for (const auto& id : expectedSwords) {
            TEST_ASSERT(mgr.HasWeapon(id), ("Catalog must contain sword: " + id).c_str());
            const WeaponConfig* w = mgr.GetWeapon(id);
            TEST_ASSERT(w != nullptr, "WeaponConfig must not be null");
            TEST_ASSERT(w->category == WeaponCategory::Sword, "Category must be Sword");
            TEST_ASSERT(!w->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(!w->subcategory.empty(), "Subcategory must not be empty");
            TEST_ASSERT(!w->description.empty(), "Description must not be empty");
            TEST_ASSERT(w->directions.size() == 4, "Must support 4 directions");
            TEST_ASSERT(w->animationFrames.size() == 4, "Must support 4 animation frames");
        }

        const std::vector<std::string> expectedBows = {
            "bow_wooden_hunting_001",
            "bow_longbow_001",
            "bow_reinforced_001",
            "bow_recurve_001",
            "bow_ancient_rune_001"
        };

        for (const auto& id : expectedBows) {
            TEST_ASSERT(mgr.HasWeapon(id), ("Catalog must contain bow: " + id).c_str());
            const WeaponConfig* w = mgr.GetWeapon(id);
            TEST_ASSERT(w != nullptr, "WeaponConfig must not be null");
            TEST_ASSERT(w->category == WeaponCategory::Bow, "Category must be Bow");
            TEST_ASSERT(!w->displayName.empty(), "Display name must not be empty");
            TEST_ASSERT(!w->subcategory.empty(), "Subcategory must not be empty");
            TEST_ASSERT(!w->description.empty(), "Description must not be empty");
            TEST_ASSERT(w->directions.size() == 4, "Must support 4 directions");
            TEST_ASSERT(w->animationFrames.size() == 4, "Must support 4 animation frames");
        }

        // Projectile / Arrow check
        TEST_ASSERT(mgr.HasWeapon("arrow_iron_001"), "Catalog must contain arrow_iron_001");
        const WeaponConfig* arrow = mgr.GetWeapon("arrow_iron_001");
        TEST_ASSERT(arrow->category == WeaponCategory::Ammunition, "Arrow category must be Ammunition");

        std::cout << "  -> All 11 weapons + 1 projectile verified with complete metadata. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 2: Category Filtering & Queries
    // ========================================================================
    {
        std::cout << "[TEST] 2. Category filtering & queries..." << std::endl;
        auto swords = mgr.GetWeaponsByCategory(WeaponCategory::Sword);
        TEST_ASSERT(swords.size() == 6, "Must retrieve exactly 6 swords");

        auto bows = mgr.GetWeaponsByCategory(WeaponCategory::Bow);
        TEST_ASSERT(bows.size() == 5, "Must retrieve exactly 5 bows");

        auto ammo = mgr.GetWeaponsByCategory(WeaponCategory::Ammunition);
        TEST_ASSERT(ammo.size() == 1, "Must retrieve exactly 1 ammunition item");

        std::cout << "  -> Category query filtering validated. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 3: AMS Asset Resolution & Disk File Existence
    // ========================================================================
    {
        std::cout << "[TEST] 3. AMS asset registry resolution & disk verification..." << std::endl;
        const auto& reg = assetMgr.GetRegistry();
        const auto& allWeapons = mgr.GetAllWeapons();

        for (const auto& w : allWeapons) {
            // Icon
            const auto* iconMeta = reg.GetMetadata(w.visuals.iconTextureId);
            TEST_ASSERT(iconMeta != nullptr, ("Icon asset missing in AMS: " + w.visuals.iconTextureId).c_str());
            TEST_ASSERT(iconMeta->width == 64 && iconMeta->height == 64, "Icon must be 64x64 px");
            TEST_ASSERT(fs::exists(fs::path("assets") / iconMeta->file), ("Icon file missing on disk: " + iconMeta->file).c_str());

            // Ground
            const auto* groundMeta = reg.GetMetadata(w.visuals.groundTextureId);
            TEST_ASSERT(groundMeta != nullptr, ("Ground asset missing in AMS: " + w.visuals.groundTextureId).c_str());
            TEST_ASSERT(groundMeta->width == 32 && groundMeta->height == 32, "Ground must be 32x32 px");
            TEST_ASSERT(fs::exists(fs::path("assets") / groundMeta->file), ("Ground file missing on disk: " + groundMeta->file).c_str());

            // Held Texture & Sheet
            const auto* heldMeta = reg.GetMetadata(w.visuals.heldTextureId);
            TEST_ASSERT(heldMeta != nullptr, ("Held texture missing in AMS: " + w.visuals.heldTextureId).c_str());
            TEST_ASSERT(heldMeta->width == 128 && heldMeta->height == 32, "Held strip must be 128x32 px (4 frames)");
            TEST_ASSERT(fs::exists(fs::path("assets") / heldMeta->file), ("Held file missing on disk: " + heldMeta->file).c_str());

            const auto* heldSheetMeta = reg.GetMetadata(w.visuals.heldSheetId);
            TEST_ASSERT(heldSheetMeta != nullptr, ("Held spritesheet metadata missing in AMS: " + w.visuals.heldSheetId).c_str());
            TEST_ASSERT(fs::exists(fs::path("assets") / heldSheetMeta->file), ("Held sheet JSON missing on disk: " + heldSheetMeta->file).c_str());

            // Anim Texture & Sheet
            const auto* animMeta = reg.GetMetadata(w.visuals.animTextureId);
            TEST_ASSERT(animMeta != nullptr, ("Anim texture missing in AMS: " + w.visuals.animTextureId).c_str());
            TEST_ASSERT(animMeta->width == 128 && animMeta->height == 32, "Anim strip must be 128x32 px (4 frames)");
            TEST_ASSERT(fs::exists(fs::path("assets") / animMeta->file), ("Anim file missing on disk: " + animMeta->file).c_str());

            const auto* animSheetMeta = reg.GetMetadata(w.visuals.animSheetId);
            TEST_ASSERT(animSheetMeta != nullptr, ("Anim spritesheet metadata missing in AMS: " + w.visuals.animSheetId).c_str());
            TEST_ASSERT(fs::exists(fs::path("assets") / animSheetMeta->file), ("Anim sheet JSON missing on disk: " + animSheetMeta->file).c_str());
        }

        std::cout << "  -> All 72 weapon assets verified in AMS registry and present on disk. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 4: Attachment Anchors & Directional Z-Order Matrix
    // ========================================================================
    {
        std::cout << "[TEST] 4. Attachment anchors & directional Z-ordering..." << std::endl;
        const auto& allWeapons = mgr.GetAllWeapons();

        for (const auto& w : allWeapons) {
            // Anchor coordinates must be within 32x32 frame
            TEST_ASSERT(w.attachment.gripAnchor.x >= 0.0f && w.attachment.gripAnchor.x <= 32.0f,
                        ("Grip X out of bounds for " + w.id).c_str());
            TEST_ASSERT(w.attachment.gripAnchor.y >= 0.0f && w.attachment.gripAnchor.y <= 32.0f,
                        ("Grip Y out of bounds for " + w.id).c_str());

            // Facing Down (South): weapon rendered in foreground (Z >= 80)
            int zDown = w.attachment.GetRenderOrder(Direction::Down);
            TEST_ASSERT(zDown >= 80, ("Facing Down Z-index must be >= 80 for " + w.id).c_str());

            // Facing Up (North): weapon rendered behind torso/shoulders (Z <= 25)
            int zUp = w.attachment.GetRenderOrder(Direction::Up);
            TEST_ASSERT(zUp <= 25, ("Facing Up Z-index must be <= 25 for " + w.id).c_str());

            // Facing Left / Right: weapon in foreground
            int zLeft = w.attachment.GetRenderOrder(Direction::Left);
            int zRight = w.attachment.GetRenderOrder(Direction::Right);
            TEST_ASSERT(zLeft >= 80 && zRight >= 80, ("Side facing Z-index must be >= 80 for " + w.id).c_str());
        }

        std::cout << "  -> Attachment anchors and directional depth-sorting validated. [PASSED]" << std::endl;
    }

    // ========================================================================
    // Test 5: Full Validation via WeaponManager::ValidateAssets
    // ========================================================================
    {
        std::cout << "[TEST] 5. WeaponManager::ValidateAssets integration..." << std::endl;
        std::vector<std::string> errors;
        bool valid = mgr.ValidateAssets(assetMgr, errors);
        TEST_ASSERT(valid, "WeaponManager::ValidateAssets should report 100% success");
        TEST_ASSERT(errors.empty(), "Errors list must be completely empty");
        std::cout << "  -> Comprehensive automated asset validation passed with 0 errors. [PASSED]" << std::endl;
    }

    CloseWindow();
    std::cout << "\n>>> ALL PHASE 15 WEAPON ASSET COLLECTION TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

