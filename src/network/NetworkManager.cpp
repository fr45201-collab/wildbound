#include "NetworkManager.hpp"
#include "ENetServer.hpp"
#include "ENetClient.hpp"
#include "NetworkProtocol.hpp"
#include "NetworkMessage.hpp"
#include "replication/ReplicationManager.hpp"
#include <iostream>

namespace wildbound::network {

NetworkManager::NetworkManager()
    : m_server(std::make_unique<ENetServer>())
    , m_client(std::make_unique<ENetClient>())
    , m_replication(std::make_unique<ReplicationManager>())
{
}

NetworkManager::~NetworkManager() {
    Shutdown();
}

bool NetworkManager::Initialize(const std::string& configPath) {
    if (m_isInitialized) return true;

    if (!m_config.LoadFromFile(configPath)) {
        // Fallback to data path
        (void)m_config.LoadFromFile("data/networking/network_config.json");
    }

    if (m_replication) {
        m_replication->Initialize();
    }

    m_isInitialized = true;
    m_role = NetworkRole::None;
    return true;
}

void NetworkManager::Shutdown() {
    if (!m_isInitialized) return;

    StopServer();
    DisconnectClient();

    if (m_replication) {
        m_replication->Reset();
    }

    m_role = NetworkRole::None;
    m_isInitialized = false;
}

ReplicationManager* NetworkManager::GetReplicationManager() noexcept {
    return m_replication.get();
}

const ReplicationManager* NetworkManager::GetReplicationManager() const noexcept {
    return m_replication.get();
}

bool NetworkManager::StartServer(uint16_t port, size_t maxClients) {
    uint16_t actualPort = (port > 0) ? port : m_config.server.bindPort;
    size_t actualClients = (maxClients > 0) ? maxClients : m_config.server.maxClients;

    if (m_server && m_server->Start(actualPort, actualClients)) {
        m_role = NetworkRole::Server;
        if (m_replication) {
            m_replication->SetRole(NetworkRole::Server);
        }
        return true;
    }
    return false;
}

void NetworkManager::StopServer() {
    if (m_server) {
        m_server->Stop();
    }
    if (m_role == NetworkRole::Server) {
        m_role = NetworkRole::None;
        if (m_replication) {
            m_replication->SetRole(NetworkRole::None);
        }
    }
}

bool NetworkManager::IsServerRunning() const noexcept {
    return m_server && m_server->IsListening();
}

uint16_t NetworkManager::GetServerPort() const noexcept {
    return m_server ? m_server->GetPort() : 0;
}

size_t NetworkManager::GetServerConnectedCount() const noexcept {
    return m_server ? m_server->GetConnectedCount() : 0;
}

const std::unordered_map<ClientId, NetworkConnection>& NetworkManager::GetServerConnections() const noexcept {
    static const std::unordered_map<ClientId, NetworkConnection> s_empty;
    return m_server ? m_server->GetConnectionManager().GetConnections() : s_empty;
}

const NetworkStatistics& NetworkManager::GetServerGlobalStats() const noexcept {
    static const NetworkStatistics s_empty;
    return m_server ? m_server->GetGlobalStats() : s_empty;
}

void NetworkManager::DisconnectServerClient(ClientId clientId, uint32_t reason) {
    if (m_server) {
        m_server->DisconnectClient(clientId, reason);
    }
}

bool NetworkManager::ConnectClient(const std::string& address, uint16_t port, uint32_t timeoutMs) {
    std::string actualAddr = !address.empty() ? address : m_config.client.serverAddress;
    uint16_t actualPort = (port > 0) ? port : m_config.client.serverPort;
    uint32_t actualTimeout = (timeoutMs > 0) ? timeoutMs : m_config.client.connectionTimeoutMs;

    if (m_client && m_client->Connect(actualAddr, actualPort, actualTimeout)) {
        m_role = NetworkRole::Client;
        if (m_replication) {
            m_replication->SetRole(NetworkRole::Client);
        }
        return true;
    }
    return false;
}

void NetworkManager::DisconnectClient() {
    if (m_client) {
        m_client->Disconnect();
    }
    if (m_role == NetworkRole::Client) {
        m_role = NetworkRole::None;
        if (m_replication) {
            m_replication->SetRole(NetworkRole::None);
        }
    }
}

bool NetworkManager::IsClientConnected() const noexcept {
    return m_client && m_client->IsConnected();
}

ConnectionState NetworkManager::GetClientState() const noexcept {
    return m_client ? m_client->GetState() : ConnectionState::Disconnected;
}

uint32_t NetworkManager::GetClientRoundTripTimeMs() const noexcept {
    return m_client ? m_client->GetRoundTripTimeMs() : 0;
}

const std::string& NetworkManager::GetClientServerAddress() const noexcept {
    static const std::string s_empty;
    return m_client ? m_client->GetServerAddress() : s_empty;
}

uint16_t NetworkManager::GetClientServerPort() const noexcept {
    return m_client ? m_client->GetServerPort() : 0;
}

const NetworkStatistics& NetworkManager::GetClientStats() const noexcept {
    static const NetworkStatistics s_empty;
    return m_client ? m_client->GetStats() : s_empty;
}

void NetworkManager::Update(float dt) {
    if (!m_isInitialized) return;

    if (m_server && m_server->IsListening()) {
        m_server->Update(dt);
    }

    if (m_client && m_client->GetState() != ConnectionState::Disconnected) {
        m_client->Update(dt);
    }

    if (m_replication) {
        m_replication->Update(dt);
    }
}

bool NetworkManager::SendTestMessage(bool reliable, const std::string& text) {
    ByteStreamWriter writer;
    PacketHeader hdr{
        .magic = PROTOCOL_MAGIC,
        .version = PROTOCOL_VERSION,
        .messageType = static_cast<uint16_t>(MessageType::Heartbeat),
        .sequenceNumber = 1,
        .senderClientId = 1,
        .payloadLength = static_cast<uint16_t>(text.size() + 2)
    };
    SerializePacketHeader(writer, hdr);
    writer.WriteString(text);

    DeliveryMode mode = reliable ? DeliveryMode::ReliableOrdered : DeliveryMode::UnreliableSequenced;
    ChannelType ch = reliable ? ChannelType::ReliableEvents : ChannelType::PlayerInput;

    if (m_role == NetworkRole::Client && m_client && m_client->IsConnected()) {
        return m_client->Send(ch, mode, writer.GetData(), writer.GetSize());
    } else if (m_role == NetworkRole::Server && m_server && m_server->IsListening()) {
        m_server->Broadcast(ch, mode, writer.GetData(), writer.GetSize());
        return true;
    }
    return false;
}

} // namespace wildbound::network

