//
// Created by hejun on 2023/8/7.
//

#include "satellite-propagation-delay-model.h"
#include "ns3/double.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SatellitePropagationDelayModel");
NS_OBJECT_ENSURE_REGISTERED(SatellitePropagationDelayModel);

TypeId
SatellitePropagationDelayModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatellitePropagationDelayModel")
            .SetParent<Object>()
            .SetGroupName("satellite")
            .AddConstructor<SatellitePropagationDelayModel>()
            .AddAttribute("Speed",
                          "传播速度, m/s",
                          DoubleValue(299792458),
                          MakeDoubleAccessor(&SatellitePropagationDelayModel::m_speed),
                          MakeDoubleChecker<double>());
    return tid;
}

Time
SatellitePropagationDelayModel::GetDelay(Ptr<ns3::MobilityModel> a, Ptr<ns3::MobilityModel> b) const
{
    double distance = a->GetDistanceFrom(b);
    double seconds = distance / m_speed;
    return Seconds(seconds);
}

void
SatellitePropagationDelayModel::SetSpeed(double speed)
{
    m_speed = speed;
}

double
SatellitePropagationDelayModel::GetSpeed() const
{
    return m_speed;
}

SatellitePropagationDelayModel::SatellitePropagationDelayModel()
{
}

SatellitePropagationDelayModel::~SatellitePropagationDelayModel()
{
}
} // namespace ns3