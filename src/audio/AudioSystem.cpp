#include "AudioSystem.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::audio {

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
    Shutdown();
}

std::string AudioSystem::NormalizeEventId(std::string_view id) const {
    std::string norm;
    norm.reserve(id.size());
    for (char c : id) {
        if (c >= 'A' && c <= 'Z') {
            norm.push_back(static_cast<char>(c + 32));
        } else {
            norm.push_back(c);
        }
    }
    return norm;
}

void AudioSystem::RegisterEvent(const std::string& eventId, const std::vector<std::string>& soundIds, float cooldown) {
    if (eventId.empty() || soundIds.empty()) return;

    AudioEventDef def;
    def.eventId = eventId;
    def.soundIds = soundIds;
    def.cooldown = cooldown;
    def.currentCooldownTimer = 0.0f;
    def.nextVariationIndex = 0;

    std::string key = NormalizeEventId(eventId);
    m_events[key] = def;
}

bool AudioSystem::Initialize(const std::string& catalogPath, const std::string& settingsPath) {
    if (m_initialized) return true;

    Logger::Info("AUDIO_SYS", "Initializing Centralized Audio System (8-bus architecture)...");
    m_settingsPath = settingsPath;

    // Check / initialize raylib hardware audio device
    if (!::IsAudioDeviceReady()) {
        ::InitAudioDevice();
        m_audioDeviceReady = ::IsAudioDeviceReady();
        if (m_audioDeviceReady) {
            Logger::Info("AUDIO_SYS", "Raylib audio hardware device initialized successfully.");
        } else {
            Logger::Warn("AUDIO_SYS", "Hardware audio device unavailable. Running in muted audio mode.");
        }
    } else {
        m_audioDeviceReady = true;
    }

    // Load persisted settings
    LoadSettings(settingsPath);

    // Load master audio catalog
    if (fs::exists(catalogPath)) {
        try {
            std::ifstream file(catalogPath);
            nlohmann::json root;
            file >> root;

            if (root.contains("assets") && root["assets"].is_array()) {
                m_catalog.clear();
                m_assetIndexMap.clear();

                for (const auto& item : root["assets"]) {
                    AudioAssetInfo info;
                    info.id = item.value("id", "");
                    info.title = item.value("title", "");
                    info.category = item.value("category", "sfx");
                    info.file = item.value("file", "");
                    info.duration = item.value("duration", 0.0f);
                    info.sampleRate = item.value("sample_rate", 44100);
                    info.channels = item.value("channels", 1);
                    info.bitDepth = item.value("bit_depth", 16);
                    info.looping = item.value("looping", false);
                    info.bus = StringToAudioBus(item.value("bus", "sfx"));
                    info.defaultVolume = item.value("default_volume", 1.0f);
                    info.description = item.value("description", "");

                    m_assetIndexMap[info.id] = m_catalog.size();
                    m_catalog.push_back(info);
                }
                Logger::Info("AUDIO_SYS", "Loaded " + std::to_string(m_catalog.size()) + " audio definitions from " + catalogPath);
            }
        }
        catch (const std::exception& ex) {
            Logger::Error("AUDIO_SYS", "Failed to parse audio catalog: " + std::string(ex.what()));
        }
    } else {
        Logger::Warn("AUDIO_SYS", "Audio catalog not found: " + catalogPath);
    }

    // Load biome configs
    LoadBiomeConfigs("data/audio/biome_audio.json");

    // ========================================================================
    // Register Standard Game Audio Events
    // ========================================================================
    // 1. Player Footsteps (8 surfaces x 3 variations)
    RegisterEvent("player.footstep.grass", { "sfx_footstep_grass_001", "sfx_footstep_grass_002", "sfx_footstep_grass_003" }, 0.15f);
    RegisterEvent("player.footstep.dirt",  { "sfx_footstep_dirt_001",  "sfx_footstep_dirt_002",  "sfx_footstep_dirt_003" },  0.15f);
    RegisterEvent("player.footstep.stone", { "sfx_footstep_stone_001", "sfx_footstep_stone_002", "sfx_footstep_stone_003" }, 0.15f);
    RegisterEvent("player.footstep.snow",  { "sfx_footstep_snow_001",  "sfx_footstep_snow_002",  "sfx_footstep_snow_003" },  0.15f);
    RegisterEvent("player.footstep.sand",  { "sfx_footstep_sand_001",  "sfx_footstep_sand_002",  "sfx_footstep_sand_003" },  0.15f);
    RegisterEvent("player.footstep.mud",   { "sfx_footstep_mud_001",   "sfx_footstep_mud_002",   "sfx_footstep_mud_003" },   0.15f);
    RegisterEvent("player.footstep.wood",  { "sfx_footstep_wood_001",  "sfx_footstep_wood_002",  "sfx_footstep_wood_003" },  0.15f);
    RegisterEvent("player.footstep.water", { "sfx_footstep_water_001", "sfx_footstep_water_002", "sfx_footstep_water_003" }, 0.15f);

    // 2. Weapons & Combat Actions
    RegisterEvent("weapon.sword.swing",  { "sfx_sword_swing_001", "sfx_sword_swing_002" }, 0.12f);
    RegisterEvent("weapon.sword.impact", { "sfx_sword_impact_metal", "sfx_sword_impact_wood", "sfx_sword_impact_stone" }, 0.08f);
    RegisterEvent("weapon.sword.sheathe", { "sfx_sword_sheathe" }, 0.2f);
    RegisterEvent("weapon.bow.draw",     { "sfx_bow_draw" }, 0.25f);
    RegisterEvent("weapon.bow.release",  { "sfx_bow_release" }, 0.15f);
    RegisterEvent("weapon.bow.equip",    { "sfx_bow_equip" }, 0.2f);
    RegisterEvent("weapon.arrow.flight", { "sfx_arrow_flight" }, 0.1f);
    RegisterEvent("weapon.arrow.impact", { "sfx_arrow_impact" }, 0.08f);
    RegisterEvent("weapon.arrow.impact_flesh", { "sfx_arrow_impact_flesh" }, 0.08f);
    RegisterEvent("weapon.arrow.impact_wood",  { "sfx_arrow_impact_wood" },  0.08f);
    RegisterEvent("weapon.arrow.impact_stone", { "sfx_arrow_impact_stone" }, 0.08f);
    RegisterEvent("weapon.shield.block", { "sfx_shield_block" }, 0.15f);
    RegisterEvent("weapon.parry",        { "sfx_parry_clink" }, 0.15f);

    // 3. Combat Health & Damage Events
    RegisterEvent("combat.player.hurt",  { "sfx_player_hurt" }, 0.25f);
    RegisterEvent("combat.player.death", { "sfx_player_death" }, 0.5f);
    RegisterEvent("combat.enemy.hurt",   { "sfx_enemy_hurt" }, 0.2f);
    RegisterEvent("combat.enemy.death",  { "sfx_enemy_death" }, 0.3f);
    RegisterEvent("combat.impact",       { "sfx_impact_flesh_001", "sfx_impact_flesh_002" }, 0.1f);
    RegisterEvent("combat.critical_hit", { "sfx_combat_critical_hit" }, 0.25f);

    // 4. Creature Specific Events
    RegisterEvent("creature.goblin.idle",   { "sfx_creature_goblin_idle" }, 2.0f);
    RegisterEvent("creature.goblin.step",   { "sfx_creature_goblin_step" }, 0.2f);
    RegisterEvent("creature.goblin.attack", { "sfx_creature_goblin_attack" }, 0.3f);
    RegisterEvent("creature.goblin.hurt",   { "sfx_creature_goblin_hurt" }, 0.25f);
    RegisterEvent("creature.goblin.death",  { "sfx_creature_goblin_death" }, 0.5f);

    RegisterEvent("creature.skeleton.step",   { "sfx_creature_skeleton_step" }, 0.2f);
    RegisterEvent("creature.skeleton.attack", { "sfx_creature_skeleton_attack" }, 0.3f);
    RegisterEvent("creature.skeleton.hurt",   { "sfx_creature_skeleton_hurt" }, 0.25f);
    RegisterEvent("creature.skeleton.death",  { "sfx_creature_skeleton_death" }, 0.5f);

    RegisterEvent("creature.wolf.idle",   { "sfx_creature_wolf_idle" }, 2.0f);
    RegisterEvent("creature.wolf.growl",  { "sfx_creature_wolf_growl" }, 1.5f);
    RegisterEvent("creature.wolf.attack", { "sfx_creature_wolf_attack" }, 0.3f);
    RegisterEvent("creature.wolf.hurt",   { "sfx_creature_wolf_hurt" }, 0.25f);
    RegisterEvent("creature.wolf.death",  { "sfx_creature_wolf_death" }, 0.5f);

    RegisterEvent("creature.wild_boar.idle",   { "sfx_creature_boar_idle" }, 2.0f);
    RegisterEvent("creature.wild_boar.step",   { "sfx_creature_boar_step" }, 0.2f);
    RegisterEvent("creature.wild_boar.attack", { "sfx_creature_boar_attack" }, 0.3f);
    RegisterEvent("creature.wild_boar.hurt",   { "sfx_creature_boar_hurt" }, 0.25f);
    RegisterEvent("creature.wild_boar.death",  { "sfx_creature_boar_death" }, 0.5f);

    RegisterEvent("creature.bat.flight", { "sfx_creature_bat_flight" }, 0.3f);
    RegisterEvent("creature.bat.attack", { "sfx_creature_bat_attack" }, 0.3f);
    RegisterEvent("creature.bat.hurt",   { "sfx_creature_bat_hurt" }, 0.25f);
    RegisterEvent("creature.bat.death",  { "sfx_creature_bat_death" }, 0.5f);

    RegisterEvent("wildlife.bird.takeoff",         { "sfx_wildlife_bird_takeoff" }, 1.0f);
    RegisterEvent("wildlife.bird.landing",         { "sfx_wildlife_bird_landing" }, 1.0f);
    RegisterEvent("wildlife.butterfly.flutter",    { "sfx_wildlife_butterfly_flutter" }, 1.5f);

    // 5. UI Events
    RegisterEvent("ui.button.hover",       { "sfx_ui_hover" }, 0.04f);
    RegisterEvent("ui.button.click",       { "sfx_ui_click" }, 0.05f);
    RegisterEvent("ui.button.confirm",     { "sfx_ui_confirm" }, 0.08f);
    RegisterEvent("ui.button.cancel",      { "sfx_ui_cancel" }, 0.08f);
    RegisterEvent("ui.tab.switch",         { "sfx_ui_tab" }, 0.06f);
    RegisterEvent("ui.panel.open",         { "sfx_ui_panel_open" }, 0.1f);
    RegisterEvent("ui.panel.close",        { "sfx_ui_panel_close" }, 0.1f);
    RegisterEvent("ui.item.select",        { "sfx_ui_item_select" }, 0.05f);
    RegisterEvent("ui.cosmetic.apply",     { "sfx_ui_cosmetic_apply" }, 0.1f);
    RegisterEvent("ui.cosmetic.remove",    { "sfx_ui_cosmetic_remove" }, 0.1f);
    RegisterEvent("ui.settings.adjust",    { "sfx_ui_settings_adjust" }, 0.03f);
    RegisterEvent("ui.error",              { "sfx_ui_error" }, 0.15f);
    RegisterEvent("ui.success",            { "sfx_ui_success" }, 0.15f);
    RegisterEvent("ui.back",               { "sfx_ui_back" }, 0.08f);
    RegisterEvent("ui.modal.confirm",      { "sfx_ui_modal_confirm" }, 0.1f);

    // 6. Item & Interaction Events
    RegisterEvent("item.pickup",           { "sfx_item_pickup" }, 0.05f);
    RegisterEvent("item.drop",             { "sfx_item_drop" }, 0.05f);
    RegisterEvent("item.equip",            { "sfx_sword_sheathe" }, 0.1f);
    RegisterEvent("item.coin",             { "sfx_coin_pickup" }, 0.05f);
    RegisterEvent("item.gem",              { "sfx_gem_pickup" }, 0.05f);
    RegisterEvent("item.consume",          { "sfx_potion_drink" }, 0.1f);
    RegisterEvent("environment.door.open", { "sfx_door_open" }, 0.2f);
    RegisterEvent("environment.door.close",{ "sfx_door_close" }, 0.2f);
    RegisterEvent("environment.chest.open",{ "sfx_chest_open" }, 0.2f);

    // 7. Match Events
    RegisterEvent("match.start",           { "sfx_match_start" }, 0.5f);
    RegisterEvent("match.victory",         { "sfx_stinger_victory" }, 1.0f);
    RegisterEvent("match.defeat",          { "sfx_stinger_defeat" }, 1.0f);
    RegisterEvent("match.countdown",       { "sfx_match_countdown" }, 0.1f);
    RegisterEvent("match.alert",           { "sfx_gameplay_alert" }, 0.2f);

    // Legacy CamelCase / Alternate Aliases
    m_events[NormalizeEventId("UI.ButtonClick")]        = m_events[NormalizeEventId("ui.button.click")];
    m_events[NormalizeEventId("UI.ButtonHover")]        = m_events[NormalizeEventId("ui.button.hover")];
    m_events[NormalizeEventId("UI.ButtonConfirm")]      = m_events[NormalizeEventId("ui.button.confirm")];
    m_events[NormalizeEventId("UI.ButtonCancel")]       = m_events[NormalizeEventId("ui.button.cancel")];
    m_events[NormalizeEventId("UI.TabSwitch")]          = m_events[NormalizeEventId("ui.tab.switch")];
    m_events[NormalizeEventId("UI.PanelOpen")]          = m_events[NormalizeEventId("ui.panel.open")];
    m_events[NormalizeEventId("UI.PanelClose")]         = m_events[NormalizeEventId("ui.panel.close")];
    m_events[NormalizeEventId("UI.ItemSelect")]         = m_events[NormalizeEventId("ui.item.select")];
    m_events[NormalizeEventId("UI.CosmeticApply")]      = m_events[NormalizeEventId("ui.cosmetic.apply")];
    m_events[NormalizeEventId("UI.CosmeticRemove")]     = m_events[NormalizeEventId("ui.cosmetic.remove")];
    m_events[NormalizeEventId("UI.SettingsAdjust")]     = m_events[NormalizeEventId("ui.settings.adjust")];
    m_events[NormalizeEventId("UI.Error")]              = m_events[NormalizeEventId("ui.error")];
    m_events[NormalizeEventId("UI.Success")]            = m_events[NormalizeEventId("ui.success")];
    m_events[NormalizeEventId("UI.Back")]               = m_events[NormalizeEventId("ui.back")];
    m_events[NormalizeEventId("UI.ModalConfirm")]       = m_events[NormalizeEventId("ui.modal.confirm")];

    m_events[NormalizeEventId("Player.Footstep.Grass")] = m_events[NormalizeEventId("player.footstep.grass")];
    m_events[NormalizeEventId("Player.Footstep.Dirt")]  = m_events[NormalizeEventId("player.footstep.dirt")];
    m_events[NormalizeEventId("Player.Footstep.Stone")] = m_events[NormalizeEventId("player.footstep.stone")];
    m_events[NormalizeEventId("Player.Footstep.Snow")]  = m_events[NormalizeEventId("player.footstep.snow")];
    m_events[NormalizeEventId("Player.Footstep.Sand")]  = m_events[NormalizeEventId("player.footstep.sand")];
    m_events[NormalizeEventId("Player.Footstep.Mud")]   = m_events[NormalizeEventId("player.footstep.mud")];
    m_events[NormalizeEventId("Player.Footstep.Wood")]  = m_events[NormalizeEventId("player.footstep.wood")];
    m_events[NormalizeEventId("Player.Footstep.Water")] = m_events[NormalizeEventId("player.footstep.water")];

    m_events[NormalizeEventId("Player.Hurt")]           = m_events[NormalizeEventId("combat.player.hurt")];
    m_events[NormalizeEventId("Player.Death")]          = m_events[NormalizeEventId("combat.player.death")];
    m_events[NormalizeEventId("Combat.SwordSwing")]     = m_events[NormalizeEventId("weapon.sword.swing")];
    m_events[NormalizeEventId("Combat.SwordImpact")]    = m_events[NormalizeEventId("weapon.sword.impact")];
    m_events[NormalizeEventId("Combat.BowDraw")]        = m_events[NormalizeEventId("weapon.bow.draw")];
    m_events[NormalizeEventId("Combat.ArrowRelease")]   = m_events[NormalizeEventId("weapon.bow.release")];

    m_events[NormalizeEventId("Item.Pickup")]           = m_events[NormalizeEventId("item.pickup")];
    m_events[NormalizeEventId("Item.Drop")]             = m_events[NormalizeEventId("item.drop")];
    m_events[NormalizeEventId("Item.Equip")]            = m_events[NormalizeEventId("item.equip")];
    m_events[NormalizeEventId("Item.Coin")]             = m_events[NormalizeEventId("item.coin")];
    m_events[NormalizeEventId("Item.Gem")]              = m_events[NormalizeEventId("item.gem")];
    m_events[NormalizeEventId("Item.Consume")]          = m_events[NormalizeEventId("item.consume")];

    m_events[NormalizeEventId("Environment.DoorOpen")]  = m_events[NormalizeEventId("environment.door.open")];
    m_events[NormalizeEventId("Environment.DoorClose")] = m_events[NormalizeEventId("environment.door.close")];
    m_events[NormalizeEventId("Environment.ChestOpen")] = m_events[NormalizeEventId("environment.chest.open")];
    m_events[NormalizeEventId("Match.Start")]           = m_events[NormalizeEventId("match.start")];
    m_events[NormalizeEventId("Match.Victory")]         = m_events[NormalizeEventId("match.victory")];
    m_events[NormalizeEventId("Match.Defeat")]          = m_events[NormalizeEventId("match.defeat")];
    m_events[NormalizeEventId("Match.Countdown")]       = m_events[NormalizeEventId("match.countdown")];
    m_events[NormalizeEventId("Match.Alert")]           = m_events[NormalizeEventId("match.alert")];

    m_initialized = true;
    return true;
}

