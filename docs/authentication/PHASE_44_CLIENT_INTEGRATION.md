# Wildbound — C++ Client Authentication & UI Integration

## 1. Overview

The Wildbound client (`wildbound`) incorporates a full authentication and profile UI layer built with C++20 and raylib. The implementation provides seamless account management, persistent sessions, and offline graceful degradation.

---

## 2. Component Architecture

The client authentication stack consists of two primary systems:

1. **`wildbound::network::http::BackendClient`**:
   - Asynchronous HTTP client providing thread-safe communication with `WildboundBackend`.
   - Manages token storage, automatic token injection in `Authorization: Bearer` headers, and local disk caching.
   - Provides methods: `Register`, `Login`, `RefreshToken`, `Logout`, `GetSettings`, `UpdateSettings`, `GetProgression`, `GetAchievements`, `ChangePassword`, `SaveSessionToFile`, `LoadSessionFromFile`.

2. **`wildbound::ui::AuthMenu`**:
   - Modal dialog rendered within raylib's 640x360 virtual canvas coordinate space.
   - Supports three operational modes:
     - `AuthMenuMode::Login`: Authenticate with email/username and password.
     - `AuthMenuMode::Register`: Create a new account with email, username, and password confirmation.
     - `AuthMenuMode::Profile`: Displays account level, total XP, current level progress bar, and reward points.
   - Fully interactive with mouse hover detection, keyboard text input focus, and password masking with bullet characters (`*`).

---

## 3. Local Session Storage & Security

### 3.1 Session Persistence File
When a player checks "Remember Me" or logs in successfully:
- The client persists the session tokens to `saved_games/session.json`.
- **Security Rule**: Plaintext passwords are NEVER written to disk or logged. Only access and refresh tokens are stored.
- **Auto-Login**: On game client startup, `BackendClient::LoadSessionFromFile()` loads the stored tokens and initiates a background refresh call. If valid, the player is automatically logged in without entering credentials.

### 3.2 Offline Graceful Fallback
If the game client cannot reach `WildboundBackend` (e.g. offline travel, local testing, network outage):
- The client gracefully alerts the user with an offline indicator.
- Single-player offline matches, local bot practice, and LAN play remain fully playable without blocking the game startup.
- Local high scores and practice stats are saved locally until cloud connectivity is restored.

