#ifndef COUNT_HEADER_H
#define COUNT_HEADER_H

#include "ns3/header.h"

namespace ns3
{

struct MyFlowId
{
    uint32_t m_srcIP;
    uint32_t m_dstIP;
    uint16_t m_srcPort;
    uint16_t m_dstPort;

    MyFlowId();
    MyFlowId(const MyFlowId& flow);
};

bool operator == (const MyFlowId& a, const MyFlowId& b);
bool operator < (const MyFlowId& a, const MyFlowId& b);

class CountHeader : public Header
{
public:

    CountHeader();
    CountHeader(const CountHeader& countHeader);
    ~CountHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    bool Empty();

    void SetFlow(MyFlowId _flow);
    MyFlowId GetFlow();
    void SetSrcIP(uint32_t _srcIP);
    uint32_t GetSrcIP();
    void SetDstIP(uint32_t _dstIP);
    uint32_t GetDstIP();
    void SetSrcPort(uint16_t _srcPort);
    uint16_t GetSrcPort();
    void SetDstPort(uint16_t _dstPort);
    uint16_t GetDstPort();
    void SetCount(int32_t _count);
    int32_t GetCount();

    uint32_t Hash();

    bool operator == (const CountHeader& o);

    MyFlowId m_flow;
    int32_t m_count;
};

bool operator < (const CountHeader& a, const CountHeader& b);

} // namespace ns3

#endif /* COUNT_HEADER_H */