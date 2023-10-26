#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/drlf-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/mpi-module.h"
#include "ns3/mobility-module.h"

#include "ns3/flow-monitor-module.h"

#include "ns3/dra-module.h"
#include "ns3/dra-utils.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DrlfMain1");

map<uint64_t, pair<int64_t , int64_t >> m_map;

void Hello(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DrlfConfig::Instance()->GetNodeContainer().GetN(); i++){
        DrlfConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DrlfRouting>()->SendHello();
    }
}

void LSA()
{
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DrlfConfig::Instance()->GetNodeContainer().GetN(); i++){
        DrlfConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DrlfRouting>()->SendAllLSA();
    }
}

void CheckNeighbor(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DrlfConfig::Instance()->GetNodeContainer().GetN(); i++){
        DrlfConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DrlfRouting>()->CheckNeighbor();
    }
}

void Dijkstra()
{
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DrlfConfig::Instance()->GetNodeContainer().GetN(); i++){
        DrlfConfig::Instance()->GetNodeContainer().Get(i)->GetObject<DrlfRouting>()->Dijkstra();
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
    uint32_t n = DrlfConfig::Instance()->GetNodeContainer().GetN();
    for (uint32_t i = 0; i < n; ++i)
    {
        SetServer(DrlfConfig::Instance()->GetNodeContainer().Get(i), port, stop);
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
    NS_LOG_FUNCTION("Create Applications " << source << " : " << dest << " : "<< port << " : "<< rate << " : "<< size << " : "<< start << " : "<< stop);
    OnOffHelper onOffHelper("ns3::UdpSocketFactory",
                            Address(InetSocketAddress(DrlfConfig::Instance()
                                                          ->GetNodeContainer()
                                                          .Get(dest)
                                                          ->GetObject<Ipv4>()
                                                          ->GetAddress(1, 0)
                                                          .GetLocal(),
                                                      port)));
    onOffHelper.SetConstantRate(DataRate(rate), size);
    ApplicationContainer apps = onOffHelper.Install(DrlfConfig::Instance()->GetNodeContainer().Get(source));
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
    NS_LOG_FUNCTION("drlf flow monitor");

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

//        Histogram delayHistogram = iter->second.delayHistogram;
//        delayHistogram.SerializeToXmlStream(std::cout, 1, " ");
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

int main(int argc, char *argv[])
{
    uint32_t ttl = 2, error = 100;
    CommandLine cmd(__FILE__);
    cmd.AddValue("ttl", "跳数: ", ttl);
    cmd.AddValue("error", "链路失效概率: ", error);
    cmd.Parse(argc, argv);

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));
    MpiInterface::Enable (&argc, &argv);
    Time::SetResolution(Time::US);

    LogComponentEnable ("DrlfMain1", LOG_LEVEL_INFO);
    //    LogComponentEnable ("Simulator", LOG_LEVEL_LOGIC);

    //    LogComponentEnable ("DrlfMain1", LOG_LEVEL_LOGIC);
    //    LogComponentEnable ("DrlfUtil", LOG_LEVEL_FUNCTION);
    //    LogComponentEnable("DrlfRouting", LOG_LEVEL_LOGIC);

    NS_LOG_INFO("ttl = " << ttl << ", error = " << error);
    float stopTime = 90 * 60;
    float lsa_startTime = 0.05; // 50ms后开始发送
    float dij_startTime = 0.5; // 500ms时计算dijkstra
    uint16_t orbits = 72;
    uint16_t sats = 22;
    int total = orbits * sats;

    int UnavailableInterval = 4;
    //    int HelloInterval = 1;
    //    float CheckNeighborInterval = 0.5;
    string sendRate = "16Kbps";
    uint16_t port = 9;
    uint32_t packetSize = 1024;

    DrlfConfig::Instance()->m_P = orbits;
    DrlfConfig::Instance()->m_S = sats;
    DrlfConfig::Instance()->m_ttl = ttl;
    DrlfConfig::Instance()->SetUnavailableInterval(UnavailableInterval);

    // 节点
    NS_LOG_LOGIC("生成 node");
    NodeContainer nodes;
    nodes.Create (total);
    DrlfConfig::Instance()->SetNodeContainer(nodes);
    // 建立拓扑
    NS_LOG_LOGIC("建立拓扑");
    DrlfRoutingHelper drlfRoutingHelper;
    DrlfUtils::topo(nodes, orbits, sats, drlfRoutingHelper);
    // 安装移动模型
    NS_LOG_LOGIC("安装移动模型");
    DrlfUtils::mobility_1584(nodes);
    // 地面站
//    NodeContainer gss;
//    SetGs(gss);
//    pair<uint16_t, uint32_t> satPair1(10, 20);
//    pair<uint16_t, uint32_t> satPair2(52, 21);
//    DrlfUtils::LinkSatAndGs(nodes.Get(DrlfConfig::Instance()->GetSatId(satPair1)), gss.Get(0), satPair1);
//    DrlfUtils::LinkSatAndGs(nodes.Get(DrlfConfig::Instance()->GetSatId(satPair2)), gss.Get(1), satPair2);
    // 安装 client 和 server
    NS_LOG_LOGIC("安装 client 和 server");
    SetAllServer(port, stopTime);
    SetRandomClient(nodes, stopTime, port, sendRate, packetSize);

    // interface down
    DrlfUtils::randomLinkError(nodes, error);

    //    // 发送 hello 包
    //    int N = stopTime/HelloInterval;
    //    for(int i=0; i < N; i++){
    //        Time onInterval = Seconds(i*HelloInterval);
    //        Simulator::Schedule (onInterval, &Hello);
    //    }
    //    // 检查邻居状态
    //    N = (int)(stopTime/CheckNeighborInterval);
    //    for(int i=0; i< N; i++){
    //        Time onInterval = Seconds(i*CheckNeighborInterval);
    //        Simulator::Schedule (onInterval, &CheckNeighbor);
    //    }
    // 发送hello, lsa
    Simulator::Schedule (Seconds(0.0), &Hello);
    Simulator::Schedule(Seconds(lsa_startTime), &LSA);
    // dijkstra
    Simulator::Schedule(Seconds(dij_startTime), &Dijkstra);

    // flow-monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop (Seconds (stopTime));
    Simulator::Run();
    flow_monitor(monitor, flowmon);
    cout << "Done." << endl;
    Simulator::Destroy();

    return 0;
}