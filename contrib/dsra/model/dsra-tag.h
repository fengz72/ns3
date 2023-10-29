//
// Created by hejun on 2023/10/28.
//

#ifndef NS3_DSRATAG_H
#define NS3_DSRATAG_H

#include "ns3/tag.h"
namespace ns3
{

class DsraTag: public Tag
{
  public:
    DsraTag();
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

    void SetErrorNode(uint16_t error);
    uint16_t GetErrorNode();

  private:
    // 0 -- hello
    // 2 -- error
    // 3 -- repair
    uint8_t m_type;

    uint16_t m_node;
    uint8_t m_ttl;
    uint16_t m_errorNode;
};

} // namespace ns3

#endif // NS3_DSRATAG_H
