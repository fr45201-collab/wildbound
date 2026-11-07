#include "BackendClient.hpp"
#include <filesystem>
#include <fstream>

namespace wildbound::network::http {

BackendClient::BackendClient(std::string baseUrl)
    : m_baseUrl(std::move(baseUrl)) {
}

BackendClient::~BackendClient() = default;

void BackendClient::Update() {
    m_httpClient.Update();
}

std::vector<HttpHeader> BackendClient::MakeAuthHeaders(bool jsonContent) const {
    std::vector<HttpHeader> headers;
    if (jsonContent) {
        headers.emplace_back("Content-Type", "application/json");
    }
    if (!m_session.accessToken.empty()) {
        headers.emplace_back("Authorization", "Bearer " + m_session.accessToken);
    }
    return headers;
}

void BackendClient::Register(
    const std::string& username,
    const std::string& email,
    const std::string& password,
    const std::string& displayName,
    std::function<void(bool success, const std::string& errorOrMessage)> callback
) {
    nlohmann::json payload;
    payload["username"] = username;
    payload["email"] = email;
    payload["password"] = password;
    payload["display_name"] = displayName;

    std::vector<HttpHeader> headers{ {"Content-Type", "application/json"} };
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/auth/register", payload.dump(), headers,
        [this, callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                if (j.value("success", false)) {
                    auto data = j["data"];
                    m_session.userId = data["user"].value("id", "");
                    m_session.username = data["user"].value("username", "");
                    m_session.email = data["user"].value("email", "");
                    m_session.displayName = data["user"].value("display_name", "");
                    m_session.accessToken = data["tokens"].value("access_token", "");
                    m_session.refreshToken = data["tokens"].value("refresh_token", "");
                    callback(true, "Registration successful");
                } else {
                    callback(false, j["error"].value("message", "Unknown error"));
                }
            } catch (...) {
                callback(false, "Failed to parse register response");
            }
        });
}

void BackendClient::Login(
    const std::string& emailOrUsername,
    const std::string& password,
    std::function<void(bool success, const std::string& errorOrMessage)> callback
) {
    nlohmann::json payload;
    payload["email_or_username"] = emailOrUsername;
    payload["password"] = password;

    std::vector<HttpHeader> headers{ {"Content-Type", "application/json"} };
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/auth/login", payload.dump(), headers,
        [this, callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                if (j.value("success", false)) {
                    auto data = j["data"];
                    m_session.userId = data["user"].value("id", "");
                    m_session.username = data["user"].value("username", "");
                    m_session.email = data["user"].value("email", "");
                    m_session.displayName = data["user"].value("display_name", "");
                    m_session.accessToken = data["tokens"].value("access_token", "");
                    m_session.refreshToken = data["tokens"].value("refresh_token", "");
                    callback(true, "Login successful");
                } else {
                    callback(false, j["error"].value("message", "Authentication failed"));
                }
            } catch (...) {
                callback(false, "Failed to parse login response");
            }
        });
}

void BackendClient::RefreshTokens(
    std::function<void(bool success, const std::string& errorOrMessage)> callback
) {
    if (m_session.refreshToken.empty()) {
        callback(false, "No active refresh token");
        return;
    }

    nlohmann::json payload;
    payload["refresh_token"] = m_session.refreshToken;

    std::vector<HttpHeader> headers{ {"Content-Type", "application/json"} };
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/auth/refresh", payload.dump(), headers,
        [this, callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                if (j.value("success", false)) {
                    auto data = j["data"];
                    m_session.accessToken = data.value("access_token", "");
                    m_session.refreshToken = data.value("refresh_token", "");
                    callback(true, "Token refreshed");
                } else {
                    callback(false, j["error"].value("message", "Refresh failed"));
                }
            } catch (...) {
                callback(false, "Failed to parse refresh response");
            }
        });
}

void BackendClient::Logout(
    std::function<void(bool success, const std::string& errorOrMessage)> callback
) {
    nlohmann::json payload;
    payload["refresh_token"] = m_session.refreshToken;

    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/auth/logout", payload.dump(), headers,
        [this, callback](const HttpResponse& resp) {
            ClearSession();
            if (resp.IsOk()) {
                callback(true, "Logged out");
            } else {
                callback(false, resp.errorMessage);
            }
        });
}

