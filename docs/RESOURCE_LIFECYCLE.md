# Wildbound Resource Lifecycle Management

## 1. Lifecycle States
Every graphical, audio, or font resource transitions through well-defined lifecycle states:

```
[ UNREGISTERED ] ──(Manifest Load)──> [ REGISTERED (Metadata Only) ]
                                                │
                                        (First Request: GetTexture)
                                                │
                                                ▼
                                         [ LOADED IN CACHE ]
                                                │
                                        (Engine Shutdown / UnloadAll)
                                                │
                                                ▼
                                          [ UNLOADED ]
```

## 2. Deduplication Policy
- When `AssetManager::GetTexture(id)` is invoked, `TextureManager` first checks its in-memory cache.
- If present, the cached `Texture2D` handle is returned immediately without disk access or GPU memory reallocation.
- If absent, it queries `AssetRegistry`, loads the file once, configures `TEXTURE_FILTER_POINT`, caches it, and returns the handle.

## 3. Fallback Texture Strategy
If a texture file cannot be located on disk or fails decompression, `TextureManager` logs an error and returns an internal 16x16 magenta/black checkerboard fallback texture (`m_fallbackTexture`). This prevents fatal crashes while making missing assets immediately noticeable during development.

## 4. Shutdown & Cleanup
- During `Game::Shutdown()`, `AssetManager::Shutdown()` is called before graphics window termination.
- All GPU textures are freed via `UnloadTexture()`.
- All audio buffers and music streams are freed via `UnloadSound()` and `UnloadMusicStream()`.
- All fonts are unloaded via `UnloadFont()`.
- The raylib audio device is closed via `CloseAudioDevice()`.
- Zero memory or handle leaks remain.

