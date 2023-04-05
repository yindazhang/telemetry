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

#include "buffer-header.h"
#include "point-to-point-net-device.h"
#include "my-queue.h"
#include "ppp-header.h"

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
    m_table.resize(arrSize);
    Simulator::Schedule(Seconds(2), &SwitchNode::GeneratePacket, this);
    Simulator::Schedule(Seconds(2), &SwitchNode::RecordUtil, this);
}

SwitchNode::~SwitchNode(){
    uint32_t send = m_utilSend;
    if(m_task == 1)
        send = m_paths.size();

    if(m_record){
        FILE* fout = fopen(output_file.c_str(), "a");
        fprintf(fout, "%d,%d,%d,%d\n", m_id, m_queueLoss, m_bufferLoss, send);
        fflush(fout);
        fclose(fout);
    }

    if(m_record && m_task == 1){
        FILE* fout = fopen((output_file + ".data").c_str(), "a");
        for(auto path : m_paths){
            fprintf(fout, "%d %d ", path.GetSrcIP(), path.GetDstIP());
            fprintf(fout, "%d %d ", path.GetSrcPort(), path.GetDstPort());
            fprintf(fout, "%d %d ", path.GetNodeId(), (int)path.GetProtocol());
            fprintf(fout, "%d\n", (int)path.GetTTL());
            fflush(fout);
        }
        fclose(fout);
    }
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_basic = ((OrbWeaver & 0x3) == 0x3);
    m_push = ((OrbWeaver & 0x5) == 0x5);
    m_pull = ((OrbWeaver & 0x9) == 0x9);
    m_final = ((OrbWeaver & 0x11) == 0x11);

    if(m_basic){
        m_queueThd += 14 * 1024;
        m_bufferThd -= 14 * 1024;
    }
}

void 
SwitchNode::SetUtilGap(uint32_t utilGap){
    m_utilGap = utilGap;
}

void
SwitchNode::SetHashSeed(uint32_t hashSeed){
    m_hashSeed = hashSeed;
}

void 
SwitchNode::SetEcmp(uint32_t ecmp){
    m_ecmp = ecmp;
}

void 
SwitchNode::SetTask(uint32_t task){
    m_task = task;
}

void 
SwitchNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
SwitchNode::SetOutput(std::string output){
    if(m_record){
        if(m_task == 1){
            output_file = output + ".switch.path";
            FILE* fout = fopen((output_file + ".data").c_str(), "w");
            fclose(fout);
        }
        else if(m_task == 2)
            output_file = output + ".switch.util";
        else
            std::cout << "Unknown task" << std::endl;
        FILE* fout = fopen(output_file.c_str(), "w");
        fclose(fout);
    }
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void 
SwitchNode::SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap){
    DeviceProperty tmp;
    tmp.devId = devId;
    tmp.generateGap = generateGap;
    m_deviceMap[m_devices[devId]] = tmp;
}

void
SwitchNode::SetDeviceCollector(uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for collector" << std::endl;
    else{
        m_deviceMap[m_devices[devId]].isCollector = true;
        m_collectorDev.push_back(devId);
    }
}

void
SwitchNode::SetDeviceUpperPull(uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for upper pull" << std::endl;
    else
        m_deviceMap[m_devices[devId]].isUpperPull = true;
}

