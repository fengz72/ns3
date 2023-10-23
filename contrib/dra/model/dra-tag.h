//
// Created by hejun on 2023/8/11.
//

#ifndef NS3_DRATAG_H
#define NS3_DRATAG_H

#include "ns3/tag.h"

namespace ns3
{

class DRATag: public Tag
{
  public:
    DRATag();
    static TypeId GetTypeId ();
    TypeId GetInstanceTypeId () const override;
    uint32_t GetSerializedSize () const override;
    void Serialize (TagBuffer i) const override;
    void Deserialize (TagBuffer i) override;
    void Print (std::ostream &os) const override;

    void SetType(uint8_t type);
    uint8_t GetType();

    void SetNode(uint16_t node);
    uint16_t GetNode();

    void SetTTL(uint8_t ttl);
    uint8_t GetTTL();

  private:
    // 1 -> hello
    uint8_t m_type;
    uint16_t m_node;
    uint8_t m_ttl;

};

} // namespace ns3

#endif // NS3_DRATAG_H
