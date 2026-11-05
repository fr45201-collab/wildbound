#pragma once

#include "CombatTypes.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

namespace wildbound::combat {

class CombatRules {
public:
    static CombatRules CreateDefault() {
        CombatRules r;
        r.friendlyFire = false;
        r.allowPvE = true;
        r.allowPvP = false;
        return r;
    }

    static CombatRules LoadFromFile(const std::string& filepath) {
        CombatRules rules = CreateDefault();
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) return rules;

            nlohmann::json j;
            file >> j;

            if (j.contains("rules")) {
                const auto& r = j["rules"];
                rules.friendlyFire = r.value("friendly_fire", false);
                rules.allowPvE = r.value("allow_pve", true);
                rules.allowPvP = r.value("allow_pvp", false);
            }
        } catch (...) {
            // Safe fallback to defaults
        }
        return rules;
    }

    bool CanTarget(Faction attackerFaction, Faction targetFaction) const {
        // 1. Player vs Player (PvP)
        if (attackerFaction == Faction::Players && targetFaction == Faction::Players) {
            return allowPvP || friendlyFire;
        }

        // 2. Player vs Monster / Monster vs Player (PvE)
        if ((attackerFaction == Faction::Players && targetFaction == Faction::Monsters) ||
            (attackerFaction == Faction::Monsters && targetFaction == Faction::Players)) {
            return allowPvE;
        }

        // 3. Same-faction non-player (e.g., Monster vs Monster friendly fire)
        if (attackerFaction == targetFaction) {
            return friendlyFire;
        }

        return false;
    }

    bool friendlyFire = false;
    bool allowPvE = true;
    bool allowPvP = false;
};

} // namespace wildbound::combat

