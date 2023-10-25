//
// Created by hejun on 2023/9/4.
//

#ifndef NS3_DRLFTAG_H
#define NS3_DRLFTAG_H

#include "ns3/tag.h"
#include "ns3/timer.h"
#include "ns3/double.h"

namespace ns3 {

class DrlfTag: public Tag
{
  public:
    DrlfTag();
    static TypeId GetTypeId (void);
    TypeId GetInstanceTypeId () const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    void SetType(uint8_t type);
    uint8_t GetType();

    void SetNode(uint16_t node);
    uint16_t GetNode();

    void SetTTL(uint8_t ttl);
    uint8_t GetTTL();
    void DeTTL();

    void SetTime(Time time);
    Time GetTime();

    void SetCost(double cost);
    double GetCost();

    void SetLsaNode(uint16_t lsaNode);
    uint16_t GetLsaNode();

  private:
    // 0 -- hello
    // 1 -- lsa
    // 2 -- error
    uint8_t m_type;

    uint16_t m_node;

    uint8_t m_ttl;
    Time m_sendTime;

    double m_cost;
    uint16_t m_lsaNode;
};



}



#endif // NS3_DRLFTAG_H
