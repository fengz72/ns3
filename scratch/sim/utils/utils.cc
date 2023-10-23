//
// Created by hejun on 2023/8/21.
//

#include "utils.h"
#include "ns3/dra-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/satellite-module.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimUtils");

std::string epoch_g("2023-09-01 00:00:00");
std::string path_100 = "/home/hejun/soft/ns3/ns-allinone-3.38/ns-3.38/scratch/sim/res/test-1.txt";
std::string path_1584 = "/home/hejun/soft/ns3/ns-allinone-3.38/ns-3.38/scratch/sim/res/starlink-1.txt";

void
topo(NodeContainer &nodes, int P, int n, Ipv4RoutingHelper &routingHelper)
{
    NS_LOG_FUNCTION("开始建立网络拓扑");

    ObjectFactory m_queueFactory;   //!< Queue Factory
    ObjectFactory m_deviceFactory;  //!< Device Factory
    ObjectFactory m_channelFactory; // channel factory
    m_queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
    m_deviceFactory.SetTypeId("ns3::PointToPointNetDevice");
    m_channelFactory.SetTypeId ("ns3::ISLChannel");

    m_deviceFactory.Set("DataRate",StringValue("100Mbps"));
    m_channelFactory.Set("PropagationDelayModel",
                         StringValue("ns3::SatellitePropagationDelayModel[Speed=290798684.3]"));

    // 安装路由协议
    NS_LOG_LOGIC("安装路由协议");
    InternetStackHelper internet;
    //Ipv4ListRoutingHelper listRouting;
//    DRARoutingHelper draRoutingHelper;
    //listRouting.Add(draRoutingHelper, 10);
    internet.SetRoutingHelper (routingHelper);
    internet.Install (nodes);

    // 配置每个节点
    for (int i = 0; i < P * n; ++i)
    {
        NS_LOG_LOGIC("正在配置第" << i << "个节点");
        int PNum = i / n;
        int satInPNum = i % n;

        // 添加netDevice
        Ptr<Node> node = nodes.Get(i);
        NetDeviceContainer netDeviceContainer;
        for (int j = 0; j < 4; ++j)
        {
            // 创建点对点设备, 设置mac地址, 并添加到节点上
            Ptr<PointToPointNetDevice> dev = m_deviceFactory.Create<PointToPointNetDevice>();
            dev->SetAddress(Mac48Address::Allocate());
            node->AddDevice(dev);
            // 创建队列, 并添加到设备上
            Ptr<Queue<Packet>> queueA = m_queueFactory.Create<Queue<Packet>>();
            dev->SetQueue(queueA);
            // 流量控制
            Ptr<NetDeviceQueueInterface> ndqiA = CreateObject<NetDeviceQueueInterface>();
            ndqiA->GetTxQueue(0)->ConnectQueueTraces(queueA);
            dev->AggregateObject(ndqiA);

            netDeviceContainer.Add(dev);
        }
        // 分配 ip
        Ipv4AddressHelper ipv4;
        std::string ip = "1." + std::to_string(PNum) + "." + std::to_string(satInPNum) + ".0";
        std::string mask = "255.255.255.0";
        ipv4.SetBase(ip.c_str(), mask.c_str());

        Ipv4InterfaceContainer ipv4InterfaceContainer = ipv4.Assign(netDeviceContainer);
    }

    // 连接同轨道前后
    NS_LOG_LOGIC("开始连接轨内链路");
    for (int j = 0; j < P; ++j)
    {
        for (int i = 0; i < n; ++i)
        {
            int cur = i + n * j; // 同轨道当前卫星
            int next = (i + 1) % n +  n * j; //同轨道后一颗卫星
            // debug
            NS_LOG_LOGIC("轨道: " + std::to_string(j) + ", " + "卫星: " + std::to_string(i));
            NS_LOG_LOGIC("cur: " + std::to_string(cur) + ", " + "next: " + std::to_string(next));

            // 使用 p2p 信道连接
            Ptr<NetDevice> netDevA = nodes.Get(cur)->GetDevice(3);
            Ptr<PointToPointNetDevice> devA = DynamicCast<PointToPointNetDevice>(netDevA);
            Ptr<NetDevice> netDevB = nodes.Get(next)->GetDevice(1);
            Ptr<PointToPointNetDevice> devB = DynamicCast<PointToPointNetDevice>(netDevB);

            Ptr<PointToPointChannel> channel = m_channelFactory.Create<PointToPointChannel>();
            devA->Attach(channel);
            devB->Attach(channel);
        }
    }

    // 连接相邻轨道
    NS_LOG_LOGIC("开始连接轨间链路");
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < P; ++j)
        {
            int cur = i + n * j; // 左面卫星
            int next = i +  ((j + 1) % P) * n; // 右面卫星
            // debug
            NS_LOG_LOGIC("轨道: " + std::to_string(j) + ", " + "卫星: " + std::to_string(i));
            NS_LOG_LOGIC("cur: " + std::to_string(cur) + ", " + "next: " + std::to_string(next));

            Ptr<PointToPointNetDevice> devA = DynamicCast<PointToPointNetDevice>(nodes.Get(cur)->GetDevice(2));
            Ptr<PointToPointNetDevice> devB = DynamicCast<PointToPointNetDevice>(nodes.Get(next)->GetDevice(4));

            Ptr<PointToPointChannel> channel = m_channelFactory.Create<PointToPointChannel>();
            devA->Attach(channel);
            devB->Attach(channel);
        }
    }
}

