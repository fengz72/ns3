//
// Created by hejun on 2023/10/29.
//

#ifndef NS3_DSRAUTILS_H
#define NS3_DSRAUTILS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

using namespace std;
namespace ns3
{
class DsraUtils
{
  public:
    static void topo(NodeContainer &nodes, int P, int n, Ipv4RoutingHelper &routingHelper);

    static void mobility_1584(NodeContainer& nodes);

    static void randomLinkError(NodeContainer& nodes, uint32_t limit);

    static void squareError(NodeContainer& nodes, uint32_t limit);
    static void satBreak(uint16_t id);

    static uint16_t nextSat(uint16_t id, int oif);

    static void AssignIp(Ptr<Node> node, pair<uint16_t , uint16_t> nodePair, int type);

    static void LinkSatAndGs(Ptr<Node> sat, Ptr<Node> gs, pair<uint16_t , uint16_t> satPair);
};

} // namespace ns3

#endif // NS3_DSRAUTILS_H
