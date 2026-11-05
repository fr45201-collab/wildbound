#pragma once

#include "CosmeticTypes.hpp"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace wildbound::entities {
    class ModularCharacter;
}

namespace wildbound::customization {

class CosmeticRegistry;

/**
 * High-level visual appearance state for a player character.
 * Stores selected cosmetic IDs per slot, decoupled from gameplay combat statistics.
 */
class CharacterAppearance {
public:
    CharacterAppearance();
    ~CharacterAppearance() = default;

    CharacterAppearance(const CharacterAppearance&) = default;
    CharacterAppearance& operator=(const CharacterAppearance&) = default;
    CharacterAppearance(CharacterAppearance&&) noexcept = default;
    CharacterAppearance& operator=(CharacterAppearance&&) noexcept = default;

    // Slot Accessors
    void SetCosmetic(CustomizationSlot slot, const std::string& cosmeticId);
    [[nodiscard]] std::string GetCosmetic(CustomizationSlot slot) const;
    [[nodiscard]] bool HasCosmetic(CustomizationSlot slot) const;
    void ClearSlot(CustomizationSlot slot);
    void ClearAll();

    // Base body spritesheet
    void SetBaseBody(const std::string& baseBodySheet) { m_baseBodySheet = baseBodySheet; }
    [[nodiscard]] const std::string& GetBaseBody() const { return m_baseBodySheet; }

    // Presets
    void ResetToDefault();

    // Synchronization with ModularCharacter
    void ApplyToModularCharacter(entities::ModularCharacter& modularChar, const CosmeticRegistry& registry) const;
    void SyncFromModularCharacter(const entities::ModularCharacter& modularChar);

    // Serialization
    [[nodiscard]] nlohmann::json ToJson() const;
    bool FromJson(const nlohmann::json& j);

    // Equality operator for change detection
    bool operator==(const CharacterAppearance& other) const = default;

private:
    std::unordered_map<CustomizationSlot, std::string> m_slots;
    std::string m_baseBodySheet = "sheet_skin_default";
};

} // namespace wildbound::customization

