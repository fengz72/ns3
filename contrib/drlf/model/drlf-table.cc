//
// Created by hejun on 2023/9/5.
//

#include "drlf-table.h"
#include "drlf-conf.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("DrlfTable");
NS_OBJECT_ENSURE_REGISTERED (DrlfTableEntry);

DrlfTableEntry::DrlfTableEntry()
{}

DrlfTableEntry::DrlfTableEntry(uint16_t id, int oif, ns3::Time time, double cost)
:m_id(id), m_oif(oif), m_update(time), m_cost(cost)
{}

void
DrlfTableEntry::SetId(uint16_t id)
{
    m_id = id;
}

uint16_t
DrlfTableEntry::GetId()
{
    return m_id;
}

void
DrlfTableEntry::SetOif(int oif)
{
    m_oif = oif;
}

int
DrlfTableEntry::GetOif()
{
    return m_oif;
}

void
DrlfTableEntry::SetTime(Time time)
{
    m_update = time;
}

Time
DrlfTableEntry::GetTime()
{
    return m_update;
}

void
DrlfTableEntry::SetCost(double cost)
{
    m_cost = cost;
}

double
DrlfTableEntry::GetCost()
{
    return m_cost;
}

TypeId
DrlfTableEntry::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DrlfTableEntry")
                            .SetParent<Object> ()
                            .AddConstructor<DrlfTableEntry>();
    return tid;
}

DrlfTable::DrlfTable()
{
}

void
DrlfTable::AddEntry(uint16_t id, int oif, double cost)
{
    if (m_table.find(id) == m_table.end()) {
        Ptr<DrlfTableEntry> entry = CreateObject<DrlfTableEntry>();
        entry->SetId(id);
        entry->SetOif(oif);
        entry->SetTime(Simulator::Now());
        entry->SetCost(cost);
        m_table.insert(make_pair(id, entry));
    }
}

void
DrlfTable::DeleteEntry(uint16_t id)
{
    if (m_table.find(id) == m_table.end())
        return;

    m_table.erase(id);
}

void
DrlfTable::Clear()
{
    m_table.clear();
}

Ptr<DrlfTableEntry>
DrlfTable::GetEntry(uint16_t destId)
{
    if (m_table.find(destId) == m_table.end()) {
        return nullptr;
    }

    auto it = m_table.find(destId);
    Ptr<DrlfTableEntry> entry = it->second;

    return entry;
}

Ptr<DrlfTableEntry>
DrlfTable::CalcNextHop(uint16_t destId)
{
    std::pair<uint16_t, uint16_t > destPair = DrlfConfig::Instance()->GetSatPair(destId);
    uint16_t minId = 0;
    uint16_t minDelta = 1 << 15;
    bool flag = false;
    double minCost = 1e9;
    for (auto it: m_table)
    {
        std::pair<uint16_t, uint16_t > cur = DrlfConfig::Instance()->GetSatPair(it.first);
        uint16_t delta = abs((int)destPair.first - cur.first) + abs((int)destPair.first - cur.first);

        if (delta < minDelta) {
            flag = true;
            minId = it.first;
            minDelta = delta;
            minCost = it.second->GetCost();
        } else if (delta == minDelta) {
            if (it.second->GetCost() < minCost)
            {
                flag = true;
                minId = it.first;
                minDelta = delta;
                minCost = it.second->GetCost();
            }
        }
    }
    if (!flag) {
        return nullptr;
    }

    auto it = m_table.find(minId);
    Ptr<DrlfTableEntry> entry = it->second;
    return entry;
}

} // namespace ns3