#pragma once

#include "BiomeData.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::entities {
    class Player;
}

namespace wildbound::world {

class World;

namespace environment {
    class EnvironmentalAnimationSystem;
}

/**
 * Manages biome definitions, active biome state, biome transitions,
 * and coordinates map loading, player positioning, camera framing, and atmospheric settings.
 */
class BiomeManager {
public:
    BiomeManager();
    ~BiomeManager();

    bool Initialize(const std::string& biomesDirectory = "data/biomes");

    // Biome queries
    [[nodiscard]] const std::string& GetActiveBiomeId() const { return m_activeBiomeId; }
    [[nodiscard]] const BiomeConfig* GetActiveBiome() const;
    [[nodiscard]] const BiomeConfig* GetBiome(const std::string& id) const;
    [[nodiscard]] const std::vector<BiomeConfig>& GetAllBiomes() const { return m_biomes; }
    [[nodiscard]] size_t GetBiomeCount() const { return m_biomes.size(); }
    [[nodiscard]] bool HasBiome(const std::string& id) const;

    // Biome transition & map loading
    bool SwitchBiome(const std::string& biomeId,
                     World& world,
                     entities::Player& player,
                     graphics::Camera2DSystem& camera,
                     resources::AssetManager& assetMgr,
                     environment::EnvironmentalAnimationSystem* envAnim = nullptr,
                     size_t mapIndex = 0);

    // Direct helper to set active biome ID without reloading map
    void SetActiveBiomeId(const std::string& id) { m_activeBiomeId = id; }

private:
    std::vector<BiomeConfig> m_biomes;
    std::unordered_map<std::string, size_t> m_biomeIndexMap;
    std::string m_activeBiomeId = "forest";
};

} // namespace wildbound::world

