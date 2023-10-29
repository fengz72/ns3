#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsra-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/mpi-module.h"
#include "ns3/mobility-module.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DsraMain2");

map<uint64_t, pair<int64_t , int64_t >> m_map;

void Hello(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DsraConfig::Instance()->GetNodeContainer().GetN(); i++){
        DsraConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DsraRouting>()->SendHello();
    }
}

void CheckNeighbor(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DsraConfig::Instance()->GetNodeContainer().GetN(); i++){
        DsraConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DsraRouting>()->CheckNeighbor();
    }
}

/// 接收回调
void RxPacketCall(std::string context,Ptr<const Packet> packet, const Address &address){
    // 排除hello包
    if (packet->GetSize() <= 100)
    {
        return;
    }

    NS_LOG_LOGIC("At time " << Simulator::Now ().GetSeconds ()
                            << "s received "
                            <<  packet->GetSize () << " bytes " << "uid = " << packet->GetUid()
                            << " from " << InetSocketAddress::ConvertFrom(address).GetIpv4 ()
                            << " port " << InetSocketAddress::ConvertFrom (address).GetPort ()
    );

    uint64_t uid = packet->GetUid();
    int64_t now = Simulator::Now().GetMicroSeconds();

    if (m_map.find(uid) != m_map.end())
    {
        m_map[uid].second = now;
    }
}

void
SetServer(Ptr<Node> node, int port, float stop)
{
    NS_LOG_FUNCTION("Set Server at node " << node);
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                          Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

    ApplicationContainer apps = sink.Install(node);
    apps.Start(Seconds(0));
    apps.Stop(Seconds(stop));

    std::ostringstream oss;
    oss << "/NodeList/"
        << node->GetId() <<"/ApplicationList/*"
        << "/$ns3::PacketSink/Rx";
    Config::Connect (oss.str (), MakeCallback (&RxPacketCall));
}

void SetAllServer(int port, float stop)
{
    NS_LOG_FUNCTION("Set Server at all node");
    uint32_t n = DsraConfig::Instance()->GetNodeContainer().GetN();
    for (uint32_t i = 0; i < n; ++i)
    {
        SetServer(DsraConfig::Instance()->GetNodeContainer().Get(i), port, stop);
    }
}

/// 发送回调
void TxPacketCall(std::string context,Ptr<const Packet> packet){
    NS_LOG_LOGIC("At time " << Simulator::Now ().GetSeconds ()
                            << "s send a packet "
                            <<  packet->GetSize () << " bytes, uid = " << packet->GetUid()
    );

    int64_t now = Simulator::Now().GetMicroSeconds();
    uint64_t uid = packet->GetUid();

    m_map[uid] = make_pair(now, -1);
}

void
SetSatClient(int source, int dest, int port, string rate, int size, float start, float stop)
{
    NS_LOG_FUNCTION("Create Applications" << source << dest << port << rate << size << start << stop);
    OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                            Address(InetSocketAddress(DsraConfig::Instance()
                                                          ->GetNodeContainer()
                                                          .Get(dest)
                                                          ->GetObject<Ipv4>()
                                                          ->GetAddress(1, 0)
                                                          .GetLocal(),
                                                      port)));
    onOffHelper.SetConstantRate(DataRate(rate), size);
    ApplicationContainer apps = onOffHelper.Install(DsraConfig::Instance()->GetNodeContainer().Get(source));
    apps.Start(Seconds(start));
    apps.Stop(Seconds(stop));

    //    std::ostringstream oss;
    //    oss << "/NodeList/"
    //        << node->GetId() <<"/ApplicationList/*"
    //        << "/$ns3::PacketSink/Rx";
    //    NS_LOG_LOGIC(oss.str());
    //    Config::Connect (oss.str (), MakeCallback (&RxPacketCall));
}

void
SetGsClient(Ptr<Node> source, Ptr<Node> dest, int destIndex, int port, string rate, int size, float start, float stop)
{
    NS_LOG_FUNCTION("Create Applications" << source << dest << port << rate << size << start << stop);
    OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                            Address(InetSocketAddress(dest->GetObject<Ipv4>()
                                                          ->GetAddress(destIndex, 0)
                                                          .GetLocal(),
                                                      port)));
    onOffHelper.SetConstantRate(DataRate(rate), size);
    ApplicationContainer apps = onOffHelper.Install(source);
    apps.Start(Seconds(start));
    apps.Stop(Seconds(stop));

    std::ostringstream oss;
    oss << "/NodeList/"
        << source->GetId() <<"/ApplicationList/*"
        << "/$ns3::OnOffApplication/Tx";
    NS_LOG_LOGIC(oss.str());
    Config::Connect (oss.str (), MakeCallback (&TxPacketCall));
}

void SetRandomClient(NodeContainer &nodes, uint32_t stopTime, int port, string rate, int size)
{
    uint32_t min = 0;
    uint32_t max = nodes.GetN() - 1;
    RngSeedManager::SetSeed(1);
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    for (uint32_t i = 1; i < stopTime; ++i)
    {
        int source = uv->GetInteger(min, max);
        int dest = -1;
        do {
            dest = uv->GetInteger(min, max);
        } while (source == dest);
        SetSatClient(source, dest, port, rate, size, i, i + 1.0);
    }
}

