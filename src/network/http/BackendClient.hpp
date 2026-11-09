#pragma once

#include "HttpClient.hpp"
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

namespace wildbound::network::http {

struct AuthSession {
    std::string userId;
    std::string username;
    std::string email;
    std::string displayName;
    std::string accessToken;
    std::string refreshToken;

    [[nodiscard]] bool IsLoggedIn() const noexcept {
        return !userId.empty() && !accessToken.empty();
    }
};

class BackendClient {
public:
    explicit BackendClient(std::string baseUrl = "http://127.0.0.1:8080");
    ~BackendClient();

    void Update();
    void Shutdown() { m_httpClient.Shutdown(); }

    [[nodiscard]] const AuthSession& GetSession() const noexcept { return m_session; }
    [[nodiscard]] bool IsLoggedIn() const noexcept { return m_session.IsLoggedIn(); }
    [[nodiscard]] bool IsAuthenticated() const noexcept { return m_session.IsLoggedIn(); }
    void SetSession(const AuthSession& session) { m_session = session; }
    void ClearSession() { m_session = {}; }

    // Authentication
    void Register(
        const std::string& username,
        const std::string& email,
        const std::string& password,
        const std::string& displayName,
        std::function<void(bool success, const std::string& errorOrMessage)> callback
    );

    void Login(
        const std::string& emailOrUsername,
        const std::string& password,
        std::function<void(bool success, const std::string& errorOrMessage)> callback
    );

    void RefreshTokens(
        std::function<void(bool success, const std::string& errorOrMessage)> callback
    );

    void Logout(
        std::function<void(bool success, const std::string& errorOrMessage)> callback
    );

    void GetProfile(
        std::function<void(bool success, const nlohmann::json& profileData, const std::string& error)> callback
    );

    void UpdateProfile(
        const nlohmann::json& updatePayload,
        std::function<void(bool success, const std::string& error)> callback
    );

    void ListCosmetics(
        const std::string& category,
        std::function<void(bool success, const nlohmann::json& items, const std::string& error)> callback
    );

    void ListMyCosmetics(
        std::function<void(bool success, const nlohmann::json& items, const std::string& error)> callback
    );

    void EquipCosmetic(
        const std::string& cosmeticId,
        std::function<void(bool success, const std::string& errorOrMessage)> callback
    );

    // Phase 45: Cosmetic Catalogue & Loadouts
    void GetCatalogue(
        const std::string& category,
        const std::string& slot,
        const std::string& rarity,
        const std::string& search,
        int limit,
        int offset,
        std::function<void(bool success, const nlohmann::json& catalog, const std::string& error)> callback
    );

    void GetMyLoadout(
        std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
    );

    void EquipLoadoutItem(
        const std::string& cosmeticId,
        int expectedVersion,
        std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
    );

    void UnequipLoadoutSlot(
        const std::string& slot,
        int expectedVersion,
        std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
    );

    void UpdateFullLoadout(
        const std::vector<std::string>& cosmetics,
        int expectedVersion,
        std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
    );

    void GetMyStatistics(
        std::function<void(bool success, const nlohmann::json& stats, const std::string& error)> callback
    );

    void GetPlayerStatistics(
        const std::string& playerId,
        std::function<void(bool success, const nlohmann::json& stats, const std::string& error)> callback
    );

    void GetMyMatches(
        int page,
        int limit,
        std::function<void(bool success, const nlohmann::json& matches, const std::string& error)> callback
    );

    // Dedicated Server Ingestion
    void SubmitMatchResult(
        const std::string& serverToken,
        const nlohmann::json& matchPayload,
        std::function<void(bool success, bool created, const std::string& error)> callback
    );

    // Phase 44: Account Settings
    void GetSettings(
        std::function<void(bool success, const nlohmann::json& settings, const std::string& error)> callback
    );

    void UpdateSettings(
        const nlohmann::json& updatePayload,
        std::function<void(bool success, const std::string& error)> callback
    );

    // Phase 44: Account Progression
    void GetProgression(
        std::function<void(bool success, const nlohmann::json& progression, const std::string& error)> callback
    );

    void GetProgressionEvents(
        int limit,
        int offset,
        std::function<void(bool success, const nlohmann::json& events, const std::string& error)> callback
    );

    // Phase 44: Achievements
    void ListAchievements(
        std::function<void(bool success, const nlohmann::json& achievements, const std::string& error)> callback
    );

    void ListMyAchievements(
        std::function<void(bool success, const nlohmann::json& achievements, const std::string& error)> callback
    );

    // Phase 44: Sessions & Account Management
    void ListSessions(
        std::function<void(bool success, const nlohmann::json& sessions, const std::string& error)> callback
    );

    void RevokeSession(
        const std::string& sessionId,
        std::function<void(bool success, const std::string& error)> callback
    );

    void LogoutAll(
        std::function<void(bool success, const std::string& error)> callback
    );

    void ChangePassword(
        const std::string& currentPassword,
        const std::string& newPassword,
        std::function<void(bool success, const std::string& error)> callback
    );

    // Phase 44: Client Session Persistence
    bool SaveSessionToFile(const std::string& filePath = "user_data/auth_session.json");
    bool LoadSessionFromFile(const std::string& filePath = "user_data/auth_session.json");

private:
    std::string m_baseUrl;
    HttpClient m_httpClient;
    AuthSession m_session;

    [[nodiscard]] std::vector<HttpHeader> MakeAuthHeaders(bool jsonContent = true) const;
};

} // namespace wildbound::network::http

