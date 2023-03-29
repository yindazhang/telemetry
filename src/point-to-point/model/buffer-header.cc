#include "buffer-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BufferHeader");

NS_OBJECT_ENSURE_REGISTERED(BufferHeader);

BufferHeader::BufferHeader()
{
    m_buffer = 0;
}

BufferHeader::BufferHeader(const BufferHeader& o)
{
    m_buffer = o.m_buffer;
}

BufferHeader::~BufferHeader()
{
}

TypeId
BufferHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::BufferHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<BufferHeader>();
    return tid;
}

TypeId
BufferHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
BufferHeader::Print(std::ostream& os) const
{
}

uint32_t
BufferHeader::GetSerializedSize() const
{
    return 2;
}

void
BufferHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU16(m_buffer);
}

uint32_t
BufferHeader::Deserialize(Buffer::Iterator start)
{
    m_buffer = start.ReadNtohU16();
    return GetSerializedSize();
}

bool 
BufferHeader::operator == (const BufferHeader& o)
{
    return m_buffer == o.m_buffer;
}

void 
BufferHeader::SetBuffer(uint16_t _buffer)
{
    m_buffer = _buffer;
}

uint16_t 
BufferHeader::GetBuffer()
{
    return m_buffer;
}

} // namespace ns3