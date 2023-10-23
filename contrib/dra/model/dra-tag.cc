//
// Created by hejun on 2023/8/11.
//

#include "dra-tag.h"
#include "ns3/socket.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (DRATag);

TypeId
DRATag::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DRATag")
                            .SetParent<Tag> ()
                            .AddConstructor<DRATag> ();
    return tid;
}

TypeId
DRATag::GetInstanceTypeId() const
{
    return GetTypeId();
}

DRATag::DRATag()
{
    this->m_type = 1;
}

uint32_t
DRATag::GetSerializedSize() const
{
    return 4;
}

void
DRATag::Serialize(ns3::TagBuffer i) const
{
    i.WriteU8(m_type);
    i.WriteU16(m_node);
    i.WriteU8(m_ttl);
}

void
DRATag::Deserialize(ns3::TagBuffer i)
{
    m_type = i.ReadU8();
    m_node = i.ReadU16();
    m_ttl = i.ReadU8();
}

void
DRATag::Print(std::ostream& os) const
{
    os << std::endl;
}

void
DRATag::SetType(uint8_t type)
{
    this->m_type = type;
}

uint8_t
DRATag::GetType(){
    return this->m_type;
}

void
DRATag::SetNode(uint16_t node)
{
    m_node = node;
}

uint16_t
DRATag::GetNode()
{
    return m_node;
}

uint8_t
DRATag::GetTTL()
{
    return m_ttl;
}

void
DRATag::SetTTL(uint8_t ttl)
{
    m_ttl = ttl;
}


} // namespace ns3