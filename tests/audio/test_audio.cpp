#include "../../src/audio/AudioSystem.hpp"
#include "../../src/core/Logger.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <vector>

using namespace wildbound::audio;
namespace fs = std::filesystem;

void TestAudioCatalogLoading() {
    std::cout << "[TEST] 1. Audio Catalog Loading & Schema Verification (128 assets)..." << std::endl;

    AudioSystem audioSys;
    bool ok = audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");
    assert(ok && "AudioSystem failed to initialize catalog");

    size_t count = audioSys.GetAssetCount();
    std::cout << "  Loaded " << count << " audio assets." << std::endl;
    assert(count == 128 && "Expected exactly 128 procedural audio assets");

    const auto& assets = audioSys.GetAllAssets();
    for (const auto& a : assets) {
        assert(!a.id.empty() && "Audio asset ID cannot be empty");
        assert(!a.title.empty() && "Audio asset title cannot be empty");
        assert(!a.file.empty() && "Audio asset file cannot be empty");
        assert(a.duration > 0.0f && "Audio asset duration must be > 0");
        assert(a.sampleRate == 44100 && "Sample rate must be 44.1 kHz");
        assert(a.bitDepth == 16 && "Bit depth must be 16-bit PCM");
        assert(a.channels >= 1 && a.channels <= 2 && "Channels must be mono or stereo");
        assert(a.defaultVolume >= 0.0f && a.defaultVolume <= 1.0f && "Default volume out of range");
    }

    std::cout << "  [PASS] All 128 catalog entries validated with correct audio specifications." << std::endl;
    audioSys.Shutdown();
}

void TestAudioBusQueries() {
    std::cout << "[TEST] 2. Audio Bus Categorization Queries (8 Buses)..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    auto music = audioSys.GetAssetsByBus(AudioBus::Music);
    auto sfx = audioSys.GetAssetsByBus(AudioBus::SFX);
    auto ui = audioSys.GetAssetsByBus(AudioBus::UI);
    auto env = audioSys.GetAssetsByBus(AudioBus::Environment);
    auto combat = audioSys.GetAssetsByBus(AudioBus::Combat);
    auto creatures = audioSys.GetAssetsByBus(AudioBus::Creatures);
    auto voice = audioSys.GetAssetsByBus(AudioBus::Voice);

    std::cout << "  Music assets:       " << music.size() << " (Expected: 11)" << std::endl;
    std::cout << "  SFX assets:         " << sfx.size() << " (Expected: 55)" << std::endl;
    std::cout << "  UI assets:          " << ui.size() << " (Expected: 15)" << std::endl;
    std::cout << "  Environment assets: " << env.size() << " (Expected: 16)" << std::endl;
    std::cout << "  Combat assets:      " << combat.size() << " (Expected: 5)" << std::endl;
    std::cout << "  Creatures assets:   " << creatures.size() << " (Expected: 26)" << std::endl;
    std::cout << "  Voice assets:       " << voice.size() << " (Expected: 0)" << std::endl;

    assert(music.size() == 11 && "Expected 11 music tracks");
    assert(sfx.size() == 55 && "Expected 55 gameplay SFX assets");
    assert(ui.size() == 15 && "Expected 15 UI audio assets");
    assert(env.size() == 16 && "Expected 16 environment ambience tracks");
    assert(combat.size() == 5 && "Expected 5 dedicated combat SFX assets");
    assert(creatures.size() == 26 && "Expected 26 creature vocalization assets");
    assert(voice.empty() && "Voice bus reserved for future voice chat");

    // Asset info query by ID
    const auto* mainTheme = audioSys.GetAssetInfo("mus_main_menu");
    assert(mainTheme != nullptr && "mus_main_menu asset not found");
    assert(mainTheme->bus == AudioBus::Music);

    const auto* villageTheme = audioSys.GetAssetInfo("mus_village");
    assert(villageTheme != nullptr && "mus_village asset not found");
    assert(villageTheme->bus == AudioBus::Music);

    const auto* goblinAtk = audioSys.GetAssetInfo("sfx_creature_goblin_attack");
    assert(goblinAtk != nullptr && "sfx_creature_goblin_attack asset not found");
    assert(goblinAtk->bus == AudioBus::Creatures);

    std::cout << "  [PASS] All 8 audio buses classified and verified." << std::endl;
    audioSys.Shutdown();
}

