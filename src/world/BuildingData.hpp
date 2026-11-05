#pragma once

#include <raylib.h>
#include <string>

namespace wildbound::world {

/**
 * Representation of a building in the settlement, including its exterior footprint,
 * entrance trigger point, interior map link, and transition coordinates.
 */
struct Building {
    std::string id;
    std::string name;
    std::string buildingType;
    std::string district;
    Rectangle exteriorBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    Vector2 entrancePosition{ 0.0f, 0.0f };
    float entranceRadius = 32.0f;
    bool isEnterable = false;
    std::string interiorMapPath;
    Vector2 interiorSpawnPosition{ 0.0f, 0.0f };
    Vector2 interiorExitPosition{ 0.0f, 0.0f };
    Vector2 exteriorReturnPosition{ 0.0f, 0.0f };
    std::string description;

    [[nodiscard]] bool IsInEntranceRange(const Vector2& pos) const {
        float dx = pos.x - entrancePosition.x;
        float dy = pos.y - entrancePosition.y;
        return (dx * dx + dy * dy) <= (entranceRadius * entranceRadius);
    }

    [[nodiscard]] bool IsInExitRange(const Vector2& pos) const {
        float dx = pos.x - interiorExitPosition.x;
        float dy = pos.y - interiorExitPosition.y;
        return (dx * dx + dy * dy) <= (entranceRadius * entranceRadius);
    }
};

} // namespace wildbound::world

