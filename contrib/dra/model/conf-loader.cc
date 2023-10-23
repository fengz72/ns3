#include <fstream>
#include <iostream>

#include "conf-loader.h"
//#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE ("DRAConfLoader");

namespace ns3{

using namespace std;

DRAConfLoader* DRAConfLoader::m_pInstance = NULL;

DRAConfLoader*
DRAConfLoader::Instance(){
	if(!m_pInstance){
		m_pInstance = new DRAConfLoader;
	}
	return m_pInstance;
}

void
DRAConfLoader::setNodeContainer(NodeContainer& nc){
    m_nodes = nc;
}

NodeContainer&
DRAConfLoader::getNodeContainer(){
    return m_nodes;
}

void
DRAConfLoader::incrementLossPacketCounter(int id)
{
    if (m_lossPacketCounter.find(id) == m_lossPacketCounter.end())
    {
        m_lossPacketCounter[id] = 0;
    }
    m_lossPacketCounter[id]++;
}

void
DRAConfLoader::prepareLinkDown()
{

}

map<int, int>&
DRAConfLoader::getLossPacketCounter()
{
    return this->m_lossPacketCounter;
}

void
DRAConfLoader::setCurrentTime(Time time)
{
    if (!isDown)
    {
        return;
    }
    m_stopTime = time;
    if (m_startTime.IsZero())
    {
        m_startTime = time;
    }
}

Time
DRAConfLoader::getDiffTime()
{
    return m_stopTime - m_startTime;
}

Time&
DRAConfLoader::getStartTime()
{
    return m_startTime;
}

Time&
DRAConfLoader::getStopTime()
{
    return m_stopTime;
}

string
DRAConfLoader::getHelloMsgString()
{
    return string("hello");
}

string
DRAConfLoader::PrintMap(map<int, int> m)
{
    stringstream ss;
    for (map<int, int>::iterator it = m.begin(); it != m.end(); ++it)
    {
        ss << it->first << ":" << it->second << endl;
    }
    return ss.str();
}

void
DRAConfLoader::incrementSuccessPacket(int id)
{
    if (m_SuccessPacket.find(id) == m_SuccessPacket.end())
    {
        m_SuccessPacket[id] = 0;
    }
    m_SuccessPacket[id]++;
}
map<int, int>&
DRAConfLoader::getSuccessPacket()
{
    return m_SuccessPacket;
}

void
DRAConfLoader::incrementSuccessDataPacket(int id)
{
    if (m_SuccessDataPacket.find(id) == m_SuccessDataPacket.end())
    {
        m_SuccessDataPacket[id] = 0;
    }
    m_SuccessDataPacket[id]++;
}
map<int, int>&
DRAConfLoader::getSuccessDataPacket()
{
    return m_SuccessDataPacket;
}

void
DRAConfLoader::incrementSendPacket(int id)
{
    if (m_SendPacket.find(id) == m_SendPacket.end())
    {
        m_SendPacket[id] = 0;
    }
    m_SendPacket[id]++;
}

map<int, int>&
DRAConfLoader::getSendPacket()
{
    return m_SendPacket;
}

void
DRAConfLoader::incrementSendDataPacket(int id)
{
    if (m_SendDataPacket.find(id) == m_SendDataPacket.end())
    {
        m_SendDataPacket[id] = 0;
    }
    m_SendDataPacket[id]++;
}
map<int, int>&
DRAConfLoader::getSendDataPacket()
{
    return m_SendDataPacket;
}

void
DRAConfLoader::incrementSendContrPacket(int id)
{
    if (m_SendContrPacket.find(id) == m_SendContrPacket.end())
    {
        m_SendContrPacket[id] = 0;
    }
    m_SendContrPacket[id]++;
}
map<int, int>&
DRAConfLoader::getSendContrPacket()
{
    return m_SendContrPacket;
}

void
DRAConfLoader::incrementSendHelloPacket(int id)
{
    if (m_SendHelloPacket.find(id) == m_SendHelloPacket.end())
    {
        m_SendHelloPacket[id] = 0;
    }
    m_SendHelloPacket[id]++;
}
map<int, int>&
DRAConfLoader::getSendHelloPacket()
{
    return m_SendHelloPacket;
}

void
DRAConfLoader::incrementRecvPacket(int id)
{
    if (m_RecvPacket.find(id) == m_RecvPacket.end())
    {
        m_RecvPacket[id] = 0;
    }
    m_RecvPacket[id]++;
}

map<int, int>&
DRAConfLoader::getRecvPacket()
{
    return m_RecvPacket;
}

uint32_t
DRAConfLoader::getPacketReceiveDelay()
{
    return m_PacketReceiveDelay;
}

void
DRAConfLoader::setPacketReceiveDelay(uint32_t delay)
{
    m_PacketReceiveDelay = delay;
}

void
DRAConfLoader::setCalculateCost(float cost)
{
    m_CalculateCost = cost;
}

float
DRAConfLoader::getCalculateCost()
{
    return m_CalculateCost;
}

void
DRAConfLoader::setLSPDelay(float cost)
{
    m_LSPDelay = cost;
}

float
DRAConfLoader::getLSPDelay()
{
    return m_LSPDelay;
}

void
DRAConfLoader::setOrbits(uint16_t orbits)
{
    m_orbits = orbits;
}
uint16_t
DRAConfLoader::getOrbits()
{
    return m_orbits;
}

void
DRAConfLoader::setSats(uint16_t sats)
{
    m_sats = sats;
}
uint16_t
DRAConfLoader::getSats()
{
    return m_sats;
}

std::pair<uint16_t, uint16_t>
DRAConfLoader::GetSatPair(uint16_t id)
{
    uint16_t orbit = id / m_sats;
    uint16_t sat = id % m_sats;

    return std::make_pair(orbit, sat);
}

std::pair<uint16_t, uint16_t>
DRAConfLoader::GetSatPair(ns3::Ipv4Address ip)
{
    uint16_t sat = (ip.Get() >> 8) & 0xff;
    uint16_t orbit = (ip.Get() >> 16) & 0xff;

    return std::make_pair(orbit, sat);
}

int
DRAConfLoader::GetType(ns3::Ipv4Address ip)
{
    return (ip.Get() >> 24) & 0xff;
}

uint16_t
DRAConfLoader::GetSatId(ns3::Ipv4Address ip)
{
    uint16_t sat = (ip.Get() >> 8) & 0xff;
    uint16_t orbit = (ip.Get() >> 16) & 0xff;

    return orbit * m_sats + sat;
}

uint16_t
DRAConfLoader::GetSatId(pair<uint16_t, uint16_t > satPair)
{
    return satPair.first * m_sats + satPair.second;
}

void
DRAConfLoader::setUnavailableInterval(int UnavailableInterval)
{
    this->m_UnavailableInterval = UnavailableInterval;
};

int
DRAConfLoader::getUnavailableInterval()
{
    return this->m_UnavailableInterval;
}



}
