#include "Entity.hpp"

namespace wildbound::entities {

Entity::Entity(uint32_t id, const std::string& name, const std::string& tag)
    : m_id(id)
    , m_name(name)
    , m_tag(tag)
{
}

} // namespace wildbound::entities

