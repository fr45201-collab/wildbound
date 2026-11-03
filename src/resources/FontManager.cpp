#include "FontManager.hpp"
#include "../core/Logger.hpp"
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

FontManager::FontManager() = default;

FontManager::~FontManager() {
    Shutdown();
}

bool FontManager::Initialize(const std::string& assetRoot) {
    if (m_initialized) return true;
    m_assetRoot = assetRoot;
    m_initialized = true;
    Logger::Info("FONT_MGR", "FontManager initialized.");
    return true;
}

void FontManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("FONT_MGR", "Shutting down FontManager, releasing loaded fonts.");
        UnloadAll();
        m_initialized = false;
    }
}

std::string FontManager::ResolvePath(const std::string& relativePath) const {
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

Font FontManager::LoadFontFile(const std::string& id, const std::string& relativePath, int fontSize) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        return it->second;
    }

    std::string resolved = ResolvePath(relativePath);
    if (!fs::exists(resolved)) {
        Logger::Error("FONT_MGR", "Font file not found for asset '" + id + "': " + resolved);
        return GetFontDefault();
    }

    Font font = LoadFontEx(resolved.c_str(), fontSize, nullptr, 0);
    if (font.texture.id == 0) {
        Logger::Error("FONT_MGR", "Failed loading font: " + resolved);
        return GetFontDefault();
    }

    SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);

    m_fonts[id] = font;
    m_fontSizes[id] = fontSize;

    Logger::Info("FONT_MGR", "Loaded font [" + id + "] size " + std::to_string(fontSize) + " from " + resolved);
    return font;
}

Font FontManager::GetFont(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        return it->second;
    }
    return GetFontDefault();
}

bool FontManager::HasFont(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fonts.find(id) != m_fonts.end();
}

bool FontManager::Unload(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        UnloadFont(it->second);
        m_fonts.erase(it);
        m_fontSizes.erase(id);
        return true;
    }
    return false;
}

void FontManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, font] : m_fonts) {
        if (font.texture.id != 0) {
            UnloadFont(font);
        }
    }
    m_fonts.clear();
    m_fontSizes.clear();
}

size_t FontManager::GetLoadedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fonts.size();
}

} // namespace wildbound::resources

