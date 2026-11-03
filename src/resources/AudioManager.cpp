#include "AudioManager.hpp"
#include "../core/Logger.hpp"
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize(const std::string& assetRoot) {
    if (m_initialized) return true;

    m_assetRoot = assetRoot;

    // Initialize raylib audio device if not already running
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        m_audioDeviceInitialized = IsAudioDeviceReady();
        if (!m_audioDeviceInitialized) {
            Logger::Warn("AUDIO_MGR", "Failed to initialize hardware audio device. Audio will be disabled.");
        } else {
            Logger::Info("AUDIO_MGR", "Hardware audio device initialized successfully.");
        }
    } else {
        m_audioDeviceInitialized = true;
    }

    m_initialized = true;
    return true;
}

void AudioManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("AUDIO_MGR", "Shutting down AudioManager, releasing audio streams and sound buffers.");
        UnloadAll();

        if (m_audioDeviceInitialized && IsAudioDeviceReady()) {
            CloseAudioDevice();
            m_audioDeviceInitialized = false;
        }

        m_initialized = false;
    }
}

std::string AudioManager::ResolvePath(const std::string& relativePath) const {
    if (relativePath.empty()) return "";
    if (fs::exists(relativePath)) return relativePath;

    const std::vector<std::string> candidates = {
        m_assetRoot + "/" + relativePath,
        "assets/" + relativePath,
        "../assets/" + relativePath,
        "../../assets/" + relativePath
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) return path;
    }

    return m_assetRoot + "/" + relativePath;
}

bool AudioManager::LoadSoundFile(const std::string& id, const std::string& relativePath) {
    if (!m_audioDeviceInitialized) return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_sounds.find(id) != m_sounds.end()) return true;

    std::string resolved = ResolvePath(relativePath);
    if (!fs::exists(resolved)) {
        Logger::Error("AUDIO_MGR", "Sound file not found for asset '" + id + "': " + resolved);
        return false;
    }

    Sound s = LoadSound(resolved.c_str());
    if (s.stream.buffer == nullptr) {
        Logger::Error("AUDIO_MGR", "Failed to decode sound file: " + resolved);
        return false;
    }

    m_sounds[id] = s;
    Logger::Info("AUDIO_MGR", "Loaded sound effect [" + id + "] from " + resolved);
    return true;
}

Sound AudioManager::GetSound(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        return it->second;
    }
    return Sound{};
}

bool AudioManager::HasSound(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sounds.find(id) != m_sounds.end();
}

void AudioManager::PlaySoundEffect(const std::string& id, float volume) {
    if (!m_audioDeviceInitialized) return;

    Sound s = GetSound(id);
    if (s.stream.buffer != nullptr) {
        SetSoundVolume(s, volume);
        PlaySound(s);
    }
}

bool AudioManager::LoadMusicFile(const std::string& id, const std::string& relativePath) {
    if (!m_audioDeviceInitialized) return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_music.find(id) != m_music.end()) return true;

    std::string resolved = ResolvePath(relativePath);
    if (!fs::exists(resolved)) {
        Logger::Error("AUDIO_MGR", "Music file not found for asset '" + id + "': " + resolved);
        return false;
    }

    Music m = LoadMusicStream(resolved.c_str());
    if (m.stream.buffer == nullptr) {
        Logger::Error("AUDIO_MGR", "Failed to stream music file: " + resolved);
        return false;
    }

    m_music[id] = m;
    Logger::Info("AUDIO_MGR", "Loaded music stream [" + id + "] from " + resolved);
    return true;
}

Music AudioManager::GetMusic(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_music.find(id);
    if (it != m_music.end()) {
        return it->second;
    }
    return Music{};
}

bool AudioManager::HasMusic(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_music.find(id) != m_music.end();
}

void AudioManager::Unload(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto soundIt = m_sounds.find(id);
    if (soundIt != m_sounds.end()) {
        UnloadSound(soundIt->second);
        m_sounds.erase(soundIt);
    }

    auto musicIt = m_music.find(id);
    if (musicIt != m_music.end()) {
        UnloadMusicStream(musicIt->second);
        m_music.erase(musicIt);
    }
}

void AudioManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, s] : m_sounds) {
        if (s.stream.buffer != nullptr) {
            UnloadSound(s);
        }
    }
    m_sounds.clear();

    for (auto& [id, m] : m_music) {
        if (m.stream.buffer != nullptr) {
            UnloadMusicStream(m);
        }
    }
    m_music.clear();
}

bool AudioManager::IsAudioReady() const {
    return m_audioDeviceInitialized && IsAudioDeviceReady();
}

} // namespace wildbound::resources

