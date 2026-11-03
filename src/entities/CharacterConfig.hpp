#pragma once

#include "CharacterStats.hpp"
#include <raylib.h>
#include <string>
#include <unordered_map>

namespace wildbound::entities {

struct CharacterCollisionConfig {
    bool enabled = true;
    float width = 16.0f;
    float height = 12.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

class CharacterConfig {
public:
    std::string id = "player";
    std::string name = "Explorer";
    std::string spritesheet = "player_spritesheet";
    CharacterStats stats;
    CharacterCollisionConfig collision;
    std::unordered_map<std::string, std::string> animationClips;

    static CharacterConfig LoadFromFile(const std::string& jsonPath);
    static CharacterConfig CreateDefault();
};

} // namespace wildbound::entities

