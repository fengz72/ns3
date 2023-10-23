//
// Created by hejun on 2023/8/8.
//

#ifndef NS3_SATELLITE_CHANNEL_HELPER_H
#define NS3_SATELLITE_CHANNEL_HELPER_H

#include "ns3/point-to-point-helper.h"

namespace ns3
{

class SatelliteChannelHelper : public PcapHelperForDevice,
                               public AsciiTraceHelperForDevice
{
  public:
    SatelliteChannelHelper();

    /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::IslNetDevice created
   * by IslHelper::Install
     */
    void SetDeviceAttribute (std::string name, const AttributeValue &value);

    /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::IslChannel created
   * by IslHelper::Install
     */
    void SetChannelAttribute (std::string name, const AttributeValue &value);

    void DisableFlowControl();

    /**
   * \param c a set of nodes
   * \return a NetDeviceContainer for nodes
   *
   * This method creates a ns3::IslChannel with the
   * attributes configured by IslHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a
   * ns3::IslNetDevice with the requested attributes,
   * a queue for this ns3::NetDevice, and associate the resulting
   * ns3::NetDevice with the ns3::Node and ns3::IslChannel.
     */
    NetDeviceContainer Install (NodeContainer c);
    NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);

    /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
     */
    void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename) override;

    /**
   * \brief Enable ascii trace output on the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
     */
    void EnableAsciiInternal (
        Ptr<OutputStreamWrapper> stream,
        std::string prefix,
        Ptr<NetDevice> nd,
        bool explicitFilename) override;

  private:
    ObjectFactory m_queueFactory;         //!< Queue Factory
    ObjectFactory m_channelFactory;       //!< Channel Factory
    ObjectFactory m_deviceFactory;        //!< Device Factory
    bool m_enableFlowControl;
};

} // namespace ns3

#endif // NS3_SATELLITE_CHANNEL_HELPER_H