void BackendClient::GetProfile(
    std::function<void(bool success, const nlohmann::json& profileData, const std::string& error)> callback
) {
    auto headers = MakeAuthHeaders(false);
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me", "", headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                if (j.value("success", false)) {
                    callback(true, j["data"], "");
                } else {
                    callback(false, {}, j["error"].value("message", "Profile lookup failed"));
                }
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::UpdateProfile(
    const nlohmann::json& updatePayload,
    std::function<void(bool success, const std::string& error)> callback
) {
    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("PATCH", m_baseUrl + "/api/v1/players/me", updatePayload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "");
        });
}

void BackendClient::ListCosmetics(
    const std::string& category,
    std::function<void(bool success, const nlohmann::json& items, const std::string& error)> callback
) {
    std::string url = m_baseUrl + "/api/v1/cosmetics";
    if (!category.empty()) {
        url += "?category=" + category;
    }

    m_httpClient.SendRequestAsync("GET", url, "", {},
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::array()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::ListMyCosmetics(
    std::function<void(bool success, const nlohmann::json& items, const std::string& error)> callback
) {
    auto headers = MakeAuthHeaders(false);
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/cosmetics", "", headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::array()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::EquipCosmetic(
    const std::string& cosmeticId,
    std::function<void(bool success, const std::string& errorOrMessage)> callback
) {
    nlohmann::json payload;
    payload["cosmetic_id"] = cosmeticId;

    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/players/me/cosmetics/equip", payload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "Cosmetic equipped");
        });
}

