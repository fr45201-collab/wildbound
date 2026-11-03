#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <mutex>

namespace wildbound::resources {

/**
 * Thread-safe Font resource cache and lifecycle manager.
 */
class FontManager {
public:
    FontManager();
    ~FontManager();

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    bool Initialize(const std::string& assetRoot = "assets");
    void Shutdown();

    Font LoadFontFile(const std::string& id, const std::string& relativePath, int fontSize = 16);
    Font GetFont(const std::string& id) const;
    bool HasFont(const std::string& id) const;

    bool Unload(const std::string& id);
    void UnloadAll();

    size_t GetLoadedCount() const;

private:
    std::string ResolvePath(const std::string& relativePath) const;

    mutable std::mutex m_mutex;
    std::string m_assetRoot = "assets";
    std::unordered_map<std::string, Font> m_fonts;
    std::unordered_map<std::string, int> m_fontSizes;
    bool m_initialized = false;
};

} // namespace wildbound::resources

