#ifndef DSRA_H
#define DSRA_H

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup dsra Description of the dsra
 */

#include "ns3/ipv4-routing-protocol.h"
#include "dsra-table-entry.h"
#include "ns3/ipv4.h"
#include "dsra-tag.h"

namespace ns3
{

class DsraRouting: public Ipv4RoutingProtocol
{
  public:
    static TypeId GetTypeId();
    DsraRouting();
    ~DsraRouting() override;

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
    void DoDispose() override;


    /// 处理信令包
    void HandleMessage(Ptr<const Packet> packet, Ptr<const ns3::NetDevice> idev);
    Ptr<Ipv4Route> LookUpNext(Ipv4Address dest, int iif);
    /// 当前卫星不为目的地时, 计算转发端口
    void CalcPort(std::pair<uint16_t, uint16_t> dest, std::vector<int>& master, std::vector<int>& slave);
    int NextHop(std::pair<uint16_t, uint16_t> dest, std::vector<int>& master, std::vector<int>& slave, int iif);

    void SendHello();
    void SendError(uint16_t node, uint16_t lsaNode);
    void Forward(DsraTag& tag);
    void AddNeighbor(uint16_t id, Time time, int oif);
    void CheckNeighbor();
    void AddGs(uint32_t ip, int oif);

    void SetId(uint32_t id);

  private:
    uint16_t m_id;
    Ptr<Ipv4> m_ipv4;
    std::map<int, Ptr<DsraTableEntry>> m_neighbor;
    std::map<uint32_t , int> m_gs;

    enum Interface: int {
        Up = 1,
        Down = 3,
        Right = 2,
        Left = 4,
    };
};


}

#endif /* DSRA_H */