void AudioSystem::Shutdown() {
    if (!m_initialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    Logger::Info("AUDIO_SYS", "Shutting down AudioSystem, releasing audio buffers...");

    // Stop and unload music
    if (m_musicActive && !m_currentMusicId.empty()) {
        auto it = m_loadedMusic.find(m_currentMusicId);
        if (it != m_loadedMusic.end() && m_audioDeviceReady) {
            ::StopMusicStream(it->second);
        }
    }
    m_musicActive = false;

    if (m_audioDeviceReady) {
        for (auto& [id, mus] : m_loadedMusic) {
            ::UnloadMusicStream(mus);
        }
        m_loadedMusic.clear();

        for (auto& [id, snd] : m_loadedSounds) {
            ::UnloadSound(snd);
        }
        m_loadedSounds.clear();
    }

    m_initialized = false;
}

std::string AudioSystem::ResolvePath(const std::string& relPath) const {
    if (relPath.empty()) return "";
    if (fs::exists(relPath)) return relPath;

    const std::vector<std::string> candidates = {
        "assets/" + relPath,
        "../assets/" + relPath,
        "../../assets/" + relPath
    };
    for (const auto& c : candidates) {
        if (fs::exists(c)) return c;
    }
    return "assets/" + relPath;
}

void AudioSystem::Update(float dt) {
    if (!m_initialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. Update cooldown timers for all registered events
    for (auto& [name, ev] : m_events) {
        if (ev.currentCooldownTimer > 0.0f) {
            ev.currentCooldownTimer = std::max(0.0f, ev.currentCooldownTimer - dt);
        }
    }

    // 2. Update active music stream
    if (m_audioDeviceReady && m_musicActive && !m_currentMusicId.empty()) {
        auto it = m_loadedMusic.find(m_currentMusicId);
        if (it != m_loadedMusic.end()) {
            ::UpdateMusicStream(it->second);

            // Handle fade transitions
            if (m_musicFadeDuration > 0.0f && m_musicFadeTimer < m_musicFadeDuration) {
                m_musicFadeTimer += dt;
                float progress = std::clamp(m_musicFadeTimer / m_musicFadeDuration, 0.0f, 1.0f);
                float effectiveVol = GetEffectiveVolume(AudioBus::Music) * m_targetMusicVolume * progress;
                ::SetMusicVolume(it->second, effectiveVol);
            }
        }
    }

    // 3. Periodic Ambient One-Shot Triggers
    if (!m_currentBiomeId.empty()) {
        m_ambientTimer += dt;
        if (m_ambientTimer >= m_ambientInterval) {
            m_ambientTimer = 0.0f;
            // Randomized interval between 7s and 14s
            m_ambientInterval = 7.0f + static_cast<float>(rand() % 70) / 10.0f;

            if (m_currentBiomeId == "forest") {
                PlaySound("sfx_wildlife_bird_takeoff", 0.4f);
            } else if (m_currentBiomeId == "desert") {
                PlaySound("sfx_env_desert_sand_shift", 0.5f);
            } else if (m_currentBiomeId == "snow") {
                PlaySound("sfx_env_ice_crack", 0.45f);
            } else if (m_currentBiomeId == "swamp") {
                PlaySound("sfx_env_swamp_bubble", 0.5f);
            } else if (m_currentBiomeId == "mountain") {
                PlaySound("sfx_env_mountain_rockfall", 0.45f);
            } else if (m_currentBiomeId == "village") {
                PlaySound("sfx_env_wood_creak", 0.4f);
            }
        }
    }
}

// ============================================================================
// Bus Volume Controls (8 Dedicated Buses)
// ============================================================================

void AudioSystem::SetMasterVolume(float volume) {
    m_settings.masterVolume = std::clamp(volume, 0.0f, 1.0f);
    ApplyBusVolumes();
}

void AudioSystem::SetBusVolume(AudioBus bus, float volume) {
    float clamped = std::clamp(volume, 0.0f, 1.0f);
    switch (bus) {
        case AudioBus::Master:      m_settings.masterVolume = clamped; break;
        case AudioBus::Music:       m_settings.musicVolume = clamped; break;
        case AudioBus::UI:          m_settings.uiVolume = clamped; break;
        case AudioBus::SFX:         m_settings.sfxVolume = clamped; break;
        case AudioBus::Environment: m_settings.environmentVolume = clamped; break;
        case AudioBus::Combat:      m_settings.combatVolume = clamped; break;
        case AudioBus::Creatures:   m_settings.creaturesVolume = clamped; break;
        case AudioBus::Voice:       m_settings.voiceVolume = clamped; break;
    }
    ApplyBusVolumes();
}

float AudioSystem::GetBusVolume(AudioBus bus) const {
    switch (bus) {
        case AudioBus::Master:      return m_settings.masterVolume;
        case AudioBus::Music:       return m_settings.musicVolume;
        case AudioBus::UI:          return m_settings.uiVolume;
        case AudioBus::SFX:         return m_settings.sfxVolume;
        case AudioBus::Environment: return m_settings.environmentVolume;
        case AudioBus::Combat:      return m_settings.combatVolume;
        case AudioBus::Creatures:   return m_settings.creaturesVolume;
        case AudioBus::Voice:       return m_settings.voiceVolume;
        default:                    return 1.0f;
    }
}

float AudioSystem::GetEffectiveVolume(AudioBus bus) const {
    if (m_settings.muteAll) return 0.0f;

    float baseBusVol = GetBusVolume(bus);
    if (bus == AudioBus::Music && !m_settings.musicEnabled) return 0.0f;
    if (bus == AudioBus::SFX && !m_settings.sfxEnabled) return 0.0f;
    if (bus == AudioBus::Environment && !m_settings.environmentEnabled) return 0.0f;

    return m_settings.masterVolume * baseBusVol;
}

void AudioSystem::SetMuteAll(bool mute) {
    m_settings.muteAll = mute;
    ApplyBusVolumes();
}

bool AudioSystem::IsMuteAll() const {
    return m_settings.muteAll;
}

void AudioSystem::ApplyBusVolumes() {
    if (!m_audioDeviceReady) return;

    if (m_musicActive && !m_currentMusicId.empty()) {
        auto it = m_loadedMusic.find(m_currentMusicId);
        if (it != m_loadedMusic.end()) {
            float eff = GetEffectiveVolume(AudioBus::Music) * m_targetMusicVolume;
            ::SetMusicVolume(it->second, eff);
        }
    }
}

void AudioSystem::SetSettings(const AudioSettings& settings) {
    m_settings = settings;
    ApplyBusVolumes();
}

bool AudioSystem::LoadSettings(const std::string& path) {
    if (!fs::exists(path)) return false;
    try {
        std::ifstream file(path);
        nlohmann::json j;
        file >> j;
        m_settings = AudioSettings::FromJson(j);
        ApplyBusVolumes();
        return true;
    }
    catch (...) {
        return false;
    }
}

bool AudioSystem::SaveSettings(const std::string& path) const {
    try {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream file(path);
        file << m_settings.ToJson().dump(4);
        return true;
    }
    catch (...) {
        return false;
    }
}

// ============================================================================
// Music Playback
// ============================================================================

bool AudioSystem::PlayMusic(const std::string& musicId, bool loop, float fadeTime) {
    if (!m_initialized) return false;

    std::lock_guard<std::mutex> lock(m_mutex);

    // If already playing this music, do nothing
    if (m_musicActive && m_currentMusicId == musicId) {
        return true;
    }

    // Stop previous music
    if (m_musicActive && !m_currentMusicId.empty()) {
        auto prevIt = m_loadedMusic.find(m_currentMusicId);
        if (prevIt != m_loadedMusic.end() && m_audioDeviceReady) {
            ::StopMusicStream(prevIt->second);
        }
    }

    const AudioAssetInfo* info = GetAssetInfo(musicId);
    if (!info) {
        Logger::Warn("AUDIO_SYS", "Music track not found in catalog: " + musicId);
        return false;
    }

    if (!m_audioDeviceReady) {
        m_currentMusicId = musicId;
        m_musicActive = true;
        return true;
    }

    // Load music if not cached
    auto it = m_loadedMusic.find(musicId);
    if (it == m_loadedMusic.end()) {
        std::string fullPath = ResolvePath(info->file);
        if (!fs::exists(fullPath)) {
            Logger::Warn("AUDIO_SYS", "Music file not found on disk: " + fullPath);
            return false;
        }

        ::Music mus = ::LoadMusicStream(fullPath.c_str());
        if (mus.stream.buffer == nullptr) {
            Logger::Error("AUDIO_SYS", "Failed to load music stream: " + fullPath);
            return false;
        }
        mus.looping = loop;
        m_loadedMusic[musicId] = mus;
        it = m_loadedMusic.find(musicId);
    }

    m_currentMusicId = musicId;
    m_musicActive = true;
    m_targetMusicVolume = info->defaultVolume;
    m_musicFadeDuration = fadeTime;
    m_musicFadeTimer = (fadeTime > 0.0f) ? 0.0f : fadeTime;

    float initialVol = (fadeTime > 0.0f) ? 0.0f : GetEffectiveVolume(AudioBus::Music) * m_targetMusicVolume;
    ::SetMusicVolume(it->second, initialVol);
    ::PlayMusicStream(it->second);

    Logger::Debug("AUDIO_SYS", "Started music playback: [" + musicId + "] (" + info->title + ")");
    return true;
}

void AudioSystem::StopMusic(float fadeTime) {
    (void)fadeTime;
    if (!m_initialized || !m_musicActive) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_audioDeviceReady) {
        auto it = m_loadedMusic.find(m_currentMusicId);
        if (it != m_loadedMusic.end()) {
            ::StopMusicStream(it->second);
        }
    }
    m_musicActive = false;
    m_currentMusicId.clear();
}

void AudioSystem::PauseMusic() {
    if (!m_initialized || !m_audioDeviceReady || !m_musicActive) return;
    auto it = m_loadedMusic.find(m_currentMusicId);
    if (it != m_loadedMusic.end()) {
        ::PauseMusicStream(it->second);
    }
}

void AudioSystem::ResumeMusic() {
    if (!m_initialized || !m_audioDeviceReady || !m_musicActive) return;
    auto it = m_loadedMusic.find(m_currentMusicId);
    if (it != m_loadedMusic.end()) {
        ::ResumeMusicStream(it->second);
    }
}

bool AudioSystem::IsMusicPlaying() const {
    if (!m_musicActive || m_currentMusicId.empty()) return false;
    if (!m_audioDeviceReady) return m_musicActive;

    auto it = m_loadedMusic.find(m_currentMusicId);
    if (it != m_loadedMusic.end()) {
        return ::IsMusicStreamPlaying(it->second);
    }
    return false;
}

// ============================================================================
// Sound Effect Playback
// ============================================================================

void AudioSystem::PlaySound(const std::string& soundId, float volumeScale, float pitchScale) {
    if (!m_initialized) return;

    const AudioAssetInfo* info = GetAssetInfo(soundId);
    if (!info) {
        Logger::Warn("AUDIO_SYS", "Sound effect not found in catalog: " + soundId);
        return;
    }

    if (!m_audioDeviceReady) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    // Load sound into cache if not present
    auto it = m_loadedSounds.find(soundId);
    if (it == m_loadedSounds.end()) {
        std::string fullPath = ResolvePath(info->file);
        if (!fs::exists(fullPath)) {
            Logger::Warn("AUDIO_SYS", "Sound file not found on disk: " + fullPath);
            return;
        }

        ::Sound snd = ::LoadSound(fullPath.c_str());
        if (snd.frameCount == 0) {
            Logger::Error("AUDIO_SYS", "Failed to load raylib sound buffer: " + fullPath);
            return;
        }
        m_loadedSounds[soundId] = snd;
        it = m_loadedSounds.find(soundId);
    }

    float busVol = GetEffectiveVolume(info->bus);
    float finalVol = std::clamp(busVol * info->defaultVolume * volumeScale, 0.0f, 1.0f);

    ::SetSoundVolume(it->second, finalVol);
    ::SetSoundPan(it->second, 0.5f); // Center stereo
    if (pitchScale != 1.0f) {
        ::SetSoundPitch(it->second, pitchScale);
    }
    ::PlaySound(it->second);
}

// ============================================================================
// Spatial Audio Playback (Distance Attenuation + Stereo Panning)
// ============================================================================

void AudioSystem::PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos, float maxDistance, float volumeScale, float pitchScale) {
    PlaySoundSpatial(soundId, sourcePos, m_listenerPos, maxDistance, volumeScale, pitchScale);
}

