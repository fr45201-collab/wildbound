#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>

namespace wildbound::network::http {

struct HttpResponse {
    int statusCode{ 0 };
    std::string body;
    std::string errorMessage;
    bool success{ false };

    [[nodiscard]] bool IsOk() const noexcept {
        return success && statusCode >= 200 && statusCode < 300;
    }
};

using HttpHeader = std::pair<std::string, std::string>;
using HttpResponseCallback = std::function<void(const HttpResponse&)>;

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) noexcept = delete;
    HttpClient& operator=(HttpClient&&) noexcept = delete;

    // Synchronous request
    HttpResponse SendRequest(
        const std::string& method,
        const std::string& url,
        const std::string& body = "",
        const std::vector<HttpHeader>& headers = {},
        int timeoutMs = 5000
    );

    // Asynchronous non-blocking request
    void SendRequestAsync(
        const std::string& method,
        const std::string& url,
        const std::string& body,
        const std::vector<HttpHeader>& headers,
        HttpResponseCallback callback,
        int timeoutMs = 5000
    );

    // Explicit shutdown to cancel and join/wait for all in-flight workers
    void Shutdown();

    [[nodiscard]] bool IsShuttingDown() const noexcept;

    // Call on main loop to dispatch completed async responses
    void Update();

private:
    struct PendingCallback {
        HttpResponseCallback callback;
        HttpResponse response;
    };

    struct WorkerTask {
        std::thread thread;
        std::atomic<bool> completed{ false };
    };

    void ReapCompletedWorkers();

    std::atomic<bool> m_isShuttingDown{ false };

    mutable std::mutex m_queueMutex;
    std::vector<PendingCallback> m_completedCallbacks;

    mutable std::mutex m_workersMutex;
    std::vector<std::unique_ptr<WorkerTask>> m_workers;
};

} // namespace wildbound::network::http

