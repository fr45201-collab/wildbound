#pragma once

#include "CosmeticDefinition.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace wildbound::resources {
    class AssetRegistry;
    class AssetManager;
}

namespace wildbound::customization {

class CosmeticRegistry {
public:
    CosmeticRegistry() = default;
    ~CosmeticRegistry() = default;

    CosmeticRegistry(const CosmeticRegistry&) = default;
    CosmeticRegistry& operator=(const CosmeticRegistry&) = default;
    CosmeticRegistry(CosmeticRegistry&&) noexcept = default;
    CosmeticRegistry& operator=(CosmeticRegistry&&) noexcept = default;

    // Registration APIs
    bool RegisterCosmetic(const CosmeticDefinition& def);
    bool UnregisterCosmetic(const std::string& id);
    void Clear();

    // Query APIs
    [[nodiscard]] const CosmeticDefinition* GetCosmetic(const std::string& id) const;
    [[nodiscard]] bool HasCosmetic(const std::string& id) const;
    [[nodiscard]] std::vector<const CosmeticDefinition*> GetCosmeticsBySlot(CustomizationSlot slot) const;
    [[nodiscard]] std::vector<const CosmeticDefinition*> GetAllCosmetics() const;
    [[nodiscard]] size_t GetCount() const noexcept { return m_definitions.size(); }

    // Catalog & Builtin Loaders
    bool LoadFromCatalog(const std::string& catalogPath, const resources::AssetRegistry* assetReg = nullptr);
    void RegisterBuiltinCosmetics(const resources::AssetRegistry* assetReg = nullptr);

    // Validation
    [[nodiscard]] bool ValidateCosmetic(const std::string& id, const resources::AssetRegistry& assetReg) const;

private:
    std::unordered_map<std::string, CosmeticDefinition> m_definitions;
    std::unordered_map<CustomizationSlot, std::vector<std::string>> m_slotIndex;
};

} // namespace wildbound::customization