void AudioSystem::PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos, Vector2 listenerPos, float maxDistance, float volumeScale, float pitchScale) {
    if (!m_initialized) return;

    float dx = sourcePos.x - listenerPos.x;
    float dy = sourcePos.y - listenerPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist >= maxDistance || maxDistance <= 0.0f) {
        return; // Audible distance culled
    }

    // Quadratic distance attenuation for natural acoustic falloff
    float linearNorm = std::clamp(1.0f - (dist / maxDistance), 0.0f, 1.0f);
    float distanceAttenuation = linearNorm * linearNorm;

    // Stereo panning based on horizontal displacement (-1.0 = full left, +1.0 = full right)
    constexpr float PAN_HORIZON = 400.0f;
    float panOffset = std::clamp(dx / PAN_HORIZON, -1.0f, 1.0f);
    float raylibPan = 0.5f + (0.5f * panOffset); // Map [-1..1] to [0..1]

    const AudioAssetInfo* info = GetAssetInfo(soundId);
    if (!info) {
        Logger::Warn("AUDIO_SYS", "Spatial sound effect not found in catalog: " + soundId);
        return;
    }

    if (!m_audioDeviceReady) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_loadedSounds.find(soundId);
    if (it == m_loadedSounds.end()) {
        std::string fullPath = ResolvePath(info->file);
        if (!fs::exists(fullPath)) return;

        ::Sound snd = ::LoadSound(fullPath.c_str());
        if (snd.frameCount == 0) return;
        m_loadedSounds[soundId] = snd;
        it = m_loadedSounds.find(soundId);
    }

    float busVol = GetEffectiveVolume(info->bus);
    float finalVol = std::clamp(busVol * info->defaultVolume * volumeScale * distanceAttenuation, 0.0f, 1.0f);

    ::SetSoundVolume(it->second, finalVol);
    ::SetSoundPan(it->second, raylibPan);
    if (pitchScale != 1.0f) {
        ::SetSoundPitch(it->second, pitchScale);
    }
    ::PlaySound(it->second);
}

