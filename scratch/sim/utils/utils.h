//
// Created by hejun on 2023/8/21.
//

#ifndef NS3_SIM_UTILS_H
#define NS3_SIM_UTILS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3 {

void topo(NodeContainer &nodes, int P, int n, Ipv4RoutingHelper &routingHelper);

void mobility_100(NodeContainer& nodes);

void mobility_1584(NodeContainer& nodes);

void randomLinkError(NodeContainer& nodes, uint32_t limit);
}


#endif // NS3_SIM_UTILS_H
