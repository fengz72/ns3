#ifndef DRLF_H
#define DRLF_H

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup drlf Description of the drlf
 */

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/drlf-module.h"

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup drlf directive

class DrlfRouting: public Ipv4RoutingProtocol
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    DrlfRouting();

    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                               const Ipv4Header& header,
                               Ptr<NetDevice> oif,
                               Socket::SocketErrno& sockerr) override;
    bool RouteInput(Ptr<const Packet> p,
                    const Ipv4Header& header,
                    Ptr<const NetDevice> idev,
                    UnicastForwardCallback ucb,
                    MulticastForwardCallback mcb,
                    LocalDeliverCallback lcb,
                    ErrorCallback ecb) override;

    void NotifyInterfaceUp(uint32_t interface) override;
    void NotifyInterfaceDown(uint32_t interface) override;
    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
    void SetIpv4(Ptr<Ipv4> ipv4) override;
    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                           Time::Unit unit = Time::S) const override;
    void DoDispose () override;

    Ptr<Ipv4Route> LookUpTable(Ipv4Address dest, int iif);
    void HandleMessage(Ptr<const Packet> packet, Ptr<const NetDevice> idev);

    void AddNeighbor(uint16_t id, int oif, Time time, double cost);
    void CheckNeighbor();

    void SendHello();
    void SendAllLSA();
    void SendLSA(uint8_t type, uint16_t node, uint16_t lsaNode, double cost);
    void SendError(uint16_t node, uint16_t lsaNode);
    void Forward(DrlfTag& tag);
    void Dijkstra();

    void SetId(int id);
    int GetId();

    void AddGs(uint32_t ip, int oif);
  private:
    int m_id;
    Ptr<Ipv4> m_ipv4;
    DrlfTable m_table;
    std::map<uint16_t, Ptr<DrlfTableEntry>> m_neighbor; // 邻居卫星
    std::map<uint16_t , int> m_gs; // 地面站设备
    std::map<std::pair<uint16_t, uint16_t>, double> m_linkStateDataBase; // 链路状态数据库
};

}

#endif /* DRLF_H */
