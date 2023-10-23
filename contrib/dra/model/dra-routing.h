//
// Created by hejun on 2023/8/11.
//

#ifndef NS3_DRARouting_H
#define NS3_DRARouting_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "dra-table-entry.h"

namespace ns3
{

class DRARouting: public Ipv4RoutingProtocol
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    DRARouting();
    ~DRARouting() override;

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

    void SendHello();
    void AddNeighbor(uint16_t id, int iif, Time time);
    void CheckNeighbor();
    /// 添加地面站
    void AddGs(uint32_t ip, int oif);

    /// 处理信令包
    void HandleMessage(Ptr<const Packet> packet, Ptr<const ns3::NetDevice> idev);
    Ptr<Ipv4Route> LookUpTable(Ipv4Address dest, int iif);
    /// 计算转发端口, 使用LSP策略
    void calcOif(std::pair<uint16_t, uint16_t> source,
                             std::pair<uint16_t, uint16_t> dest,
                             std::vector<int> &res);

    void SetId(uint16_t id);
    uint16_t GetId();
  private:
    std::map<int , Ptr<DRATableEntry>> m_table; // interface: entry
    std::map<uint32_t, int> m_gs; // ip: interface
    uint16_t m_id;
    Ptr<Ipv4> m_ipv4;

    enum Interface: int {
        Up = 1,
        Down = 3,
        Right = 2,
        Left = 4,
    };
};

} // namespace ns3

#endif // NS3_DRARouting_H