// ============================================================================
// Event-Driven Audio Dispatch
// ============================================================================

void AudioSystem::DispatchEvent(const std::string& eventId) {
    std::string norm = NormalizeEventId(eventId);
    auto it = m_events.find(norm);
    if (it == m_events.end()) {
        Logger::Debug("AUDIO_SYS", "Unhandled audio event: " + eventId);
        return;
    }

    AudioEventDef& def = it->second;
    if (def.currentCooldownTimer > 0.0f) {
        return; // Suppressed by cooldown throttling
    }

    if (def.soundIds.empty()) return;

    // Select variation
    const std::string& chosenSound = def.soundIds[def.nextVariationIndex % def.soundIds.size()];
    def.nextVariationIndex = (def.nextVariationIndex + 1) % def.soundIds.size();
    def.currentCooldownTimer = def.cooldown;

    PlaySound(chosenSound);
}

void AudioSystem::DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, float maxDistance) {
    DispatchEventSpatial(eventId, sourcePos, m_listenerPos, maxDistance);
}

void AudioSystem::DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, Vector2 listenerPos, float maxDistance) {
    std::string norm = NormalizeEventId(eventId);
    auto it = m_events.find(norm);
    if (it == m_events.end()) return;

    AudioEventDef& def = it->second;
    if (def.currentCooldownTimer > 0.0f) return;
    if (def.soundIds.empty()) return;

    const std::string& chosenSound = def.soundIds[def.nextVariationIndex % def.soundIds.size()];
    def.nextVariationIndex = (def.nextVariationIndex + 1) % def.soundIds.size();
    def.currentCooldownTimer = def.cooldown;

    PlaySoundSpatial(chosenSound, sourcePos, listenerPos, maxDistance);
}

