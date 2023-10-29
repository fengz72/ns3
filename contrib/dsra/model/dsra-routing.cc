#include "dsra-routing.h"
#include "dsra-config.h"
#include "dsra-tag.h"

namespace ns3
{
using namespace std;
NS_LOG_COMPONENT_DEFINE ("DsraRouting");
NS_OBJECT_ENSURE_REGISTERED (DsraRouting);

TypeId
DsraRouting::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DsraRouting")
                            .SetParent<Object>()
                            .AddConstructor<DsraRouting>();
    return tid;
}

DsraRouting::DsraRouting()
{}

DsraRouting::~DsraRouting()
{}

Ptr<Ipv4Route>
DsraRouting::RouteOutput(Ptr<ns3::Packet> p,
                         const ns3::Ipv4Header& header,
                         Ptr<ns3::NetDevice> oif,
                         Socket::SocketErrno& sockerr)
{
    NS_LOG_FUNCTION(this << p << header.GetDestination() << oif << sockerr);

    auto route = LookUpNext(header.GetDestination(), -1);
    if (route)
    {
        sockerr = Socket::ERROR_NOTERROR;
        NS_LOG_INFO( Simulator::Now() << " node " << DsraConfig::Instance()->GetSatPair(m_id) <<" send a packet "<< p << " form " << route->GetSource() << " to "<< header.GetDestination());
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }

    return route;
}

bool
DsraRouting::RouteInput(Ptr<const ns3::Packet> p, const ns3::Ipv4Header& header, Ptr<const ns3::NetDevice> idev, ns3::Ipv4RoutingProtocol::UnicastForwardCallback ucb, ns3::Ipv4RoutingProtocol::MulticastForwardCallback mcb, ns3::Ipv4RoutingProtocol::LocalDeliverCallback lcb, ns3::Ipv4RoutingProtocol::ErrorCallback ecb)
{
    NS_LOG_FUNCTION (this << p << header << idev << &lcb << &ecb);
    if (!header.GetDestination().IsBroadcast()) {
        NS_LOG_INFO( Simulator::Now() << " node " << DsraConfig::Instance()->GetSatPair(m_id) <<" receive a packet "<< p << " , ttl " << (int)header.GetTtl() << ", form " << header.GetSource() << " to " << header.GetDestination() );
    }

    NS_ASSERT(m_ipv4);
    // Check if input device supports IP
    NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
    uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);

    bool retVal = false;
    retVal = m_ipv4->IsDestinationAddress(header.GetDestination(), iif);
    // 是发送给当前节点的包
    if (retVal)
    {
        NS_LOG_LOGIC("Address " << header.GetDestination() << " is a match for local delivery");
        // 多播
        if (header.GetDestination().IsMulticast())
        {
            Ptr<Packet> packetCopy = p->Copy();
            lcb(packetCopy, header, iif);
            retVal = true;
            // Fall through
        }
        // 广播
        else if(header.GetDestination().IsBroadcast()){
            NS_LOG_LOGIC("Broadcast to " << header.GetDestination());

            DsraTag tag;
            bool found = p->PeekPacketTag(tag); // 是否含有tag, 只有信令包才含有tag
            if (found)
            {
                HandleMessage(p, idev);
            }

            return true;
        }
        // 发送给上层
        else
        {
            NS_LOG_INFO("Local delivery to " << header.GetDestination());
            lcb(p, header, iif);
            return true;
        }
    }

    // 是发送给当前卫星的
    if (DsraConfig::Instance()->GetSatId(header.GetDestination()) == m_id
        && DsraConfig::Instance()->GetType(header.GetDestination()) == SatelliteNode) {
        lcb(p, header, iif);
        NS_LOG_INFO("Local delivery to " << header.GetDestination());
        return true;
    }

    // Next, try to find a route
    NS_LOG_LOGIC("Unicast destination- looking up route");
    Ptr<Ipv4Route> rtentry = LookUpNext(header.GetDestination(), iif);

    if (rtentry)
    {
        NS_LOG_INFO( Simulator::Now() << " node " << DsraConfig::Instance()->GetSatPair(m_id) <<" send a packet "<< p->GetSize() << " bytes form " << rtentry->GetSource() << " to "<< rtentry->GetDestination());
        ucb(rtentry, p, header);
        return true;
    }
    else
    {
        NS_LOG_LOGIC("Did not find unicast destination- returning false");
        return false; // Let other routing protocols try to handle this
                      // route request.
    }
}

