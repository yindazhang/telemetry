#include "switch-node.h"

#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/ipv4-header.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/global-value.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/socket.h"

#include "telemetry-header.h"
#include "path-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SwitchNode");

NS_OBJECT_ENSURE_REGISTERED(SwitchNode);


TypeId
SwitchNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SwitchNode")
            .SetParent<Node>()
            .SetGroupName("PointToPoint")
            .AddConstructor<SwitchNode>();
    return tid;
}

SwitchNode::SwitchNode() : Node() {
    m_finalHop = m_orbweaver = m_removeHeader = m_localBatch = false;
    m_array.resize(arrSize);
    Simulator::Schedule(Seconds(2), &SwitchNode::OrbWeaverSend, this);
    Simulator::Schedule(Seconds(2), &SwitchNode::CollectorSend, this);
}

void 
SwitchNode::SetFinalHop(){
    m_finalHop = true;
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_removeHeader = ((OrbWeaver & 0x3) == 0x3);
    m_localBatch = ((OrbWeaver & 0x5) == 0x5);
}

uint32_t
SwitchNode::AddDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    uint32_t index = m_devices.size();
    m_devices.push_back(device);
    device->SetNode(this);
    device->SetIfIndex(index);
    device->SetReceiveCallback(MakeCallback(&SwitchNode::ReceiveFromDevice, this));
    Simulator::ScheduleWithContext(GetId(), Seconds(0.0), &NetDevice::Initialize, device);
    NotifyDeviceAdded(device);
    return index;
}

void 
SwitchNode::AddUdpIpHeader(Ptr<Packet> packet)
{
    UdpHeader udpHeader;
    udpHeader.SetDestinationPort(10000);
    udpHeader.SetSourcePort(10000);
    packet->AddHeader(udpHeader);

    Ipv4Header ipHeader;
    ipHeader.SetSource(Ipv4Address("11.11.11.11"));
    ipHeader.SetDestination(Ipv4Address("10.8.15.1"));
    ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
    ipHeader.SetPayloadSize(packet->GetSize());
    ipHeader.SetTtl(255);
    ipHeader.SetTos(0);
    ipHeader.SetMayFragment();
    packet->AddHeader(ipHeader);
}

bool
SwitchNode::AddPathHeader(Ptr<Packet> packet, uint32_t batchSize)
{
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    uint8_t number = teleHeader.GetNumber();
    if(number < 75){
        for(uint32_t i = 0;i < batchSize;++i){
            PathHeader pathHeader = m_queue.front();
            m_queue.pop();
            packet->AddHeader(pathHeader);
        }
        teleHeader.SetNumber(number + batchSize);
    }
    packet->AddHeader(teleHeader);
    return true;
}

Ptr<Packet> 
SwitchNode::GeneratePacket()
{
    uint32_t batchSize = 1;
    if(m_localBatch)
        batchSize = 4;

    if(m_queue.size() < batchSize)
        return nullptr;

    Ptr<Packet> packet = Create<Packet>(0);
    TelemetryHeader teleHeader;
    teleHeader.SetNumber(0);
    teleHeader.SetTtl(255);
    packet->AddHeader(teleHeader);

    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(1);
    packet->ReplacePacketTag(priorityTag);

    AddPathHeader(packet, batchSize);
    if(!m_removeHeader)
        AddUdpIpHeader(packet);
    return packet;
}

void
SwitchNode::CollectorSend(){
    if(m_finalHop && m_orbweaver){
        Simulator::Schedule(NanoSeconds(12000), &SwitchNode::CollectorSend, this);
        for(auto devId : m_routeCollector){
            if(m_mask.find(devId) == m_mask.end())
                std::cout << "Cannot find devId " << devId << std::endl;
            m_mask[devId] = 0;
            m_mask_counter[devId] = false;
        }
    }
}

void
SwitchNode::OrbWeaverSend(){
    if(m_orbweaver){
        Simulator::Schedule(NanoSeconds(300), &SwitchNode::OrbWeaverSend, this);
        for(auto devId : m_routeOrbWeaver){
            if(m_mask.find(devId) == m_mask.end())
                std::cout << "Cannot find devId " << devId << std::endl;
            if(!m_mask[devId]){
                Ptr<Packet> packet = GeneratePacket();
                if(packet != nullptr){
                    Ptr<NetDevice> dev = m_devices[devId];
                    if(m_removeHeader)
                        dev->Send(packet, dev->GetBroadcast(), 0x0700);
                    else
                        dev->Send(packet, dev->GetBroadcast(), 0x0800);
                }
            }
            m_mask_counter[devId] = 0;
            m_mask[devId] = false;
        }
    }
}

void 
SwitchNode::CacheInfo(PathHeader pathHeader){
    uint32_t hash = pathHeader.Hash();
    uint32_t arrIndex = hash % m_array.size();

    if(m_array[arrIndex].Empty() || !(m_array[arrIndex] == pathHeader)){
        m_array[arrIndex] = pathHeader;

        if(m_queue.size() > m_array.size()){
            m_queue.pop();
            std::cout << "Information Loss" << std::endl;
        }
        m_queue.push(pathHeader);
    }
}


