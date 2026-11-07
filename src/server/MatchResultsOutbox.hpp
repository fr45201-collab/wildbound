#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "../network/http/HttpClient.hpp"
#include <nlohmann/json.hpp>

namespace wildbound::server {

struct OutboxEntry {
    std::string matchId;
    std::string payloadJson;
    std::filesystem::path filePath;
    float retryTimer{ 0.0f };
    int attemptCount{ 0 };
};

class MatchResultsOutbox {
public:
    MatchResultsOutbox(
        std::string backendUrl = "http://127.0.0.1:8080",
        std::string serverSecret = "wildbound-server-internal-secret-token",
        std::filesystem::path outboxDir = "user_data/outbox"
    );
    ~MatchResultsOutbox();

    void Initialize();
    void EnqueueMatchResult(const std::string& matchId, const nlohmann::json& matchPayload);
    void Update(float dt);

    [[nodiscard]] size_t GetPendingCount() const noexcept { return m_pendingEntries.size(); }

private:
    std::string m_backendUrl;
    std::string m_serverSecret;
    std::filesystem::path m_outboxDir;
    network::http::HttpClient m_httpClient;
    std::vector<OutboxEntry> m_pendingEntries;
    float m_scanTimer{ 0.0f };

    void LoadExistingOutboxFiles();
    void TrySubmitEntry(OutboxEntry& entry);
};

} // namespace wildbound::server

