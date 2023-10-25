//
// Created by hejun on 2023/9/11.
//

#ifndef NS3_DRLFCONFIG_H
#define NS3_DRLFCONFIG_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"

using namespace std;

namespace ns3
{

class DrlfConfig
{
  public:
    static DrlfConfig* Instance();

    void SetUnavailableInterval(int unavailableInterval);
    int GetUnavailableInterval();

    void SetNodeContainer(NodeContainer& nc);
    NodeContainer& GetNodeContainer();

    /// 以us为单位
    uint64_t GetCalculateCost();

    uint16_t GetSatId(Ipv4Address ip);
    uint16_t GetSatId(pair<uint16_t, uint16_t> satPair);
    int GetTypeFromIp(Ipv4Address ip);
    pair<uint16_t, uint16_t> GetSatPair(uint16_t id);
    pair<uint16_t, uint16_t> GetSatPair(Ipv4Address);

    void SetMaxHello(Time now);
    Time GetMaxHello();
    void SetMaxLsa(Time now);
    Time GetMaxLsa();

    uint16_t m_P;
    uint16_t m_S;
    uint8_t m_ttl;
    long m_calcCost;

    enum NodeType: int
    {
        SatelliteNode = 1,
        GsNode = 2
    };

  private:
    DrlfConfig();
    static DrlfConfig* m_instance;

    int m_UnavailableInterval;

    NodeContainer m_nodes;


    Time m_maxHello;
    Time m_maxLsa;
    Time m_maxError;

};



} // namespace ns3

#endif // NS3_DRLFCONFIG_H