bool
SwitchNode::ReceiveFromDeviceUser(Ptr<Packet> packet)
{
    Ipv4Header ipHeader;
    packet->RemoveHeader(ipHeader);
    uint8_t proto = ipHeader.GetProtocol();
    uint8_t ttl = ipHeader.GetTtl();
    uint32_t src = ipHeader.GetSource().Get();
    uint32_t dst = ipHeader.GetDestination().Get();

    auto vec = m_routeForward[dst];
    if(vec.size() <= 0){
        std::cout << "Unknown Destination for Routing" << std::endl;
        return false;
    }
    if(ttl == 0)
        return false;

    if(proto != 6){
        std::cout << "Unknown Protocol" << std::endl;
        return false;
    }
    ipHeader.SetTtl(ttl - 1);

    uint32_t devId = -1;

    TcpHeader tcpHeader;
    packet->RemoveHeader(tcpHeader);

    uint16_t srcPort = tcpHeader.GetSourcePort();
    uint16_t dstPort = tcpHeader.GetDestinationPort();

    PathHeader pathHeader;
    memset((char*)(&pathHeader), 0, sizeof(PathHeader));
    pathHeader.SetSrcIP(src);
    pathHeader.SetDstIP(dst);
    pathHeader.SetSrcPort(srcPort);
    pathHeader.SetDstPort(dstPort);
    pathHeader.SetProtocol(proto);

    uint32_t hash = pathHeader.Hash();
    devId = vec[hash % vec.size()];

    if(m_orbweaver){
        pathHeader.SetNodeId(m_id);
        pathHeader.SetTTL(ttl);
        CacheInfo(pathHeader);
    }

    packet->AddHeader(tcpHeader);
    packet->AddHeader(ipHeader);

    if(m_mask.find(devId) != m_mask.end()){
        m_mask_counter[devId] += packet->GetSize();
        if(m_mask_counter[devId] > 1400)
            m_mask[devId] = true;
    }

    Ptr<NetDevice> dev = m_devices[devId];
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

bool
SwitchNode::ReceiveFromDeviceIdle(Ptr<Packet> packet, uint16_t protocol)
{
    Ipv4Header ipHeader;
    UdpHeader udpHeader;
    TelemetryHeader teleHeader;
    uint8_t ttl;

    if(protocol == 0x0800){
        packet->RemoveHeader(ipHeader);
        uint8_t proto = ipHeader.GetProtocol();
        ttl = ipHeader.GetTtl();
        ipHeader.SetTtl(ttl - 1);
        if(proto != 17){
            std::cout << "Not UDP protocol for IDLE" << std::endl;
            return false;
        }
        packet->RemoveHeader(udpHeader);
    }
    else{
        packet->RemoveHeader(teleHeader);
        ttl = teleHeader.GetTtl();
        teleHeader.SetTtl(ttl - 1);
        packet->AddHeader(teleHeader);
    }

    if(ttl == 0){
        return false;
    }

    if(m_routeCollector.size() == 0){
        std::cout << "No Route to collector" << std::endl;
        return false;
    }

    uint32_t devId = m_routeCollector[rand() % m_routeCollector.size()];

    if(protocol == 0x0800){
        packet->AddHeader(udpHeader);
        ipHeader.SetPayloadSize(packet->GetSize());
        packet->AddHeader(ipHeader);
    }

    if(m_mask.find(devId) != m_mask.end()){
        m_mask_counter[devId] += packet->GetSize();
        if(m_mask_counter[devId] > 1400)
            m_mask[devId] = true;
    }

    Ptr<NetDevice> dev = m_devices[devId];
    return dev->Send(packet, dev->GetBroadcast(), protocol);
}

bool
SwitchNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    if(protocol != 0x0700 && protocol != 0x0800){
        std::cout << "Unknown protocol" << std::endl;
        return false;
    }

    Ptr<Packet> packet = p->Copy();

    uint32_t priority = 0;
    SocketPriorityTag priorityTag;
    if(packet->PeekPacketTag(priorityTag)){
        priority = (priorityTag.GetPriority() & 0x1);
    }

    if(priority == 0){
        if(protocol != 0x0800){
            std::cout << "Unknown protocol" << std::endl;
            return false;
        }
        return ReceiveFromDeviceUser(packet);
    }
    else{
        return ReceiveFromDeviceIdle(packet, protocol);
    }
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void
SwitchNode::AddHostRouteCollector(uint32_t devId)
{
    m_mask[devId] = false; 
    m_mask_counter[devId] = 0;
    m_routeCollector.push_back(devId);
}

void
SwitchNode::AddHostRouteOrbWeaver(uint32_t devId)
{
    m_mask[devId] = false; 
    m_mask_counter[devId] = 0;
    m_routeOrbWeaver.push_back(devId);
}

} // namespace ns3
