#pragma once

#include <raylib.h>
#include <string>
#include <vector>

namespace wildbound::world {

struct BiomeRegion {
    std::string biomeId;
    std::string name;
    Rectangle bounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    std::string tileset;
    std::string audioProfile;
    std::string settlementId;
};

struct SettlementDef {
    std::string id;
    std::string name;
    std::string biome;
    Vector2 center{ 0.0f, 0.0f };
    float radius = 400.0f;
};

struct PointOfInterest {
    std::string id;
    std::string name;
    Vector2 position{ 0.0f, 0.0f };
    std::string type;
    std::string biome;
};

struct SpawnRegionDef {
    std::string regionId;
    Vector2 center{ 0.0f, 0.0f };
    std::string biome;
    std::vector<Vector2> spawnPoints;
};

struct LootLocationDef {
    std::string id;
    Vector2 position{ 0.0f, 0.0f };
    std::string containerType;
    std::string parentPoi;
};

struct TraversalRoute {
    std::string id;
    std::string name;
    std::string startPoi;
    std::string endPoi;
    float distanceUnits = 0.0f;
    std::string type;
};

struct Chokepoint {
    std::string id;
    std::string name;
    Vector2 position{ 0.0f, 0.0f };
    float widthUnits = 64.0f;
    std::string type;
};

struct WorldLandmark {
    std::string id;
    std::string name;
    std::string type;
    std::string biome;
    Vector2 position{ 0.0f, 0.0f };
};

struct BattleRoyaleWorldDef {
    std::string worldId = "battle_royale_island";
    std::string name = "Isle of Wildbound";
    std::string description;
    std::string mapPath = "maps/world/battle_royale_island.json";
    int widthTiles = 320;
    int heightTiles = 320;
    float worldWidth = 10240.0f;
    float worldHeight = 10240.0f;
    int tileSize = 32;
    Rectangle playableBounds{ 960.0f, 960.0f, 8320.0f, 8320.0f };
    Vector2 defaultSpawn{ 4160.0f, 5920.0f };
    std::vector<std::string> biomes;
    int settlementsCount = 5;
    int poiCount = 14;
    int spawnRegionsCount = 12;
    int lootMarkersCount = 45;
};

} // namespace wildbound::world

