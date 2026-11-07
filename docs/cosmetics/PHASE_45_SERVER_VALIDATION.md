# WILDBOUND — PHASE 45: SERVER-SIDE COSMETIC VALIDATION SPECIFICATION

## 1. Architectural Mandate: Server Gameplay Authority

In Wildbound, **the dedicated server owns gameplay truth and authoritative state**. Clients are never trusted with appearance claims or cosmetic inventory records. A malicious client cannot simply broadcast an appearance packet claiming to wear legendary or unowned cosmetics.

### Core Architectural Rules:
1. **Zero Database Access on Dedicated Server**: `WildboundServer` never connects directly to PostgreSQL. Dedicated game servers run in isolated network zones and interact with persistent data strictly via authenticated HTTP calls to the Go backend.
2. **Dedicated Server Secret Token (`X-Server-Token`)**: All server-to-backend communication uses the high-entropy shared secret configured in `configs/dedicated_server.json` and `backend/configs/development.json`.
3. **Independent Loadout Retrieval**: When a client connects to `WildboundServer`, the server queries the Go backend independently to retrieve the player's true verified loadout.
4. **Authoritative Fallback**: If the backend is unreachable or returns non-200, the server applies a clean default adventurer loadout, ensuring gameplay simulation is never stalled.

---

## 2. Server-to-Backend Validation Pipeline

```
+------------------+         +-----------------------+         +--------------------+
|  Game Client     |         |  Dedicated Game Server|         |  Go REST Backend   |
|  (User: Alice)   |         |  (WildboundServer)    |         |  (localhost:8080)  |
+--------+---------+         +-----------+-----------+         +---------+----------+
         |                               |                               |
         | 1. ENet Connect &             |                               |
         |    Join Match                 |                               |
         |------------------------------>|                               |
         |                               |                               |
         |                               | 2. GET /api/v1/internal/      |
         |                               |    players/{user_id}/loadout  |
         |                               |    Header: X-Server-Token     |
         |                               |------------------------------>|
         |                               |                               |
         |                               |    3. Query `player_loadouts` |
         |                               |       & Verify Ownership      |
         |                               |       in PostgreSQL           |
         |                               |                               |
         |                               | 4. Return Verified Loadout    |
         |                               |    HTTP 200 OK (Slots + Assets)
         |                               |<------------------------------|
         |                               |                               |
         |                               | 5. Validate Cosmetic IDs      |
         |                               |    Against Server Registry    |
         |                               |                               |
         |                               | 6. Apply to Alice's Player    |
         |                               |    Entity (ModularCharacter)  |
         |                               |                               |
         | 7. MsgPlayerAppearanceSync    |                               |
         |    (Authoritative Appearance) |                               |
         |<------------------------------|                               |
         |                               |                               |
         |                               | 8. Broadcast Appearance to    |
         |                               |    all other players          |
         |                               |------------------------------>|
```

---

## 3. Dedicated Server Implementation (`DedicatedServer.cpp`)

### 3.1 Startup Cosmetic Registry Initialisation
On server startup, `DedicatedServer::InitializeCosmetics()` loads the official cosmetic catalogue from `data/cosmetics/costume_catalog.json` into memory:
```cpp
void DedicatedServer::InitializeCosmetics() {
    std::filesystem::path catPath = "data/cosmetics/costume_catalog.json";
    if (std::filesystem::exists(catPath)) {
        m_serverCosmeticRegistry.LoadCatalogFromFile(catPath.string());
        LOG_INFO("Dedicated server cosmetic registry loaded: " + 
                 std::to_string(m_serverCosmeticRegistry.GetItems().size()) + " definitions.");
    }
}
```

### 3.2 Asynchronous Backend Validation Query
When a player completes handshake and authenticates, `ValidateAndApplyPlayerCosmetics(peerId, userId)` is triggered:
```cpp
void DedicatedServer::ValidateAndApplyPlayerCosmetics(uint32_t peerId, const std::string& userId) {
    std::string endpoint = "/api/v1/internal/players/" + userId + "/loadout";
    
    m_cosmeticHttpClient.Get(endpoint, [this, peerId, userId](const HttpResponse& resp) {
        if (resp.statusCode == 200) {
            auto json = nlohmann::json::parse(resp.body);
            ApplyVerifiedLoadout(peerId, json["data"]);
        } else {
            LOG_WARNING("Backend verification failed for user " + userId + 
                        " (HTTP " + std::to_string(resp.statusCode) + "). Applying clean default appearance.");
            ApplyDefaultAppearance(peerId);
        }
        BroadcastPlayerAppearance(peerId);
    });
}
```

### 3.3 Strict Identity and Attribute Isolation
Cosmetics are **strictly cosmetic**:
- They only set asset keys on `ModularCharacter::CharacterEquipment`.
- Entity attributes (`Health`, `MaxHealth`, `Armor`, `Defense`, `MoveSpeed`) are never altered by cosmetics.
- Any attempt to send modified stats along with appearance is ignored by server systems.

