#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <cstdint>

namespace wildbound::resources {

/**
 * High-performance, thread-safe texture cache and lifecycle manager.
 * Guarantees zero duplicate loads, point filtering for pixel art, and safe RAII cleanup.
 */
class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    bool Initialize(const std::string& assetRoot = "assets");
    void Shutdown();

    // Texture acquisition & caching
    Texture2D LoadTextureFile(const std::string& id, const std::string& relativePath);
    Texture2D GetTexture(const std::string& id) const;
    bool HasTexture(const std::string& id) const;

    bool Unload(const std::string& id);
    void UnloadAll();

    // Metrics & Inspection
    size_t GetLoadedTextureCount() const;
    size_t GetTotalMemoryUsageBytes() const;
    Texture2D GetFallbackTexture() const { return m_fallbackTexture; }

    void SetAssetRoot(const std::string& root) { m_assetRoot = root; }
    const std::string& GetAssetRoot() const { return m_assetRoot; }

private:
    void CreateFallbackTexture();
    std::string ResolvePath(const std::string& relativePath) const;

    mutable std::mutex m_mutex;
    std::string m_assetRoot = "assets";
    std::unordered_map<std::string, Texture2D> m_textures;
    std::unordered_map<std::string, size_t> m_memoryUsage;

    Texture2D m_fallbackTexture{};
    bool m_initialized = false;
};

} // namespace wildbound::resources

