#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace wildbound::cosmetics {

enum class OwnershipState : uint8_t {
    Locked = 0,
    Owned = 1
};

inline std::string_view OwnershipStateToString(OwnershipState s) noexcept {
    switch (s) {
        case OwnershipState::Owned:  return "owned";
        case OwnershipState::Locked: return "locked";
        default:                     return "locked";
    }
}

inline OwnershipState StringToOwnershipState(std::string_view s) noexcept {
    if (s == "owned" || s == "Owned") return OwnershipState::Owned;
    return OwnershipState::Locked;
}

struct CosmeticOwnershipRecord {
    std::string cosmeticId;
    OwnershipState state = OwnershipState::Locked;
    std::string acquisitionSource = "none"; // "starter", "prototype_unlock", "dev_grant", etc.
    std::string acquisitionDate = "";       // Timestamp string e.g. "2026-11-05T12:00:00Z"
    std::string selectedVariantId = "variant_default";

    [[nodiscard]] bool IsOwned() const noexcept {
        return state == OwnershipState::Owned;
    }

    nlohmann::json ToJson() const {
        return nlohmann::json{
            { "cosmeticId", cosmeticId },
            { "state", std::string(OwnershipStateToString(state)) },
            { "acquisitionSource", acquisitionSource },
            { "acquisitionDate", acquisitionDate },
            { "selectedVariantId", selectedVariantId }
        };
    }

    static CosmeticOwnershipRecord FromJson(const nlohmann::json& j) {
        CosmeticOwnershipRecord record;
        if (j.contains("cosmeticId") && j["cosmeticId"].is_string()) {
            record.cosmeticId = j["cosmeticId"].get<std::string>();
        }
        if (j.contains("state") && j["state"].is_string()) {
            record.state = StringToOwnershipState(j["state"].get<std::string>());
        }
        if (j.contains("acquisitionSource") && j["acquisitionSource"].is_string()) {
            record.acquisitionSource = j["acquisitionSource"].get<std::string>();
        }
        if (j.contains("acquisitionDate") && j["acquisitionDate"].is_string()) {
            record.acquisitionDate = j["acquisitionDate"].get<std::string>();
        }
        if (j.contains("selectedVariantId") && j["selectedVariantId"].is_string()) {
            record.selectedVariantId = j["selectedVariantId"].get<std::string>();
        }
        return record;
    }
};

} // namespace wildbound::cosmetics

