//
// Created by hejun on 2023/8/14.
//

#include "dra-routing-helper.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("DRARoutingHelper");

namespace ns3
{

DRARoutingHelper::DRARoutingHelper()
{}

DRARoutingHelper::DRARoutingHelper(const ns3::DRARoutingHelper&)
{

}

DRARoutingHelper*
DRARoutingHelper::Copy() const
{
    return new DRARoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
DRARoutingHelper::Create(Ptr<Node> node) const
{
    int id = node->GetId();
    NS_LOG_LOGIC ("Adding DRARouting Protocol to node " << id);

    auto routing = CreateObject<DRARouting>();
    routing->SetId(id);
    node->AggregateObject(routing);

    return routing;
}

void
DRARoutingHelper::PopulateRoutingTables()
{}

void
DRARoutingHelper::RecomputeRoutingTables()
{}

uint32_t
DRARoutingHelper::AllocateRouterId()
{
    NS_LOG_FUNCTION_NOARGS ();
    static uint32_t routerId = 0;
    return routerId++;
}

} // namespace ns3