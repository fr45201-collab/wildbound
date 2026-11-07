#include "MatchResultsOutbox.hpp"
#include "../core/Logger.hpp"
#include <fstream>
#include <system_error>

namespace wildbound::server {

using core::Logger;

MatchResultsOutbox::MatchResultsOutbox(
    std::string backendUrl,
    std::string serverSecret,
    std::filesystem::path outboxDir
)
    : m_backendUrl(std::move(backendUrl)),
      m_serverSecret(std::move(serverSecret)),
      m_outboxDir(std::move(outboxDir)) {
}

MatchResultsOutbox::~MatchResultsOutbox() = default;

void MatchResultsOutbox::Initialize() {
    std::error_code ec;
    std::filesystem::create_directories(m_outboxDir, ec);
    LoadExistingOutboxFiles();
}

void MatchResultsOutbox::LoadExistingOutboxFiles() {
    std::error_code ec;
    if (!std::filesystem::exists(m_outboxDir, ec)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(m_outboxDir, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::ifstream inFile(entry.path());
            if (inFile.is_open()) {
                std::string content((std::istreambuf_iterator<char>(inFile)),
                                    std::istreambuf_iterator<char>());
                try {
                    auto j = nlohmann::json::parse(content);
                    std::string matchId = j.value("match_id", entry.path().stem().string());

                    OutboxEntry item;
                    item.matchId = matchId;
                    item.payloadJson = content;
                    item.filePath = entry.path();
                    item.retryTimer = 1.0f; // immediate try
                    m_pendingEntries.push_back(std::move(item));

                    Logger::Info("OUTBOX", "Recovered pending match outbox item: " + matchId);
                } catch (...) {
                    Logger::Warn("OUTBOX", "Corrupt outbox file found: " + entry.path().string());
                }
            }
        }
    }
}

void MatchResultsOutbox::EnqueueMatchResult(const std::string& matchId, const nlohmann::json& matchPayload) {
    std::string content = matchPayload.dump(2);
    std::filesystem::path filePath = m_outboxDir / ("match_" + matchId + ".json");

    std::ofstream outFile(filePath, std::ios::trunc);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        Logger::Error("OUTBOX", "Failed to write outbox match result to disk: " + filePath.string());
    }

    OutboxEntry item;
    item.matchId = matchId;
    item.payloadJson = content;
    item.filePath = filePath;
    item.retryTimer = 0.0f; // attempt immediately
    m_pendingEntries.push_back(std::move(item));

    Logger::Info("OUTBOX", "Enqueued match " + matchId + " for backend delivery.");
    TrySubmitEntry(m_pendingEntries.back());
}

void MatchResultsOutbox::TrySubmitEntry(OutboxEntry& entry) {
    entry.attemptCount++;
    std::vector<network::http::HttpHeader> headers{
        {"Content-Type", "application/json"},
        {"X-Server-Token", m_serverSecret}
    };

    std::string matchId = entry.matchId;
    std::filesystem::path fPath = entry.filePath;

    m_httpClient.SendRequestAsync("POST", m_backendUrl + "/api/v1/internal/matches/results",
        entry.payloadJson, headers,
        [this, matchId, fPath](const network::http::HttpResponse& resp) {
            if (resp.IsOk()) {
                Logger::Info("OUTBOX", "Match result " + matchId + " successfully accepted by backend (Status " +
                                       std::to_string(resp.statusCode) + ").");
                std::error_code ec;
                std::filesystem::remove(fPath, ec);

                // Remove from pending
                std::erase_if(m_pendingEntries, [&matchId](const OutboxEntry& e) {
                    return e.matchId == matchId;
                });
            } else {
                Logger::Warn("OUTBOX", "Failed to deliver match result " + matchId + " to backend (HTTP " +
                                       std::to_string(resp.statusCode) + "): " + resp.errorMessage + ". Will retry.");
                // Find entry and back off retry
                for (auto& item : m_pendingEntries) {
                    if (item.matchId == matchId) {
                        item.retryTimer = static_cast<float>(std::min(60, 5 * item.attemptCount));
                        break;
                    }
                }
            }
        });
}

void MatchResultsOutbox::Update(float dt) {
    m_httpClient.Update();

    for (auto& entry : m_pendingEntries) {
        if (entry.retryTimer > 0.0f) {
            entry.retryTimer -= dt;
            if (entry.retryTimer <= 0.0f) {
                TrySubmitEntry(entry);
            }
        }
    }
}

} // namespace wildbound::server
