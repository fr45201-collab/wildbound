#include "NetworkConnection.hpp"

namespace wildbound::network {

NetworkConnection::NetworkConnection(ClientId id, void* peerHandle, std::string address, uint16_t port)
    : m_id(id)
    , m_peerHandle(peerHandle)
    , m_address(std::move(address))
    , m_port(port)
    , m_state(ConnectionState::Connecting)
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    m_lastActivityTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

} // namespace wildbound::network

