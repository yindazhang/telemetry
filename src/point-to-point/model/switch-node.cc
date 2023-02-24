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
    m_orbweaver = false;
    m_array.resize(arrSize);
    Simulator::Schedule(Seconds(2), &SwitchNode::CollectorSend, this);
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
}

void 
SwitchNode::SetEcmp(uint32_t Ecmp){
    m_ecmp = Ecmp;
}

void 
SwitchNode::SetFinalSwitch(){
    m_finalSwitch = true;
}

void 
SwitchNode::SetCollectorGap(uint32_t CollectorGap){
    m_collectorGap = CollectorGap;
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void
SwitchNode::AddHostRouteCollector(uint32_t devId)
{
    // m_mask[devId] = false; 
    // m_mask_counter[devId] = 0;
    m_routeCollector.push_back(devId);
}

void
SwitchNode::AddHostRoutePolling(uint32_t devId)
{
    // m_mask[devId] = false; 
    // m_mask_counter[devId] = 0;
    m_routePolling.push_back(devId);
}

void
SwitchNode::AddHostRoutePushing(uint32_t devId)
{
    // m_mask[devId] = false; 
    // m_mask_counter[devId] = 0;
    m_routePushing.push_back(devId);
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

/*
void 
SwitchNode::ClearOrbWeaverMask(uint32_t devId){
    m_mask[devId] = false;
    m_mask_counter[devId] = 0;
}

void 
SwitchNode::SetOrbWeaverMask(Ptr<Packet> packet, uint32_t devId){
    if(m_mask.find(devId) != m_mask.end()){
        m_mask_counter[devId] += packet->GetSize();
        if(m_mask_counter[devId] > 1400)
            m_mask[devId] = true;
    }
}
*/

bool
SwitchNode::AddPathHeader(Ptr<Packet> packet)
{   
    if(m_queue.size() < batchSize)
        return false;

    // Start Parse Header
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    // End Parse Header

    uint8_t number = teleHeader.GetNumber();
    if(number < 75){
        for(uint32_t i = 0;i < batchSize;++i){
            PathHeader pathHeader = m_queue.front();
            m_queue.pop();
            packet->AddHeader(pathHeader);
        }
        teleHeader.SetNumber(number + batchSize);
    }

    // Start Deparse Header
    packet->AddHeader(teleHeader);
    // End Deparse Header

    return true;
}

Ptr<Packet> 
SwitchNode::GeneratePacket(uint8_t number)
{
    Ptr<Packet> packet = Create<Packet>(0);
    TelemetryHeader teleHeader;
    teleHeader.SetNumber(number);
    teleHeader.SetTtl(255);
    packet->AddHeader(teleHeader);

    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(1);
    packet->ReplacePacketTag(priorityTag);

    return packet;
}

void
SwitchNode::CollectorSend(){
    if(m_orbweaver){
        Simulator::Schedule(NanoSeconds(m_collectorGap), &SwitchNode::CollectorSend, this);

        for(auto devId : m_routeCollector){
            Ptr<Packet> packet = GeneratePacket(0);
            if(packet != nullptr){
                Ptr<NetDevice> dev = m_devices[devId];
                dev->Send(packet, dev->GetBroadcast(), 0x0700);
            }
        }
        for(auto devId : m_routePolling){
            Ptr<Packet> packet = GeneratePacket(255);
            if(packet != nullptr){
                Ptr<NetDevice> dev = m_devices[devId];
                dev->Send(packet, dev->GetBroadcast(), 0x0700);
            }
        }
        for(auto devId : m_routePushing){
            Ptr<Packet> packet = GeneratePacket(254);
            if(packet != nullptr){
                Ptr<NetDevice> dev = m_devices[devId];
                dev->Send(packet, dev->GetBroadcast(), 0x0700);
            }
        }
    }
}

void 
SwitchNode::CacheInfo(Ptr<Packet> packet){
    // Start Parse Header
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    // End Parse Header

    uint8_t number = teleHeader.GetNumber();
    if(number == 0 || number > 80)
        return;

    for(uint8_t i = 0;i < number;++i){
        PathHeader pathHeader;
        packet->RemoveHeader(pathHeader);

        if(m_queue.size() > queueSize){
            m_queue.pop();
            std::cout << "Information Loss for IDLE" << std::endl;
        }
        m_queue.push(pathHeader);
    }
}

void 
SwitchNode::CacheInfo(PathHeader pathHeader){
    uint32_t hash = pathHeader.Hash();
    uint32_t arrIndex = hash % m_array.size();

    if(m_array[arrIndex].Empty() || !(m_array[arrIndex] == pathHeader)){
        m_array[arrIndex] = pathHeader;

        if(m_queue.size() > queueSize){
            m_queue.pop();
            std::cout << "Information Loss for User" << std::endl;
        }
        m_queue.push(pathHeader);
    }
}


bool
SwitchNode::IngressPipelineUser(Ptr<Packet> packet)
{
    // Start Parse Header
    Ipv4Header ipHeader;
    TcpHeader tcpHeader;

    packet->RemoveHeader(ipHeader);
    packet->RemoveHeader(tcpHeader);
    // End Parse Header

    uint8_t proto = ipHeader.GetProtocol();
    uint8_t ttl = ipHeader.GetTtl();
    uint32_t src = ipHeader.GetSource().Get();
    uint32_t dst = ipHeader.GetDestination().Get();

    uint16_t srcPort = tcpHeader.GetSourcePort();
    uint16_t dstPort = tcpHeader.GetDestinationPort();

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

    // Start Deparse Header
    packet->AddHeader(tcpHeader);
    packet->AddHeader(ipHeader);
    // End Deparse Header

    uint32_t devId = -1;

    PathHeader pathHeader;
    memset((char*)(&pathHeader), 0, sizeof(PathHeader));
    pathHeader.SetSrcIP(src);
    pathHeader.SetDstIP(dst);
    pathHeader.SetSrcPort(srcPort);
    pathHeader.SetDstPort(dstPort);
    pathHeader.SetProtocol(proto);

    // Get route of the user
    uint32_t hash = pathHeader.Hash();
    if(m_ecmp == 1)
        hash = Hash32((char*)&dst, sizeof(dst));

    devId = vec[hash % vec.size()];
    // SetOrbWeaverMask(packet, devId);
    Ptr<NetDevice> dev = m_devices[devId];
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

bool 
SwitchNode::EgressPipelineUser(Ptr<Packet> packet){
    if(!m_orbweaver)
        return true;
    
    // Start Parse Header
    Ipv4Header ipHeader;
    TcpHeader tcpHeader;

    packet->RemoveHeader(ipHeader);
    packet->RemoveHeader(tcpHeader);
    // End Parse Header

    uint8_t proto = ipHeader.GetProtocol();
    uint8_t ttl = ipHeader.GetTtl();
    uint32_t src = ipHeader.GetSource().Get();
    uint32_t dst = ipHeader.GetDestination().Get();

    uint16_t srcPort = tcpHeader.GetSourcePort();
    uint16_t dstPort = tcpHeader.GetDestinationPort();


    PathHeader pathHeader;
    memset((char*)(&pathHeader), 0, sizeof(PathHeader));
    pathHeader.SetSrcIP(src);
    pathHeader.SetDstIP(dst);
    pathHeader.SetSrcPort(srcPort);
    pathHeader.SetDstPort(dstPort);
    pathHeader.SetProtocol(proto);

    pathHeader.SetNodeId(m_id);
    pathHeader.SetTTL(ttl);       
    CacheInfo(pathHeader);

    // Start Deparse Header
    packet->AddHeader(tcpHeader);
    packet->AddHeader(ipHeader);
    // End Deparse Header

    return true;
}


bool
SwitchNode::IngressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev)
{
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    uint8_t number = teleHeader.GetNumber();
    uint8_t ttl = teleHeader.GetTtl();
    teleHeader.SetTtl(ttl - 1);
    packet->AddHeader(teleHeader);

    if(number == 0)
        return dev->Send(packet, dev->GetBroadcast(), 0x0700);
    else if(number > 0 && number < 80)
        CacheInfo(packet);
    else
        std::cout << "Unknown number in IDLE packets: " << int(number) << std::endl;
    return false;
}

bool 
SwitchNode::EgressPipelineIdle(Ptr<Packet> packet){
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    uint8_t number = teleHeader.GetNumber();
    packet->AddHeader(teleHeader);

    if(number == 0){
        return AddPathHeader(packet);
    }
    else if(number == 255){
        if(m_queue.size() < queueSize * 0.25){
            packet->RemoveHeader(teleHeader);
            teleHeader.SetNumber(0);
            packet->AddHeader(teleHeader);
            return true;
        }
        else
            return false;
    }
    else if(number == 254){

        if(m_queue.size() < batchSize){
            int64_t nsNow = Simulator::Now().GetNanoSeconds();
            if(nsNow - m_lastTime > 1000000000){
                m_orbweaver = false;
                std::cout << "Node " << m_id << " stops in " << nsNow << " ns" << std::endl;
            }
        }
        else{
            m_lastTime = Simulator::Now().GetNanoSeconds();
        }

        if(m_queue.size() > queueSize * 0.75){
            packet->RemoveHeader(teleHeader);
            teleHeader.SetNumber(0);
            packet->AddHeader(teleHeader);
            return AddPathHeader(packet);
        }
        else
            return false;
    }
    else
        std::cout << "Unknown number in IDLE packets: " << int(number) << std::endl;
    return true;
}

bool
SwitchNode::EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol){
    if(priority == 0){
        if(protocol != 0x0800){
            std::cout << "Unknown protocol for User packet" << std::endl;
            return false;
        }
        return EgressPipelineUser(packet);
    }
    else{
        if(protocol != 0x0700){
            std::cout << "Unknown protocol for IDLE packet" << std::endl;
            return false;
        }
        return EgressPipelineIdle(packet);
    }
}

bool
SwitchNode::IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        if(protocol != 0x0800){
            std::cout << "Unknown protocol for User packet" << std::endl;
            return false;
        }
        return IngressPipelineUser(packet);
    }
    else{
        if(protocol != 0x0700){
            std::cout << "Unknown protocol for IDLE packet" << std::endl;
            return false;
        }
        return IngressPipelineIdle(packet, dev);
    }
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

    return IngressPipeline(packet, priority, protocol, device);
}

} // namespace ns3
