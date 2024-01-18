#include "count-header.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/header.h"
#include "ns3/log.h"

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CountHeader");

NS_OBJECT_ENSURE_REGISTERED(CountHeader);


MyFlowId::MyFlowId()
{
    m_srcIP = 0;
    m_dstIP = 0;
    m_srcPort = 0;
    m_dstPort = 0;
}

MyFlowId::MyFlowId(const MyFlowId& o)
{
    m_srcIP = o.m_srcIP;
    m_dstIP = o.m_dstIP;
    m_srcPort = o.m_srcPort;
    m_dstPort = o.m_dstPort;
}

bool operator == (const MyFlowId& a, const MyFlowId& b){
    return (a.m_srcIP == b.m_srcIP) && (a.m_dstIP == b.m_dstIP) &&
        (a.m_srcPort == b.m_srcPort) && (a.m_dstPort == b.m_dstPort);
}

bool operator < (const MyFlowId& a, const MyFlowId& b){
    return std::tie(a.m_srcIP, a.m_dstIP, a.m_srcPort, a.m_dstPort) <
          std::tie(b.m_srcIP, b.m_dstIP, b.m_srcPort, b.m_dstPort);
}

CountHeader::CountHeader()
{
    m_flow.m_srcIP = 0;
    m_flow.m_dstIP = 0;
    m_flow.m_srcPort = 0;
    m_flow.m_dstPort = 0;
    m_count = 0;
}

CountHeader::CountHeader(const CountHeader& o)
{
    m_flow.m_srcIP = o.m_flow.m_srcIP;
    m_flow.m_dstIP = o.m_flow.m_dstIP;
    m_flow.m_srcPort = o.m_flow.m_srcPort;
    m_flow.m_dstPort = o.m_flow.m_dstPort;
    m_count = o.m_count;
}

CountHeader::~CountHeader()
{
}

TypeId
CountHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CountHeader")
                            .SetParent<Header>()
                            .SetGroupName("PointToPoint")
                            .AddConstructor<CountHeader>();
    return tid;
}

TypeId
CountHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
CountHeader::Print(std::ostream& os) const
{
}

uint32_t
CountHeader::GetSerializedSize() const
{
    return 16;
}

void
CountHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU32(m_flow.m_srcIP);
    start.WriteHtonU32(m_flow.m_dstIP);
    start.WriteHtonU16(m_flow.m_srcPort);
    start.WriteHtonU16(m_flow.m_dstPort);
    start.WriteHtonU32(m_count);
}

uint32_t
CountHeader::Deserialize(Buffer::Iterator start)
{
    m_flow.m_srcIP = start.ReadNtohU32();
    m_flow.m_dstIP = start.ReadNtohU32();
    m_flow.m_srcPort = start.ReadNtohU16();
    m_flow.m_dstPort = start.ReadNtohU16();
    m_count = start.ReadNtohU32();
    return GetSerializedSize();
}

bool 
CountHeader::operator == (const CountHeader& o)
{
    return (m_flow.m_srcIP == o.m_flow.m_srcIP) && (m_flow.m_dstIP == o.m_flow.m_dstIP) &&
        (m_flow.m_srcPort == o.m_flow.m_srcPort) && (m_flow.m_dstPort == o.m_flow.m_dstPort) &&
        (m_count == o.m_count);
}

bool 
CountHeader::Empty()
{
    return (m_flow.m_srcIP == 0) && (m_flow.m_dstIP == 0) &&
        (m_flow.m_srcPort == 0) && (m_flow.m_dstPort == 0) &&
        (m_count == 0);
}

uint32_t
CountHeader::Hash()
{
    Hasher hasher;
    hasher.GetHash32((char*)(&m_flow.m_srcIP), 4);
    hasher.GetHash32((char*)(&m_flow.m_dstIP), 4);
    hasher.GetHash32((char*)(&m_flow.m_srcPort), 2);
    hasher.GetHash32((char*)(&m_flow.m_dstPort), 2);
    return hasher.GetHash32((char*)(&m_count), 4);
}

void 
CountHeader::SetFlow(MyFlowId _flow){
    m_flow.m_srcIP = _flow.m_srcIP;
    m_flow.m_dstIP = _flow.m_dstIP;
    m_flow.m_srcPort = _flow.m_srcPort;
    m_flow.m_dstPort = _flow.m_dstPort;
}
    
MyFlowId
CountHeader::GetFlow(){
    return m_flow;
}

void 
CountHeader::SetSrcIP(uint32_t _srcIP)
{
    m_flow.m_srcIP = _srcIP;
}

uint32_t 
CountHeader::GetSrcIP()
{
    return m_flow.m_srcIP;
}

void 
CountHeader::SetDstIP(uint32_t _dstIP)
{
    m_flow.m_dstIP = _dstIP;
}

uint32_t 
CountHeader::GetDstIP()
{
    return m_flow.m_dstIP;
}

void 
CountHeader::SetSrcPort(uint16_t _srcPort)
{
    m_flow.m_srcPort = _srcPort;
}

uint16_t 
CountHeader::GetSrcPort()
{
    return m_flow.m_srcPort;
}

void 
CountHeader::SetDstPort(uint16_t _dstPort)
{
    m_flow.m_dstPort = _dstPort;
}

uint16_t 
CountHeader::GetDstPort()
{
    return m_flow.m_dstPort;
}

void 
CountHeader::SetCount(int32_t _count)
{
    m_count = _count;
}

int32_t 
CountHeader::GetCount()
{
    return m_count;
}

bool operator < (const CountHeader& a, const CountHeader& b)
{
    return std::tie(a.m_flow.m_srcIP, a.m_flow.m_dstIP, a.m_flow.m_srcPort, a.m_flow.m_dstPort, a.m_count) <
          std::tie(b.m_flow.m_srcIP, b.m_flow.m_dstIP, b.m_flow.m_srcPort, b.m_flow.m_dstPort, b.m_count);
}

} // namespace ns3