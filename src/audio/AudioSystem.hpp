#pragma once

#include "AudioData.hpp"
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

namespace wildbound::audio {

/**
 * Event configuration for cooldowns and multi-sound variations.
 */
struct AudioEventDef {
    std::string eventId;
    std::vector<std::string> soundIds;
    float cooldown = 0.0f;
    float currentCooldownTimer = 0.0f;
    size_t nextVariationIndex = 0;
};

/**
 * Centralized High-Performance Audio Management System.
 * Manages 8 audio buses, spatial attenuation, stereo panning, procedural music streaming,
 * sound effect caching, event-driven dispatch, and biome environmental ambience.
 */
class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    bool Initialize(const std::string& catalogPath = "data/audio/audio_catalog.json",
                    const std::string& settingsPath = "data/config/audio_settings.json");
    void Shutdown();

    void Update(float dt);

    // Audio Bus Volume Controls (8 Dedicated Buses)
    void SetMasterVolume(float volume);
    void SetBusVolume(AudioBus bus, float volume);
    [[nodiscard]] float GetBusVolume(AudioBus bus) const;
    [[nodiscard]] float GetEffectiveVolume(AudioBus bus) const;

    void SetMuteAll(bool mute);
    [[nodiscard]] bool IsMuteAll() const;

    // Settings Persistence
    bool LoadSettings(const std::string& path = "data/config/audio_settings.json");
    bool SaveSettings(const std::string& path = "data/config/audio_settings.json") const;
    [[nodiscard]] const AudioSettings& GetSettings() const { return m_settings; }
    void SetSettings(const AudioSettings& settings);

    // Listener & Spatial Audio
    void SetListenerPosition(Vector2 pos) { m_listenerPos = pos; }
    [[nodiscard]] Vector2 GetListenerPosition() const { return m_listenerPos; }

    void PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos,
                          float maxDistance = 600.0f, float volumeScale = 1.0f, float pitchScale = 1.0f);
    void PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos, Vector2 listenerPos,
                          float maxDistance = 600.0f, float volumeScale = 1.0f, float pitchScale = 1.0f);

    // Music Playback
    bool PlayMusic(const std::string& musicId, bool loop = true, float fadeTime = 0.5f);
    void StopMusic(float fadeTime = 0.5f);
    void PauseMusic();
    void ResumeMusic();
    [[nodiscard]] bool IsMusicPlaying() const;
    [[nodiscard]] const std::string& GetCurrentMusicId() const { return m_currentMusicId; }

    // Sound Effect Playback
    void PlaySound(const std::string& soundId, float volumeScale = 1.0f, float pitchScale = 1.0f);

    // Event-Driven Audio Dispatch
    void RegisterEvent(const std::string& eventId, const std::vector<std::string>& soundIds, float cooldown = 0.0f);
    void DispatchEvent(const std::string& eventId);
    void DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, float maxDistance = 600.0f);
    void DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, Vector2 listenerPos, float maxDistance = 600.0f);

    // Biome Ambient Audio Integration
    bool LoadBiomeConfigs(const std::string& configPath = "data/audio/biome_audio.json");
    void SetBiome(const std::string& biomeId);
    [[nodiscard]] const std::string& GetCurrentBiome() const { return m_currentBiomeId; }

    // Catalog & Metadata Queries
    [[nodiscard]] const std::vector<AudioAssetInfo>& GetAllAssets() const { return m_catalog; }
    [[nodiscard]] const AudioAssetInfo* GetAssetInfo(const std::string& id) const;
    [[nodiscard]] std::vector<const AudioAssetInfo*> GetAssetsByBus(AudioBus bus) const;
    [[nodiscard]] size_t GetAssetCount() const { return m_catalog.size(); }
    [[nodiscard]] size_t GetLoadedSoundCount() const { return m_loadedSounds.size(); }

private:
    std::string ResolvePath(const std::string& relPath) const;
    void ApplyBusVolumes();
    std::string NormalizeEventId(std::string_view id) const;

    mutable std::mutex m_mutex;
    bool m_initialized = false;
    bool m_audioDeviceReady = false;

    AudioSettings m_settings;
    std::string m_settingsPath = "data/config/audio_settings.json";

    std::vector<AudioAssetInfo> m_catalog;
    std::unordered_map<std::string, size_t> m_assetIndexMap;
    std::unordered_map<std::string, BiomeAudioConfig> m_biomeConfigs;
    std::unordered_map<std::string, AudioEventDef> m_events;

    // Raylib loaded resources cache
    std::unordered_map<std::string, ::Sound> m_loadedSounds;
    std::unordered_map<std::string, ::Music> m_loadedMusic;

    // Listener world position
    Vector2 m_listenerPos{ 0.0f, 0.0f };

    // Active music state
    std::string m_currentMusicId;
    std::string m_currentBiomeId;
    bool m_musicActive = false;
    float m_musicFadeTimer = 0.0f;
    float m_musicFadeDuration = 0.0f;
    float m_targetMusicVolume = 1.0f;

    // Ambient periodic one-shot timers
    float m_ambientTimer = 0.0f;
    float m_ambientInterval = 8.0f;
};

} // namespace wildbound::audio