void
SwitchNode::SetDeviceLowerPull(uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for lower pull" << std::endl;
    else
        m_deviceMap[m_devices[devId]].isLowerPull = true;
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

Ptr<Packet> 
SwitchNode::CreatePacket(uint8_t type)
{
    Ptr<Packet> packet = Create<Packet>(0);
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(2);
    packet->ReplacePacketTag(priorityTag);
    return packet;
}

void 
SwitchNode::RecordUtil(){
    if(m_orbweaver){
        for(auto it = m_bytes.begin();it != m_bytes.end();++it){
            if(it->second > 0){
                m_utilSend += 1;
                UtilHeader utilHeader;
                memset((char*)(&utilHeader), 0, sizeof(UtilHeader));
                utilHeader.SetNodeId(m_id);
                utilHeader.SetPortId(it->first);
                utilHeader.SetTime(Simulator::Now().GetMicroSeconds());
                utilHeader.SetByte(it->second);

                m_utilBuffer.push(utilHeader);
                if(m_utilBuffer.size() * sizeof(UtilHeader) > m_bufferThd){
                    m_utilBuffer.pop();
                    m_bufferLoss += 1;
                }

                it->second = 0;
            }
        }
        Simulator::Schedule(NanoSeconds(m_utilGap), &SwitchNode::RecordUtil, this);
    }
}

void
SwitchNode::GeneratePacket(){
    if(m_orbweaver){
        int64_t nsNow = Simulator::Now().GetNanoSeconds();
        int64_t nextTime = 0xffffffffffffL;

        for(auto it = m_deviceMap.begin();it != m_deviceMap.end();++it){
            if(nsNow >= it->second.m_lastTime + it->second.generateGap){
                it->second.m_lastTime = nsNow;
                Ptr<Packet> packet = CreatePacket(0);
                if(packet != nullptr){
                    //if(m_seedSize + packet->GetSize() <= m_seedThd){
                    //    m_seedSize += packet->GetSize();
                    //    (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0170);
                    //}
                    (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0170);
                }
            }
            nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &SwitchNode::GeneratePacket, this);
    }
}

bool
SwitchNode::AddTeleHeader(Ptr<Packet> packet)
{
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(1);
    packet->ReplacePacketTag(priorityTag);
    if(m_task == 1){
        if(m_pathBuffer.size() < batchSize)
            return false;
        for(uint32_t i = 0;i < batchSize;++i){
            PathHeader pathHeader = m_pathBuffer.front();
            m_pathBuffer.pop();
            packet->AddHeader(pathHeader);
        }
        return true;
    }
    else if(m_task == 2){
        if(m_utilBuffer.size() < batchSize)
            return false;
        for(uint32_t i = 0;i < batchSize;++i){
            UtilHeader utilHeader = m_utilBuffer.front();
            m_utilBuffer.pop();
            packet->AddHeader(utilHeader);
        }
        return true;
    }
    else{
        std::cout << "Unknown task " << m_task << std::endl;
        return false;
    }
}

void 
SwitchNode::BufferData(Ptr<Packet> packet){
    if(m_basic){
        std::cout << "Basic in Buffer?" << std::endl;
        return;
    }

    if(m_task == 1){
        for(uint8_t i = 0;i < batchSize;++i){
            PathHeader pathHeader;
            packet->RemoveHeader(pathHeader);
            m_pathBuffer.push(pathHeader);
            if(m_pathBuffer.size() * sizeof(PathHeader) > m_bufferThd){
                m_pathBuffer.pop();
                m_bufferLoss += 1;
            }
        }
    }
    else if(m_task == 2){
        for(uint8_t i = 0;i < batchSize;++i){
            UtilHeader utilHeader;
            packet->RemoveHeader(utilHeader);
            m_utilBuffer.push(utilHeader);
            if(m_utilBuffer.size() * sizeof(UtilHeader) > m_bufferThd){
                m_utilBuffer.pop();
                m_bufferLoss += 1;
            }
        }
    }
    else{
        std::cout << "Unknown task" << std::endl;
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
    uint32_t hash = 0;

    if(m_ecmp == 1){
        dst = dst + m_hashSeed;
        hash = Hash32((char*)&dst, sizeof(dst));
    }
    else{
        PathHeader pathHeader;
        memset((char*)(&pathHeader), 0, sizeof(PathHeader));
        pathHeader.SetSrcIP(src + m_hashSeed);
        pathHeader.SetDstIP(dst + m_hashSeed);
        pathHeader.SetSrcPort(srcPort);
        pathHeader.SetDstPort(dstPort);
        pathHeader.SetProtocol(proto);
        hash = pathHeader.Hash();
    }

    devId = vec[hash % vec.size()];
    if(m_task == 2)
        m_bytes[devId] += packet->GetSize();

    Ptr<NetDevice> dev = m_devices[devId];

    if(m_userSize + packet->GetSize() <= m_userThd){
        m_userSize += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0800);
    }
    return false;
}

bool 
SwitchNode::EgressPipelineUser(Ptr<Packet> packet){
    if(!m_orbweaver || m_task != 1)
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

    if(m_record && m_paths.find(pathHeader) == m_paths.end())
        m_paths.insert(pathHeader);   
    
    uint32_t arrIndex = pathHeader.Hash() % m_table.size();
    if(m_table[arrIndex].Empty() || !(m_table[arrIndex] == pathHeader)){
        m_table[arrIndex] = pathHeader;
        m_pathBuffer.push(pathHeader);
        if(m_pathBuffer.size() * sizeof(PathHeader) > m_bufferThd){
            m_pathBuffer.pop();
            m_bufferLoss += 1;
        }
    }

    return true;
}

bool 
SwitchNode::IngressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic){
        uint32_t devId = m_collectorDev[rand() % m_collectorDev.size()];
        dev = m_devices[devId];
    }
    else{
        dev = m_devices[m_devices.size() - 1];
    }

    if(m_queueSize + packet->GetSize() <= m_queueThd){
        m_queueSize += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0171);
    }
    m_queueLoss += 1;
    return false;
}

bool 
SwitchNode::IngressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic || m_push){
        std::cout << "Unknown config (pull in basic/push)" << std::endl;
        return false;
    }
    //if(m_seedSize + packet->GetSize() <= m_seedThd){
    //    m_seedSize += packet->GetSize();
    //    return dev->Send(packet, dev->GetBroadcast(), 0x0172);
    //}
    //return false;
    return dev->Send(packet, dev->GetBroadcast(), 0x0172);
}