// ============================================================================
// Biome Audio Integration
// ============================================================================

bool AudioSystem::LoadBiomeConfigs(const std::string& configPath) {
    if (!fs::exists(configPath)) return false;
    try {
        std::ifstream file(configPath);
        nlohmann::json root;
        file >> root;

        if (root.contains("biomes") && root["biomes"].is_object()) {
            m_biomeConfigs.clear();
            for (auto& [bId, bObj] : root["biomes"].items()) {
                BiomeAudioConfig cfg;
                cfg.biomeId = bId;
                cfg.musicTrackId = bObj.value("music", "");
                if (bObj.contains("ambient_layers") && bObj["ambient_layers"].is_array()) {
                    cfg.ambientLayers = bObj["ambient_layers"].get<std::vector<std::string>>();
                }
                cfg.defaultVolume = bObj.value("default_volume", 0.75f);
                m_biomeConfigs[bId] = cfg;
            }
            return true;
        }
    }
    catch (...) {
        return false;
    }
    return false;
}

void AudioSystem::SetBiome(const std::string& biomeId) {
    if (m_currentBiomeId == biomeId) return;
    m_currentBiomeId = biomeId;
    m_ambientTimer = 0.0f;

    auto it = m_biomeConfigs.find(biomeId);
    if (it != m_biomeConfigs.end()) {
        if (!it->second.musicTrackId.empty()) {
            PlayMusic(it->second.musicTrackId, true, 1.0f);
        }
    }
}

// ============================================================================
// Catalog Queries
// ============================================================================

const AudioAssetInfo* AudioSystem::GetAssetInfo(const std::string& id) const {
    auto it = m_assetIndexMap.find(id);
    if (it != m_assetIndexMap.end() && it->second < m_catalog.size()) {
        return &m_catalog[it->second];
    }
    return nullptr;
}

std::vector<const AudioAssetInfo*> AudioSystem::GetAssetsByBus(AudioBus bus) const {
    std::vector<const AudioAssetInfo*> res;
    for (const auto& a : m_catalog) {
        if (a.bus == bus) {
            res.push_back(&a);
        }
    }
    return res;
}

} // namespace wildbound::audio
