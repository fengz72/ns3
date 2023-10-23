//
// Created by hejun on 2023/8/8.
//

#include "ns3//satellite-channel-helper.h"
#include "ns3/string.h"
#include "ns3/point-to-point-module.h"
#include "ns3/isl-channel.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ISLHelper");

SatelliteChannelHelper::SatelliteChannelHelper()
{
    m_queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
    m_deviceFactory.SetTypeId("ns3::PointToPointNetDevice");
    m_channelFactory.SetTypeId ("ns3::ISLChannel");
    m_channelFactory.Set("PropagationDelayModel",
                         StringValue("ns3::SatellitePropagationDelayModel[Speed=290798684.3]"));
    m_enableFlowControl = true;
}

void
SatelliteChannelHelper::SetDeviceAttribute(std::string name, const ns3::AttributeValue& value)
{
    m_deviceFactory.Set(name, value);
}

void
SatelliteChannelHelper::SetChannelAttribute(std::string name, const ns3::AttributeValue& value)
{
    m_channelFactory.Set(name, value);
}

void
SatelliteChannelHelper::DisableFlowControl()
{
    m_enableFlowControl = false;
}

void
SatelliteChannelHelper::EnablePcapInternal(std::string prefix,
                                           Ptr<ns3::NetDevice> nd,
                                           bool promiscuous,
                                           bool explicitFilename)
{
    //
    // All of the Pcap enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type PointToPointNetDevice.
    //
    Ptr<PointToPointNetDevice> device = nd->GetObject<PointToPointNetDevice>();
    if (!device)
    {
        NS_LOG_INFO("SatelliteChannelHelper::EnablePcapInternal(): Device "
                    << device << " not of type ns3::PointToPointNetDevice");
        return;
    }

    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename)
    {
        filename = prefix;
    }
    else
    {
        filename = pcapHelper.GetFilenameFromDevice(prefix, device);
    }

    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_PPP);
    pcapHelper.HookDefaultSink<PointToPointNetDevice>(device, "PromiscSniffer", file);
}

void
SatelliteChannelHelper::EnableAsciiInternal(Ptr<ns3::OutputStreamWrapper> stream,
                                            std::string prefix,
                                            Ptr<ns3::NetDevice> nd,
                                            bool explicitFilename)
{
    // All of the ascii enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type PointToPointNetDevice.
    //
    Ptr<PointToPointNetDevice> device = nd->GetObject<PointToPointNetDevice>();
    if (!device)
    {
        NS_LOG_INFO("SatelliteChannelHelper::EnableAsciiInternal(): Device "
                    << device << " not of type ns3::PointToPointNetDevice");
        return;
    }

    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and do a Hook*WithoutContext
    // since there will be one file per context and therefore the context would
    // be redundant.
    //
    if (!stream)
    {
        //
        // Set up an output stream object to deal with private ofstream copy
        // constructor and lifetime issues.  Let the helper decide the actual
        // name of the file given the prefix.
        //
        AsciiTraceHelper asciiTraceHelper;

        std::string filename;
        if (explicitFilename)
        {
            filename = prefix;
        }
        else
        {
            filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
        }

        Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

        //
        // The MacRx trace source provides our "r" event.
        //
        asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<PointToPointNetDevice>(device,
                                                                                     "MacRx",
                                                                                     theStream);

        //
        // The "+", '-', and 'd' events are driven by trace sources actually in the
        // transmit queue.
        //
        Ptr<Queue<Packet>> queue = device->GetQueue();
        asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Enqueue",
                                                                             theStream);
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue<Packet>>(queue, "Drop", theStream);
        asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue<Packet>>(queue,
                                                                             "Dequeue",
                                                                             theStream);

        // PhyRxDrop trace source for "d" event
        asciiTraceHelper.HookDefaultDropSinkWithoutContext<PointToPointNetDevice>(device,
                                                                                  "PhyRxDrop",
                                                                                  theStream);

        return;
    }

    //
    // If we are provided an OutputStreamWrapper, we are expected to use it, and
    // to providd a context.  We are free to come up with our own context if we
    // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
    // compatibility and simplicity, we just use Config::Connect and let it deal
    // with the context.
    //
    // Note that we are going to use the default trace sinks provided by the
    // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
    // but the default trace sinks are actually publicly available static
    // functions that are always there waiting for just such a case.
    //
    uint32_t nodeid = nd->GetNode()->GetId();
    uint32_t deviceid = nd->GetIfIndex();
    std::ostringstream oss;

    oss << "/NodeList/" << nd->GetNode()->GetId() << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointNetDevice/MacRx";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointNetDevice/TxQueue/Enqueue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointNetDevice/TxQueue/Dequeue";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointNetDevice/TxQueue/Drop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

    oss.str("");
    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid
        << "/$ns3::PointToPointNetDevice/PhyRxDrop";
    Config::Connect(oss.str(),
                    MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer
SatelliteChannelHelper::Install(ns3::NodeContainer c)
{
    NS_ASSERT(c.GetN() == 2);
    return Install(c.Get(0), c.Get(1));
}

NetDeviceContainer
SatelliteChannelHelper::Install(Ptr<ns3::Node> a, Ptr<ns3::Node> b)
{
    NetDeviceContainer container;

    Ptr<PointToPointNetDevice> dev1 = m_deviceFactory.Create<PointToPointNetDevice>();
    dev1->SetAddress(Mac48Address::Allocate());
    a->AddDevice(dev1);
    // 创建队列, 并添加到设备上
    Ptr<Queue<Packet>> queueA = m_queueFactory.Create<Queue<Packet>>();
    dev1->SetQueue(queueA);

    Ptr<PointToPointNetDevice> dev2 = m_deviceFactory.Create<PointToPointNetDevice>();
    dev2->SetAddress(Mac48Address::Allocate());
    b->AddDevice(dev2);
    // 创建队列, 并添加到设备上
    Ptr<Queue<Packet>> queueB = m_queueFactory.Create<Queue<Packet>>();
    dev2->SetQueue(queueB);

    // 流量控制
    if (m_enableFlowControl)
    {
        // Aggregate NetDeviceQueueInterface objects
        Ptr<NetDeviceQueueInterface> ndqiA = CreateObject<NetDeviceQueueInterface>();
        ndqiA->GetTxQueue(0)->ConnectQueueTraces(queueA);
        dev1->AggregateObject(ndqiA);
        Ptr<NetDeviceQueueInterface> ndqiB = CreateObject<NetDeviceQueueInterface>();
        ndqiB->GetTxQueue(0)->ConnectQueueTraces(queueB);
        dev2->AggregateObject(ndqiB);
    }

    Ptr<ISLChannel> channel = m_channelFactory.Create<ISLChannel>();
    dev1->Attach(channel);
    dev2->Attach(channel);
    container.Add(dev1);
    container.Add(dev2);

    return container;
}

} // namespace ns3