Ptr<Ipv4Route>
DsraRouting::LookUpNext(ns3::Ipv4Address dest, int iif)
{
    NS_LOG_FUNCTION(this << dest << iif);
    std::pair<uint16_t, uint16_t> destSatPair = DsraConfig::Instance()->GetSatPair(dest);
    std::pair<uint16_t, uint16_t> sourceSatPair = DsraConfig::Instance()->GetSatPair(m_id);
    int type = DsraConfig::Instance()->GetType(dest);

    int oif = -1;
    // 目的地为本卫星
    if (destSatPair == sourceSatPair)
    {
        if (type == GsNode)
        {
            if (m_gs.find(dest.Get()) != m_gs.end())
            {
                oif = m_gs.find(dest.Get())->second;
            }
            else
            {
                NS_LOG_LOGIC("node " << m_id << " 无法找到地面站 " << dest << " 的路由");
                return nullptr;
            }
        }
        else if (type == SatelliteNode)
        {
            oif = 0;
        }
    }
    else
    {
        std::vector<int> master, slave;
        CalcPort(DsraConfig::Instance()->GetSatPair(dest), master, slave);

        oif = NextHop(destSatPair, master, slave, iif);
    }


    if(oif == -1){
        NS_LOG_LOGIC("No route found!");
        return nullptr;
    }

    NS_LOG_LOGIC("node " << m_id << ", destination " << dest << ", oif " << oif);
    Ptr<Ipv4Route> rtentry = Create<Ipv4Route> ();
    rtentry->SetDestination(dest);
    rtentry->SetSource(m_ipv4->GetAddress(oif, 0).GetLocal());
    rtentry->SetGateway (Ipv4Address("0.0.0.0"));
    rtentry->SetOutputDevice (m_ipv4->GetNetDevice (oif));
    return rtentry;
}

void
DsraRouting::CalcPort(std::pair<uint16_t, uint16_t> dest, std::vector<int>& master, std::vector<int>& slave)
{
    NS_LOG_FUNCTION(this);

    master.clear();
    slave.clear();

    std::pair<uint16_t, uint16_t> source = DsraConfig::Instance()->GetSatPair(m_id);
    uint16_t P = DsraConfig::Instance()->m_P;
    uint16_t S = DsraConfig::Instance()->m_S;

    int d_down = (S + dest.second - source.second ) % S; // 向下的距离
    int d_up = S - d_down; // 向上的距离
    int oif_y = (d_up <= d_down)? Up: Down; // 选择y方向上的哪个端口

    int d_right = (P + dest.first - source.first) % P; // 向右的距离
    int d_left = P - d_right; // 向左的距离
    int oif_x = (d_left <= d_right)? Left: Right; // 选择x方向上的哪个端口

    if (source.first == dest.first) // 同轨道
    {
        master.push_back(oif_y);

        slave.push_back(Right);
        slave.push_back(Left);
        return;
    }
    else // 不同轨道
    {
        master.push_back(oif_x);

        if (dest.second == source.second) // 同一平面
        {
            slave.push_back(Up);
            slave.push_back(Down);
            return;
        }
        else
        {
            master.push_back(oif_y);

            slave.push_back((oif_y == Up) ? Down : Up);
            slave.push_back((oif_x == Right) ? Left : Right);
        }
    }
}

int
DsraRouting::NextHop(std::pair<uint16_t, uint16_t> dest, std::vector<int>& master, std::vector<int>& slave, int iif)
{
    NS_LOG_FUNCTION(this);
    std::pair<uint16_t, uint16_t> source = DsraConfig::Instance()->GetSatPair(m_id);

    int cost[5] = {0};
    if (master.size() == 1)
    {
        for (auto it = master.begin(); it != master.end(); ++it)
        {
            int cur = *it;

            if (cur != iif) cost[cur] |= 1 << 3;
            if (!DsraConfig::Instance()->IsBlock(source, dest, cur)) cost[cur] |= 1 << 2;
            cost[cur] |= 1 << 1;
        }

        for (auto it = slave.begin(); it != slave.end(); ++it)
        {
            int cur = *it;

            if (cur != iif) cost[cur] |= 1 << 3;
            if (!DsraConfig::Instance()->IsBlock(source, dest, cur)) cost[cur] |= 1 << 2;
            cost[cur] |= 1;
        }
    }
    else
    {
        for (auto it = master.begin(); it != master.end(); ++it)
        {
            int cur = *it;

            if (cur != iif) cost[cur] |= 1 << 2;
            cost[cur] |= 1 << 1;
            if (!DsraConfig::Instance()->IsBlock(source, dest, cur)) cost[cur] |= 1;

        }

        for (auto it = slave.begin(); it != slave.end(); ++it)
        {
            int cur = *it;

            if (cur != iif) cost[cur] |= 1 << 2;
            if (!DsraConfig::Instance()->IsBlock(source, dest, cur)) cost[cur] |= 1;
        }
    }

    int oif = -1, value = 0;
    for (int i = 1; i <= 4; ++i)
    {
        if (m_neighbor.find(i) != m_neighbor.end() && cost[i] > value)
        {
            oif = i;
            value = cost[i];
        }
    }

    return oif;
}

