#include "CosmeticsOwnershipManager.hpp"
#include "../core/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace wildbound::cosmetics {

namespace {

std::string GetCurrentTimestampString() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &nowTime);
#else
    localtime_r(&nowTime, &tmBuf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

const std::vector<std::string> STARTER_COSMETICS = {
    "cosmetic_skin_default",
    "cosmetic_skin_pale",
    "cosmetic_skin_tan",
    "cosmetic_skin_dark",
    "cosmetic_hair_short",
    "cosmetic_hair_medium",
    "cosmetic_hair_long",
    "cosmetic_tunic_adventurer",
    "cosmetic_pants_leather",
    "cosmetic_boots_leather"
};

} // anonymous namespace

CosmeticsOwnershipManager::CosmeticsOwnershipManager() = default;

bool CosmeticsOwnershipManager::Initialize(const std::string& savePath) {
    m_savePath = savePath.empty() ? DEFAULT_SAVE_PATH : savePath;
    m_records.clear();

    // First seed standard starter items
    SeedDefaults();

    // Check if persistence file exists
    if (std::filesystem::exists(m_savePath)) {
        if (!Load(m_savePath)) {
            core::Logger::Warn("COSMETICS_OWNERSHIP", "Failed to load cosmetic ownership from: " + m_savePath + ". Using defaults.");
        }
    } else {
        // Save initial defaults
        Save(m_savePath);
    }

    m_initialized = true;
    return true;
}

void CosmeticsOwnershipManager::SeedDefaults() {
    for (const auto& id : STARTER_COSMETICS) {
        if (m_records.find(id) == m_records.end()) {
            CosmeticOwnershipRecord record;
            record.cosmeticId = id;
            record.state = OwnershipState::Owned;
            record.acquisitionSource = "starter";
            record.acquisitionDate = GetCurrentTimestampString();
            record.selectedVariantId = "variant_default";
            m_records[id] = record;
        }
    }
}

void CosmeticsOwnershipManager::RegisterCosmetic(const std::string& cosmeticId, bool defaultOwned) {
    if (cosmeticId.empty()) return;
    if (m_records.find(cosmeticId) == m_records.end()) {
        CosmeticOwnershipRecord record;
        record.cosmeticId = cosmeticId;
        record.state = defaultOwned ? OwnershipState::Owned : OwnershipState::Locked;
        record.acquisitionSource = defaultOwned ? "starter" : "none";
        record.acquisitionDate = defaultOwned ? GetCurrentTimestampString() : "";
        record.selectedVariantId = "variant_default";
        m_records[cosmeticId] = record;
    }
}

bool CosmeticsOwnershipManager::IsOwned(const std::string& cosmeticId) const {
    if (cosmeticId.empty()) return true; // Empty slot is always valid
    auto it = m_records.find(cosmeticId);
    if (it != m_records.end()) {
        return it->second.IsOwned();
    }
    return false;
}

OwnershipState CosmeticsOwnershipManager::GetOwnershipState(const std::string& cosmeticId) const {
    if (cosmeticId.empty()) return OwnershipState::Owned;
    auto it = m_records.find(cosmeticId);
    if (it != m_records.end()) {
        return it->second.state;
    }
    return OwnershipState::Locked;
}

