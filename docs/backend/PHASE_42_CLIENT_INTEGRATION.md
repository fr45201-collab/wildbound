# Wildbound — Phase 42: Client Integration & HTTP Client Abstraction

## 1. Overview
The Wildbound game client is built in C++20 using raylib. Gameplay communication occurs via ENet over low-latency UDP, while persistent account interactions (registration, login, cosmetic equipping, statistics inspection) communicate with the Go backend over HTTP/REST.

## 2. Non-Blocking Asynchronous HTTP Client
To ensure smooth 60 FPS rendering without frame stutter or blocking network stalls on the main thread, the HTTP client runs all requests asynchronously on worker threads and dispatches results back to the main thread via an explicit queue:

```cpp
#include "network/http/BackendClient.hpp"

// In Game / Menu Initialization:
auto backend = std::make_unique<wildbound::network::http::BackendClient>("http://127.0.0.1:8080");

// In Main Loop:
backend->Update(); // Dispatches completed callbacks safely on the main thread

// Example Login:
backend->Login("hunter_alpha", "SecretPassword2026!", [](bool success, const std::string& msg) {
    if (success) {
        std::cout << "Logged in successfully! Ready for matchmaking.\n";
    } else {
        std::cerr << "Login failed: " << msg << "\n";
    }
});
```

## 3. Platform Implementations
- **Windows:** Uses native WinHTTP (`winhttp.h`, `winhttp.lib`) for connection pooling, TLS/HTTPS readiness, and timeout control.
- **Linux:** Uses standard POSIX sockets (`sys/socket.h`, `netdb.h`) with socket timeout options, ensuring zero third-party library dependencies.

