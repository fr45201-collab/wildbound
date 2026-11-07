#include "CoordinatorServer.hpp"
#include "../core/Logger.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>

using wildbound::core::Logger;
using wildbound::core::LogLevel;
using wildbound::coordinator::CoordinatorServer;
using wildbound::coordinator::CoordinatorConfig;

static std::atomic<bool> g_coordinatorRunning{ true };

static void SignalHandler(int signum) {
    (void)signum;
    g_coordinatorRunning = false;
}

int main(int argc, char* argv[]) {
    CoordinatorConfig config = CoordinatorConfig::GetDefault();
    std::string configPath = "assets/config/coordinator.json";
    bool testRunMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            std::cout << "========================================================\n"
                      << "WILDBOUND COORDINATOR SERVICE\n"
                      << "========================================================\n"
                      << "Usage: WildboundCoordinator.exe [options]\n\n"
                      << "Options:\n"
                      << "  --config <path>       Path to JSON config (default: assets/config/coordinator.json)\n"
                      << "  --port <port>         UDP port to listen on (default: 7780)\n"
                      << "  --tick-rate <hz>      Simulation tick rate in Hz (10-120, default: 30)\n"
                      << "  --log-level <level>   Log verbosity: trace, debug, info, warn, error\n"
                      << "  --test-run            Initialize coordinator, verify tick loop, and exit cleanly\n"
                      << "  --help, -h            Show this help manual\n"
                      << "  --version, -v         Display coordinator version\n"
                      << "========================================================\n";
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            std::cout << "Wildbound Coordinator Service v1.0.0 (Phase 41 Lobby & Matchmaking)\n";
            return 0;
        } else if (arg == "--test-run" || arg == "--headless-verify") {
            testRunMode = true;
        } else if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            config.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--tick-rate" && i + 1 < argc) {
            config.tickRate = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--log-level" && i + 1 < argc) {
            config.logLevel = argv[++i];
        }
    }

    // Load file config with CLI overrides taking priority
    (void)config.LoadFromFile(configPath);
    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--port" && i + 1 < argc) config.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        else if (arg == "--tick-rate" && i + 1 < argc) config.tickRate = static_cast<uint32_t>(std::stoul(argv[++i]));
        else if (arg == "--log-level" && i + 1 < argc) config.logLevel = argv[++i];
    }

    LogLevel level = LogLevel::Info;
    if (config.logLevel == "debug" || config.logLevel == "trace") level = LogLevel::Debug;
    else if (config.logLevel == "warn") level = LogLevel::Warning;
    else if (config.logLevel == "error") level = LogLevel::Error;
    Logger::Init(level);

    Logger::Info("COORDINATOR", "=== Initializing Wildbound Coordinator ===");
    CoordinatorServer server;
    if (!server.Initialize(config)) {
        Logger::Error("COORDINATOR", "Failed to initialize Coordinator configuration.");
        return 1;
    }

    if (!server.Start()) {
        Logger::Error("COORDINATOR", "Failed to start Coordinator server.");
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    float tickDt = 1.0f / static_cast<float>(config.tickRate);

    if (testRunMode) {
        Logger::Info("COORDINATOR", "Test-run mode enabled. Running 5 simulation ticks...");
        for (int i = 0; i < 5; ++i) {
            server.Update(tickDt);
        }
        Logger::Info("COORDINATOR", "Headless verification ticks completed successfully.");
        server.Stop();
        return 0;
    }

    Logger::Info("COORDINATOR", "Entering main coordinator simulation loop...");
    auto nextTick = std::chrono::steady_clock::now();
    const auto tickInterval = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<float>(tickDt)
    );

    while (g_coordinatorRunning) {
        server.Update(tickDt);

        nextTick += tickInterval;
        auto now = std::chrono::steady_clock::now();
        if (nextTick > now) {
            std::this_thread::sleep_until(nextTick);
        } else {
            nextTick = now;
        }
    }

    Logger::Info("COORDINATOR", "Termination signal received. Shutting down cleanly...");
    server.Stop();
    Logger::Info("COORDINATOR", server.GetDiagnostics().FormatSummary());
    return 0;
}
