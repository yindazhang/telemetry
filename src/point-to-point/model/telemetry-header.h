#ifndef TELEMETRY_HEADER_H
#define TELEMETRY_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class TelemetryHeader : public Header
{
public:

    TelemetryHeader();
    ~TelemetryHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    void SetNumber(uint8_t _number);
    uint8_t GetNumber();

private:
    uint8_t m_number;
};

} // namespace ns3

#endif /* TELEMETRY_HEADER_H */