#include "GameModeRules.hpp"

namespace wildbound::gamemodes {

GameModeRules::GameModeRules(const GameModeDefinition& def) {
    SetFromDefinition(def);
}

void GameModeRules::SetFromDefinition(const GameModeDefinition& def) {
    m_teamsEnabled = def.teamsEnabled;
    m_teamSize = def.teamSize;
    m_friendlyFireEnabled = def.friendlyFireEnabled;
    m_botsAllowed = def.botsAllowed;
    m_respawnEnabled = def.respawnEnabled;
    m_stormEnabled = def.stormEnabled;
    m_competitiveResults = def.competitiveResults;
    m_victoryCondition = def.victoryCondition;
}

} // namespace wildbound::gamemodes

