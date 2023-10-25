#include "drlf-helper.h"
#include "ns3/node.h"
#include "ns3/drlf-routing.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("DrlfRoutingHelper");

DrlfRoutingHelper::DrlfRoutingHelper()
{}

DrlfRoutingHelper::DrlfRoutingHelper(const ns3::DrlfRoutingHelper&)
{}

DrlfRoutingHelper*
DrlfRoutingHelper::Copy() const
{
    return new DrlfRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
DrlfRoutingHelper::Create(Ptr<Node> node) const
{
    int id = node->GetId();
    NS_LOG_LOGIC ("Adding DrlfRouting Protocol to node " << id);

    auto routing = CreateObject<DrlfRouting>();
    routing->SetId(id);
    node->AggregateObject(routing);

    return routing;
}

void
DrlfRoutingHelper::PopulateRoutingTables()
{}

void
DrlfRoutingHelper::RecomputeRoutingTables()
{}

uint32_t
DrlfRoutingHelper::AllocateRouterId()
{
    NS_LOG_FUNCTION_NOARGS ();
    static uint32_t routerId = 0;
    return routerId++;
}

}
