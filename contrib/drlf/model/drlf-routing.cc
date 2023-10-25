#include "drlf-routing.h"

#include "ns3/core-module.h"
#include "ns3/drlf-conf.h"
#include "ns3/drlf-tag.h"
#include "ns3/rng-seed-manager.h"

namespace ns3
{

using namespace std;

NS_LOG_COMPONENT_DEFINE ("DrlfRouting");
NS_OBJECT_ENSURE_REGISTERED (DrlfRouting);

DrlfRouting::DrlfRouting()
{}

TypeId
DrlfRouting::GetTypeId()
{
    static TypeId tid = TypeId ("ns3::DrlfRouting")
                            .SetParent<Object> ()
                            .AddConstructor<DrlfRouting>();
    return tid;
}

Ptr<Ipv4Route>
DrlfRouting::RouteOutput(Ptr<ns3::Packet> p, const ns3::Ipv4Header& header, Ptr<ns3::NetDevice> oif, Socket::SocketErrno& sockerr)
{
    NS_LOG_FUNCTION(this << p << header.GetDestination() << oif << sockerr);

    Ipv4Address dest = header.GetDestination();
    if (DrlfConfig::Instance()->GetTypeFromIp(dest) == DrlfConfig::GsNode && p->GetSize() < 100)
    {
        NS_LOG_LOGIC("过滤掉发送到gs的hello包");
        return nullptr;
    }

    // 查表得到下一跳
    auto route = LookUpTable(dest, -1);
    if (route)
    {
        sockerr = Socket::ERROR_NOTERROR;
        NS_LOG_INFO( Simulator::Now() << " node " << DrlfConfig::Instance()->GetSatPair(m_id) <<" send a packet "<< p << " form " << route->GetSource() << " to "<< header.GetDestination());
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }

    return route;
}

bool
DrlfRouting::RouteInput(Ptr<const ns3::Packet> p,
                        const ns3::Ipv4Header& header,
                        Ptr<const ns3::NetDevice> idev,
                        UnicastForwardCallback ucb,
                        MulticastForwardCallback mcb,
                        LocalDeliverCallback lcb,
                        ErrorCallback ecb)
{
    NS_LOG_FUNCTION (this << p << header << idev << &lcb << &ecb);

    if (!header.GetDestination().IsBroadcast()) {
        NS_LOG_INFO( Simulator::Now() << " node " << DrlfConfig::Instance()->GetSatPair(m_id) <<" receive a data packet "<< p << " , ttl " << (int)header.GetTtl() << ", form " << header.GetSource() << " to " << header.GetDestination() );
    }
    else
    {
        NS_LOG_LOGIC( Simulator::Now() << " node " << DrlfConfig::Instance()->GetSatPair(m_id) <<" receive a control packet "<< p << " , ttl " << (int)header.GetTtl() << ", form " << header.GetSource() << " to " << header.GetDestination() );
    }

    NS_ASSERT(m_ipv4);
    // Check if input device supports IP
    NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
    uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);

    // 接收端口
    Ipv4Address dest = header.GetDestination();
    bool retVal = false;
    retVal = m_ipv4->IsDestinationAddress(dest, iif);
    // 是发给本端口的包
    if (retVal)
    {
        // 广播包
        if(dest.IsBroadcast()){
            NS_LOG_LOGIC("Broadcast to " << dest);

            DrlfTag tag;
            bool found = p->PeekPacketTag(tag); // 是否含有tag, 只有信令包才含有tag
            if (!found)
            {
                lcb(p, header, iif);
            }
            else
            {
                HandleMessage(p, idev);
            }
        }
        // 单播包
        else
        {
            NS_LOG_LOGIC("Local delivery to " << dest);
            lcb(p, header, iif);
        }

        return true;
    }

    // 是发给本节点的包
    if (DrlfConfig::Instance()->GetSatId(dest) == m_id) {
        if (DrlfConfig::Instance()->GetTypeFromIp(dest) == DrlfConfig::SatelliteNode) {
            NS_LOG_LOGIC("Local delivery to " << dest);
            lcb(p, header, iif);
            return true;
        }
    }

