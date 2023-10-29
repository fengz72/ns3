#ifndef DSRA_HELPER_H
#define DSRA_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"

namespace ns3
{
class DsraRoutingHelper: public Ipv4RoutingHelper
{
  public:
    DsraRoutingHelper();
    DsraRoutingHelper(const DsraRoutingHelper&);
    DsraRoutingHelper* Copy() const;
    Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;
    static void PopulateRoutingTables ();
    static void RecomputeRoutingTables ();
    static uint32_t AllocateRouterId ();

  private:
    DsraRoutingHelper &operator = (const DsraRoutingHelper& o);
};

}

#endif /* DSRA_HELPER_H */