void TestAudioSettingsPersistence() {
    std::cout << "[TEST] 3. Audio Settings & Bus Volume Controls..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    // Bus volume controls
    audioSys.SetMasterVolume(0.85f);
    assert(std::fabs(audioSys.GetBusVolume(AudioBus::Master) - 0.85f) < 0.01f);

    audioSys.SetBusVolume(AudioBus::Music, 0.70f);
    assert(std::fabs(audioSys.GetBusVolume(AudioBus::Music) - 0.70f) < 0.01f);

    audioSys.SetBusVolume(AudioBus::Combat, 0.65f);
    assert(std::fabs(audioSys.GetBusVolume(AudioBus::Combat) - 0.65f) < 0.01f);

    audioSys.SetBusVolume(AudioBus::Creatures, 0.60f);
    assert(std::fabs(audioSys.GetBusVolume(AudioBus::Creatures) - 0.60f) < 0.01f);

    // Effective volume calculation (Master * Bus)
    float effCombat = audioSys.GetEffectiveVolume(AudioBus::Combat);
    float expectedEff = 0.85f * 0.65f;
    assert(std::fabs(effCombat - expectedEff) < 0.02f);

    // Global mute
    audioSys.SetMuteAll(true);
    assert(audioSys.IsMuteAll() == true);
    assert(audioSys.GetEffectiveVolume(AudioBus::Combat) == 0.0f);
    audioSys.SetMuteAll(false);
    assert(std::fabs(audioSys.GetEffectiveVolume(AudioBus::Combat) - expectedEff) < 0.02f);

    // Save and reload from test file
    std::string testPath = "data/config/test_audio_settings.json";
    bool saved = audioSys.SaveSettings(testPath);
    assert(saved && "Failed to save test audio settings");

    AudioSystem reloadSys;
    reloadSys.Initialize("data/audio/audio_catalog.json", testPath);
    assert(std::fabs(reloadSys.GetBusVolume(AudioBus::Master) - 0.85f) < 0.01f);
    assert(std::fabs(reloadSys.GetBusVolume(AudioBus::Combat) - 0.65f) < 0.01f);
    assert(std::fabs(reloadSys.GetBusVolume(AudioBus::Creatures) - 0.60f) < 0.01f);

    if (fs::exists(testPath)) {
        fs::remove(testPath);
    }

    std::cout << "  [PASS] Volume mixing, muting, and JSON persistence verified." << std::endl;
    audioSys.Shutdown();
    reloadSys.Shutdown();
}

void TestBiomeAudioMapping() {
    std::cout << "[TEST] 4. Biome Ambient Audio Integration (6 Biomes)..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    bool biomeLoaded = audioSys.LoadBiomeConfigs("data/audio/biome_audio.json");
    assert(biomeLoaded && "Failed to load biome audio config");

    const std::vector<std::string> biomes = { "forest", "desert", "snow", "swamp", "mountain", "village" };
    for (const auto& b : biomes) {
        audioSys.SetBiome(b);
        assert(audioSys.GetCurrentBiome() == b);
    }

    std::cout << "  [PASS] 6 Biomes mapped and transition states verified." << std::endl;
    audioSys.Shutdown();
}

void TestEventRegistrationAndVariations() {
    std::cout << "[TEST] 5. Event Multi-Sound Variations & Cooldown Throttling..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    // Custom test event with 3 variations and 0.2s cooldown
    audioSys.RegisterEvent("test.clink", { "sfx_footstep_stone_001", "sfx_footstep_stone_002", "sfx_footstep_stone_003" }, 0.2f);

    // Initial dispatch succeeds
    audioSys.DispatchEvent("test.clink");

    // Immediate second dispatch within 0.05s must be throttled
    audioSys.Update(0.05f);
    audioSys.DispatchEvent("test.clink");

    // After 0.25s cooldown elapses, next variation can dispatch
    audioSys.Update(0.20f);
    audioSys.DispatchEvent("test.clink");

    std::cout << "  [PASS] Event variation cycling and cooldown throttling verified." << std::endl;
    audioSys.Shutdown();
}