void
DsraRouting::HandleMessage(Ptr<const ns3::Packet> packet, Ptr<const ns3::NetDevice> idev)
{
    NS_LOG_FUNCTION(this);
    DsraTag tag;
    packet->PeekPacketTag(tag);
    uint8_t type = tag.GetType();
    int iif = m_ipv4->GetInterfaceForDevice(idev);

    if (type == 0)
    {
        uint16_t from = tag.GetNode();
        NS_LOG_LOGIC("sat " << m_id << " receive a hello message, from sat: " << from);

        AddNeighbor(from, Simulator::Now(), iif);
    }
    else if (type == 2)
    {
        uint16_t node = tag.GetNode();
        uint16_t lsaNode = tag.GetErrorNode();

        tag.DeTTL();
        uint8_t ttl = tag.GetTTL();
        // 防止收到自己广播出去的error包
        if (node == m_id || lsaNode == m_id) {
            return;
        }

        auto error = make_pair(min(node, lsaNode), max(node, lsaNode));
        auto block = DsraConfig::Instance()->m_block;
        if (block.find(error) != block.end())
        {
            block.erase(error);
        }

        if (ttl != 0)
        {
            Forward(tag);
        }
    }
}

void
DsraRouting::CheckNeighbor()
{
    NS_LOG_FUNCTION(this);

    Time now = Simulator::Now();
    for (auto it = m_neighbor.begin(); it != m_neighbor.begin(); ++it)
    {
        if (now - it->second->m_update > Seconds(DsraConfig::Instance()->m_UnavailableInterval))
        {
            uint16_t node = it->second->m_satId;
            m_neighbor.erase(it);
            SendError((uint16_t)m_id, node);
        }
    }
}

void
DsraRouting::AddNeighbor(uint16_t id, ns3::Time time, int oif)
{
    NS_LOG_FUNCTION(this);
    Ptr<DsraTableEntry> entry = CreateObject<DsraTableEntry>();
    entry->m_satId = id;
    entry->m_oif = oif;
    entry->m_update = time;

    m_neighbor[oif] = entry;
}

void
DsraRouting::SendHello()
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet>(1);
    DsraTag tag;
    tag.SetType(0);
    tag.SetNode(m_id);
    packet->AddPacketTag(tag);

    Ptr<Socket> m_socket = Socket::CreateSocket (DsraConfig::Instance()->GetNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
}

void
DsraRouting::Forward(ns3::DsraTag& tag)
{
    Ptr<Packet> packet = Create<Packet>(1);
    packet->AddPacketTag(tag);
    Ptr<Socket> m_socket = Socket::CreateSocket (DsraConfig::Instance()->GetNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
}

void
DsraRouting::SendError(uint16_t node, uint16_t lsaNode)
{
    DsraTag tag;
    tag.SetType(2);
    tag.SetNode(node);
    tag.SetTTL(DsraConfig::Instance()->m_ttl);
    tag.SetErrorNode(lsaNode);
}

void
DsraRouting::AddGs(uint32_t ip, int oif)
{
    m_gs[ip] = oif;
}

void
DsraRouting::SetId(uint32_t id)
{
    m_id = id;
}

void
DsraRouting::SetIpv4(Ptr<ns3::Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void
DsraRouting::DoDispose()
{
    Ipv4RoutingProtocol::DoDispose();
}

void
DsraRouting::NotifyInterfaceUp(uint32_t interface)
{
}

void
DsraRouting::NotifyInterfaceDown(uint32_t interface)
{
}

void
DsraRouting::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
DsraRouting::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
DsraRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
}

}
