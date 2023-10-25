//
// Created by hejun on 2023/9/4.
//

#include "drlf-tag.h"
#include "ns3/core-module.h"

NS_LOG_COMPONENT_DEFINE ("DrlfTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DrlfTag);

DrlfTag::DrlfTag()
{
    this->m_type = 0;
    this->m_node = 0;
    this->m_lsaNode = 0;
    this->m_ttl = 0;
    this->m_cost = 0.0;

    this->m_sendTime = Simulator::Now();
}

TypeId
DrlfTag::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DrlfTag")
                            .SetParent<Tag> ()
                            .AddConstructor<DrlfTag> ();
    return tid;
}

TypeId
DrlfTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
DrlfTag::GetSerializedSize() const
{
    return 14 + sizeof(m_sendTime.GetTimeStep());
}

void
DrlfTag::Serialize(ns3::TagBuffer i) const
{
    i.WriteU8(m_type);
    i.WriteU16(m_node);
    i.WriteU8(m_ttl);
    int64_t time = m_sendTime.GetTimeStep();
    i.Write((const uint8_t*)&time, sizeof(int64_t));
    i.WriteDouble(m_cost);
    i.WriteU16(m_lsaNode);
}

void
DrlfTag::Deserialize(ns3::TagBuffer i)
{
    m_type = i.ReadU8();
    m_node = i.ReadU16();
    m_ttl = i.ReadU8();
    int64_t time;
    i.Read((uint8_t*)&time, sizeof(int64_t));
    m_sendTime = Time(time);
    m_cost = i.ReadDouble();
    m_lsaNode = i.ReadU16();
}

void
DrlfTag::Print(std::ostream& os) const
{
    os << "type: " << (int)m_type
       << ", node: " << m_node
       << ", lsaNode: " << m_lsaNode
       << ", ttl: " << (int)m_ttl
       << ", sendTime: " << m_sendTime
       << ", cost: " << m_cost
       << std::endl;
}

void
DrlfTag::SetType(uint8_t type)
{
    m_type = type;
}

uint8_t
DrlfTag::GetType()
{
    return m_type;
}

void
DrlfTag::SetNode(uint16_t node)
{
    m_node = node;
}

uint16_t
DrlfTag::GetNode()
{
    return m_node;
}

void
DrlfTag::SetTTL(uint8_t ttl)
{
    m_ttl = ttl;
}

uint8_t
DrlfTag::GetTTL()
{
    return m_ttl;
}

void
DrlfTag::DeTTL()
{
    m_ttl--;
}

void
DrlfTag::SetTime(ns3::Time time)
{
    m_sendTime = time;
}

Time
DrlfTag::GetTime()
{
    return m_sendTime;
}

void
DrlfTag::SetCost(double cost)
{
    m_cost = cost;
}

double
DrlfTag::GetCost()
{
    return m_cost;
}

void
DrlfTag::SetLsaNode(uint16_t lsaNode)
{
    m_lsaNode = lsaNode;
}

uint16_t
DrlfTag::GetLsaNode()
{
    return m_lsaNode;
}

}