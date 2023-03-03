#include "buffer-switch.h"

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

NS_LOG_COMPONENT_DEFINE("BufferSwitch");

NS_OBJECT_ENSURE_REGISTERED(BufferSwitch);


TypeId
BufferSwitch::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::BufferSwitch")
            .SetParent<Node>()
            .SetGroupName("PointToPoint")
            .AddConstructor<BufferSwitch>();
    return tid;
}

BufferSwitch::BufferSwitch() : Node() {
    m_orbweaver = false;
    m_table.resize(arrSize);
    Simulator::Schedule(Seconds(2), &BufferSwitch::GeneratePacket, this);
}

void
BufferSwitch::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
}

void 
BufferSwitch::SetEcmp(uint32_t Ecmp){
    m_ecmp = Ecmp;
}

void
BufferSwitch::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void 
BufferSwitch::SetGenerateGap(uint32_t devId, uint32_t generateGap){
    DeviceProperty tmp;
    tmp.devId = devId;
    tmp.generateGap = generateGap;
    m_deviceMap[m_devices[devId]] = tmp;
}

void
BufferSwitch::SetDeviceCollector(uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap" << std::endl;
    else
        m_deviceMap[m_devices[devId]].isCollector = true;
}

uint32_t
BufferSwitch::AddDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    uint32_t index = m_devices.size();
    m_devices.push_back(device);
    device->SetNode(this);
    device->SetIfIndex(index);
    device->SetReceiveCallback(MakeCallback(&BufferSwitch::ReceiveFromDevice, this));
    Simulator::ScheduleWithContext(GetId(), Seconds(0.0), &NetDevice::Initialize, device);
    NotifyDeviceAdded(device);
    return index;
}

bool
BufferSwitch::AddPathHeader(Ptr<Packet> packet)
{   
    if(m_buffer.size() < batchSize)
        return false;

    // Start Parse Header
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    // End Parse Header

    uint8_t number = teleHeader.GetNumber();
    if(number < 75){
        for(uint32_t i = 0;i < batchSize;++i){
            PathHeader pathHeader = m_buffer.front();
            m_buffer.pop();
            packet->AddHeader(pathHeader);
        }
        teleHeader.SetNumber(batchSize);
    }

    // Start Deparse Header
    packet->AddHeader(teleHeader);
    // End Deparse Header

    return true;
}

Ptr<Packet> 
BufferSwitch::CreatePacket(uint8_t type)
{
    Ptr<Packet> packet = Create<Packet>(0);
    TelemetryHeader teleHeader;
    teleHeader.SetNumber(type);
    packet->AddHeader(teleHeader);

    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(1);
    packet->ReplacePacketTag(priorityTag);

    return packet;
}

void
BufferSwitch::GeneratePacket(){
    if(m_orbweaver){
        int64_t nsNow = Simulator::Now().GetNanoSeconds();
        int64_t nextTime = 0x7fffffff;

        for(auto it = m_deviceMap.begin();it != m_deviceMap.end();++it){
            if(nsNow >= it->second.m_lastTime + it->second.generateGap){
                it->second.m_lastTime = nsNow;
                Ptr<Packet> packet = CreatePacket(0);
                if(packet != nullptr)
                    (it->first)->Send(packet, dev->GetBroadcast(), 0x0700);
            }
            nextTime = min(nextTime, it->second.m_lastTime + it->second.generateGap - nsNow);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &BufferSwitch::GeneratePacket, this);
    }
}

void 
BufferSwitch::BufferData(Ptr<Packet> packet){
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

        if(m_buffer.size() > queueSize){
            m_buffer.pop();
            // std::cout << "Information Loss for IDLE" << std::endl;
        }
        m_buffer.push(pathHeader);
    }
}

bool
BufferSwitch::IngressPipelineUser(Ptr<Packet> packet)
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
    uint32_t hash = 0;

    if(m_ecmp == 1)
        hash = Hash32((char*)&dst, sizeof(dst));
    else{
        PathHeader pathHeader;
        memset((char*)(&pathHeader), 0, sizeof(PathHeader));
        pathHeader.SetSrcIP(src);
        pathHeader.SetDstIP(dst);
        pathHeader.SetSrcPort(srcPort);
        pathHeader.SetDstPort(dstPort);
        pathHeader.SetProtocol(proto);
        hash = pathHeader.Hash();
    }

    devId = vec[hash % vec.size()];
    Ptr<NetDevice> dev = m_devices[devId];
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

bool 
BufferSwitch::EgressPipelineUser(Ptr<Packet> packet){
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

    // Start Deparse Header
    packet->AddHeader(tcpHeader);
    packet->AddHeader(ipHeader);
    // End Deparse Header

    PathHeader pathHeader;
    memset((char*)(&pathHeader), 0, sizeof(PathHeader));

    pathHeader.SetSrcIP(src);
    pathHeader.SetDstIP(dst);
    pathHeader.SetSrcPort(srcPort);
    pathHeader.SetDstPort(dstPort);
    pathHeader.SetProtocol(proto);

    pathHeader.SetNodeId(m_id);
    pathHeader.SetTTL(ttl);       
    
    uint32_t arrIndex = pathHeader.Hash() % m_table.size();
    if(m_table[arrIndex].Empty() || !(m_table[arrIndex] == pathHeader)){
        m_table[arrIndex] = pathHeader;
        if(m_buffer.size() > queueSize){
            m_buffer.pop();
            // std::cout << "Information Loss for User" << std::endl;
        }
        m_buffer.push(pathHeader);
    }

    return true;
}


bool
BufferSwitch::IngressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev)
{
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    uint8_t number = teleHeader.GetNumber();
    packet->AddHeader(teleHeader);

    if(number > 0 && number < 80)
        BufferData(packet);
    else
        std::cout << "Unknown number in IDLE packets: " << int(number) << std::endl;

    return false;
}

bool 
BufferSwitch::EgressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev){
    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);
    uint8_t type = teleHeader.GetNumber();
    packet->AddHeader(teleHeader);

    if(type == 0){
        if(AddPathHeader(packet)){
            m_lastTime = Simulator::Now().GetNanoSeconds();
            return true;
        }
        else{
            int64_t nsNow = Simulator::Now().GetNanoSeconds();
            if(nsNow - m_lastTime > 1000000000){
                m_orbweaver = false;
                std::cout << "Switch " << m_id << " stops in " << nsNow << " ns" << std::endl;
            }
            return false;
        }
    }
    else if(type < 0 || type > 80){
        std::cout << "Unknown number in IDLE packets: " << int(number) << std::endl;
        return false;
    }

    return true;
}

bool
BufferSwitch::EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
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
        return EgressPipelineIdle(packet, dev);
    }
}

bool
BufferSwitch::IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
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
BufferSwitch::ReceiveFromDevice(Ptr<NetDevice> device,
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