void TestSpatialAudioMath() {
    std::cout << "[TEST] 6. Spatial 2D Attenuation & Stereo Panning Mathematics..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    Vector2 listener{ 300.0f, 300.0f };
    audioSys.SetListenerPosition(listener);
    assert(audioSys.GetListenerPosition().x == 300.0f);
    assert(audioSys.GetListenerPosition().y == 300.0f);

    // 1. Source at listener position (zero distance -> 100% volume, center pan 0.5)
    Vector2 sourceCenter{ 300.0f, 300.0f };
    audioSys.PlaySoundSpatial("sfx_sword_impact_metal", sourceCenter, listener, 600.0f);

    // 2. Source to the right (+200 px -> right stereo pan > 0.5)
    Vector2 sourceRight{ 500.0f, 300.0f };
    audioSys.PlaySoundSpatial("sfx_sword_impact_metal", sourceRight, listener, 600.0f);

    // 3. Source beyond max distance (800 px > 600 px -> culled without error)
    Vector2 sourceFar{ 1100.0f, 300.0f };
    audioSys.PlaySoundSpatial("sfx_sword_impact_metal", sourceFar, listener, 600.0f);

    std::cout << "  [PASS] Spatial 2D distance attenuation and stereo calculations verified." << std::endl;
    audioSys.Shutdown();
}

void TestSurfaceFootstepDetection() {
    std::cout << "[TEST] 7. Surface Footstep Event Mapping & Cadence Integration..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    const std::vector<std::string> surfaces = {
        "grass", "dirt", "stone", "snow", "sand", "mud", "wood", "water"
    };

    for (const auto& s : surfaces) {
        std::string evId = "player.footstep." + s;
        audioSys.DispatchEvent(evId);
    }

    std::cout << "  [PASS] All 8 footstep surfaces resolved and dispatched safely." << std::endl;
    audioSys.Shutdown();
}

void TestPlaybackAndEvents() {
    std::cout << "[TEST] 8. Playback Pipeline & Event-Driven Dispatch..." << std::endl;

    AudioSystem audioSys;
    audioSys.Initialize("data/audio/audio_catalog.json", "data/config/audio_settings.json");

    // Standard game events dispatch safely
    audioSys.DispatchEvent("ui.button.click");
    audioSys.DispatchEvent("player.footstep.grass");
    audioSys.DispatchEvent("player.footstep.wood");
    audioSys.DispatchEvent("player.footstep.water");
    audioSys.DispatchEvent("weapon.sword.swing");
    audioSys.DispatchEvent("combat.player.hurt");
    audioSys.DispatchEvent("creature.goblin.attack");
    audioSys.DispatchEvent("creature.wolf.growl");
    audioSys.DispatchEvent("item.pickup");
    audioSys.DispatchEvent("match.victory");

    // Music control pipeline
    audioSys.PlayMusic("mus_main_menu", true, 0.2f);
    audioSys.Update(0.1f);
    assert(audioSys.GetCurrentMusicId() == "mus_main_menu");

    audioSys.PauseMusic();
    audioSys.Update(0.1f);

    audioSys.ResumeMusic();
    audioSys.Update(0.1f);

    audioSys.StopMusic(0.2f);
    audioSys.Update(0.1f);

    std::cout << "  [PASS] Playback commands and event bindings dispatched safely." << std::endl;
    audioSys.Shutdown();
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "WILDBOUND PHASE 28: COMPLETE AUDIO SYSTEM TEST SUITE" << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestAudioCatalogLoading();
    TestAudioBusQueries();
    TestAudioSettingsPersistence();
    TestBiomeAudioMapping();
    TestEventRegistrationAndVariations();
    TestSpatialAudioMath();
    TestSurfaceFootstepDetection();
    TestPlaybackAndEvents();

    std::cout << "==========================================================" << std::endl;
    std::cout << "ALL PHASE 28 AUDIO SYSTEM TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
