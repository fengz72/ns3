//
// Created by hejun on 2023/10/10.
//

#include "dsra-config.h"

namespace ns3
{
DsraConfig* DsraConfig::m_instance = nullptr;

DsraConfig*
DsraConfig::Instance()
{
    if (!m_instance)
    {
        m_instance = new DsraConfig;
    }
    return m_instance;
}

DsraConfig::DsraConfig()
{
    m_ttl = 5;
}

std::pair<uint16_t, uint16_t>
DsraConfig::GetSatPair(uint16_t id)
{
    return std::make_pair<uint16_t, uint16_t>(id / m_S, id % m_S);
}

std::pair<uint16_t, uint16_t>
DsraConfig::GetSatPair(ns3::Ipv4Address ip)
{
    return std::make_pair((ip.Get() >> 16) & 0xff, (ip.Get() >> 8) & 0xff);
}

uint16_t
DsraConfig::GetSatId(ns3::Ipv4Address ip)
{
    uint16_t sat = (ip.Get() >> 8) & 0xff;
    uint16_t orbit = (ip.Get() >> 16) & 0xff;

    return orbit * m_S + sat;
}

uint16_t
DsraConfig::GetSatId(std::pair<uint16_t, uint16_t> satPair)
{
    return satPair.first * m_S + satPair.second;
}

int
DsraConfig::GetType(Ipv4Address ip)
{
    return (ip.Get() >> 24) & 0xff;
}

void
DsraConfig::SetNodeContainer(ns3::NodeContainer& nc)
{
    m_nodes = nc;
}

NodeContainer&
DsraConfig::GetNodeContainer()
{
    return m_nodes;
}

bool
DsraConfig::IsBlock(std::pair<uint16_t, uint16_t> source, std::pair<uint16_t, uint16_t> dest, int oif)
{
    int d_down = (m_S + dest.second - source.second ) % m_S; // 向下的距离
    int d_up = m_S - d_down; // 向上的距离
    int oif_y = (d_up <= d_down)? 1: 3; // 选择y方向上的哪个端口

    int d_right = (m_P + dest.first - source.first) % m_P; // 向右的距离
    int d_left = m_P - d_right; // 向左的距离
    int oif_x = (d_left <= d_right)? 4: 2; // 选择x方向上的哪个端口

    int next_oif = (oif == oif_y) ? oif_x : oif_y;

    std::pair<uint16_t, uint16_t> mid;
    if (oif == 1 || oif == 3)
    {
        mid = std::make_pair(source.first, dest.second);
    }
    else
    {
        mid = std::make_pair(dest.first, source.second);
    }

    int count = 0;
    std::pair<uint16_t, uint16_t> cur = std::make_pair(source.first, source.second);
    while (count < (m_ttl+1) && cur != mid)
    {
        std::pair<uint16_t, uint16_t> next = Next(cur, oif);

        uint16_t curId = GetSatId(cur);
        uint16_t nextId = GetSatId(next);
        uint16_t minId = std::min(curId, nextId);
        uint16_t maxId = std::max(curId, nextId);
        if (m_block.count(std::make_pair(minId, maxId)))
        {
            return true;
        }
        cur = next;
        count++;
    }

    if (count == (m_ttl+1))
    {
        return false;
    }

    while (count < (m_ttl+1) && cur != dest)
    {
        std::pair<uint16_t, uint16_t> next = Next(cur, next_oif);

        uint16_t curId = GetSatId(cur);
        uint16_t nextId = GetSatId(next);
        uint16_t minId = std::min(curId, nextId);
        uint16_t maxId = std::max(curId, nextId);
        if (m_block.count(std::make_pair(minId, maxId)))
        {
            return true;
        }
        cur = next;
        count++;
    }

    return false;
}

std::pair<uint16_t, uint16_t>
DsraConfig::Next(std::pair<uint16_t, uint16_t> cur, int oif)
{
    std::pair<uint16_t, uint16_t> next;
    if (oif == 1) {
        next = std::make_pair(cur.first, (m_S + cur.second - 1) % m_S);
    } else if (oif == 3) {
        next = std::make_pair(cur.first, (cur.second + 1) % m_S);
    } else if (oif == 2) {
        next = std::make_pair((cur.first + 1) % m_P, cur.second);
    } else {
        next = std::make_pair((m_P + cur.first - 1) % m_P, cur.second);
    }

    return next;
}

} // namespace ns