const CosmeticOwnershipRecord* CosmeticsOwnershipManager::GetRecord(const std::string& cosmeticId) const {
    auto it = m_records.find(cosmeticId);
    if (it != m_records.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string CosmeticsOwnershipManager::GetSelectedVariant(const std::string& cosmeticId) const {
    auto it = m_records.find(cosmeticId);
    if (it != m_records.end()) {
        return it->second.selectedVariantId;
    }
    return "variant_default";
}

bool CosmeticsOwnershipManager::UnlockPrototype(const std::string& cosmeticId, const std::string& source) {
    if (cosmeticId.empty()) return false;
    auto& record = m_records[cosmeticId];
    record.cosmeticId = cosmeticId;
    record.state = OwnershipState::Owned;
    record.acquisitionSource = source.empty() ? "prototype_unlock" : source;
    record.acquisitionDate = GetCurrentTimestampString();
    if (record.selectedVariantId.empty()) {
        record.selectedVariantId = "variant_default";
    }

    if (m_onOwnershipChanged) {
        m_onOwnershipChanged(cosmeticId, OwnershipState::Owned);
    }

    Save();
    return true;
}

bool CosmeticsOwnershipManager::GrantCosmetic(const std::string& cosmeticId, const std::string& source) {
    return UnlockPrototype(cosmeticId, source.empty() ? "granted" : source);
}

bool CosmeticsOwnershipManager::RevokeCosmetic(const std::string& cosmeticId) {
    if (cosmeticId.empty()) return false;
    auto it = m_records.find(cosmeticId);
    if (it != m_records.end()) {
        it->second.state = OwnershipState::Locked;
        it->second.acquisitionSource = "revoked";
        if (m_onOwnershipChanged) {
            m_onOwnershipChanged(cosmeticId, OwnershipState::Locked);
        }
        Save();
        return true;
    }
    return false;
}

bool CosmeticsOwnershipManager::SetSelectedVariant(const std::string& cosmeticId, const std::string& variantId) {
    if (cosmeticId.empty()) return false;
    auto& record = m_records[cosmeticId];
    record.cosmeticId = cosmeticId;
    record.selectedVariantId = variantId.empty() ? "variant_default" : variantId;
    Save();
    return true;
}

void CosmeticsOwnershipManager::ResetOwnership() {
    m_records.clear();
    SeedDefaults();
    Save();
}

size_t CosmeticsOwnershipManager::GetOwnedRecordsCount() const noexcept {
    size_t count = 0;
    for (const auto& [_, record] : m_records) {
        if (record.IsOwned()) {
            ++count;
        }
    }
    return count;
}

size_t CosmeticsOwnershipManager::GetLockedRecordsCount() const noexcept {
    size_t count = 0;
    for (const auto& [_, record] : m_records) {
        if (!record.IsOwned()) {
            ++count;
        }
    }
    return count;
}

bool CosmeticsOwnershipManager::Save(const std::string& customPath) const {
    const std::string path = customPath.empty() ? m_savePath : customPath;
    if (path.empty()) return false;

    try {
        std::filesystem::path dir = std::filesystem::path(path).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }

        nlohmann::json root;
        root["version"] = 1;
        nlohmann::json recordsArray = nlohmann::json::array();
        for (const auto& [_, record] : m_records) {
            recordsArray.push_back(record.ToJson());
        }
        root["records"] = recordsArray;

        std::ofstream file(path);
        if (!file.is_open()) {
            core::Logger::Error("COSMETICS_OWNERSHIP", "Failed to open file for writing cosmetic ownership: " + path);
            return false;
        }
        file << root.dump(4);
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("COSMETICS_OWNERSHIP", std::string("Exception saving cosmetic ownership: ") + e.what());
        return false;
    }
}

bool CosmeticsOwnershipManager::Load(const std::string& customPath) {
    const std::string path = customPath.empty() ? m_savePath : customPath;
    if (path.empty() || !std::filesystem::exists(path)) return false;

    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            core::Logger::Error("COSMETICS_OWNERSHIP", "Failed to open file for reading cosmetic ownership: " + path);
            return false;
        }

        nlohmann::json root;
        file >> root;

        if (root.contains("records") && root["records"].is_array()) {
            for (const auto& item : root["records"]) {
                CosmeticOwnershipRecord record = CosmeticOwnershipRecord::FromJson(item);
                if (!record.cosmeticId.empty()) {
                    m_records[record.cosmeticId] = record;
                }
            }
        }

        // Always re-ensure starter defaults are owned even after loading older files
        SeedDefaults();
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("COSMETICS_OWNERSHIP", std::string("Exception loading cosmetic ownership: ") + e.what());
        return false;
    }
}

} // namespace wildbound::cosmetics

