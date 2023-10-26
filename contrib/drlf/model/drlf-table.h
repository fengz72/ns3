//
// Created by hejun on 2023/9/5.
//

#ifndef NS3_DRLFTABLE_H
#define NS3_DRLFTABLE_H

#include "ns3/timer.h"

namespace ns3
{

class DrlfTableEntry: public Object
{
  public:
    static TypeId GetTypeId();

    DrlfTableEntry();
    DrlfTableEntry(uint16_t id, int oif, Time time, double cost);

    void SetId(uint16_t id);
    uint16_t GetId();

    void SetOif(int oif);
    int GetOif();

    void SetTime(Time time);
    Time GetTime();

    void SetCost(double cost);
    double GetCost();

  private:
    uint16_t m_id;
    int m_oif;
    Time m_update;
    double m_cost;
};

class DrlfTable
{
  public:
    DrlfTable();
    ~DrlfTable();

    void AddEntry(uint16_t id, int oif, double cost);
    void DeleteEntry(uint16_t id);
    void Clear();
    Ptr<DrlfTableEntry> GetEntry(uint16_t destId);
    Ptr<DrlfTableEntry> CalcNextHop(uint16_t destId);

  private:
    std::map<uint16_t, Ptr<DrlfTableEntry>> m_table;
};

} // namespace ns3

#endif // NS3_DRLFTABLE_H
