//
// Created by hejun on 2023/8/14.
//
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mpi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/dra-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DraMain3");

map<uint64_t, pair<int64_t , int64_t >> m_map;

void SendHello()
{
    for(uint32_t i=0; i< DRAConfLoader::Instance()->getNodeContainer().GetN(); i++){
        DRAConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<DRARouting>()->SendHello();
    }
}

void CheckNeighbor()
{
    for(uint32_t i=0; i<DRAConfLoader::Instance()->getNodeContainer().GetN(); i++){
        DRAConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<DRARouting>()->CheckNeighbor();
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
    NS_LOG_LOGIC(oss.str());
    Config::Connect (oss.str (), MakeCallback (&RxPacketCall));
}

void SetAllServer(int port, float stop)
{
    NS_LOG_FUNCTION("Set Server at all node");
    uint32_t n = DRAConfLoader::Instance()->getNodeContainer().GetN();
    for (uint32_t i = 0; i < n; ++i)
    {
        SetServer(DRAConfLoader::Instance()->getNodeContainer().Get(i), port, stop);
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
    NS_LOG_FUNCTION("Create Applications " << source << dest << port << rate << size << start << stop);
    OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                            Address(InetSocketAddress(DRAConfLoader::Instance()
                                                          ->getNodeContainer()
                                                          .Get(dest)
                                                          ->GetObject<Ipv4>()
                                                          ->GetAddress(1, 0)
                                                          .GetLocal(),
                                                      port)));
    onOffHelper.SetConstantRate(DataRate(rate), size);
    ApplicationContainer apps = onOffHelper.Install(DRAConfLoader::Instance()->getNodeContainer().Get(source));
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
    NS_LOG_FUNCTION("Create Applications " << source << dest << port << rate << size << start << stop);
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

void SetRandomClient(NodeContainer nodes, uint32_t stopTime, int port, string rate, int size)
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
    NS_LOG_FUNCTION("dra flow monitor");

    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;
    int j=0;
    float AvgThroughput = 0;
    Time Jitter;
    Time Delay;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        SentPackets += iter->second.txPackets;
        ReceivedPackets += iter->second.rxPackets;
        LostPackets += iter->second.txPackets - iter->second.rxPackets;
        AvgThroughput += iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024;
        Delay += iter->second.delaySum;
        Jitter += iter->second.jitterSum;
        j = j + 1;

        Histogram delayHistogram = iter->second.delayHistogram;
        delayHistogram.SerializeToXmlStream(std::cout, 1, " ");
    }

    NS_LOG_UNCOND("--------Total Results of the simulation----------");
    AvgThroughput = AvgThroughput / j;
    NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
    NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets = " << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio = " << ((LostPackets * 100) / SentPackets) << "%");
    NS_LOG_UNCOND("Packet delivery ratio = " << (( ReceivedPackets * 100) / SentPackets) << "%");
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("End to End Delay = " << Delay);
    NS_LOG_UNCOND("Average Delay = " << Delay / ReceivedPackets);
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

void delay()
{
    vector<pair<int64_t, int64_t> > res;
    res.reserve(m_map.size());
    for (auto it: m_map)
    {
        res.emplace_back(it.second.first, it.second.second - it.second.first);
    }

    sort(res.begin(), res.end(), [](pair<int64_t, int64_t> a, pair<int64_t, int64_t > b){return a.first < b.first;});
    for (auto it: res)
    {
        NS_LOG_INFO(it);
    }
}

void test(int argc, char *argv[])
{
    NS_LOG_INFO("dra-main-3 start");
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));
    MpiInterface::Enable (&argc, &argv);
    Time::SetResolution(Time::US);

    LogComponentEnable ("DraMain3", LOG_LEVEL_INFO);

    uint32_t stopTime = 5 * 60;
    float app_start_time = 1.0;
    float app_stop_time = stopTime;
    uint16_t orbits = 72;
    uint16_t sats = 22;
    int total = orbits * sats;

    int UnavailableInterval = 4;
    int HelloInterval = 1;
    float CheckNeighborInterval = 0.5;
    string sendRate = "10Mbps";
    uint16_t port = 9;
    uint32_t packetSize = 1024;

    DRAConfLoader::Instance()->setUnavailableInterval(UnavailableInterval);
    DRAConfLoader::Instance()->setOrbits(orbits);
    DRAConfLoader::Instance()->setSats(sats);

    // 节点
    NS_LOG_LOGIC("生成 node");
    NodeContainer nodes;
    nodes.Create (total);
    DRAConfLoader::Instance()->setNodeContainer(nodes);
    // 建立拓扑
    NS_LOG_LOGIC("建立拓扑");
    DRARoutingHelper draRoutingHelper;
    topo(nodes, orbits, sats, draRoutingHelper);
    // 安装移动模型
    NS_LOG_LOGIC("安装移动模型");
    mobility_1584(nodes);
    // 地面站
    NodeContainer gss;
    SetGs(gss);
    pair<uint16_t, uint32_t> satPair1(10, 20);
    pair<uint16_t, uint32_t> satPair2(52, 21);
    LinkSatAndGs(nodes.Get(DRAConfLoader::Instance()->GetSatId(satPair1)), gss.Get(0), satPair1);
    LinkSatAndGs(nodes.Get(DRAConfLoader::Instance()->GetSatId(satPair2)), gss.Get(1), satPair2);
    // 安装 client 和 server
    NS_LOG_LOGIC("安装 client 和 server");
    SetServer(gss.Get(1), port, stopTime);
    SetGsClient(gss.Get(0), gss.Get(1), 1, 9, sendRate, packetSize, app_start_time, app_stop_time);

    // 发送 hello 包
    int N = stopTime/HelloInterval;
    for(int i=0; i < N; i++){
        Time onInterval = Seconds(i*HelloInterval);
        Simulator::Schedule (onInterval, &SendHello);
    }
    // 检查邻居状态
    N = (int)(stopTime/CheckNeighborInterval);
    for(int i=0; i< N; i++){
        Time onInterval = Seconds(i*CheckNeighborInterval);
        Simulator::Schedule (onInterval, &CheckNeighbor);
    }

    // flow-monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop (Seconds (stopTime));
    Simulator::Run();
    flow_monitor(monitor, flowmon);

    delay();
    cout << "Done." << endl;
    Simulator::Destroy();
}

int
main(int argc, char *argv[])
{
    test(argc, argv);
}