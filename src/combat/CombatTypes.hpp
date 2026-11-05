#pragma once

#include <cstdint>
#include <string>

namespace wildbound::combat {

using CombatantId = uint32_t;

enum class CombatantType {
    Player,
    Enemy,
    NPC,
    Neutral
};

enum class Faction {
    Players,
    Monsters,
    Neutral
};

enum class DamageType {
    Physical,
    Magical,
    Environmental
};

inline std::string DamageTypeToString(DamageType type) {
    switch (type) {
        case DamageType::Physical:      return "Physical";
        case DamageType::Magical:       return "Magical";
        case DamageType::Environmental: return "Environmental";
        default:                        return "Unknown";
    }
}

inline DamageType StringToDamageType(const std::string& str) {
    if (str == "magical") return DamageType::Magical;
    if (str == "environmental") return DamageType::Environmental;
    return DamageType::Physical;
}

inline std::string FactionToString(Faction faction) {
    switch (faction) {
        case Faction::Players:  return "Players";
        case Faction::Monsters: return "Monsters";
        case Faction::Neutral:  return "Neutral";
        default:                return "Unknown";
    }
}

inline Faction StringToFaction(const std::string& str) {
    if (str == "monsters" || str == "monster") return Faction::Monsters;
    if (str == "neutral") return Faction::Neutral;
    return Faction::Players;
}

} // namespace wildbound::combat

