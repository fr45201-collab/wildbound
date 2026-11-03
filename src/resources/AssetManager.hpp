#pragma once

#include "AssetRegistry.hpp"
#include "TextureManager.hpp"
#include "SpriteSheet.hpp"
#include "AnimationMetadata.hpp"
#include "FontManager.hpp"
#include "AudioManager.hpp"
#include "AssetValidator.hpp"
#include <memory>
#include <unordered_map>

namespace wildbound::resources {

/**
 * Centralized Asset Management System (AMS) Facade.
 * Coordinates registries, caches, texture streaming, spritesheets,
 * audio buffers, fonts, and validation across the engine.
 */
class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    bool Initialize(const std::string& assetRoot = "assets");
    void Shutdown();

    // Manifest operations
    bool LoadManifest(const std::string& manifestPath = "manifests/asset_manifest.json");
    ValidationReport ValidateAssets();

    // Textures & Sprites
    Texture2D GetTexture(const std::string& assetId);
    bool HasTexture(const std::string& assetId) const;

    // Spritesheets
    bool RegisterSpriteSheet(const SpriteSheet& sheet);
    const SpriteSheet* GetSpriteSheet(const std::string& sheetId) const;
    bool LoadSpriteSheetMetadata(const std::string& jsonFilePath);

    // Animations
    bool RegisterAnimation(const AnimationClip& clip);
    const AnimationClip* GetAnimation(const std::string& animName) const;
    bool LoadAnimationMetadata(const std::string& jsonFilePath);

    // Fonts & Audio
    Font GetFont(const std::string& fontId);
    Sound GetSound(const std::string& soundId);
    Music GetMusic(const std::string& musicId);

    // Subsystem accessors
    AssetRegistry& GetRegistry() { return *m_registry; }
    const AssetRegistry& GetRegistry() const { return *m_registry; }
    TextureManager& GetTextureManager() { return *m_textureManager; }
    FontManager& GetFontManager() { return *m_fontManager; }
    AudioManager& GetAudioManager() { return *m_audioManager; }

    const std::string& GetAssetRoot() const { return m_assetRoot; }

private:
    std::string ResolvePath(const std::string& relativePath) const;

    std::string m_assetRoot = "assets";
    std::unique_ptr<AssetRegistry> m_registry;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<FontManager> m_fontManager;
    std::unique_ptr<AudioManager> m_audioManager;

    std::unordered_map<std::string, SpriteSheet> m_spritesheets;
    std::unordered_map<std::string, AnimationClip> m_animations;

    bool m_initialized = false;
};

} // namespace wildbound::resources

