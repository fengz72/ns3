//
// Created by hejun on 2023/8/14.
//

#ifndef NS3_DRAROUTINGHELPER_H
#define NS3_DRAROUTINGHELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/dra-routing.h"

namespace ns3
{

class DRARoutingHelper: public Ipv4RoutingHelper
{
  public:
    DRARoutingHelper();
    DRARoutingHelper(const DRARoutingHelper &);
    DRARoutingHelper* Copy (void) const;
    virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;
    static void PopulateRoutingTables (void);
    static void RecomputeRoutingTables (void);
    static uint32_t AllocateRouterId ();

  private:
    DRARoutingHelper &operator = (const DRARoutingHelper &o);
};

} // namespace ns3

#endif // NS3_DRAROUTINGHELPER_H
