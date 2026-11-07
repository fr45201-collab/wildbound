#include "LinuxSignals.hpp"

#if defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))
#include <csignal>
#include <cstring>
#include <atomic>

namespace wildbound::platform::linux_os {

static TerminationSignalHandler s_activeHandler = nullptr;

static void PosixSignalDispatcher(int signum) {
    const char* reason = "POSIX OS Signal Received";
    switch (signum) {
        case SIGINT:
            reason = "SIGINT Interruption (Ctrl+C)";
            break;
        case SIGTERM:
            reason = "SIGTERM Termination Request";
            break;
        case SIGHUP:
            reason = "SIGHUP Terminal Hangup";
            break;
        default:
            break;
    }

    if (s_activeHandler) {
        s_activeHandler(reason);
    }
}

bool LinuxSignals::RegisterTerminationHandler(TerminationSignalHandler handler) {
    s_activeHandler = std::move(handler);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = PosixSignalDispatcher;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    bool success = true;
    if (sigaction(SIGINT, &sa, nullptr) != 0) success = false;
    if (sigaction(SIGTERM, &sa, nullptr) != 0) success = false;
    if (sigaction(SIGHUP, &sa, nullptr) != 0) success = false;

    // Ignore SIGPIPE so broken network sockets do not kill the server process
    signal(SIGPIPE, SIG_IGN);

    return success;
}

void LinuxSignals::UnregisterTerminationHandler() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);

    s_activeHandler = nullptr;
}

} // namespace wildbound::platform::linux_os

#else // !Linux/Unix

namespace wildbound::platform::linux_os {

bool LinuxSignals::RegisterTerminationHandler(TerminationSignalHandler) { return false; }
void LinuxSignals::UnregisterTerminationHandler() {}

} // namespace wildbound::platform::linux_os

#endif

