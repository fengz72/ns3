//
// Created by hejun on 2023/9/11.
//

#include "drlf-conf.h"

namespace ns3
{
using namespace::std;

DrlfConfig* DrlfConfig::m_instance = nullptr;

DrlfConfig*
DrlfConfig::Instance()
{
    if (!m_instance)
    {
        m_instance = new DrlfConfig;
    }
    return m_instance;
}

DrlfConfig::DrlfConfig()
{
    m_calcCost = 0;
    m_maxHello = Seconds(0.0);
    m_maxLsa = Seconds(0.0);
    m_maxError = Seconds(0.0);
}

int
DrlfConfig::GetUnavailableInterval()
{
    return m_UnavailableInterval;
}

void
DrlfConfig::SetUnavailableInterval(int unavailableInterval)
{
    m_UnavailableInterval = unavailableInterval;
}

void
DrlfConfig::SetNodeContainer(NodeContainer& nc)
{
    m_nodes = nc;
}

NodeContainer&
DrlfConfig::GetNodeContainer()
{
    return m_nodes;
}

int
DrlfConfig::GetTypeFromIp(ns3::Ipv4Address ip)
{
    int type = (ip.Get() >> 24) & 0xff;
    return type;
}

uint16_t
DrlfConfig::GetSatId(Ipv4Address ip)
{
    uint16_t sat = (ip.Get() >> 8) & 0xff;
    uint16_t orbit = (ip.Get() >> 16) & 0xff;

    return orbit * m_S + sat;
}

uint16_t
DrlfConfig::GetSatId(pair<uint16_t, uint16_t> satPair)
{
    return satPair.first * m_S + satPair.second;
}

uint64_t
DrlfConfig::GetCalculateCost()
{
    switch (m_ttl)
    {
    case 1: return 35;
    case 2: return 100;
    case 3: return 200;
    case 4: return 400;
    case 5: return 600;
    }

    return 0;
}

pair<uint16_t, uint16_t>
DrlfConfig::GetSatPair(uint16_t id)
{
    uint16_t sat = id % m_S;
    uint16_t orbit = id / m_S;
    return pair<uint16_t, uint16_t>(orbit, sat);
}

pair<uint16_t, uint16_t>
DrlfConfig::GetSatPair(Ipv4Address ip)
{
    uint16_t sat = (ip.Get() >> 8) & 0xff;
    uint16_t orbit = (ip.Get() >> 16) & 0xff;

    return make_pair(orbit, sat);
}

void
DrlfConfig::SetMaxHello(Time now)
{
    m_maxHello = max(m_maxHello, now);
}

Time
DrlfConfig::GetMaxHello()
{
    return m_maxHello;
}

void
DrlfConfig::SetMaxLsa(Time now)
{
    m_maxLsa = max(m_maxLsa, now);
}

Time
DrlfConfig::GetMaxLsa()
{
    return m_maxLsa;
}

} // namespace ns3