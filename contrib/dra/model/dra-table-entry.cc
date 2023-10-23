//
// Created by hejun on 2023/8/14.
//

#include "dra-table-entry.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("DraTableEntry");
NS_OBJECT_ENSURE_REGISTERED (DRATableEntry);

TypeId
DRATableEntry::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DraTableEntry")
                            .SetParent<Object> ()
                            .AddConstructor<DRATableEntry>();
    return tid;
}

DRATableEntry::DRATableEntry()
{}

DRATableEntry::DRATableEntry(uint16_t satId, int oif, Time time)
    :m_satId(satId),
      m_oif(oif),
      m_update(time)
{}

void
DRATableEntry::SetSatId(uint16_t satId)
{
    m_satId = satId;
}

uint16_t
DRATableEntry::GetSatId()
{
    return m_satId;
}

void
DRATableEntry::SetOif(int oif)
{
    m_oif = oif;
}

int
DRATableEntry::GetOif()
{
    return m_oif;
}

void
DRATableEntry::SetTime(ns3::Time time)
{
    m_update = time;
}

Time
DRATableEntry::GetTime()
{
    return m_update;
}

} // namespace ns3