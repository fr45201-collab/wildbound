#include "WindowsConsole.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#include <windows.h>
#include <iostream>

namespace wildbound::platform::windows {

static TerminationSignalHandler s_activeHandler = nullptr;

static BOOL WINAPI WindowsConsoleCtrlHandler(DWORD ctrlType) {
    std::string reason;
    switch (ctrlType) {
        case CTRL_C_EVENT:
            reason = "Ctrl+C Interruption";
            break;
        case CTRL_BREAK_EVENT:
            reason = "Ctrl+Break Interruption";
            break;
        case CTRL_CLOSE_EVENT:
            reason = "Console Window Closed";
            break;
        case CTRL_LOGOFF_EVENT:
            reason = "User Logoff Event";
            break;
        case CTRL_SHUTDOWN_EVENT:
            reason = "System Shutdown Event";
            break;
        default:
            return FALSE;
    }

    if (s_activeHandler) {
        s_activeHandler(reason);
        return TRUE;
    }
    return FALSE;
}

bool WindowsConsole::RegisterTerminationHandler(TerminationSignalHandler handler) {
    s_activeHandler = std::move(handler);
    return SetConsoleCtrlHandler(WindowsConsoleCtrlHandler, TRUE) == TRUE;
}

void WindowsConsole::UnregisterTerminationHandler() {
    SetConsoleCtrlHandler(WindowsConsoleCtrlHandler, FALSE);
    s_activeHandler = nullptr;
}

void WindowsConsole::ConfigureConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
}

} // namespace wildbound::platform::windows

#else // !_WIN32

namespace wildbound::platform::windows {

bool WindowsConsole::RegisterTerminationHandler(TerminationSignalHandler) { return false; }
void WindowsConsole::UnregisterTerminationHandler() {}
void WindowsConsole::ConfigureConsole() {}

} // namespace wildbound::platform::windows

#endif

