# WILDBOUND — NETWORK CONFIGURATION

## 1. Overview
The Wildbound network configuration is externalized in JSON, allowing server operators and clients to adjust ports, client caps, timeouts, simulation tick rates, and buffer limits without recompilation.

---

## 2. Configuration Schema & File Format

Configuration file: `assets/data/networking/network_config.json` (and `data/networking/network_config.json`):

```json
{
  "server": {
    "bind_port": 17777,
    "max_clients": 32,
    "tick_rate": 60,
    "max_incoming_bandwidth": 0,
    "max_outgoing_bandwidth": 0,
    "peer_timeout_ms": 5000,
    "disconnect_timeout_ms": 3000
  },
  "client": {
    "server_address": "127.0.0.1",
    "server_port": 17777,
    "connection_timeout_ms": 5000,
    "simulation_rate": 60
  },
  "transport": {
    "channel_count": 5,
    "max_packet_size": 65535,
    "default_mtu": 1400,
    "enable_crc32": false
  }
}
```

---

## 3. C++ Configuration Representation

Loaded and queried via `NetworkConfiguration` (`NetworkConfiguration.hpp` / `.cpp`):

```cpp
struct ServerConfig {
    uint16_t bindPort{ 17777 };
    size_t maxClients{ 32 };
    uint32_t tickRate{ 60 };
    uint32_t peerTimeoutMs{ 5000 };
    uint32_t disconnectTimeoutMs{ 3000 };
};

struct ClientConfig {
    std::string serverAddress{ "127.0.0.1" };
    uint16_t serverPort{ 17777 };
    uint32_t connectionTimeoutMs{ 5000 };
    uint32_t simulationRate{ 60 };
};

struct TransportConfig {
    size_t channelCount{ 5 };
    size_t maxPacketSize{ 65535 };
    size_t defaultMtu{ 1400 };
};
```
