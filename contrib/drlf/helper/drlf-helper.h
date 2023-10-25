#ifndef DRLF_HELPER_H
#define DRLF_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/core-module.h"

namespace ns3
{

class DrlfRoutingHelper: public Ipv4RoutingHelper
{
  public:
    DrlfRoutingHelper();
    DrlfRoutingHelper(const DrlfRoutingHelper &);
    DrlfRoutingHelper* Copy () const override;
    Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const override;
    static void PopulateRoutingTables ();
    static void RecomputeRoutingTables ();
    static uint32_t AllocateRouterId ();

  private:
    DrlfRoutingHelper &operator = (const DrlfRoutingHelper &o);
};

}

#endif /* DRLF_HELPER_H */
