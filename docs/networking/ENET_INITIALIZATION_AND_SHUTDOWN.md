# WILDBOUND — ENET INITIALIZATION AND SHUTDOWN

## 1. Overview
ENet requires global library initialization via `enet_initialize()` before any network hosts or sockets can be created, and clean deinitialization via `enet_deinitialize()` upon engine termination. In Windows environments, `enet_initialize()` performs mandatory Winsock startup (`WSAStartup`) and high-resolution timer calibration (`timeBeginPeriod(1)`).

---

## 2. RAII Reference Counting Architecture

Because multiple subsystems (or test suites) may instantiate transport hosts independently, Wildbound encapsulates global ENet state inside an atomic reference counter within `ENetTransport`:

```cpp
namespace wildbound::network {

static std::atomic<int> s_enetInitRefCount{ 0 };

bool ENetTransport::InitializeENet() {
    if (s_enetInitRefCount.fetch_add(1) == 0) {
        if (enet_initialize() != 0) {
            s_enetInitRefCount.store(0);
            return false;
        }
    }
    return true;
}

void ENetTransport::DeinitializeENet() {
    int prev = s_enetInitRefCount.fetch_sub(1);
    if (prev == 1) {
        enet_deinitialize();
    } else if (prev < 1) {
        s_enetInitRefCount.store(0);
    }
}

bool ENetTransport::IsENetInitialized() noexcept {
    return s_enetInitRefCount.load() > 0;
}

}
```

### Guarantees Provided:
1. **Thread-Safe Idempotence**: Multiple `InitializeENet()` calls safely increment the atomic counter without redundant Winsock initialization.
2. **Deterministic Cleanup**: `enet_deinitialize()` is triggered if and only if the last active transport instance terminates.
3. **Safety Under Re-Initialization**: If the engine or test harness completely shuts down ENet and initializes it again, the counter starts fresh and re-executes `enet_initialize()` cleanly.

---

## 3. Host-Level Initialization and Teardown

Each `ENetTransport` instance manages an `ENetHost`:

```cpp
bool ENetTransport::Initialize(uint16_t port, size_t maxPeers) {
    if (m_host) Shutdown();
    if (!IsENetInitialized()) InitializeENet();

    m_isServer = (port > 0);
    if (m_isServer) {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;
        m_host = enet_host_create(&address, maxPeers, static_cast<size_t>(ChannelType::Count), 0, 0);
    } else {
        m_host = enet_host_create(nullptr, 1, static_cast<size_t>(ChannelType::Count), 0, 0);
    }

    return m_host != nullptr;
}

void ENetTransport::Shutdown() {
    if (!m_host) return;

    for (auto& [id, peer] : m_peers) {
        if (peer) enet_peer_disconnect_now(peer, 0);
    }
    m_peers.clear();
    m_peerToId.clear();

    enet_host_destroy(m_host);
    m_host = nullptr;
}
```

### Destruction Sequencing
1. Peers are immediately severed using `enet_peer_disconnect_now(peer, 0)`.
2. Peer mapping tables (`m_peers`, `m_peerToId`) are cleared.
3. The underlying socket and host memory are freed via `enet_host_destroy(m_host)`.
4. Global ref-count is decremented in `~ENetTransport()`.

