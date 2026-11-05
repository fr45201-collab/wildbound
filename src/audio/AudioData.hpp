#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>

namespace wildbound::audio {

enum class AudioBus {
    Master = 0,
    Music,
    UI,
    SFX,
    Environment,
    Combat,
    Creatures,
    Voice
};

inline std::string_view AudioBusToString(AudioBus bus) {
    switch (bus) {
        case AudioBus::Master:      return "master";
        case AudioBus::Music:       return "music";
        case AudioBus::UI:          return "ui";
        case AudioBus::SFX:         return "sfx";
        case AudioBus::Environment: return "environment";
        case AudioBus::Combat:      return "combat";
        case AudioBus::Creatures:   return "creatures";
        case AudioBus::Voice:       return "voice";
        default:                    return "unknown";
    }
}

inline AudioBus StringToAudioBus(std::string_view str) {
    if (str == "master")      return AudioBus::Master;
    if (str == "music")       return AudioBus::Music;
    if (str == "ui")          return AudioBus::UI;
    if (str == "sfx")         return AudioBus::SFX;
    if (str == "environment") return AudioBus::Environment;
    if (str == "combat")      return AudioBus::Combat;
    if (str == "creatures")   return AudioBus::Creatures;
    if (str == "voice")       return AudioBus::Voice;
    return AudioBus::SFX;
}

struct AudioSettings {
    float masterVolume = 0.8f;
    float musicVolume = 0.7f;
    float sfxVolume = 0.8f;
    float uiVolume = 0.8f;
    float environmentVolume = 0.7f;
    float combatVolume = 0.8f;
    float creaturesVolume = 0.75f;
    float voiceVolume = 0.8f;
    bool muteAll = false;
    bool musicEnabled = true;
    bool sfxEnabled = true;
    bool environmentEnabled = true;

    nlohmann::json ToJson() const {
        return {
            { "master_volume", masterVolume },
            { "music_volume", musicVolume },
            { "sfx_volume", sfxVolume },
            { "ui_volume", uiVolume },
            { "environment_volume", environmentVolume },
            { "combat_volume", combatVolume },
            { "creatures_volume", creaturesVolume },
            { "voice_volume", voiceVolume },
            { "mute_all", muteAll },
            { "music_enabled", musicEnabled },
            { "sfx_enabled", sfxEnabled },
            { "environment_enabled", environmentEnabled }
        };
    }

    static AudioSettings FromJson(const nlohmann::json& j) {
        AudioSettings s;
        s.masterVolume = j.value("master_volume", 0.8f);
        s.musicVolume = j.value("music_volume", 0.7f);
        s.sfxVolume = j.value("sfx_volume", 0.8f);
        s.uiVolume = j.value("ui_volume", 0.8f);
        s.environmentVolume = j.value("environment_volume", 0.7f);
        s.combatVolume = j.value("combat_volume", 0.8f);
        s.creaturesVolume = j.value("creatures_volume", 0.75f);
        s.voiceVolume = j.value("voice_volume", 0.8f);
        s.muteAll = j.value("mute_all", false);
        s.musicEnabled = j.value("music_enabled", true);
        s.sfxEnabled = j.value("sfx_enabled", true);
        s.environmentEnabled = j.value("environment_enabled", true);
        return s;
    }
};

struct AudioAssetInfo {
    std::string id;
    std::string title;
    std::string category;
    std::string file;
    float duration = 0.0f;
    int sampleRate = 44100;
    int channels = 1;
    int bitDepth = 16;
    bool looping = false;
    AudioBus bus = AudioBus::SFX;
    float defaultVolume = 1.0f;
    std::string description;
};

struct BiomeAudioConfig {
    std::string biomeId;
    std::string musicTrackId;
    std::vector<std::string> ambientLayers;
    float defaultVolume = 0.75f;
};

} // namespace wildbound::audio
