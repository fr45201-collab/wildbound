# AudioManager Specification

## 1. Class Overview

`wildbound::audio::AudioSystem` serves as the centralized audio manager and engine gateway for Wildbound. It provides safe lifecycle control, multi-bus routing, thread-safe resource caching, spatial sound dispatching, and biome streaming.

```cpp
namespace wildbound::audio {
    class AudioSystem;
}
```

---

## 2. Public API Surface

### 2.1 Lifecycle & Configuration
```cpp
bool Initialize(const std::string& catalogPath = "data/audio/audio_catalog.json",
                const std::string& settingsPath = "data/config/audio_settings.json");
void Shutdown();
void Update(float dt);
```
- **`Initialize`**: Checks hardware device availability via raylib (`IsAudioDeviceReady()`). Loads master JSON catalog and audio settings. Pre-registers all standard gameplay events and aliases.
- **`Shutdown`**: Stops active music streams, unloads all cached `::Sound` and `::Music` raylib buffers, and resets internal state.
- **`Update`**: Decays event cooldown timers, updates active music stream buffers (`UpdateMusicStream`), and executes ambient periodic sound scheduling.

### 2.2 Bus & Volume Controls
```cpp
void SetMasterVolume(float volume);
void SetBusVolume(AudioBus bus, float volume);
float GetBusVolume(AudioBus bus) const;
float GetEffectiveVolume(AudioBus bus) const;

void SetMuteAll(bool mute);
bool IsMuteAll() const;
```
- Volumes are automatically clamped to $[0.0..1.0]$.
- Changes immediately re-evaluate effective volume and update active streaming music buffers.

### 2.3 Settings Persistence
```cpp
bool LoadSettings(const std::string& path = "data/config/audio_settings.json");
bool SaveSettings(const std::string& path = "data/config/audio_settings.json") const;
const AudioSettings& GetSettings() const;
void SetSettings(const AudioSettings& settings);
```
- Serializes bus volume settings and mute states to JSON with formatted indentation.

### 2.4 Music Streaming
```cpp
bool PlayMusic(const std::string& musicId, bool loop = true, float fadeTime = 0.5f);
void StopMusic(float fadeTime = 0.5f);
void PauseMusic();
void ResumeMusic();
bool IsMusicPlaying() const;
const std::string& GetCurrentMusicId() const;
```
- Streams audio from disk via raylib `Music` handles.
- Smoothly fades in over `fadeTime` seconds.

### 2.5 Sound Playback & Spatial Dispatch
```cpp
void PlaySound(const std::string& soundId, float volumeScale = 1.0f, float pitchScale = 1.0f);
void PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos,
                      float maxDistance = 600.0f, float volumeScale = 1.0f, float pitchScale = 1.0f);
void PlaySoundSpatial(const std::string& soundId, Vector2 sourcePos, Vector2 listenerPos,
                      float maxDistance = 600.0f, float volumeScale = 1.0f, float pitchScale = 1.0f);

void RegisterEvent(const std::string& eventId, const std::vector<std::string>& soundIds, float cooldown = 0.0f);
void DispatchEvent(const std::string& eventId);
void DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, float maxDistance = 600.0f);
void DispatchEventSpatial(const std::string& eventId, Vector2 sourcePos, Vector2 listenerPos, float maxDistance = 600.0f);
```

### 2.6 Listener & Biome Integration
```cpp
void SetListenerPosition(Vector2 pos);
Vector2 GetListenerPosition() const;

bool LoadBiomeConfigs(const std::string& configPath = "data/audio/biome_audio.json");
void SetBiome(const std::string& biomeId);
const std::string& GetCurrentBiome() const;
```

---

## 3. Resource Management & Caching

Raylib `Sound` and `Music` instances are loaded on-demand (lazy initialization) and retained in thread-safe hash maps:
- `std::unordered_map<std::string, ::Sound> m_loadedSounds;`
- `std::unordered_map<std::string, ::Music> m_loadedMusic;`

Resource lookups are protected by `std::mutex m_mutex`. When running in headless environments or where audio hardware is disabled, the system operates silently without throwing exceptions or crashing.

