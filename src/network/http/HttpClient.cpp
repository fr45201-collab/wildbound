#include "HttpClient.hpp"
#include <thread>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

namespace wildbound::network::http {

HttpClient::HttpClient() = default;

HttpClient::~HttpClient() {
    Shutdown();
}

bool HttpClient::IsShuttingDown() const noexcept {
    return m_isShuttingDown.load(std::memory_order_acquire);
}

void HttpClient::ReapCompletedWorkers() {
    auto it = m_workers.begin();
    while (it != m_workers.end()) {
        if ((*it)->completed.load(std::memory_order_acquire)) {
            if ((*it)->thread.joinable()) {
                (*it)->thread.join();
            }
            it = m_workers.erase(it);
        } else {
            ++it;
        }
    }
}

void HttpClient::Shutdown() {
    bool expected = false;
    if (!m_isShuttingDown.compare_exchange_strong(expected, true)) {
        // Already shutting down or shut down. Ensure any remaining workers are joined if called repeatedly.
        std::vector<std::unique_ptr<WorkerTask>> remaining;
        {
            std::lock_guard<std::mutex> lock(m_workersMutex);
            remaining = std::move(m_workers);
            m_workers.clear();
        }
        for (auto& task : remaining) {
            if (task && task->thread.joinable()) {
                task->thread.join();
            }
        }
        return;
    }

    std::vector<std::unique_ptr<WorkerTask>> workersToJoin;
    {
        std::lock_guard<std::mutex> lock(m_workersMutex);
        workersToJoin = std::move(m_workers);
        m_workers.clear();
    }

    for (auto& task : workersToJoin) {
        if (task && task->thread.joinable()) {
            task->thread.join();
        }
    }
    workersToJoin.clear();

    {
        std::lock_guard<std::mutex> qLock(m_queueMutex);
        m_completedCallbacks.clear();
    }
}

#ifdef _WIN32

static std::wstring ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size);
    return result;
}

static std::string ToNarrow(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &result[0], size, nullptr, nullptr);
    return result;
}

HttpResponse HttpClient::SendRequest(
    const std::string& method,
    const std::string& url,
    const std::string& body,
    const std::vector<HttpHeader>& headers,
    int timeoutMs
) {
    HttpResponse response;

    URL_COMPONENTS urlComp{};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength = static_cast<DWORD>(-1);
    urlComp.dwHostNameLength = static_cast<DWORD>(-1);
    urlComp.dwUrlPathLength = static_cast<DWORD>(-1);
    urlComp.dwExtraInfoLength = static_cast<DWORD>(-1);

    std::wstring wUrl = ToWide(url);
    if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.size()), 0, &urlComp)) {
        response.errorMessage = "Failed to parse URL: " + url;
        return response;
    }

    std::wstring host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
    INTERNET_PORT port = urlComp.nPort;
    bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    HINTERNET hSession = WinHttpOpen(
        L"WildboundClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession) {
        response.errorMessage = "WinHttpOpen failed";
        return response;
    }

    WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        response.errorMessage = "WinHttpConnect failed to " + ToNarrow(host);
        WinHttpCloseHandle(hSession);
        return response;
    }

    std::wstring wMethod = ToWide(method);
    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        wMethod.c_str(),
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );
    if (!hRequest) {
        response.errorMessage = "WinHttpOpenRequest failed";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    // Add Headers
    std::wstring allHeaders;
    for (const auto& [name, val] : headers) {
        allHeaders += ToWide(name) + L": " + ToWide(val) + L"\r\n";
    }

    BOOL sendResult = WinHttpSendRequest(
        hRequest,
        allHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : allHeaders.c_str(),
        static_cast<DWORD>(allHeaders.size()),
        body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
        static_cast<DWORD>(body.size()),
        static_cast<DWORD>(body.size()),
        0
    );

    if (!sendResult || !WinHttpReceiveResponse(hRequest, nullptr)) {
        response.errorMessage = "Failed to send HTTP request or receive response";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    // Read Status Code
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX
    );
    response.statusCode = static_cast<int>(statusCode);

    // Read Response Body
    DWORD bytesAvailable = 0;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
        std::vector<char> buffer(bytesAvailable);
        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
            response.body.append(buffer.data(), bytesRead);
        }
    }

    response.success = true;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

#else