void
mobility_100(NodeContainer& nodes)
{
    NS_LOG_FUNCTION("安装移动模型");
    JulianDate time(epoch_g);

    std::ifstream file(path_100);
    if (!file.is_open()) {
        NS_LOG_ERROR("tle文件打开失败");
        std::exit(EXIT_FAILURE);
    }

    uint32_t count = 0;
    while (file.peek() != std::char_traits<char>::eof() && count < nodes.GetN()) {
        NS_LOG_LOGIC("正在安装第" << count << "个节点");
        std::string name;
        std::string line1;
        std::string line2;
        std::getline(file, name);
        std::getline(file, line1);
        std::getline(file, line2);
        // 根据tle创建卫星
        Ptr<Satellite> sat = CreateObject<Satellite>();
        sat->SetName(name);
        sat->SetTleInfo(line1, line2);
        // 创建移动模型
        Ptr<SatellitePositionMobilityModel> satModel = CreateObject<SatellitePositionMobilityModel>();
        satModel->SetStartTime(time);
        satModel->SetSatellite(sat);
        // 将移动模型聚合到节点上
        Ptr<Node> node = nodes.Get(count);
        Ptr<MobilityModel> model = node->GetObject<MobilityModel>();
        if (! model) {
            model = DynamicCast<MobilityModel>(satModel);
            node -> AggregateObject(model);
        }

        count++;
    }
    NS_LOG_LOGIC("移动模型安装完成");
}

void
mobility_1584(NodeContainer& nodes)
{
    NS_LOG_FUNCTION("安装移动模型");
    JulianDate time(epoch_g);

    std::ifstream file(path_1584);
    if (!file.is_open()) {
        NS_LOG_ERROR("tle文件打开失败");
        std::exit(EXIT_FAILURE);
    }

    uint32_t count = 0;
    while (file.peek() != std::char_traits<char>::eof() && count < nodes.GetN()) {
        NS_LOG_LOGIC("正在安装第" << count << "个节点");
        std::string name;
        std::string line1;
        std::string line2;
        std::getline(file, name);
        std::getline(file, line1);
        std::getline(file, line2);
        // 根据tle创建卫星
        Ptr<Satellite> sat = CreateObject<Satellite>();
        sat->SetName(name);
        sat->SetTleInfo(line1, line2);
        // 创建移动模型
        Ptr<SatellitePositionMobilityModel> satModel = CreateObject<SatellitePositionMobilityModel>();
        satModel->SetStartTime(time);
        satModel->SetSatellite(sat);
        // 将移动模型聚合到节点上
        Ptr<Node> node = nodes.Get(count);
        Ptr<MobilityModel> model = node->GetObject<MobilityModel>();
        if (! model) {
            model = DynamicCast<MobilityModel>(satModel);
            node -> AggregateObject(model);
        }

        count++;
    }
    NS_LOG_LOGIC("移动模型安装完成");
}

void
randomLinkError(NodeContainer& nodes, uint32_t limit)
{
    NS_LOG_FUNCTION("randomLinkError");
    uint32_t min = 1;
    uint32_t max = 1000;
    RngSeedManager::SetSeed(1);
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();

    uint32_t count = 0;
    for (uint32_t i = 0; i < nodes.GetN(); i++)
    {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4> ();
        for (int j = 1; j <= 4; ++j)
        {
            uint32_t tmp = uv->GetInteger(min, max);
            if (tmp <= limit) {
                NS_LOG_LOGIC("node " << i << ", interface " << j << " down");
                count++;
                Simulator::Schedule (Seconds (0.0),&Ipv4::SetDown, ipv4, j);
            }
        }
    }

    NS_LOG_UNCOND("total down interface: " << count);
}



}