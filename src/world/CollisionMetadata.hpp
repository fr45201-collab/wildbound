#pragma once

#include <raylib.h>
#include <string>
#include <string_view>

namespace wildbound::world {

enum class CollisionType {
    None = 0,
    Solid,       // Blocks entity movement (trees, rocks, fences, buildings)
    Impassable,  // Impassable deep terrain
    Water,       // Water surfaces (requires swimming or boat)
    Walkable,    // Pathway, bridge, open ground
    Trigger      // Region trigger
};

struct CollisionBox {
    Rectangle bounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    CollisionType type = CollisionType::Solid;
    std::string tag;
    bool isTrigger = false;

    bool IsSolid() const {
        return type == CollisionType::Solid || type == CollisionType::Impassable || type == CollisionType::Water;
    }

    bool Intersects(const Rectangle& other) const {
        return CheckCollisionRecs(bounds, other);
    }

    bool Contains(Vector2 point) const {
        return CheckCollisionPointRec(point, bounds);
    }

    bool ContainsPoint(Vector2 point) const {
        return Contains(point);
    }

    static std::string_view TypeToString(CollisionType t);
    static CollisionType StringToType(std::string_view s);
};

} // namespace wildbound::world

