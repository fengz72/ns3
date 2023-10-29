#include "dsra-helper.h"

#include "ns3/dsra-routing.h"
NS_LOG_COMPONENT_DEFINE("DsraRoutingHelper");

namespace ns3
{
DsraRoutingHelper::DsraRoutingHelper()
{}

DsraRoutingHelper::DsraRoutingHelper(const ns3::DsraRoutingHelper&)
{}

DsraRoutingHelper*
DsraRoutingHelper::Copy() const
{
    return new DsraRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
DsraRoutingHelper::Create(Ptr<ns3::Node> node) const
{
    uint32_t id = node->GetId();
    NS_LOG_LOGIC ("Adding DsraRouting Protocol to node " << id);
    
    auto routing = CreateObject<DsraRouting>();
    routing->SetId(id);
    node->AggregateObject(routing);

    return routing;
}

void
DsraRoutingHelper::PopulateRoutingTables()
{}

void
DsraRoutingHelper::RecomputeRoutingTables()
{}

uint32_t
DsraRoutingHelper::AllocateRouterId()
{
    NS_LOG_FUNCTION_NOARGS ();
    static uint32_t routerId = 0;
    return routerId++;
}
}
