//
// Created by hejun on 2023/10/28.
//

#ifndef NS3_DSRATABLEENTRY_H
#define NS3_DSRATABLEENTRY_H

#include "ns3/core-module.h"

namespace ns3
{

class DsraTableEntry: public Object
{
  public:
    static TypeId GetTypeId();
    DsraTableEntry();

    uint16_t m_satId;
    int m_oif;
    Time m_update;
};

} // namespace ns3

#endif // NS3_DSRATABLEENTRY_H