    // Next, try to find a route
    Ptr<Ipv4Route> rtentry = LookUpTable(header.GetDestination(), iif);
    if (rtentry)
    {
        int oif = m_ipv4->GetInterfaceForDevice(rtentry->GetOutputDevice());
        NS_LOG_INFO("Found unicast destination- calling unicast callback, oif = " << oif);
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
DrlfRouting::NotifyInterfaceUp(uint32_t interface)
{}

void
DrlfRouting::NotifyInterfaceDown(uint32_t interface)
{}

void
DrlfRouting::NotifyAddAddress(uint32_t interface, ns3::Ipv4InterfaceAddress address)
{}

void
DrlfRouting::NotifyRemoveAddress(uint32_t interface, ns3::Ipv4InterfaceAddress address)
{}

void
DrlfRouting::SetIpv4(Ptr<ns3::Ipv4> ipv4)
{
    m_ipv4 = ipv4;
}

void
DrlfRouting::PrintRoutingTable(Ptr<ns3::OutputStreamWrapper> stream, Time::Unit unit) const
{
    NS_LOG_FUNCTION (this << stream);
}

void
DrlfRouting::DoDispose()
{
    Ipv4RoutingProtocol::DoDispose();
}


int
DrlfRouting::GetId()
{
    return m_id;
}

void
DrlfRouting::SetId(int id)
{
    m_id = id;
}

Ptr<Ipv4Route>
DrlfRouting::LookUpTable(Ipv4Address dest, int iif)
{
    NS_LOG_FUNCTION(this);
    pair<uint16_t, uint16_t > destPair = DrlfConfig::Instance()->GetSatPair(dest);
    uint16_t destId = DrlfConfig::Instance()->GetSatId(dest);
    pair<uint16_t, uint16_t > sourcePair = DrlfConfig::Instance()->GetSatPair(m_id);

    int type = DrlfConfig::Instance()->GetTypeFromIp(dest);
    int oif = -1;

    if (destPair == sourcePair)
    {
        if (type == DrlfConfig::GsNode)
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
        else if (type == DrlfConfig::SatelliteNode)
        {
            oif = 0;
        }
    }
    else //不是发送到当前卫星
    {
        Ptr<DrlfTableEntry> entry = m_table.GetEntry(destId);
        // 目的卫星不在路由表时, 计算下一跳
        if (entry == nullptr)
        {
            entry = m_table.CalcNextHop(destId);
        }
        // 计算下一跳也为空时
        if (entry == nullptr)
        {
            return nullptr;
        }

        oif = entry->GetOif();
    }

    NS_LOG_LOGIC("node " << m_id << ", destination " << dest << ", oif " << oif);
    Ptr<Ipv4Route> rtentry = Create<Ipv4Route> ();
    rtentry->SetDestination(dest);
    rtentry->SetSource(m_ipv4->GetAddress (oif, 0).GetLocal ());
    rtentry->SetGateway (Ipv4Address("0.0.0.0"));
    rtentry->SetOutputDevice (m_ipv4->GetNetDevice (oif));
    return rtentry;
}

void
DrlfRouting::HandleMessage(Ptr<const Packet> packet, Ptr<const NetDevice> idev)
{
    NS_LOG_FUNCTION(this);
    DrlfTag tag;
    packet->PeekPacketTag(tag);
    uint8_t type = tag.GetType();

    int iif = m_ipv4->GetInterfaceForDevice(idev);

    if (type == 0) //hello 包
    {
        double cost = (Simulator::Now() - tag.GetTime()).GetDouble();
        uint16_t from = tag.GetNode();
        NS_LOG_LOGIC("sat " << m_id << " receive a hello message, from sat: " << from);
        AddNeighbor(from, iif, Simulator::Now(), cost);

        // 记录最后收到hello包的时间
        DrlfConfig::Instance()->SetMaxHello(Simulator::Now());
    }
    else if (type == 1) // lsa包
    {
        uint16_t node = tag.GetNode();
        uint16_t lsaNode = tag.GetLsaNode();
        uint16_t minNode = min(node, lsaNode);
        uint16_t maxNode = max(node, lsaNode);
        tag.DeTTL();
        uint8_t ttl = tag.GetTTL();
        double cost = tag.GetCost();

        pair<uint16_t, uint16_t> myPair = make_pair(minNode, maxNode);
        // 防止收到自己广播出去的lsa
        if (node == m_id || m_linkStateDataBase.find(myPair) != m_linkStateDataBase.end()) {
            auto item = m_linkStateDataBase.find(myPair);
            item->second = max(item->second, cost);
            return;
        }

        m_linkStateDataBase.insert(make_pair(myPair, cost));

        // 如果ttl不为0, 继续广播
        if (ttl != 0) {
            Forward(tag);
        } else {
            // ttl = 0时, 记录最大lsa到达时间
            DrlfConfig::Instance()->SetMaxLsa(Simulator::Now());
        }
        NS_LOG_LOGIC("sat " << m_id << " receive a lsa message, node: " << node << ", lsaNode: " << lsaNode << ", ttl: " << (int)ttl);
    }
    else if (type == 2) // 链路损坏包
    {
        uint16_t node = tag.GetNode();
        uint16_t lsaNode = tag.GetLsaNode();
        tag.DeTTL();
        uint8_t ttl = tag.GetTTL();
        // 防止收到自己广播出去的error包
        if (node == m_id || lsaNode == m_id) {
            return;
        }

        auto error = make_pair(min(node, lsaNode), max(node, lsaNode));
        if (m_linkStateDataBase.find(error) != m_linkStateDataBase.end()) {
            m_linkStateDataBase.erase(error);
            RngSeedManager::SetSeed(10);
            Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            uint32_t random = uv->GetInteger(0, 10);
            Simulator::Schedule(Simulator::Now()
                                    + MicroSeconds(DrlfConfig::Instance()->GetCalculateCost())
                                    + MicroSeconds(random),
                                &DrlfRouting::Dijkstra,
                                this);
        }
        // 如果ttl不为0, 继续广播
        if (ttl != 0) {
            Forward(tag);
        }
    }
}

void
DrlfRouting::AddNeighbor(uint16_t id, int oif, Time time, double cost)
{
    NS_LOG_FUNCTION(this);
    Ptr<DrlfTableEntry> entry = CreateObject<DrlfTableEntry>();
    entry->SetId(id);
    entry->SetOif(oif);
    entry->SetTime(time);
    entry->SetCost(cost);
    m_neighbor[id] = entry;
}

void
DrlfRouting::CheckNeighbor()
{
    NS_LOG_FUNCTION(this);

    Time now = Simulator::Now();
    for (auto it = m_neighbor.begin(); it != m_neighbor.begin(); ++it)
    {
        if (now - it->second->GetTime() > Seconds(DrlfConfig::Instance()->GetUnavailableInterval()))
        {
            uint16_t node = it->second->GetId();
            m_neighbor.erase(it);
            SendError((uint16_t) m_id, node);
        }
    }
}

void
DrlfRouting::SendHello()
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet>(1);
    DrlfTag tag;
    Time now = Simulator::Now();
    tag.SetType(0);
    tag.SetNode(m_id);
    tag.SetTime(now);
    packet->AddPacketTag(tag);

    Ptr<Socket> m_socket = Socket::CreateSocket (DrlfConfig::Instance()->GetNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
}

void
DrlfRouting::Dijkstra()
{
    NS_LOG_FUNCTION(this << m_id);
    std::set<uint16_t> set;
    for (const auto& item : m_linkStateDataBase) {
        set.insert(item.first.first);
        set.insert(item.first.second);
    }

    int index = 0;
    map<uint16_t , int> idToIndex; // 存储id与index的映射
    vector<uint16_t> indexToId;
    for (auto it : set) {
        indexToId.push_back(it);
        idToIndex.insert(make_pair<>(static_cast<uint16_t>(it), index));
        index++;
    }

    int total = set.size();
    if (total == 0) {
        // 清空表
        m_table.Clear();
        NS_LOG_LOGIC(m_id << " not found node, dijkstra done");
        return;
    }

    double INF = 1e9;
    vector<double> dist(total, INF); // 距离
    vector<bool> visited(total, false);
    vector<uint16_t> previous(total); // previous[i] = j, 
    vector<vector<double>> g(total, vector<double>(total, INF));
    for (const auto& item : m_linkStateDataBase)
    {
        int first = idToIndex[item.first.first];
        int second = idToIndex[item.first.second];
        double cost = item.second;

        g[second][first] = cost;
        g[first][second] = cost;
    }

    int source = idToIndex[m_id];
    // 初始化距离和pre
    for (int i = 0; i < total; ++i) {
        dist[i] = g[source][i];
        previous[i] = source;
    }
    dist[source] = 0;
    visited[source] = true;

    for (int i = 1; i < total; ++i)
    {
        // 找出距离最小值的节点
        int k = 0;
        for (int j = 0; j < total; ++j)
        {
            if (!visited[j] && (k == 0 || dist[j] < dist[k]))
            {
                k = j;
            }
        }

        visited[k] = true;

        // 以距离最近的节点为中转节点, 更新节点的距离
        for (int j = 0; j < total; ++j)
        {
            if (dist[k] + g[k][j] < dist[j]) {
                dist[j] = dist[k] + g[k][j];
                previous[j] = k;
            }
        }
    }

    // 清空表
    m_table.Clear();

    // 将计算结果添加到路由表
    for (int i = 0; i < total; ++i)
    {
        uint16_t dest = indexToId[i];

        if (i == source) continue;
        if (dist[i] >= INF)
        {
            NS_LOG_LOGIC("node: " << m_id << " , dest: " << dest << " , can't find a way");
            continue;
        }



        // 找到下一跳
        int p = i;
        while (previous[p] != source) p = previous[p];
        uint16_t nextHop = indexToId[p];
        if (m_neighbor.find(nextHop) == m_neighbor.end()) {
            NS_LOG_LOGIC("node: " << dest << " , nextHop: " << nextHop << " , can't find in neighbor");
            continue;
        }

        auto nextHopEntry = m_neighbor[nextHop];
        int oif = nextHopEntry->GetOif();
        double cost = dist[i];

        m_table.AddEntry(dest, oif, cost);
    }

    NS_LOG_LOGIC(m_id << " dijkstra done");

//    auto start_time = chrono::system_clock::now();
//    auto end_time = chrono::system_clock::now();
//    long duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
//    DrlfConfig::Instance()->m_calcCost += duration;
}

void
DrlfRouting::SendLSA(uint8_t type, uint16_t node, uint16_t lsaNode, double cost)
{
    Time now = Simulator::Now();
    DrlfTag tag;
    tag.SetType(type);
    tag.SetNode(node);
    tag.SetTTL(DrlfConfig::Instance()->m_ttl);
    tag.SetTime(now);
    tag.SetCost(cost);
    tag.SetLsaNode(lsaNode);

    Forward(tag);
}

void
DrlfRouting::Forward(DrlfTag& tag)
{
    Ptr<Packet> packet = Create<Packet>(1);
    packet->AddPacketTag(tag);
    Ptr<Socket> m_socket = Socket::CreateSocket (DrlfConfig::Instance()->GetNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
}

void
DrlfRouting::SendError(uint16_t node, uint16_t lsaNode)
{
    DrlfTag tag;
    tag.SetType(2);
    tag.SetNode(node);
    tag.SetTTL(DrlfConfig::Instance()->m_ttl);
    tag.SetLsaNode(lsaNode);

    Forward(tag);
}

void
DrlfRouting::SendAllLSA()
{
    for (auto it: m_neighbor)
    {
        uint16_t node = it.second->GetId();
        double cost = it.second->GetCost();
        // 将 neighbor 加入链路状态数据库
        m_linkStateDataBase.insert(make_pair(make_pair(m_id, node), cost));
        SendLSA(1, (uint16_t) m_id, node, cost);
    }
}

void
DrlfRouting::AddGs(uint32_t ip, int oif)
{
    m_gs[ip] = oif;
}

}
