#include "AssetManager.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <filesystem>
#include <vector>

using wildbound::core::Logger;
namespace fs = std::filesystem;

namespace wildbound::resources {

AssetManager::AssetManager()
    : m_registry(std::make_unique<AssetRegistry>())
    , m_textureManager(std::make_unique<TextureManager>())
    , m_fontManager(std::make_unique<FontManager>())
    , m_audioManager(std::make_unique<AudioManager>())
{
}

AssetManager::~AssetManager() {
    Shutdown();
}

std::string AssetManager::ResolvePath(const std::string& relativePath) const {
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

bool AssetManager::Initialize(const std::string& assetRoot) {
    if (m_initialized) {
        return true;
    }

    m_assetRoot = assetRoot;
    Logger::Info("ASSET_MGR", "Initializing Asset Management System with root: " + m_assetRoot);

    m_textureManager->Initialize(m_assetRoot);
    m_fontManager->Initialize(m_assetRoot);
    m_audioManager->Initialize(m_assetRoot);

    // Auto-discover and load master manifest if present
    std::string defaultManifest = ResolvePath("manifests/asset_manifest.json");
    if (fs::exists(defaultManifest)) {
        LoadManifest(defaultManifest);
    }

    m_initialized = true;
    Logger::Info("ASSET_MGR", "Asset Management System initialized successfully.");
    return true;
}

void AssetManager::Shutdown() {
    if (m_initialized) {
        Logger::Info("ASSET_MGR", "Shutting down Asset Management System.");

        m_spritesheets.clear();
        m_animations.clear();

        if (m_textureManager) m_textureManager->Shutdown();
        if (m_fontManager) m_fontManager->Shutdown();
        if (m_audioManager) m_audioManager->Shutdown();
        if (m_registry) m_registry->Clear();

        m_initialized = false;
    }
}

bool AssetManager::LoadManifest(const std::string& manifestPath) {
    std::string resolved = ResolvePath(manifestPath);
    bool ok = m_registry->LoadManifest(resolved);
    if (ok) {
        Logger::Info("ASSET_MGR", "Manifest loaded: " + resolved);
    }
    return ok;
}

ValidationReport AssetManager::ValidateAssets() {
    return AssetValidator::ValidateAll(*m_registry, m_assetRoot);
}

Texture2D AssetManager::GetTexture(const std::string& assetId) {
    // 1. Check if already loaded in TextureManager
    if (m_textureManager->HasTexture(assetId)) {
        return m_textureManager->GetTexture(assetId);
    }

    // 2. Query registry metadata
    const AssetMetadata* meta = m_registry->GetMetadata(assetId);
    if (meta != nullptr) {
        return m_textureManager->LoadTextureFile(assetId, meta->file);
    }

    // 3. Fallback: attempt relative path directly
    return m_textureManager->LoadTextureFile(assetId, assetId);
}

bool AssetManager::HasTexture(const std::string& assetId) const {
    if (m_textureManager->HasTexture(assetId)) return true;
    const AssetMetadata* meta = m_registry->GetMetadata(assetId);
    return (meta != nullptr && meta->type == AssetType::Texture);
}

bool AssetManager::RegisterSpriteSheet(const SpriteSheet& sheet) {
    if (sheet.GetId().empty()) return false;
    m_spritesheets[sheet.GetId()] = sheet;
    Logger::Debug("ASSET_MGR", "Registered spritesheet: [" + sheet.GetId() + "]");
    return true;
}

const SpriteSheet* AssetManager::GetSpriteSheet(const std::string& sheetId) const {
    auto it = m_spritesheets.find(sheetId);
    if (it != m_spritesheets.end()) {
        return &it->second;
    }
    return nullptr;
}

bool AssetManager::LoadSpriteSheetMetadata(const std::string& jsonFilePath) {
    std::string resolved = ResolvePath(jsonFilePath);
    if (!fs::exists(resolved)) {
        Logger::Error("ASSET_MGR", "Spritesheet metadata file not found: " + resolved);
        return false;
    }

    try {
        std::ifstream file(resolved);
        nlohmann::json j;
        file >> j;
        SpriteSheet sheet = SpriteSheet::FromJson(j);
        return RegisterSpriteSheet(sheet);
    }
    catch (const std::exception& ex) {
        Logger::Error("ASSET_MGR", "Failed parsing spritesheet JSON '" + resolved + "': " + ex.what());
        return false;
    }
}

bool AssetManager::RegisterAnimation(const AnimationClip& clip) {
    if (!clip.IsValid()) return false;
    m_animations[clip.name] = clip;
    Logger::Debug("ASSET_MGR", "Registered animation: [" + clip.name + "]");
    return true;
}

const AnimationClip* AssetManager::GetAnimation(const std::string& animName) const {
    auto it = m_animations.find(animName);
    if (it != m_animations.end()) {
        return &it->second;
    }
    return nullptr;
}

bool AssetManager::LoadAnimationMetadata(const std::string& jsonFilePath) {
    std::string resolved = ResolvePath(jsonFilePath);
    if (!fs::exists(resolved)) {
        Logger::Error("ASSET_MGR", "Animation metadata file not found: " + resolved);
        return false;
    }

    try {
        std::ifstream file(resolved);
        nlohmann::json j;
        file >> j;
        AnimationClip clip = AnimationClip::FromJson(j);
        return RegisterAnimation(clip);
    }
    catch (const std::exception& ex) {
        Logger::Error("ASSET_MGR", "Failed parsing animation JSON '" + resolved + "': " + ex.what());
        return false;
    }
}

Font AssetManager::GetFont(const std::string& fontId) {
    if (m_fontManager->HasFont(fontId)) {
        return m_fontManager->GetFont(fontId);
    }

    const AssetMetadata* meta = m_registry->GetMetadata(fontId);
    if (meta != nullptr) {
        return m_fontManager->LoadFontFile(fontId, meta->file, 16);
    }

    return m_fontManager->GetFont(fontId);
}

Sound AssetManager::GetSound(const std::string& soundId) {
    if (m_audioManager->HasSound(soundId)) {
        return m_audioManager->GetSound(soundId);
    }

    const AssetMetadata* meta = m_registry->GetMetadata(soundId);
    if (meta != nullptr) {
        m_audioManager->LoadSoundFile(soundId, meta->file);
    }
    return m_audioManager->GetSound(soundId);
}

Music AssetManager::GetMusic(const std::string& musicId) {
    if (m_audioManager->HasMusic(musicId)) {
        return m_audioManager->GetMusic(musicId);
    }

    const AssetMetadata* meta = m_registry->GetMetadata(musicId);
    if (meta != nullptr) {
        m_audioManager->LoadMusicFile(musicId, meta->file);
    }
    return m_audioManager->GetMusic(musicId);
}

} // namespace wildbound::resources

