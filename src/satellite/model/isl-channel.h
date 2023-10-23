//
// Created by hejun on 2023/8/7.
//

#ifndef NS3_ISLCHANNEL_H
#define NS3_ISLCHANNEL_H

#include "ns3/point-to-point-module.h"
#include "satellite-propagation-delay-model.h"

namespace ns3
{

class ISLChannel: public PointToPointChannel
{
  public:
    static TypeId GetTypeId();
    ISLChannel();

    ///< 继承自p2p channel
    bool TransmitStart(Ptr<const Packet> p, Ptr<PointToPointNetDevice> src, Time txTime) override;
    Time GetDelay() const;

    /**
   * \brief Get the propagation delay model
   * \return propagation delay
     */
    Ptr<SatellitePropagationDelayModel> GetPropagationDelayModel () const;

    /**
   * \brief Set the propagation delay model
   * \param delay propagation delay
     */
    void SetPropagationDelayModel (Ptr<SatellitePropagationDelayModel> delay);

  private:
    ///< 延迟模型, 速度取光速的97%, 290,798,684.3m/s
    Ptr<SatellitePropagationDelayModel> m_delay;

    TracedCallback<Ptr<const Packet>, // Packet being transmitted
                   Ptr<NetDevice>,    // Transmitting NetDevice
                   Ptr<NetDevice>,    // Receiving NetDevice
                   Time,              // Amount of time to transmit the pkt
                   Time               // Last bit receive time (relative to now)
                   >
        m_txrxPointToPoint;
};

} // namespace ns3

#endif // NS3_ISLCHANNEL_H
