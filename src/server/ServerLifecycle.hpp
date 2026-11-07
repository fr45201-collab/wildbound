#pragma once

#include <cstdint>
#include <string_view>

namespace wildbound::server {

enum class ServerLifecycleState : uint8_t {
    Uninitialized = 0,
    Initializing  = 1,
    Starting      = 2,
    Running       = 3,
    Stopping      = 4,
    Stopped       = 5,
    Failed        = 6
};

inline constexpr std::string_view ServerLifecycleStateToString(ServerLifecycleState state) noexcept {
    switch (state) {
        case ServerLifecycleState::Uninitialized: return "Uninitialized";
        case ServerLifecycleState::Initializing:  return "Initializing";
        case ServerLifecycleState::Starting:      return "Starting";
        case ServerLifecycleState::Running:       return "Running";
        case ServerLifecycleState::Stopping:      return "Stopping";
        case ServerLifecycleState::Stopped:       return "Stopped";
        case ServerLifecycleState::Failed:        return "Failed";
        default:                                  return "Unknown";
    }
}

} // namespace wildbound::server