HttpResponse HttpClient::SendRequest(
    const std::string& method,
    const std::string& url,
    const std::string& body,
    const std::vector<HttpHeader>& headers,
    int timeoutMs
) {
    HttpResponse response;
    // Basic socket fallback for Linux/Unix
    // Parse URL (assumes http://host:port/path)
    std::string prefix = "http://";
    if (url.substr(0, prefix.size()) != prefix) {
        response.errorMessage = "Only http:// supported in socket fallback";
        return response;
    }

    std::string rest = url.substr(prefix.size());
    size_t slashPos = rest.find('/');
    std::string hostPort = (slashPos != std::string::npos) ? rest.substr(0, slashPos) : rest;
    std::string path = (slashPos != std::string::npos) ? rest.substr(slashPos) : "/";

    std::string host = hostPort;
    int port = 80;
    size_t colonPos = hostPort.find(':');
    if (colonPos != std::string::npos) {
        host = hostPort.substr(0, colonPos);
        port = std::stoi(hostPort.substr(colonPos + 1));
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        response.errorMessage = "Failed to create socket";
        return response;
    }

    struct hostent* he = gethostbyname(host.c_str());
    if (!he) {
        close(sock);
        response.errorMessage = "Failed to resolve host " + host;
        return response;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));
    memcpy(&serverAddr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        response.errorMessage = "Failed to connect to " + host;
        return response;
    }

    std::ostringstream reqStream;
    reqStream << method << " " << path << " HTTP/1.1\r\n";
    reqStream << "Host: " << host << "\r\n";
    reqStream << "User-Agent: WildboundClient/1.0\r\n";
    reqStream << "Connection: close\r\n";
    if (!body.empty()) {
        reqStream << "Content-Length: " << body.size() << "\r\n";
    }
    for (const auto& [k, v] : headers) {
        reqStream << k << ": " << v << "\r\n";
    }
    reqStream << "\r\n";
    if (!body.empty()) {
        reqStream << body;
    }

    std::string rawReq = reqStream.str();
    send(sock, rawReq.data(), rawReq.size(), 0);

    char buf[4096];
    std::string rawResp;
    int n = 0;
    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
        rawResp.append(buf, n);
    }
    close(sock);

    size_t headerEnd = rawResp.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        std::string headPart = rawResp.substr(0, headerEnd);
        response.body = rawResp.substr(headerEnd + 4);

        std::istringstream hStream(headPart);
        std::string httpVer;
        hStream >> httpVer >> response.statusCode;
        response.success = true;
    } else {
        response.errorMessage = "Malformed HTTP response";
    }

    return response;
}

#endif

void HttpClient::SendRequestAsync(
    const std::string& method,
    const std::string& url,
    const std::string& body,
    const std::vector<HttpHeader>& headers,
    HttpResponseCallback callback,
    int timeoutMs
) {
    bool rejected = false;
    {
        std::lock_guard<std::mutex> lock(m_workersMutex);
        ReapCompletedWorkers();

        if (m_isShuttingDown.load(std::memory_order_relaxed)) {
            rejected = true;
        } else {
            auto task = std::make_unique<WorkerTask>();
            WorkerTask* pTask = task.get();
            task->thread = std::thread([this, pTask, method, url, body, headers, callback = std::move(callback), timeoutMs]() {
                HttpResponse resp;
                try {
                    resp = SendRequest(method, url, body, headers, timeoutMs);
                } catch (const std::exception& ex) {
                    resp.success = false;
                    resp.errorMessage = std::string("Exception: ") + ex.what();
                } catch (...) {
                    resp.success = false;
                    resp.errorMessage = "Unknown exception in SendRequest";
                }

                {
                    std::lock_guard<std::mutex> qLock(m_queueMutex);
                    if (!m_isShuttingDown.load(std::memory_order_relaxed)) {
                        m_completedCallbacks.push_back({ std::move(callback), std::move(resp) });
                    }
                }

                pTask->completed.store(true, std::memory_order_release);
            });
            m_workers.push_back(std::move(task));
        }
    }

    if (rejected && callback) {
        HttpResponse rej;
        rej.statusCode = 0;
        rej.success = false;
        rej.errorMessage = "HttpClient is shutting down";
        callback(rej);
    }
}

void HttpClient::Update() {
    {
        std::lock_guard<std::mutex> wLock(m_workersMutex);
        ReapCompletedWorkers();
    }

    std::vector<PendingCallback> toDispatch;
    {
        std::lock_guard<std::mutex> qLock(m_queueMutex);
        if (m_completedCallbacks.empty()) {
            return;
        }
        toDispatch = std::move(m_completedCallbacks);
        m_completedCallbacks.clear();
    }

    for (const auto& item : toDispatch) {
        if (item.callback) {
            item.callback(item.response);
        }
    }
}

} // namespace wildbound::network::http

