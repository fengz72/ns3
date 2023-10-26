#ifndef CONF_LOADER_H
#define CONF_LOADER_H

#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"

#include <iostream>
#include <map>
#include <stdint.h>
#include <string>

using namespace std;

namespace ns3
{

class NodeContainer;
class Ipv4Address;

class DRAConfLoader
{
  public:
    static DRAConfLoader* Instance();

    void setNodeContainer(NodeContainer& nc);
    NodeContainer& getNodeContainer();

    void prepareLinkDown();

    void setCurrentTime(Time time);
    Time getDiffTime();

    Time& getStartTime();
    Time& getStopTime();

    void setUnavailableInterval(int UnavailableInterval);

    int getUnavailableInterval();

    string getHelloMsgString();

    string PrintMap(map<int, int> m);

    void incrementLossPacketCounter(int id);
    map<int, int>& getLossPacketCounter();

    void incrementSuccessPacket(int id);
    map<int, int>& getSuccessPacket();
    void incrementSuccessDataPacket(int id);
    map<int, int>& getSuccessDataPacket();

    void incrementSendPacket(int id);
    map<int, int>& getSendPacket();
    void incrementSendDataPacket(int id);
    map<int, int>& getSendDataPacket();
    void incrementSendContrPacket(int id);
    map<int, int>& getSendContrPacket();
    void incrementSendHelloPacket(int id);
    map<int, int>& getSendHelloPacket();

    void incrementRecvPacket(int id);
    map<int, int>& getRecvPacket();

    uint32_t getPacketReceiveDelay();
    void setPacketReceiveDelay(uint32_t delay);

    void setCalculateCost(float cost);
    float getCalculateCost();

    void setLSPDelay(float cost);
    float getLSPDelay();

    void setOrbits(uint16_t orbits);
    uint16_t getOrbits();

    void setSats(uint16_t sats);
    uint16_t getSats();

    std::pair<uint16_t, uint16_t> GetSatPair(uint16_t id);
    std::pair<uint16_t, uint16_t> GetSatPair(Ipv4Address ip);

    uint16_t GetSatId(Ipv4Address ip);
    uint16_t GetSatId(pair<uint16_t, uint16_t> satPair);

    int GetType(Ipv4Address ip);

  private:
    DRAConfLoader()
    {
        isDown = false;
    };

    DRAConfLoader(const DRAConfLoader&){};
    static DRAConfLoader* m_pInstance;

    float m_CalculateCost;
    float m_LSPDelay;
    uint32_t m_PacketReceiveDelay;

    bool isDown;
    Time m_startTime;
    Time m_stopTime;
    int m_UnavailableInterval; // 超过这个时间, 认为链路断开

    map<int, int> m_lossPacketCounter; // 每个节点丢失的包

    map<int, int> m_SendPacket; // 每个节点发送的包(不包含转发)
    map<int, int> m_SendDataPacket; // 发送的数据包
    map<int, int> m_SendContrPacket; // 发送的信令包 (包含 hello 包)
    map<int, int> m_SendHelloPacket; // 发送的 hello 包

    map<int, int> m_SuccessPacket; // 以自己为目的的包
    map<int, int> m_SuccessDataPacket; // 以自己为目的数据包

    map<int, int> m_RecvPacket; // 收到的包

    NodeContainer m_nodes;

    uint16_t m_orbits;
    uint16_t m_sats;
};

enum NodeType: int
{
    SatelliteNode = 1,
    GsNode = 2
};

} // namespace ns3

#endif /* CONF_LOADER_H */