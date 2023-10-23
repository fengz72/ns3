//
// Created by hejun on 2023/8/7.
//

#ifndef NS3_PROPAGATIONDELAYMODEL_H
#define NS3_PROPAGATIONDELAYMODEL_H

#include "ns3/object.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
namespace ns3
{

class SatellitePropagationDelayModel :public Object
{
  public:
    static TypeId GetTypeId();
    SatellitePropagationDelayModel();
    ~SatellitePropagationDelayModel() override;

    Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
    void SetSpeed(double speed);
    double GetSpeed() const;

  private:
    double m_speed;
};

} // namespace ns3

#endif // NS3_PROPAGATIONDELAYMODEL_H