uint16_t 
SwitchNode::EgressPipelineSeed(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic || m_push){
        if(AddTeleHeader(packet)){
            m_lastTime = Simulator::Now().GetNanoSeconds();
            return 0x0171;
        }
        else{
            int64_t nsNow = Simulator::Now().GetNanoSeconds();
            if(nsNow - m_lastTime > 2000000000){
                m_orbweaver = false;
                std::cout << "Switch " << m_id << " stops in " << nsNow << " ns" << std::endl;
            }
            return 0;
        }
    }
    else{
        DeviceProperty property = m_deviceMap[dev];

        if(property.isCollector){
            if(AddTeleHeader(packet))
                return 0x0171;
            else
                return 0;
        }

        uint32_t bufferSize = m_pathBuffer.size() * sizeof(PathHeader);
        if(m_task == 2)
            bufferSize = m_utilBuffer.size() * sizeof(UtilHeader);
            
        if(bufferSize < batchSize){
            int64_t nsNow = Simulator::Now().GetNanoSeconds();
            if(nsNow - m_lastTime > 2000000000){
                m_orbweaver = false;
                std::cout << "Switch " << m_id << " stops in " << nsNow << " ns" << std::endl;
            }
        }
        else{
            m_lastTime = Simulator::Now().GetNanoSeconds();
        }

        if(m_pull && property.isLowerPull){
            if(bufferSize < m_bufferThd * 0.5)
                return 0x172;
        }
        else if(m_final && (property.isUpperPull || property.isLowerPull)){
            BufferHeader bufferHeader;
            bufferHeader.SetBuffer(bufferSize);
            packet->AddHeader(bufferHeader);
            return 0x172;
        }

        return 0;
    }
}

uint16_t
SwitchNode::EgressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic)
        return 1;
    else{
        BufferData(packet);
        return 0;
    }
}

uint16_t
SwitchNode::EgressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    DeviceProperty property = m_deviceMap[dev];
    if(m_final){
        BufferHeader bufferHeader;
        packet->RemoveHeader(bufferHeader);

        uint16_t size = bufferHeader.GetBuffer();
        uint32_t bufferSize = m_pathBuffer.size();
        if(m_task == 2)
            bufferSize = m_utilBuffer.size();

        if(property.isUpperPull){
            if(size < bufferSize){
                if(AddTeleHeader(packet))
                    return 0x171;
            }
            else{
                bufferHeader.SetBuffer(bufferSize);
                packet->AddHeader(bufferHeader);
                return 0x172;
            }
        }
        else if(property.isLowerPull){
            if(size + 1 < bufferSize){
                if(AddTeleHeader(packet))
                    return 0x171;
            }
            else{
                bufferHeader.SetBuffer(bufferSize);
                packet->AddHeader(bufferHeader);
                return 0x172;
            }
        }
        else
            std::cout << "Unknown Protocol for EgressPipelinePull for Final" << std::endl;
    }
    else if(m_pull){
        if(property.isLowerPull)
            std::cout << "Unknown Protocol for EgressPipelinePull for Pull" << std::endl;
        if(AddTeleHeader(packet))
            return 0x171;
    }
    else
        std::cout << "EgressPipelinePull for other protocol?" << std::endl;
       
    return 0;
}

uint16_t
SwitchNode::EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        m_userSize -= packet->GetSize();
        if(m_userSize < 0)
            std::cout << "Error for userSize" << std::endl;
        return EgressPipelineUser(packet); 
    }
    else{
        if(protocol == 0x0171){
            m_queueSize -= packet->GetSize();
            if(m_queueSize < 0)
                std::cout << "Error for queueSize" << std::endl;
        }

        //else{
        //    m_seedSize -= packet->GetSize();
        //    if(m_seedSize < 0)
        //        std::cout << "Error for seedSize" << std::endl;
        //}

        if(protocol == 0x0170)
            return EgressPipelineSeed(packet, dev);
        else if(protocol == 0x0171)
            return EgressPipelinePush(packet, dev);
        else if(protocol == 0x0172)
            return EgressPipelinePull(packet, dev);
        else{
            std::cout << "Unknown Protocol for EgressPipeline" << std::endl;
            return 0;
        }
    }
}

bool
SwitchNode::IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        return IngressPipelineUser(packet);
    }
    else{
        if(protocol == 0x0171)
            return IngressPipelinePush(packet, dev);
        else if(protocol == 0x0172)
            return IngressPipelinePull(packet, dev);
        else{
            std::cout << "Unknown Protocol for IngressPipeline" << std::endl;
            return false;
        }
    }
}

bool
SwitchNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    Ptr<Packet> packet = p->Copy();

    uint32_t priority = 0;
    SocketPriorityTag priorityTag;
    if(packet->PeekPacketTag(priorityTag))
        priority = priorityTag.GetPriority();

    return IngressPipeline(packet, priority, protocol, device);
}

} // namespace ns3