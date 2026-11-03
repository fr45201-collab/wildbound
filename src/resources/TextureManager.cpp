#include "TextureManager.hpp"
#include "../core/Logger.hpp"
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

TextureManager::TextureManager() = default;

TextureManager::~TextureManager() {
    Shutdown();
}

bool TextureManager::Initialize(const std::string& assetRoot) {
    if (m_initialized) {
        return true;
    }

    m_assetRoot = assetRoot;
    CreateFallbackTexture();
    m_initialized = true;

    Logger::Info("TEXTURE_MGR", "TextureManager initialized. Asset root: " + m_assetRoot);
    return true;
}

void TextureManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("TEXTURE_MGR", "Shutting down TextureManager, releasing cached textures.");
        UnloadAll();

        if (m_fallbackTexture.id != 0) {
            UnloadTexture(m_fallbackTexture);
            m_fallbackTexture = {};
        }

        m_initialized = false;
    }
}

void TextureManager::CreateFallbackTexture() {
    if (m_fallbackTexture.id != 0) return;
    if (!IsWindowReady()) return;

    // Generate 16x16 magenta/black missing texture checkerboard
    Image checkImg = GenImageChecked(16, 16, 8, 8, MAGENTA, BLACK);
    m_fallbackTexture = LoadTextureFromImage(checkImg);
    UnloadImage(checkImg);

    SetTextureFilter(m_fallbackTexture, TEXTURE_FILTER_POINT);
}

std::string TextureManager::ResolvePath(const std::string& relativePath) const {
    if (relativePath.empty()) return "";

    if (fs::exists(relativePath)) {
        return relativePath;
    }

    const std::vector<std::string> candidates = {
        m_assetRoot + "/" + relativePath,
        "assets/" + relativePath,
        "../assets/" + relativePath,
        "../../assets/" + relativePath,
        "../../../assets/" + relativePath
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return path;
        }
    }

    return m_assetRoot + "/" + relativePath;
}

Texture2D TextureManager::LoadTextureFile(const std::string& id, const std::string& relativePath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. Check if already cached
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }

    // 2. Resolve on disk
    std::string resolvedPath = ResolvePath(relativePath);
    if (!fs::exists(resolvedPath)) {
        Logger::Error("TEXTURE_MGR", "Texture file not found for asset '" + id + "': " + resolvedPath);
        return m_fallbackTexture;
    }

    // 3. Load via raylib
    Texture2D tex = LoadTexture(resolvedPath.c_str());
    if (tex.id == 0) {
        Logger::Error("TEXTURE_MGR", "Failed decoding texture file: " + resolvedPath);
        return m_fallbackTexture;
    }

    // 4. Enforce strict nearest-neighbor point filtering for pixel-art
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    // 5. Calculate memory footprint (RGBA32 = 4 bytes/pixel)
    size_t memBytes = static_cast<size_t>(tex.width) * static_cast<size_t>(tex.height) * 4;

    m_textures[id] = tex;
    m_memoryUsage[id] = memBytes;

    Logger::Info("TEXTURE_MGR", "Loaded texture [" + id + "] (" + std::to_string(tex.width) + "x" +
                 std::to_string(tex.height) + ", " + std::to_string(memBytes / 1024) + " KB) from " + resolvedPath);

    return tex;
}

Texture2D TextureManager::GetTexture(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }
    return m_fallbackTexture;
}

bool TextureManager::HasTexture(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_textures.find(id) != m_textures.end();
}

bool TextureManager::Unload(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        UnloadTexture(it->second);
        m_textures.erase(it);
        m_memoryUsage.erase(id);
        Logger::Debug("TEXTURE_MGR", "Unloaded texture [" + id + "]");
        return true;
    }
    return false;
}

void TextureManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, tex] : m_textures) {
        if (tex.id != 0) {
            UnloadTexture(tex);
        }
    }
    m_textures.clear();
    m_memoryUsage.clear();
}

size_t TextureManager::GetLoadedTextureCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_textures.size();
}

size_t TextureManager::GetTotalMemoryUsageBytes() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& [id, bytes] : m_memoryUsage) {
        total += bytes;
    }
    return total;
}

} // namespace wildbound::resources

