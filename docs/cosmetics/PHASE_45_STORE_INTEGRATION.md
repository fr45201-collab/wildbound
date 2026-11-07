# WILDBOUND — PHASE 45: STORE INTEGRATION SPECIFICATION

## 1. Overview & Architecture

The cosmetic store in Wildbound bridges the C++ raylib client with the Go persistent backend. In previous prototypes (Phase 26), cosmetic unlocks and equips were handled strictly through local memory and disk mocks (`cosmetics_save.json`). In Phase 45, the store integration enforces server authorization and persistent backend synchronization while preserving seamless offline fallback capabilities.

```
+-----------------------------------------------------------------------+
|                            C++ CLIENT                                 |
|                                                                       |
|   +-----------------------+           +---------------------------+   |
|   | CosmeticsStoreManager | <-------> | CosmeticsOwnershipManager |   |
|   +-----------------------+           +---------------------------+   |
|               |                                     |                 |
|               | (Asynchronous HTTP)                 |                 |
|               v                                     v                 |
|   +---------------------------------------------------------------+   |
|   |                 wildbound::network::BackendClient             |   |
|   +---------------------------------------------------------------+   |
+-----------------------------------|-----------------------------------+
                                    | HTTPS / JSON
                                    v
+-----------------------------------------------------------------------+
|                            GO BACKEND                                 |
|                                                                       |
|   +----------------------+               +------------------------+   |
|   |   Public Catalogue   |               |   Player Ownership &   |   |
|   |    GET /cosmetics    |               |    Loadout Endpoints   |   |
|   +----------------------+               +------------------------+   |
|               |                                     |                 |
|               v                                     v                 |
|   +---------------------------------------------------------------+   |
|   |                 PostgreSQL 16 (wildbound_db)                  |   |
|   +---------------------------------------------------------------+   |
+-----------------------------------------------------------------------+
```

---

## 2. Store Manager Backend Integration (`CosmeticsStoreManager`)

### 2.1 Backend Client Connection
`CosmeticsStoreManager` receives a shared or raw pointer to `wildbound::network::http::BackendClient`:
```cpp
void SetBackendClient(wildbound::network::http::BackendClient* backendClient) noexcept;
[[nodiscard]] wildbound::network::http::BackendClient* GetBackendClient() const noexcept;
```

When `m_backendClient` is non-null and `m_backendClient->IsAuthenticated()` returns `true`:
- All equipping actions route through `BackendClient::EquipLoadoutItem`.
- All unequipping actions route through `BackendClient::UnequipLoadoutSlot`.
- Full loadout synchronization triggers upon opening the store UI.

### 2.2 Rejection of Unverified Client Unlocks
Direct client-side unlocking of cosmetics without backend authorization is strictly forbidden:
```cpp
bool CosmeticsStoreManager::UnlockPrototypeSelected() {
    if (m_backendClient && m_backendClient->IsAuthenticated()) {
        LOG_WARNING("Direct client-side prototype unlocking is disabled when authenticated. "
                    "Cosmetics must be acquired via authoritative backend events or server grants.");
        return false;
    }
    // Offline development fallback only
    ...
}
```
All cosmetic grants must originate from server-authenticated events via `POST /api/v1/internal/cosmetics/grant` with `X-Server-Token`.

---

## 3. Asynchronous Equip / Unequip Flow

### 3.1 Equipping from Store UI
When a player clicks "Equip" in the store or wardrobe UI:
1. `CosmeticsStoreManager::EquipSelected()` is invoked.
2. The UI checks `m_ownershipManager->IsOwned(cosmeticId)`. If unowned, equip is rejected immediately in the UI.
3. If authenticated, `BackendClient::EquipLoadoutItem(cosmeticId, currentVersion, callback)` issues an HTTP POST request to `/api/v1/players/me/loadout/equip`.
4. While the HTTP request is in flight, the UI displays a pending spinner.
5. On HTTP 200 OK:
   - `m_loadoutVersion` updates to `response.loadout_version`.
   - The visual model (`ModularCharacter`) updates equipment layers.
   - A success notification is shown to the player.
6. On HTTP 409 Conflict:
   - The client fetches the latest loadout via `BackendClient::GetMyLoadout()` to reconcile version mismatch.
7. On HTTP 403 Forbidden:
   - The ownership state is marked invalid and re-synchronized.

---

## 4. Offline Fallback & Graceful Degradation

If the Go backend is unreachable, the network is disconnected, or the game is executed in offline single-player mode:
- `m_backendClient` is null or unauthenticated.
- `CosmeticsOwnershipManager` falls back to `data/cosmetics/cosmetics_save.json` local storage.
- Default starter cosmetics (`clo_basic_traveler_001`, `clo_p_traveler_pants_001`, `head_traveler_hood_001`, `bot_leather_boots_001`) remain accessible.
- When network connectivity is re-established and the player logs in, `SyncWithBackend()` automatically queries `/api/v1/players/me/cosmetics` and authoritative backend data supersedes local mock files.

