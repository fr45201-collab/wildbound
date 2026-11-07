#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace wildbound::gamemodes {

enum class BotDifficulty {
    Easy,
    Normal,
    Hard
};

[[nodiscard]] const char* BotDifficultyToString(BotDifficulty diff) noexcept;
[[nodiscard]] BotDifficulty StringToBotDifficulty(const std::string& str) noexcept;

/**
 * Concrete behavioral tuning parameters tied to bot difficulty.
 */
struct BotDifficultyParams {
    float perceptionRange = 280.0f;
    float reactionDelay = 0.35f;
    float attackCooldownMultiplier = 1.0f;
    float movementSpeedMultiplier = 1.0f;
    float stormEvacuateThreshold = 20.0f;
    float retreatHealthPercentage = 0.25f;
    float aggressionWeight = 0.65f;
};

/**
 * Configuration container for offline bot match setup and AI difficulty profiles.
 */
class BotMatchConfiguration {
public:
    BotMatchConfiguration();

    bool LoadDifficultyProfiles(const std::string& path);
    [[nodiscard]] BotDifficultyParams GetParams(BotDifficulty diff) const;

    void SetBotCount(uint32_t count) noexcept { m_botCount = count; }
    [[nodiscard]] uint32_t GetBotCount() const noexcept { return m_botCount; }

    void SetDifficulty(BotDifficulty diff) noexcept { m_difficulty = diff; }
    [[nodiscard]] BotDifficulty GetDifficulty() const noexcept { return m_difficulty; }

    [[nodiscard]] BotDifficultyParams GetActiveParams() const { return GetParams(m_difficulty); }

private:
    uint32_t m_botCount = 15;
    BotDifficulty m_difficulty = BotDifficulty::Normal;
    std::unordered_map<std::string, BotDifficultyParams> m_profiles;

    void InitializeDefaults();
};

} // namespace wildbound::gamemodes

