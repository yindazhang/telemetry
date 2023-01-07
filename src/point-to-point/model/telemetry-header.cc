#include "telemetry-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TelemetryHeader");

NS_OBJECT_ENSURE_REGISTERED(TelemetryHeader);

TelemetryHeader::TelemetryHeader()
{
}

TelemetryHeader::~TelemetryHeader()
{
}

TypeId
TelemetryHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TelemetryHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<TelemetryHeader>();
    return tid;
}

TypeId
TelemetryHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
TelemetryHeader::Print(std::ostream& os) const
{
}

uint32_t
TelemetryHeader::GetSerializedSize() const
{
    return 1;
}

void
TelemetryHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(m_number);
}

uint32_t
TelemetryHeader::Deserialize(Buffer::Iterator start)
{
    m_number = start.ReadU8();
    return GetSerializedSize();
}

void 
TelemetryHeader::SetNumber(uint8_t _number)
{
    m_number = _number;
}

uint8_t 
TelemetryHeader::GetNumber()
{
    return m_number;
}

} // namespace ns3