//
// Created by hejun on 2023/8/14.
//

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mpi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/satellite-module.h"

#include "ns3/dra-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DraMain1");

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

void
SetServer(Ptr<Node> node, int port, float stop)
{
    NS_LOG_FUNCTION("Set Server at node " << node);
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                          Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

    ApplicationContainer apps = sink.Install(node);
    apps.Start(Seconds(0));
    apps.Stop(Seconds(stop));
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

//    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier ());
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


void test(int argc, char *argv[])
{
    uint32_t linkError = 0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("error", "链路失效概率: ", linkError);
    cmd.Parse(argc, argv);

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));
    MpiInterface::Enable (&argc, &argv);
    Time::SetResolution(Time::US);

    LogComponentEnable ("DraMain1", LOG_LEVEL_INFO);
    LogComponentEnable ("DraUtils", LOG_LEVEL_INFO);
//    LogComponentEnable("DRARouting", LOG_LEVEL_DEBUG);

    NS_LOG_INFO("dra-main-1 start, error = " << linkError);

    uint32_t stopTime = 90 * 60;
    uint16_t orbits = 72;
    uint16_t sats = 22;
    int total = orbits * sats;

    int UnavailableInterval = 4;
    int HelloInterval = 1;
    float CheckNeighborInterval = 0.5;
    string sendRate = "16kbps";
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
    // 安装 client 和 server
    NS_LOG_LOGIC("安装 client 和 server");
    SetAllServer(port, stopTime);
    //SetClient(0, 50, 9, sendRate, packetSize, app_start_time, app_stop_time);
    SetRandomClient(nodes, stopTime, port, sendRate, packetSize);

    // interface down
    randomLinkError(nodes, linkError);

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
    cout << "Done." << endl;
    Simulator::Destroy();
}

int
main(int argc, char *argv[])
{
    test(argc, argv);
}