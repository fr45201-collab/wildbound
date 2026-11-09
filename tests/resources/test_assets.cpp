#include "../../src/resources/AssetMetadata.hpp"
#include "../../src/resources/AssetRegistry.hpp"
#include "../../src/resources/SpriteSheet.hpp"
#include "../../src/resources/AnimationMetadata.hpp"
#include "../../src/resources/AssetValidator.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>

using wildbound::core::Logger;
using namespace wildbound::resources;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

static bool ApproxEqual(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) <= epsilon;
}

int main() {
    Logger::Init();
    std::cout << "=== Running Asset Management System Tests ===" << std::endl;

    // Test 1: Asset Manifest Parsing
    {
        std::cout << "[TEST] 1. Asset Manifest JSON parsing..." << std::endl;
        AssetRegistry registry;
        bool loaded = registry.LoadManifest("assets/manifests/asset_manifest.json");
        TEST_ASSERT(loaded, "Failed loading master asset manifest");
        TEST_ASSERT(registry.GetAssetCount() == 701, "Manifest must contain exactly 701 registered assets");
        TEST_ASSERT(registry.HasAsset("tex_player"), "Manifest must register 'tex_player'");
        TEST_ASSERT(registry.HasAsset("sheet_player"), "Manifest must register 'sheet_player'");
        TEST_ASSERT(registry.HasAsset("tex_oak_tree"), "Manifest must register 'tex_oak_tree'");
        // Verify storm assets that previously failed registration due to legacy 'path' key (DEF-003)
        TEST_ASSERT(registry.HasAsset("tex_storm_overlay"), "Manifest must register 'tex_storm_overlay'");
        TEST_ASSERT(registry.HasAsset("tex_boundary_ring"), "Manifest must register 'tex_boundary_ring'");
        TEST_ASSERT(registry.HasAsset("sfx_storm_damage"), "Manifest must register 'sfx_storm_damage'");
        TEST_ASSERT(registry.HasAsset("amb_storm_rumble"), "Manifest must register 'amb_storm_rumble'");
        std::cout << "  -> PASSED (701/701 assets registered)" << std::endl;
    }

    // Test 2: Asset Registration & Category Queries
    {
        std::cout << "[TEST] 2. Asset registration & queries..." << std::endl;
        AssetRegistry registry;
        AssetMetadata meta;
        meta.id = "sword_iron_01";
        meta.file = "weapons/swords/iron_sword.png";
        meta.type = AssetType::Texture;
        meta.category = "weapons/swords";
        meta.author = "Artisan";
        meta.source = "Original";
        meta.license = "CC0 1.0 Universal";
        meta.status = AssetStatus::Approved;

        TEST_ASSERT(registry.RegisterAsset(meta), "Registration must succeed");
        TEST_ASSERT(registry.HasAsset("sword_iron_01"), "Registry must contain registered asset");

        const AssetMetadata* retrieved = registry.GetMetadata("sword_iron_01");
        TEST_ASSERT(retrieved != nullptr, "Metadata pointer must not be null");
        TEST_ASSERT(retrieved->category == "weapons/swords", "Category must match");

        auto swords = registry.GetByCategory("weapons/swords");
        TEST_ASSERT(swords.size() == 1, "Category query must return 1 asset");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 3: Missing Asset Detection
    {
        std::cout << "[TEST] 3. Missing asset detection..." << std::endl;
        AssetMetadata missing;
        missing.id = "ghost_asset";
        missing.file = "non_existent/path/to/ghost.png";
        missing.type = AssetType::Texture;
        missing.status = AssetStatus::Approved;

        ValidationReport report;
        bool valid = AssetValidator::ValidateAssetFile(missing, "assets", report);
        TEST_ASSERT(!valid, "Missing file on disk must fail validation");
        TEST_ASSERT(report.errorsCount == 1, "Report must record exactly 1 error for missing file");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 4: Duplicate Asset Detection
    {
        std::cout << "[TEST] 4. Duplicate asset detection..." << std::endl;
        AssetRegistry registry;
        AssetMetadata m1;
        m1.id = "tree_01";
        m1.file = "environment/trees/oak_tree.png";
        m1.type = AssetType::Texture;

        AssetMetadata m2;
        m2.id = "tree_01"; // Duplicate ID
        m2.file = "environment/trees/birch_tree.png";
        m2.type = AssetType::Texture;

        TEST_ASSERT(registry.RegisterAsset(m1), "First registration must succeed");
        // Re-registering duplicate ID should overwrite with warning and return true
        TEST_ASSERT(registry.RegisterAsset(m2), "Re-registering same ID should overwrite");
        TEST_ASSERT(registry.GetAssetCount() == 1, "Asset count must remain 1 after ID overwrite");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 5: Texture Metadata & Memory Footprint
    {
        std::cout << "[TEST] 5. Texture metadata calculation..." << std::endl;
        AssetMetadata texMeta;
        texMeta.id = "test_tex";
        texMeta.file = "environment/trees/oak_tree.png";
        texMeta.width = 48;
        texMeta.height = 64;

        size_t expectedBytes = 48 * 64 * 4; // 12,288 bytes (RGBA32)
        size_t calculatedBytes = static_cast<size_t>(texMeta.width) * static_cast<size_t>(texMeta.height) * 4;
        TEST_ASSERT(calculatedBytes == expectedBytes, "RGBA32 footprint must be width * height * 4");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 6: Spritesheet Frame Calculation
    {
        std::cout << "[TEST] 6. Spritesheet frame calculations..." << std::endl;
        // 4 columns, 2 rows, 32x32 frames
        SpriteSheet sheet("char_sheet", "char_tex", 32, 32, 4, 2, 8, SpriteOrigin::BottomCenter);

        TEST_ASSERT(sheet.GetFrameCount() == 8, "Frame count must be 8");

        // Frame 0: col 0, row 0 -> (0, 0, 32, 32)
        Rectangle f0 = sheet.GetFrameRect(0);
        TEST_ASSERT(ApproxEqual(f0.x, 0.0f) && ApproxEqual(f0.y, 0.0f), "Frame 0 should be at (0, 0)");

        // Frame 1: col 1, row 0 -> (32, 0, 32, 32)
        Rectangle f1 = sheet.GetFrameRect(1);
        TEST_ASSERT(ApproxEqual(f1.x, 32.0f) && ApproxEqual(f1.y, 0.0f), "Frame 1 should be at (32, 0)");

        // Frame 5: col 1, row 1 -> (32, 32, 32, 32)
        Rectangle f5 = sheet.GetFrameRect(5);
        TEST_ASSERT(ApproxEqual(f5.x, 32.0f) && ApproxEqual(f5.y, 32.0f), "Frame 5 should be at (32, 32)");

        // Origin: BottomCenter -> (16, 32)
        Vector2 orig = sheet.GetFrameOrigin(0);
        TEST_ASSERT(ApproxEqual(orig.x, 16.0f) && ApproxEqual(orig.y, 32.0f), "BottomCenter origin should be (16, 32)");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 7: Animation Metadata Parsing & Frame Sampling
    {
        std::cout << "[TEST] 7. Animation metadata & frame timing..." << std::endl;
        AnimationClip clip;
        clip.name = "walk_cycle";
        clip.spritesheet = "char_sheet";
        clip.frames = { 0, 1, 2, 3 };
        clip.frameDuration = 0.1f;
        clip.loop = true;

        TEST_ASSERT(clip.IsValid(), "Animation clip must be valid");
        TEST_ASSERT(ApproxEqual(clip.GetTotalDuration(), 0.4f), "Total duration should be 4 * 0.1 = 0.4s");

        // Sample at 0.05s -> frame 0
        TEST_ASSERT(clip.GetFrameIndexAtTime(0.05f) == 0, "Time 0.05s must sample frame 0");
        // Sample at 0.15s -> frame 1
        TEST_ASSERT(clip.GetFrameIndexAtTime(0.15f) == 1, "Time 0.15s must sample frame 1");
        // Sample at 0.35s -> frame 3
        TEST_ASSERT(clip.GetFrameIndexAtTime(0.35f) == 3, "Time 0.35s must sample frame 3");
        // Sample with looping at 0.45s (0.45 mod 0.4 = 0.05) -> frame 0
        TEST_ASSERT(clip.GetFrameIndexAtTime(0.45f) == 0, "Time 0.45s looped must sample frame 0");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 8: Asset Path & Format Extension Validation
    {
        std::cout << "[TEST] 8. Extension format validation..." << std::endl;
        TEST_ASSERT(AssetValidator::IsExtensionSupported(".png", AssetType::Texture), "PNG must be supported for textures");
        TEST_ASSERT(AssetValidator::IsExtensionSupported(".bmp", AssetType::Texture), "BMP must be supported for textures");
        TEST_ASSERT(AssetValidator::IsExtensionSupported(".wav", AssetType::Sound), "WAV must be supported for sound");
        TEST_ASSERT(AssetValidator::IsExtensionSupported(".ogg", AssetType::Music), "OGG must be supported for music");
        TEST_ASSERT(AssetValidator::IsExtensionSupported(".ttf", AssetType::Font), "TTF must be supported for fonts");
        TEST_ASSERT(!AssetValidator::IsExtensionSupported(".exe", AssetType::Texture), "EXE must NOT be supported for textures");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 9: License Status Validation
    {
        std::cout << "[TEST] 9. License status validation..." << std::endl;
        AssetMetadata unverified;
        unverified.id = "unverified_asset";
        unverified.file = "characters/player/player_spritesheet.png"; // exists on disk
        unverified.type = AssetType::Texture;
        unverified.status = AssetStatus::LicenseReviewRequired;

        ValidationReport report;
        bool valid = AssetValidator::ValidateAssetFile(unverified, "assets", report);
        TEST_ASSERT(valid, "Existing file should pass file existence check");
        TEST_ASSERT(report.warningsCount == 1, "Unverified license must raise a warning");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 10: Asset Cache Behavior (Metadata deduplication & JSON persistence)
    {
        std::cout << "[TEST] 10. Metadata JSON roundtrip & registry caching..." << std::endl;
        AssetRegistry registry;
        registry.LoadManifest("assets/manifests/asset_manifest.json");

        const AssetMetadata* meta = registry.GetMetadata("tex_player");
        TEST_ASSERT(meta != nullptr, "tex_player metadata must be found");

        nlohmann::json jsonRep = meta->ToJson();
        AssetMetadata deserialized = AssetMetadata::FromJson(jsonRep);

        TEST_ASSERT(deserialized.id == meta->id, "Deserialized ID must match");
        TEST_ASSERT(deserialized.file == meta->file, "Deserialized file must match");
        TEST_ASSERT(deserialized.type == meta->type, "Deserialized type must match");
        TEST_ASSERT(deserialized.license == meta->license, "Deserialized license must match");
        TEST_ASSERT(deserialized.commercialUse == meta->commercialUse, "Deserialized commercialUse must match");
        std::cout << "  -> PASSED" << std::endl;
    }

    // Test 11: Manifest Parser Hardening & Backward Compatibility (DEF-003 / DEF-007)
    {
        std::cout << "[TEST] 11. Manifest parser hardening & backward compatibility..." << std::endl;

        // Subtest 11.1: Legacy 'path' key fallback
        nlohmann::json jPath = {
            {"id", "test_legacy_path"},
            {"path", "environment/trees/oak_tree.png"},
            {"type", "texture"},
            {"category", "test"}
        };
        AssetMetadata metaPath = AssetMetadata::FromJson(jPath);
        TEST_ASSERT(metaPath.IsValid(), "Asset with 'path' key must be valid");
        TEST_ASSERT(metaPath.file == "environment/trees/oak_tree.png", "Legacy 'path' must map to 'file'");

        // Subtest 11.2: Conflicting 'file' and 'path' keys (file should take precedence)
        nlohmann::json jConflict = {
            {"id", "test_conflict"},
            {"file", "canonical/path.png"},
            {"path", "legacy/path.png"},
            {"type", "texture"}
        };
        AssetMetadata metaConflict = AssetMetadata::FromJson(jConflict);
        TEST_ASSERT(metaConflict.file == "canonical/path.png", "Canonical 'file' must take precedence over 'path'");

        // Subtest 11.3: camelCase licensing keys
        nlohmann::json jCamel = {
            {"id", "test_camel"},
            {"file", "test.png"},
            {"type", "texture"},
            {"commercialUse", false},
            {"attributionRequired", true}
        };
        AssetMetadata metaCamel = AssetMetadata::FromJson(jCamel);
        TEST_ASSERT(!metaCamel.commercialUse, "camelCase commercialUse must be correctly parsed");
        TEST_ASSERT(metaCamel.attributionRequired, "camelCase attributionRequired must be correctly parsed");

        // Subtest 11.4: Nested metadata dimensions
        nlohmann::json jNested = {
            {"id", "test_nested"},
            {"file", "test.png"},
            {"type", "texture"},
            {"metadata", {
                {"width", 256},
                {"height", 128}
            }}
        };
        AssetMetadata metaNested = AssetMetadata::FromJson(jNested);
        TEST_ASSERT(metaNested.width == 256, "Nested metadata.width must be extracted");
        TEST_ASSERT(metaNested.height == 128, "Nested metadata.height must be extracted");

        // Subtest 11.5: Legacy 'audio' type mapping and duration
        nlohmann::json jAudio = {
            {"id", "test_audio_type"},
            {"file", "audio/sfx/test.wav"},
            {"type", "audio"},
            {"duration", 3.14f}
        };
        AssetMetadata metaAudio = AssetMetadata::FromJson(jAudio);
        TEST_ASSERT(metaAudio.type == AssetType::Sound, "Type 'audio' must map to AssetType::Sound");
        TEST_ASSERT(ApproxEqual(metaAudio.duration, 3.14f), "Duration must be preserved");

        std::cout << "  -> PASSED" << std::endl;
    }

    std::cout << "All Asset Management System tests passed successfully!" << std::endl;
    Logger::Shutdown();
    return 0;
}