void BackendClient::GetCatalogue(
    const std::string& category,
    const std::string& slot,
    const std::string& rarity,
    const std::string& search,
    int limit,
    int offset,
    std::function<void(bool success, const nlohmann::json& catalog, const std::string& error)> callback
) {
    std::string url = m_baseUrl + "/api/v1/cosmetics?";
    if (!category.empty()) url += "category=" + category + "&";
    if (!slot.empty()) url += "slot=" + slot + "&";
    if (!rarity.empty()) url += "rarity=" + rarity + "&";
    if (!search.empty()) url += "search=" + search + "&";
    if (limit > 0) url += "limit=" + std::to_string(limit) + "&";
    if (offset >= 0) url += "offset=" + std::to_string(offset) + "&";

    m_httpClient.SendRequestAsync("GET", url, "", {},
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::GetMyLoadout(
    std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
) {
    auto headers = MakeAuthHeaders(false);
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/loadout", "", headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::EquipLoadoutItem(
    const std::string& cosmeticId,
    int expectedVersion,
    std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
) {
    nlohmann::json payload;
    payload["cosmetic_id"] = cosmeticId;
    if (expectedVersion > 0) {
        payload["expected_version"] = expectedVersion;
    }

    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/players/me/loadout/equip", payload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::UnequipLoadoutSlot(
    const std::string& slot,
    int expectedVersion,
    std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
) {
    nlohmann::json payload;
    payload["slot"] = slot;
    if (expectedVersion > 0) {
        payload["expected_version"] = expectedVersion;
    }

    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/players/me/loadout/unequip", payload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::UpdateFullLoadout(
    const std::vector<std::string>& cosmetics,
    int expectedVersion,
    std::function<void(bool success, const nlohmann::json& loadout, const std::string& error)> callback
) {
    nlohmann::json payload;
    payload["cosmetics"] = cosmetics;
    if (expectedVersion > 0) {
        payload["expected_version"] = expectedVersion;
    }

    auto headers = MakeAuthHeaders(true);
    m_httpClient.SendRequestAsync("PUT", m_baseUrl + "/api/v1/players/me/loadout", payload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::GetMyStatistics(
    std::function<void(bool success, const nlohmann::json& stats, const std::string& error)> callback
) {
    auto headers = MakeAuthHeaders(false);
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/statistics", "", headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::GetPlayerStatistics(
    const std::string& playerId,
    std::function<void(bool success, const nlohmann::json& stats, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/" + playerId + "/statistics", "", {},
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::GetMyMatches(
    int page,
    int limit,
    std::function<void(bool success, const nlohmann::json& matches, const std::string& error)> callback
) {
    std::string url = m_baseUrl + "/api/v1/players/me/matches?page=" + std::to_string(page) + "&limit=" + std::to_string(limit);
    auto headers = MakeAuthHeaders(false);
    m_httpClient.SendRequestAsync("GET", url, "", headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::SubmitMatchResult(
    const std::string& serverToken,
    const nlohmann::json& matchPayload,
    std::function<void(bool success, bool created, const std::string& error)> callback
) {
    std::vector<HttpHeader> headers{
        {"Content-Type", "application/json"},
        {"X-Server-Token", serverToken}
    };

    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/internal/matches/results", matchPayload.dump(), headers,
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                bool success = j.value("success", false);
                bool created = j["data"].value("created", false);
                callback(success, created, "");
            } catch (...) {
                callback(false, false, "JSON parse error");
            }
        });
}

// ----------------------------------------------------------------------------
// Phase 44: Account Settings
// ----------------------------------------------------------------------------

void BackendClient::GetSettings(
    std::function<void(bool success, const nlohmann::json& settings, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/settings", "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::UpdateSettings(
    const nlohmann::json& updatePayload,
    std::function<void(bool success, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("PATCH", m_baseUrl + "/api/v1/players/me/settings", updatePayload.dump(), MakeAuthHeaders(true),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "");
        });
}

// ----------------------------------------------------------------------------
// Phase 44: Account Progression
// ----------------------------------------------------------------------------

void BackendClient::GetProgression(
    std::function<void(bool success, const nlohmann::json& progression, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/progression", "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::GetProgressionEvents(
    int limit,
    int offset,
    std::function<void(bool success, const nlohmann::json& events, const std::string& error)> callback
) {
    std::string url = m_baseUrl + "/api/v1/players/me/progression/events?limit=" + std::to_string(limit) +
                      "&offset=" + std::to_string(offset);
    m_httpClient.SendRequestAsync("GET", url, "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

// ----------------------------------------------------------------------------
// Phase 44: Achievements
// ----------------------------------------------------------------------------

void BackendClient::ListAchievements(
    std::function<void(bool success, const nlohmann::json& achievements, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/achievements", "", {{"Accept", "application/json"}},
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::ListMyAchievements(
    std::function<void(bool success, const nlohmann::json& achievements, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/players/me/achievements", "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

// ----------------------------------------------------------------------------
// Phase 44: Sessions & Account Management
// ----------------------------------------------------------------------------

void BackendClient::ListSessions(
    std::function<void(bool success, const nlohmann::json& sessions, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("GET", m_baseUrl + "/api/v1/auth/sessions", "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, {}, resp.errorMessage);
                return;
            }
            try {
                auto j = nlohmann::json::parse(resp.body);
                callback(j.value("success", false), j.value("data", nlohmann::json::object()), "");
            } catch (...) {
                callback(false, {}, "JSON parse error");
            }
        });
}

void BackendClient::RevokeSession(
    const std::string& sessionId,
    std::function<void(bool success, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("DELETE", m_baseUrl + "/api/v1/auth/sessions/" + sessionId, "", MakeAuthHeaders(false),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "");
        });
}

void BackendClient::LogoutAll(
    std::function<void(bool success, const std::string& error)> callback
) {
    m_httpClient.SendRequestAsync("POST", m_baseUrl + "/api/v1/auth/logout-all", "", MakeAuthHeaders(false),
        [this, callback](const HttpResponse& resp) {
            ClearSession();
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "");
        });
}

void BackendClient::ChangePassword(
    const std::string& currentPassword,
    const std::string& newPassword,
    std::function<void(bool success, const std::string& error)> callback
) {
    nlohmann::json payload;
    payload["current_password"] = currentPassword;
    payload["new_password"] = newPassword;

    m_httpClient.SendRequestAsync("PATCH", m_baseUrl + "/api/v1/account/password", payload.dump(), MakeAuthHeaders(true),
        [callback](const HttpResponse& resp) {
            if (!resp.IsOk()) {
                callback(false, resp.errorMessage.empty() ? resp.body : resp.errorMessage);
                return;
            }
            callback(true, "");
        });
}

bool BackendClient::SaveSessionToFile(const std::string& filePath) {
    try {
        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        nlohmann::json j;
        j["user_id"] = m_session.userId;
        j["username"] = m_session.username;
        j["email"] = m_session.email;
        j["display_name"] = m_session.displayName;
        j["access_token"] = m_session.accessToken;
        j["refresh_token"] = m_session.refreshToken;

        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        file << j.dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

bool BackendClient::LoadSessionFromFile(const std::string& filePath) {
    try {
        if (!std::filesystem::exists(filePath)) return false;
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        auto j = nlohmann::json::parse(file);
        m_session.userId = j.value("user_id", "");
        m_session.username = j.value("username", "");
        m_session.email = j.value("email", "");
        m_session.displayName = j.value("display_name", "");
        m_session.accessToken = j.value("access_token", "");
        m_session.refreshToken = j.value("refresh_token", "");
        return m_session.IsLoggedIn();
    } catch (...) {
        return false;
    }
}

} // namespace wildbound::network::http

