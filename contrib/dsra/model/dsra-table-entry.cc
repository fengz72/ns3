//
// Created by hejun on 2023/10/28.
//

#include "dsra-table-entry.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("DsraTableEntry");
NS_OBJECT_ENSURE_REGISTERED (DsraTableEntry);

TypeId
DsraTableEntry::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DsraTableEntry")
                            .SetParent<Object> ()
                            .AddConstructor<DsraTableEntry>();
    return tid;
}

DsraTableEntry::DsraTableEntry()
{
    m_oif = 0;
    m_satId = 0;
    m_update = Seconds(0.0);
}

} // namespace ns3