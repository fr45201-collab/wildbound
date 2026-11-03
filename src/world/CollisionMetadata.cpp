#include "CollisionMetadata.hpp"

namespace wildbound::world {

std::string_view CollisionBox::TypeToString(CollisionType t) {
    switch (t) {
        case CollisionType::Solid:      return "solid";
        case CollisionType::Impassable: return "impassable";
        case CollisionType::Water:      return "water";
        case CollisionType::Walkable:   return "walkable";
        case CollisionType::Trigger:    return "trigger";
        default:                        return "none";
    }
}

CollisionType CollisionBox::StringToType(std::string_view s) {
    if (s == "solid")      return CollisionType::Solid;
    if (s == "impassable") return CollisionType::Impassable;
    if (s == "water")      return CollisionType::Water;
    if (s == "walkable")   return CollisionType::Walkable;
    if (s == "trigger")    return CollisionType::Trigger;
    return CollisionType::None;
}

} // namespace wildbound::world

