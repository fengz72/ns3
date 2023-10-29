//
// Created by hejun on 2023/10/10.
//

#ifndef NS3_DSRACONFIG_H
#define NS3_DSRACONFIG_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/ipv4.h"

namespace ns3
{

class DsraConfig
{
  public:
    static DsraConfig* Instance();

    std::pair<uint16_t, uint16_t> GetSatPair(uint16_t id);
    std::pair<uint16_t, uint16_t> GetSatPair(Ipv4Address ip);
    uint16_t GetSatId(Ipv4Address ip);
    uint16_t GetSatId(std::pair<uint16_t, uint16_t> satPair);
    int GetType(Ipv4Address ip);

    bool IsBlock(std::pair<uint16_t, uint16_t> source,
                 std::pair<uint16_t, uint16_t> dest,
                 int oif);

    uint8_t m_ttl;
    uint16_t m_P;
    uint16_t m_S;
    int m_UnavailableInterval;
    std::set<std::pair<uint16_t, uint16_t>> m_block;

    void SetNodeContainer(NodeContainer& nc);
    NodeContainer& GetNodeContainer();

  private:
    DsraConfig();
    static DsraConfig* m_instance;
    std::pair<uint16_t, uint16_t> Next(std::pair<uint16_t, uint16_t> cur, int oif);

    NodeContainer m_nodes;
};

enum NodeType: int
{
    SatelliteNode = 1,
    GsNode = 2
};

} // namespace ns

#endif // NS3_DSRACONFIG_H
