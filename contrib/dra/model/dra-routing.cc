//
// Created by hejun on 2023/8/11.
//

#include "dra-routing.h"

#include "dra-config.h"
#include "dra-tag.h"

#include "ns3/core-module.h"
#include "ns3/ipv4.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("DRARouting");
NS_OBJECT_ENSURE_REGISTERED (DRARouting);

TypeId
DRARouting::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DRARouting")
                            .SetParent<Object> ()
                            .AddConstructor<DRARouting>();
    return tid;
}

DRARouting::DRARouting()
{
    NS_LOG_FUNCTION(this);
}

DRARouting::~DRARouting()
{
    NS_LOG_FUNCTION (this);
}

Ptr<Ipv4Route>
DRARouting::RouteOutput(Ptr<ns3::Packet> p,
                        const ns3::Ipv4Header& header,
                        Ptr<ns3::NetDevice> oif,
                        Socket::SocketErrno& sockerr)
{
    NS_LOG_FUNCTION(this << p << header.GetDestination() << oif << sockerr);

    // 统计每个节点发送了多少包
    DRAConfLoader::Instance()->incrementSendPacket(m_id);
    // 发送的数据包
    DRAConfLoader::Instance()->incrementSendDataPacket(m_id);

    NS_LOG_LOGIC ("Unicast destination- looking up");

    auto route = LookUpTable(header.GetDestination(), -1);
    if (route)
    {
        sockerr = Socket::ERROR_NOTERROR;
        NS_LOG_LOGIC( Simulator::Now() << " node " << m_id <<" send a packet "<< p->GetSize() << " form " << route->GetSource() << " to "<< header.GetDestination());
        NS_LOG_INFO("DraRouting::RouteOutput");
        NS_LOG_INFO( Simulator::Now() << " node " << DRAConfLoader::Instance()->GetSatPair(m_id) <<" send a packet "<< p << " form " << route->GetSource() << " to "<< header.GetDestination());
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }

    return route;
}

