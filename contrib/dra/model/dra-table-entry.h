//
// Created by hejun on 2023/8/14.
//

#ifndef NS3_DRATABLEENTRY_H
#define NS3_DRATABLEENTRY_H

#include "ns3/nstime.h"
#include "ns3/core-module.h"

namespace ns3
{

class DRATableEntry: public Object
{
  public:
    static TypeId GetTypeId();
    DRATableEntry();
    DRATableEntry(uint16_t satId, int oif, Time time);
    void SetSatId(uint16_t satId);
    uint16_t GetSatId();
    void SetOif(int oif);
    int GetOif();
    void SetTime(Time time);
    Time GetTime();

  private:
    uint16_t m_satId;
    int m_oif;
    Time m_update;
};

} // namespace ns3

#endif // NS3_DRATABLEENTRY_H
