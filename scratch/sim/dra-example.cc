//
// Created by hejun on 2023/8/14.
//
#include "utils/utils.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dra-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4.h"
#include "ns3/mpi-module.h"
#include "ns3/mobility-module.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DRAScratch");

void RxPacketCall(std::string context,Ptr<const Packet> packet, const Address &address);


void Hello(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DRAConfLoader::Instance()->getNodeContainer().GetN(); i++){
        DRAConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<DRARouting>()->SendHello();
    }
}

void CheckNeighbor(){
    NS_LOG_FUNCTION(Simulator::Now());
    for(uint32_t i=0; i<DRAConfLoader::Instance()->getNodeContainer().GetN(); i++){
        DRAConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<DRARouting>()->CheckNeighbor();
    }
}

void
SetSatClient(int source, int dest, int port, string rate, int size, float start, float stop)
{
    NS_LOG_FUNCTION("Create Applications " << source << " : " << dest << " : "<< port << " : "<< rate << " : "<< size << " : "<< start << " : "<< stop);
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
}

void
SetServer(int node, int port, float stop)
{
    NS_LOG_FUNCTION("Set Server at node " << node);
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                          Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

    ApplicationContainer apps = sink.Install(DRAConfLoader::Instance()->getNodeContainer().Get(node));
    apps.Start(Seconds(0));
    apps.Stop(Seconds(stop));

    std::ostringstream oss;

    oss << "/NodeList/"
        << node <<"/ApplicationList/*"
        << "/$ns3::PacketSink/Rx";
    NS_LOG_LOGIC(oss.str());
    Config::Connect (oss.str (), MakeCallback (&RxPacketCall));
}

void SetServer(int port, float stop)
{
    uint32_t n = DRAConfLoader::Instance()->getNodeContainer().GetN();
    for (uint32_t i = 0; i < n; ++i)
    {
        SetServer(i, port, stop);
    }
}

void
flow_monitor(Ptr<FlowMonitor> monitor, FlowMonitorHelper &flowmon)
{
    NS_LOG_FUNCTION("dra flow monitor");
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;
    int j=0;
    float AvgThroughput = 0;
    Time Jitter;
    Time Delay;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

        NS_LOG_INFO("----Flow ID:" <<iter->first);
        NS_LOG_INFO("Src Addr" <<t.sourceAddress << "Dst Addr "<< t.destinationAddress);
        NS_LOG_INFO("Sent Packets=" <<iter->second.txPackets);
        NS_LOG_INFO("Received Packets =" <<iter->second.rxPackets);
        NS_LOG_INFO("Sent Bytes=" <<iter->second.txBytes);
        NS_LOG_INFO("Received Bytes =" <<iter->second.rxBytes);
        NS_LOG_INFO("Lost Packets =" <<iter->second.txPackets-iter->second.rxPackets);
        NS_LOG_INFO("Packet delivery ratio =" <<iter->second.rxPackets*100/iter->second.txPackets << "%");
        NS_LOG_INFO("Packet loss ratio =" << (iter->second.txPackets-iter->second.rxPackets)*100/iter->second.txPackets << "%");
        NS_LOG_INFO("Delay =" <<iter->second.delaySum);
        NS_LOG_INFO("Jitter =" <<iter->second.jitterSum);
        NS_LOG_INFO("Throughput =" <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps");

        SentPackets = SentPackets +(iter->second.txPackets);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);

        j = j + 1;
    }

    AvgThroughput = AvgThroughput/j;
    NS_LOG_UNCOND("--------Total Results of the simulation----------");
    NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
    NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio =" << ((LostPackets*100)/SentPackets)<< "%");
    NS_LOG_UNCOND("Packet delivery ratio =" << ((ReceivedPackets*100)/SentPackets)<< "%");
    NS_LOG_UNCOND("Average Throughput =" << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("End to End Delay =" << Delay);
    NS_LOG_UNCOND("Average Delay =" << Delay / ReceivedPackets);
    //NS_LOG_UNCOND("End to End Jitter delay =" << Jitter);
    //NS_LOG_UNCOND("Total Flod id " << j);
    //monitor->SerializeToXmlFile("manet-routing.xml", true, true);
}

/// 追踪丢失的包函数
void DRAPacketLossTrace(const Ipv4Header& header,
                   Ptr<const Packet> p,
                   Ipv4L3Protocol::DropReason reason,
                   Ptr<Ipv4> ipv4,
                   uint32_t)
{
    int id = ipv4->GetObject<Node>()->GetId();
    DRAConfLoader::Instance()->incrementLossPacketCounter(id);
}

/// 追踪丢失的包函数
void DRAPacketRxTrace(Ptr<const Packet> p,
                 Ptr<Ipv4> ipv4,
                 uint32_t oif)
{
    int id = ipv4->GetObject<Node>()->GetId();
    DRAConfLoader::Instance()->incrementRecvPacket(id);
}

/// sink 回调
void RxPacketCall(std::string context,Ptr<const Packet> packet, const Address &address){
    NS_LOG_LOGIC("At time " << Simulator::Now ().GetSeconds ()
                             << "s packet sink received "
                             <<  packet->GetSize () << " bytes from "
                             << InetSocketAddress::ConvertFrom(address).GetIpv4 ()
                             << " port " << InetSocketAddress::ConvertFrom (address).GetPort ()
    );
}

