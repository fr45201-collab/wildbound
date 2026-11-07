#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <memory>
#include <future>

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

    // Call on main loop to dispatch completed async responses
    void Update();

private:
    struct PendingCallback {
        HttpResponseCallback callback;
        HttpResponse response;
    };

    std::mutex m_queueMutex;
    std::vector<PendingCallback> m_completedCallbacks;
};

} // namespace wildbound::network::http