bool
DRARouting::RouteInput(Ptr<const ns3::Packet> p,
                       const ns3::Ipv4Header& header,
                       Ptr<const ns3::NetDevice> idev,
                       ns3::Ipv4RoutingProtocol::UnicastForwardCallback ucb,
                       ns3::Ipv4RoutingProtocol::MulticastForwardCallback mcb,
                       ns3::Ipv4RoutingProtocol::LocalDeliverCallback lcb,
                       ns3::Ipv4RoutingProtocol::ErrorCallback ecb)
{
    NS_LOG_FUNCTION (this << p << header << idev << &lcb << &ecb);

    NS_LOG_LOGIC( Simulator::Now() << " node " << m_id <<" receive a packet\t"<< p->GetSize() << "\t" << header.GetSource() << "\t"<<header.GetDestination() );
    if (!header.GetDestination().IsBroadcast()) {
        NS_LOG_INFO("DraRouting::RouteInput");
        NS_LOG_INFO( Simulator::Now() << " node " << DRAConfLoader::Instance()->GetSatPair(m_id) <<" receive a packet "<< p << " , ttl " << (int)header.GetTtl() << ", form " << header.GetSource() << " to " << header.GetDestination() );
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
        // 统计接收的包
        DRAConfLoader::Instance()->incrementSuccessPacket(m_id);
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

            DRATag tag;
            bool found = p->PeekPacketTag(tag); // 是否含有tag, 只有信令包才含有tag
            if (!found)
            {
                lcb(p, header, iif);
            }
            else
            {
                HandleMessage(p, idev);
            }

            return true;
        }
        // 发送给上层
        else
        {
            NS_LOG_LOGIC("Local delivery to " << header.GetDestination());
            NS_LOG_INFO("Local delivery to " << header.GetDestination());

            // 接收的数据包
            DRAConfLoader::Instance()->incrementSuccessDataPacket(m_id);

            lcb(p, header, iif);
            return true;
        }
    }

    // 是发送给当前卫星的
    if (DRAConfLoader::Instance()->GetSatId(header.GetDestination()) == m_id
        && DRAConfLoader::Instance()->GetType(header.GetDestination()) == SatelliteNode) {
        // 接收的数据包
        DRAConfLoader::Instance()->incrementSuccessDataPacket(m_id);

        lcb(p, header, iif);
        NS_LOG_INFO("Local delivery to " << header.GetDestination());
        return true;
    }

    // Next, try to find a route
    NS_LOG_LOGIC("Unicast destination- looking up route");
    Ptr<Ipv4Route> rtentry = LookUpTable(header.GetDestination(),
                                         iif);
    if (rtentry)
    {
        NS_LOG_LOGIC("Found unicast destination- calling unicast callback");
        NS_LOG_INFO( Simulator::Now() << " node " << DRAConfLoader::Instance()->GetSatPair(m_id) <<" send a packet "<< p->GetSize() << " bytes form " << rtentry->GetSource() << " to "<< rtentry->GetDestination());
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

void
DRARouting::HandleMessage(Ptr<const ns3::Packet> packet,
                          Ptr<const ns3::NetDevice> idev)
{
    NS_LOG_FUNCTION(this);
    DRATag tag;
    packet->PeekPacketTag(tag);

    uint8_t type = tag.GetType();
    uint16_t from = tag.GetNode();

    if (type == 1)
    {
        NS_LOG_LOGIC("node " << m_id << " receive a hello message, from node " << from);
        int iif = m_ipv4->GetInterfaceForDevice(idev);
        AddNeighbor(from, iif, Simulator::Now());
    }
}

Ptr<Ipv4Route>
DRARouting::LookUpTable(Ipv4Address dest, int iif)
{
    NS_LOG_FUNCTION(this << dest);
    std::pair<uint16_t, uint16_t> destSatPair = DRAConfLoader::Instance()->GetSatPair(dest);
    std::pair<uint16_t, uint16_t> sourceSatPair = DRAConfLoader::Instance()->GetSatPair(m_id);

    int type = DRAConfLoader::Instance()->GetType(dest);

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
        std::vector<int> oif_list;
        calcOif(sourceSatPair, destSatPair, oif_list);

        for (const auto& item : oif_list)
        {
            if (m_table.find(item) != m_table.end() && item != iif)
            {
                oif = item;
                break;
            }
        }
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

/**
 * 优先向长边. 相同时优先轨内.
 * @param source
 * @param dest
 * @param res
 */
void
DRARouting::calcOif(std::pair<uint16_t, uint16_t> source,
                    std::pair<uint16_t, uint16_t> dest,
                    std::vector<int> &res)
{
    NS_LOG_FUNCTION(this);

    uint16_t orbits = DRAConfLoader::Instance()->getOrbits();
    uint16_t sats = DRAConfLoader::Instance()->getSats();

    int d_down = (dest.second - source.second + sats) % sats; // 向下的距离
    int d_up = sats - d_down; // 向上的距离
    int d_y = min(d_down, d_up); // y方向上的最小距离
    int oif_y = (d_up <= d_down)? Up: Down; // 选择y方向上的哪个端口

    int d_right = (dest.first - source.first + orbits) % orbits; // 向右的距离
    int d_left = orbits - d_right; // 向左的距离
    int d_x = min(d_left, d_right); // x方向上的最小距离
    int oif_x = (d_left <= d_right)? Left: Right; // 选择x方向上的哪个端口

    if (d_y >= d_x)
    {
        res.push_back(oif_y);
        res.push_back(oif_x);
    }
    else
    {
        res.push_back(oif_x);
        res.push_back(oif_y);
    }
//
//
//    uint16_t curOrbit = source.first;
//    uint16_t curSat = source.second;
//
//    uint16_t destOrbit = dest.first;
//    uint16_t destSat = dest.second;
//
//
//
//    uint16_t orbitDelta = (destOrbit - curOrbit + orbits) % orbits;
//    uint16_t satDelta = (destSat - curSat + sats) % sats;
//
//    if (curOrbit == destOrbit) {
//        // 目的卫星在当前卫星下面
//        if (destSat > curSat && (destSat - curSat) <= sats / 2) {
//            res.push_back(3);
//        }else { // 目的卫星在上面
//            res.push_back(1);
//        }
//        res.push_back(2);
//        res.push_back(4);
//    }
//    else if (curSat == destSat)
//    {
//        // 目的卫星在右面
//        if (destOrbit > curOrbit && (destOrbit - curOrbit) <= orbits / 2) {
//            res.push_back(2);
//        } else { // 目的卫星在左面
//            res.push_back(4);
//        }
//        res.push_back(1);
//        res.push_back(3);
//    }
//    else if (orbitDelta <= orbits / 2 && satDelta <= sats / 2)
//    {
//        // 左下
//        res.push_back(3);
//        res.push_back(4);
//    }
//    else if (orbitDelta <= orbits / 2 && satDelta > sats / 2)
//    {
//        // 左上
//        res.push_back(1);
//        res.push_back(4);
//    }
//    else if (orbitDelta > orbits / 2 && satDelta <= sats / 2)
//    {
//        // 右下
//        res.push_back(3);
//        res.push_back(2);
//
//    }
//    else if (orbitDelta > orbits / 2 && satDelta > sats / 2)
//    {
//        // 右上
//        res.push_back(1);
//        res.push_back(2);
//    }
}

void
DRARouting::SendHello()
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet>(1);
    DRATag tag;
    tag.SetType(1);
    tag.SetNode(m_id);
    tag.SetTTL(64);
    packet->AddPacketTag(tag);

    Ptr<Socket> m_socket = Socket::CreateSocket (DRAConfLoader::Instance()->getNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);

    NS_LOG_LOGIC(m_id << " send a hello message");

    // 向周围四个节点发送
    for (int i = 0; i < 4; ++i)
    {
        // 统计每个节点发送了多少包
        DRAConfLoader::Instance()->incrementSendPacket(m_id);
        // 发送的信令包
        DRAConfLoader::Instance()->incrementSendContrPacket(m_id);
        // 发送的 hello 包
        DRAConfLoader::Instance()->incrementSendHelloPacket(m_id);
    }
}

void
DRARouting::AddNeighbor(uint16_t id, int iif, Time time)
{
    NS_LOG_FUNCTION(this);
    Ptr<DRATableEntry> entry = CreateObject<DRATableEntry>(id, iif, time);
    m_table[iif] = entry;
}

void
DRARouting::CheckNeighbor()
{
    NS_LOG_FUNCTION(this);

    Time now = Simulator::Now();
    for (auto it = m_table.begin(); it != m_table.end(); ++it)
    {
        if (now - it->second->GetTime() > Seconds(DRAConfLoader::Instance()->getUnavailableInterval())){
            m_table.erase(it);
        }
    }
}

void
DRARouting::AddGs(uint32_t ip, int oif)
{
    m_gs[ip] = oif;
}

void
DRARouting::SetIpv4(Ptr<ns3::Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void
DRARouting::DoDispose()
{
    Ipv4RoutingProtocol::DoDispose();
}

void
DRARouting::NotifyInterfaceUp(uint32_t interface)
{}

void
DRARouting::NotifyInterfaceDown(uint32_t interface)
{}

void
DRARouting::NotifyAddAddress(uint32_t interface, ns3::Ipv4InterfaceAddress address)
{}

void
DRARouting::NotifyRemoveAddress(uint32_t interface, ns3::Ipv4InterfaceAddress address)
{}

void
DRARouting::PrintRoutingTable(Ptr<ns3::OutputStreamWrapper> stream, Time::Unit unit) const
{
    NS_LOG_FUNCTION (this << stream);
}

void
DRARouting::SetId(uint16_t id)
{
    m_id = id;
}

uint16_t
DRARouting::GetId()
{
    return m_id;
}


} // namespace ns3