void Mobility(Ptr<Node> n1, Ptr<Node> n2)
{
    MobilityHelper mobility;
    Vector xyz = n1->GetObject<MobilityModel>()->GetPosition();
    Vector xyz1 = n2->GetObject<MobilityModel>()->GetPosition();

    double dis = mobility.GetDistanceSquaredBetween(n1, n2);
    NS_LOG_UNCOND("(" << xyz.x << ", " << xyz.y << ", " << xyz.z << ")");
    NS_LOG_UNCOND("(" << xyz1.x << ", " << xyz1.y << ", " << xyz1.z << ")");
    NS_LOG_UNCOND(dis);
}

int main (int argc, char *argv[])
{

    uint32_t linkError = 0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("linkError", "链路失效概率: ", linkError);
    cmd.Parse(argc, argv);

    GlobalValue::Bind("SimulatorImplementationType",
                      StringValue("ns3::DistributedSimulatorImpl"));
//    GlobalValue::Bind("SchedulerType", StringValue("ns3::DistributedSimulatorImpl"));
    MpiInterface::Enable (&argc, &argv);


    std::cout << "start!" << endl;
    Time::SetResolution(Time::US);
//    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
//    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//    LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("DRAScratch", LOG_LEVEL_DEBUG);
    LogComponentEnable ("SimUtils", LOG_LEVEL_DEBUG);
    LogComponentEnable("DRARouting", LOG_LEVEL_DEBUG);

//    float app_start_time = 1.0;
//    float app_stop_time = 10.0;
    uint32_t stopTime = 10;
//    uint32_t stopTime = 60;

    uint16_t orbits = 72;
    uint16_t sats = 22;
    int total = orbits * sats;

    int UnavailableInterval = 4;
//    int HelloInterval = 1;
//    float CheckNeighborInterval = 0.5;

    //    string dataRate = "1Gbps";//"1Gbps";
    //    string delay = "2ms";
    //string dest_ip = "10.0.1.2";
    string sendRate = "8Kb/s";//"100Mb/s";
    uint32_t packetReceiveDelay = 0;

//    uint16_t port = 9;
//    uint32_t packetSize = 512;
    float CALCULATE_COST = 0.001;
    //    float simulateTime = app_stop_time;
    //    float simulateInterval = 0.5;



    DRAConfLoader::Instance()->setUnavailableInterval(UnavailableInterval);

    DRAConfLoader::Instance()->setPacketReceiveDelay(packetReceiveDelay);
    DRAConfLoader::Instance()->setCalculateCost(CALCULATE_COST);
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

    NodeContainer gs;
    gs.Create(1);
    Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();
    Vector v(-2180372.26386576, 4387576.92360332, 4069974.82528781);
    mobility->SetPosition(v);
    gs.Get(0)->AggregateObject(mobility);

    Simulator::Schedule(Seconds(2.0), &Mobility, nodes.Get(0), gs.Get(0));
    Simulator::Schedule(Seconds(2.0), &Mobility, nodes.Get(1), gs.Get(0));

//    // 安装 client 和 server
//    NS_LOG_LOGIC("安装 client 和 server");
//    SetServer(port, stopTime);
////    int source = 31 * 22 + 18, dest = 42 * 22 + 3;
////    SetSatClient(source, dest, port, sendRate, packetSize, app_start_time, app_stop_time);
//    uint32_t min = 0;
//    uint32_t max = nodes.GetN() - 1;
//    RngSeedManager::SetSeed(1);
//    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
//    for (uint32_t i = 1; i < stopTime; ++i)
//    {
//        int source = uv->GetInteger(min, max);
//        int dest = -1;
//        do {
//            dest = uv->GetInteger(min, max);
//        } while (source == dest);
//        SetClient(source, dest, port, sendRate, packetSize, i, i + 1.0);
//    }
//
//
//    // 发送 hello 包
//    int N = stopTime/HelloInterval;
//    for(int i=0; i< N; i++){
//        Time onInterval = Seconds(i*HelloInterval);
//        Simulator::Schedule (onInterval, &Hello);
//    }
//    // 检查邻居状态
//    N = (int)(stopTime/CheckNeighborInterval);
//    for(int i=0; i< N; i++){
//        Time onInterval = Seconds(i*CheckNeighborInterval);
//        Simulator::Schedule (onInterval, &CheckNeighbor);
//    }
//    // interface down
//    randomLinkError(nodes, linkError);
//
//    // trace
//    Config::ConnectWithoutContext("/NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback(&DRAPacketRxTrace));
//    Config::ConnectWithoutContext("/NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&DRAPacketLossTrace));

    if (stopTime != 0)
    {
        Simulator::Stop (Seconds (stopTime));
    }

    // flow-monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Run();

    flow_monitor(monitor, flowmon);

    cout << "Done." << endl;

//    cout << "Send: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSendPacket()) << endl;
//    cout << "SendData: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSendDataPacket()) << endl;
//    cout << "SendContr: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSendContrPacket()) << endl;
//    cout << "SendHello: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSendHelloPacket()) << endl;
//
//    cout << "Success: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSuccessPacket()) << endl;
//    cout << "SuccessData: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getSuccessDataPacket()) << endl;
//
//
//    cout << "Lost packets: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getLossPacketCounter()) << endl;
//    cout << "Receive: " << endl << DRAConfLoader::Instance()->PrintMap(DRAConfLoader::Instance()->getRecvPacket()) << endl;

    Simulator::Destroy();

    return 0;
}
