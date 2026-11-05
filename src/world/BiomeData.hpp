#pragma once

#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::world {

/**
 * Definition of a terrain tile type within a biome tileset.
 */
struct TerrainType {
    std::string name;
    int gid = 0;
    bool walkable = true;
};

/**
 * Atmospheric visual and environmental parameters for a biome.
 */
struct AtmosphereConfig {
    std::string weather = "temperate";
    float fogDensity = 0.35f;
    float cloudSpeed = 15.0f;
    bool cloudLayerEnabled = true;
    Color ambientTint{ 255, 255, 255, 255 };
};

/**
 * Complete metadata and asset specification for a geographic biome in Wildbound.
 */
struct BiomeConfig {
    std::string id;
    std::string displayName;
    std::string description;
    std::string tileset;
    std::string tilesetImage;
    std::vector<TerrainType> terrain;
    std::vector<std::string> vegetation;
    std::vector<std::string> environmentObjects;
    std::vector<std::string> animations;
    AtmosphereConfig atmosphere;
    std::vector<std::string> ambientEntities;
    std::vector<std::string> maps;
    Vector2 defaultSpawn{ 320.0f, 320.0f };
};

} // namespace wildbound::world

