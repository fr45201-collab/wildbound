#pragma once

#include "CosmeticsOwnership.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace wildbound::network::http {
    class BackendClient;
}

namespace wildbound::cosmetics {

class CosmeticsOwnershipManager {
public:
    static constexpr const char* DEFAULT_SAVE_PATH = "user_data/cosmetics/cosmetic_ownership.json";

    CosmeticsOwnershipManager();
    ~CosmeticsOwnershipManager() = default;

    bool Initialize(const std::string& savePath = DEFAULT_SAVE_PATH);

    // Queries
    [[nodiscard]] bool IsOwned(const std::string& cosmeticId) const;
    [[nodiscard]] OwnershipState GetOwnershipState(const std::string& cosmeticId) const;
    [[nodiscard]] const CosmeticOwnershipRecord* GetRecord(const std::string& cosmeticId) const;
    [[nodiscard]] std::string GetSelectedVariant(const std::string& cosmeticId) const;

    // Mutators
    bool UnlockPrototype(const std::string& cosmeticId, const std::string& source = "prototype_unlock");
    bool GrantCosmetic(const std::string& cosmeticId, const std::string& source = "granted");
    bool RevokeCosmetic(const std::string& cosmeticId);
    bool SetSelectedVariant(const std::string& cosmeticId, const std::string& variantId);

    // Registration and Defaults
    void RegisterCosmetic(const std::string& cosmeticId, bool defaultOwned = false);
    void SeedDefaults();
    void ResetOwnership();

    // Batch queries / statistics
    [[nodiscard]] size_t GetTotalRecordsCount() const noexcept { return m_records.size(); }
    [[nodiscard]] size_t GetOwnedRecordsCount() const noexcept;
    [[nodiscard]] size_t GetLockedRecordsCount() const noexcept;
    [[nodiscard]] const std::unordered_map<std::string, CosmeticOwnershipRecord>& GetAllRecords() const noexcept { return m_records; }

    // Persistence
    bool Save(const std::string& customPath = "") const;
    bool Load(const std::string& customPath = "");

    // Backend Integration (Phase 45)
    void SetBackendClient(network::http::BackendClient* client) noexcept { m_backendClient = client; }
    [[nodiscard]] network::http::BackendClient* GetBackendClient() const noexcept { return m_backendClient; }
    void SyncWithBackend(std::function<void(bool success)> onComplete = nullptr);
    [[nodiscard]] bool IsSyncedWithBackend() const noexcept { return m_syncedWithBackend; }

    // Callbacks
    using OwnershipChangedCallback = std::function<void(const std::string& cosmeticId, OwnershipState state)>;
    void SetOwnershipChangedCallback(OwnershipChangedCallback callback) { m_onOwnershipChanged = std::move(callback); }

    [[nodiscard]] const std::string& GetSavePath() const noexcept { return m_savePath; }

private:
    std::unordered_map<std::string, CosmeticOwnershipRecord> m_records;
    std::string m_savePath = DEFAULT_SAVE_PATH;
    OwnershipChangedCallback m_onOwnershipChanged;
    network::http::BackendClient* m_backendClient = nullptr;
    bool m_syncedWithBackend = false;
    bool m_initialized = false;
};

} // namespace wildbound::cosmetics

