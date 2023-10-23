//
// Created by hejun on 2023/8/21.
//

#ifndef NS3_SIM_UTILS_H
#define NS3_SIM_UTILS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

using namespace std;
namespace ns3 {

void topo(NodeContainer &nodes, int P, int n, Ipv4RoutingHelper &routingHelper);

void mobility_100(NodeContainer& nodes);

void mobility_1584(NodeContainer& nodes);

void randomLinkError(NodeContainer& nodes, uint32_t limit);

void squareError(NodeContainer& nodes, uint32_t limit);
void satBreak(uint16_t id);

void AssignIp(Ptr<Node> node, pair<uint16_t , uint16_t> nodePair, int type);

void LinkSatAndGs(Ptr<Node> sat, Ptr<Node> gs, pair<uint16_t , uint16_t> satPair);
}


#endif // NS3_SIM_UTILS_H
