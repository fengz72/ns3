//
// Created by hejun on 2023/10/28.
//

#include "dsra-tag.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("DsraTag");
NS_OBJECT_ENSURE_REGISTERED (DsraTag);

DsraTag::DsraTag()
{
    m_type = 0;
    m_node = 0;
    m_ttl = 5;
    m_errorNode = 0;
}

TypeId
DsraTag::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DsraTag")
                            .SetParent<Tag> ()
                            .AddConstructor<DsraTag> ();
    return tid;
}

TypeId
DsraTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
DsraTag::GetSerializedSize() const
{
    return 6;
}

void
DsraTag::Serialize(ns3::TagBuffer i) const
{
    i.WriteU8(m_type);
    i.WriteU16(m_node);
    i.WriteU16(m_errorNode);
    i.WriteU8(m_ttl);
}

void
DsraTag::Deserialize(ns3::TagBuffer i)
{
    m_type = i.ReadU8();
    m_node = i.ReadU16();
    m_errorNode = i.ReadU16();
    m_ttl = i.ReadU8();
}

void
DsraTag::Print(std::ostream& os) const
{
    os << "type: " << (int)m_type
       << ", node: " << m_node
       << ", ttl: " << (int)m_ttl;
    if (m_type != 0)
    {
        os << ", errorNode: " << m_errorNode;
    }
    os << std::endl;
}

void
DsraTag::SetType(uint8_t type)
{
    m_type = type;
}
uint8_t
DsraTag::GetType()
{
    return m_type;
}

void
DsraTag::SetTTL(uint8_t ttl)
{
    m_ttl = ttl;
}
uint8_t
DsraTag::GetTTL()
{
    return m_ttl;
}

void
DsraTag::SetNode(uint16_t node)
{
    m_node = node;
}
uint16_t
DsraTag::GetNode()
{
    return m_node;
}

void
DsraTag::SetErrorNode(uint16_t error)
{
    m_node = error;
}

uint16_t
DsraTag::GetErrorNode()
{
    return m_node;
}

void
DsraTag::DeTTL()
{
    m_ttl--;
}

} // namespace ns3