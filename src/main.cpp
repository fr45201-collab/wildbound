#include "core/Game.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <string_view>

using wildbound::core::Game;
using wildbound::core::Logger;
using wildbound::core::LogLevel;

int main(int argc, char* argv[]) {
    // Initialize central logging subsystem
    Logger::Init(LogLevel::Debug);

    bool testRunMode = false;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--test-run" || arg == "--headless-verify") {
            testRunMode = true;
        }
    }

    try {
        Game game;
        if (!game.Initialize()) {
            Logger::Error("ENGINE", "Game initialization failed. Aborting execution.");
            Logger::Shutdown();
            return 1;
        }

        if (testRunMode) {
            Logger::Info("ENGINE", "Running in test verification mode (--test-run). Exiting after initialization.");
            game.Shutdown();
            Logger::Shutdown();
            return 0;
        }

        // Enter primary engine game loop
        game.Run();

        // Orderly shutdown
        game.Shutdown();
        Logger::Shutdown();
        return 0;
    }
    catch (const std::exception& ex) {
        Logger::Error("ENGINE", std::string("Unhandled fatal exception: ") + ex.what());
        Logger::Shutdown();
        return 1;
    }
    catch (...) {
        Logger::Error("ENGINE", "Unhandled unknown fatal exception occurred.");
        Logger::Shutdown();
        return 1;
    }
}

