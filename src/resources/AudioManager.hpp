#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <mutex>

namespace wildbound::resources {

/**
 * Audio resource management foundation.
 * Manages raylib audio device lifecycle and caches Sound and Music stream objects.
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool Initialize(const std::string& assetRoot = "assets");
    void Shutdown();

    // Sound effects (WAV / OGG)
    bool LoadSoundFile(const std::string& id, const std::string& relativePath);
    Sound GetSound(const std::string& id) const;
    bool HasSound(const std::string& id) const;
    void PlaySoundEffect(const std::string& id, float volume = 1.0f);

    // Music streams (OGG / WAV)
    bool LoadMusicFile(const std::string& id, const std::string& relativePath);
    Music GetMusic(const std::string& id) const;
    bool HasMusic(const std::string& id) const;

    void Unload(const std::string& id);
    void UnloadAll();

    bool IsAudioReady() const;

private:
    std::string ResolvePath(const std::string& relativePath) const;

    mutable std::mutex m_mutex;
    std::string m_assetRoot = "assets";
    std::unordered_map<std::string, Sound> m_sounds;
    std::unordered_map<std::string, Music> m_music;

    bool m_audioDeviceInitialized = false;
    bool m_initialized = false;
};

} // namespace wildbound::resources

