//
// Created by hejun on 2023/8/7.
//

#include "isl-channel.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ISLChannel");
NS_OBJECT_ENSURE_REGISTERED(ISLChannel);

TypeId
ISLChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ISLChannel")
            .SetParent<PointToPointChannel>()
            .SetGroupName("satellite")
            .AddConstructor<ISLChannel>()
            .AddAttribute("PropagationDelayModel",
                          "A pointer to the propagation delay model attached to this channel.",
                          StringValue("ns3::SatellitePropagationDelayModel[Speed=290798684.3]"),
                          MakePointerAccessor(&ISLChannel::m_delay),
                          MakePointerChecker<SatellitePropagationDelayModel>())
            .AddTraceSource("TxRxISLChannel",
                    "Trace source indicating transmission of packet "
                    "from the PointToPointChannel, used by the Animation "
                    "interface.",
                    MakeTraceSourceAccessor(&ISLChannel::m_txrxPointToPoint),
                    "ns3::PointToPointChannel::TxRxAnimationCallback");
    return tid;
}

ISLChannel::ISLChannel()
    : PointToPointChannel()
{
    NS_LOG_FUNCTION_NOARGS();
}

Time
ISLChannel::GetDelay() const{
    NS_LOG_FUNCTION(this);
    Ptr<MobilityModel> mobility0 = this->GetDevice(0)->GetNode()->GetObject<MobilityModel>();
    Ptr<MobilityModel> mobility1 = this->GetDevice(1)->GetNode()->GetObject<MobilityModel>();
    return m_delay->GetDelay(mobility0, mobility1);
}

bool
ISLChannel::TransmitStart(Ptr<const Packet> p, Ptr<PointToPointNetDevice> src, Time txTime)
{
    NS_LOG_FUNCTION(this << p << src);
    NS_LOG_LOGIC("UID is " << p->GetUid() << ")");

    uint32_t wire = src == this->GetDevice(0) ? 0 : 1;
    Ptr<PointToPointNetDevice> dst = this->GetPointToPointDevice(1-wire);
    NS_LOG_LOGIC("src is " << src << ", dst is " << dst);
    Simulator::ScheduleWithContext(dst->GetNode()->GetId(),
                                   txTime + this->GetDelay(),
                                   &PointToPointNetDevice::Receive,
                                   dst,
                                   p->Copy());

    // Call the tx anim callback on the net device
    m_txrxPointToPoint(p, src, dst, txTime, txTime + this->GetDelay());
    return true;
}

Ptr<SatellitePropagationDelayModel>
ISLChannel::GetPropagationDelayModel(void) const
{
    return m_delay;
}

void ISLChannel::SetPropagationDelayModel(Ptr<SatellitePropagationDelayModel> delay){
    m_delay = delay;
};

} // namespace ns3