void flow_monitor(Ptr<FlowMonitor> monitor, FlowMonitorHelper &flowmon)
{
    NS_LOG_FUNCTION("Dsra flow monitor");

    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;
    int j=0;
    float AvgThroughput = 0;
    Time Jitter;
    Time Delay;

    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        SentPackets += iter->second.txPackets;
        ReceivedPackets += iter->second.rxPackets;
        LostPackets += iter->second.txPackets - iter->second.rxPackets;
        AvgThroughput += iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024;
        Delay += iter->second.delaySum;
        Jitter += iter->second.jitterSum;
        j = j + 1;

    }

    NS_LOG_UNCOND("--------Total Results of the simulation----------");
    AvgThroughput = AvgThroughput / j;
    NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
    NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets = " << LostPackets);
    if (SentPackets != 0) {
        NS_LOG_UNCOND("Packet Loss ratio = " << ((LostPackets * 100) / SentPackets) << "%");
        NS_LOG_UNCOND("Packet delivery ratio = " << (( ReceivedPackets * 100) / SentPackets) << "%");
    }
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("End to End Delay = " << Delay);
    if (ReceivedPackets != 0) {
        NS_LOG_UNCOND("Average Delay = " << Delay / ReceivedPackets);
    }
    NS_LOG_UNCOND("End to End Jitter delay = " << Jitter);
}

void SetGs(NodeContainer& nodes)
{
    nodes.Create(2);
    Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();
    Vector v(-2180372.26386576, 4387576.92360332, 4069974.82528781); // 北京
    mobility->SetPosition(v);
    nodes.Get(0)->AggregateObject(mobility);

    mobility = CreateObject<ConstantPositionMobilityModel>();
    Vector v1(3934975.24255082, 0.0, 5002822.25774072); // 伦敦
    mobility->SetPosition(v1);
    nodes.Get(1)->AggregateObject(mobility);

    // 安装路由协议
    InternetStackHelper internet;
    Ipv4StaticRoutingHelper routingHelper;
    internet.SetRoutingHelper(routingHelper);
    internet.Install (nodes);
}

void test(int argc, char *argv[])
{
    uint32_t ttl = 5, len = 5;
    CommandLine cmd(__FILE__);
    cmd.AddValue("ttl", "跳数: ", ttl);
    cmd.AddValue("len", "边长: ", len);
    cmd.Parse(argc, argv);

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));
    MpiInterface::Enable (&argc, &argv);
    Time::SetResolution(Time::US);

    LogComponentEnable ("DsraMain2", LOG_LEVEL_INFO);

    float stopTime = 90 * 60;
    float hello_startTime = 0.1; // hello包起始时间
    uint16_t orbits = 72;
    uint16_t sats = 22;
    int total = orbits * sats;

    int UnavailableInterval = 4;
    string sendRate = "16kbps";
    uint16_t port = 9;
    uint32_t packetSize = 1024;

    DsraConfig::Instance()->m_P = orbits;
    DsraConfig::Instance()->m_S = sats;
    DsraConfig::Instance()->m_ttl = ttl;
    DsraConfig::Instance()->m_UnavailableInterval = UnavailableInterval;

    // 节点
    NS_LOG_LOGIC("生成 node");
    NodeContainer nodes;
    nodes.Create (total);
    DsraConfig::Instance()->SetNodeContainer(nodes);
    // 建立拓扑
    NS_LOG_LOGIC("建立拓扑");
    DsraRoutingHelper DsraRoutingHelper;
    DsraUtils::topo(nodes, orbits, sats, DsraRoutingHelper);
    // 安装移动模型
    NS_LOG_LOGIC("安装移动模型");
    DsraUtils::mobility_1584(nodes);

    // 地面站
    NodeContainer gss;
    SetGs(gss);
    pair<uint16_t, uint32_t> satPair1(10, 20);
    pair<uint16_t, uint32_t> satPair2(52, 21);
    DsraUtils::LinkSatAndGs(nodes.Get(DsraConfig::Instance()->GetSatId(satPair1)), gss.Get(0), satPair1);
    DsraUtils::LinkSatAndGs(nodes.Get(DsraConfig::Instance()->GetSatId(satPair2)), gss.Get(1), satPair2);

    // random server
    SetAllServer(port, stopTime);
    SetRandomClient(nodes, stopTime, port, sendRate, packetSize);

    // interface down
    DsraUtils::squareError(nodes, len);

    // 发送hello
    Simulator::Schedule (Seconds(hello_startTime), &Hello);

    // flow-monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop (Seconds (stopTime));
    Simulator::Run();
    flow_monitor(monitor, flowmon);
    cout << "Done." << endl;
    Simulator::Destroy();
}

int main(int argc, char *argv[])
{
    test(argc, argv);

    return 0;
}