#pragma once

#include "GameModeDefinition.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace wildbound::gamemodes {

/**
 * Registry of available game modes in Wildbound.
 */
class GameModeRegistry {
public:
    GameModeRegistry();
    ~GameModeRegistry() = default;

    bool LoadFromDirectory(const std::string& directoryPath);
    void RegisterMode(const GameModeDefinition& def);

    [[nodiscard]] const GameModeDefinition* GetMode(const std::string& id) const;
    [[nodiscard]] bool HasMode(const std::string& id) const;
    [[nodiscard]] std::vector<GameModeDefinition> GetAllModes() const;
    [[nodiscard]] size_t GetModeCount() const noexcept { return m_modes.size(); }

    void Clear();
    void RegisterBuiltinDefaults();

private:
    std::unordered_map<std::string, GameModeDefinition> m_modes;
};

} // namespace wildbound::gamemodes

