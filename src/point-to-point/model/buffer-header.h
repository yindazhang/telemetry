#ifndef BUFFER_HEADER_H
#define BUFFER_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class BufferHeader : public Header
{
public:

    BufferHeader();
    BufferHeader(const BufferHeader& bufferHeader);
    ~BufferHeader() override;

    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;

    void SetBuffer(uint16_t _buffer);
    uint16_t GetBuffer();

    bool operator == (const BufferHeader& o);

    uint16_t m_buffer;
};

} // namespace ns3

#endif /* BUFFER_HEADER_H */