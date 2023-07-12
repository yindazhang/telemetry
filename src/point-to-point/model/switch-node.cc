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

#include "tele-header.h"
#include "point-to-point-net-device.h"
#include "my-queue.h"
#include "ppp-header.h"

#include <unordered_set>

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
    Simulator::Schedule(Seconds(2), &SwitchNode::GenerateUtil, this);
    Simulator::Schedule(Seconds(1), &SwitchNode::SetQueueThd, this);
}

SwitchNode::~SwitchNode(){
    if(m_record){
        if(m_path){
            std::string out_file = output_file + ".switch.path";
            FILE* fout = fopen(out_file.c_str(), "a");
            for(auto it = m_teleSend[m_pathType].begin();it != m_teleSend[m_pathType].end();++it){
                fprintf(fout, "%d,%d,%d,%d,%d,%d,%d\n", m_id, it->first, m_pathType, 
                    m_queueLoss[m_pathType][it->first], m_bufferLoss[m_pathType][it->first], it->second, m_paths.size());
            }
            fflush(fout);
            fclose(fout);
        }
        if(m_port){
            std::string out_file = output_file + ".switch.util";
            FILE* fout = fopen(out_file.c_str(), "a");
            for(auto it = m_teleSend[m_portType].begin();it != m_teleSend[m_portType].end();++it){
                fprintf(fout, "%d,%d,%d,%d,%d,%d,%d\n", m_id, it->first, m_portType, 
                    m_queueLoss[m_portType][it->first], m_bufferLoss[m_portType][it->first], it->second, m_paths.size());
            }
            fflush(fout);
            fclose(fout);
        }
        if(m_drop){
            std::string out_file = output_file + ".switch.drop";
            FILE* fout = fopen(out_file.c_str(), "a");
            for(auto it = m_teleSend[m_dropType].begin();it != m_teleSend[m_dropType].end();++it){
                fprintf(fout, "%d,%d,%d,%d,%d,%d,%d\n", m_id, it->first, m_dropType, 
                    m_queueLoss[m_dropType][it->first], m_bufferLoss[m_dropType][it->first], it->second, m_paths.size());
            }
            fflush(fout);
            fclose(fout);
        }
    }

    if(m_record && m_path){
        FILE* fout = fopen((output_file + ".switch.path.data").c_str(), "a");
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

uint32_t 
SwitchNode::GetBufferSize(){
    uint32_t bufferSize = m_teleQueue.size;
    
    for(uint32_t i = 0;i < m_devices.size();++i){
        auto dev = DynamicCast<PointToPointNetDevice>(m_devices[i]);
        if(dev != nullptr)
            bufferSize += dev->GetQueue()->GetNBytes();
    }
    return bufferSize;
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_postcard = (OrbWeaver == 0x2);
    m_basic = ((OrbWeaver & 0x3) == 0x3);
    m_pull = ((OrbWeaver & 0x9) == 0x9);
    m_final = ((OrbWeaver & 0x11) == 0x11);
    m_push = ((OrbWeaver & 0x21) == 0x21);

    if(m_basic){
        m_queueThd += 14 * 1024;
        m_bufferThd -= 14 * 1024;
    }
}

void
SwitchNode::SetQueueThd(){
    if(m_basic){
        std::unordered_set<uint32_t> st;
        for(auto it = m_teleForward.begin();it != m_teleForward.end();++it){
            auto vec = it->second;
            for(uint32_t port : vec)
                st.insert(port);
        }
        if(st.size() > 0)
            m_queueThd /= st.size();
        std::cout << "QueueThd: " << m_queueThd << std::endl;
    }
}

void 
SwitchNode::SetPath(int8_t pathType = 1){
    m_path = true;
    m_pathType = pathType;
}

void 
SwitchNode::SetPort(int8_t portType = 2){
    m_port = true;
    m_portType = portType;
}

void 
SwitchNode::SetDrop(int8_t dropType = 4){
    m_drop = true;
    m_dropType = dropType;
}

void 
SwitchNode::SetGenerate(int64_t bandwidth){
    m_generate = true;
    m_generateGap = ((double)1e9) / bandwidth * 1024.0;
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
SwitchNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
SwitchNode::SetCollector(uint32_t number){
    m_collector = number;
}

void 
SwitchNode::SetOutput(std::string output){
    output_file = output;
    FILE* fout;
    if(m_record){
        if(m_path){
            std::string out_file = output_file + ".switch.path";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
            fout = fopen((out_file + ".data").c_str(), "w");
            fclose(fout);
        }
        if(m_port){
            std::string out_file = output_file + ".switch.util";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
        }
        if(m_drop){
            std::string out_file = output_file + ".switch.drop";
            fout = fopen(out_file.c_str(), "w");
            fclose(fout);
        }
    }
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void 
SwitchNode::AddTeleRouteTo(uint8_t dest, uint32_t devId)
{
    m_teleForward[dest].push_back(devId);

    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for collector" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in AddTeleRouteTo" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
    }
}

void 
SwitchNode::SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap){
    DeviceProperty tmp;
    tmp.devId = devId;
    tmp.generateGap = generateGap;
    m_deviceMap[m_devices[devId]] = tmp;
}

void
SwitchNode::SetDeviceUpperPull(uint8_t dest, uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for upper pull" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in SetDeviceUpperPull" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
        m_deviceMap[m_devices[devId]].isUpperPull[dest] = true;
    }
}

void
SwitchNode::SetDeviceLowerPull(uint8_t dest, uint32_t devId)
{
    if(m_deviceMap.find(m_devices[devId]) == m_deviceMap.end())
        std::cout << "Unknown devId " << devId << " in deviceMap for lower pull" << std::endl;
    else{
        if(std::find(m_deviceMap[m_devices[devId]].collectorDst.begin(), 
        m_deviceMap[m_devices[devId]].collectorDst.end(), dest) != m_deviceMap[m_devices[devId]].collectorDst.end())
            std::cout << "Error in SetDeviceLowerPull" << std::endl;
        m_deviceMap[m_devices[devId]].collectorDst.push_back(dest);
        m_deviceMap[m_devices[devId]].isLowerPull[dest] = true;
    }
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
SwitchNode::CreatePacket(uint8_t priority)
{
    Ptr<Packet> packet = Create<Packet>();
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(priority);
    packet->ReplacePacketTag(priorityTag);
    return packet;
}

bool
SwitchNode::BatchPath(PathHeader path, uint8_t dest){
    m_teleQueue.pathBatch[dest].push_back(path);
    if(m_teleQueue.pathBatch[dest].size() % batchSize == 0){
        m_teleSend[m_pathType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i){
            if(m_record && m_paths.find(m_teleQueue.pathBatch[dest][i]) == m_paths.end())
                m_paths.insert(m_teleQueue.pathBatch[dest][i]);   
            packet->AddHeader(m_teleQueue.pathBatch[dest][i]);
        }
        m_teleQueue.pathBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_pathType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size + packet->GetSize() > m_bufferThd){
            m_bufferLoss[m_pathType][dest] += batchSize;
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size += packet->GetSize();
        return true;
    }

    return false;
}

bool
SwitchNode::BatchUtil(UtilHeader util, uint8_t dest){
    m_teleQueue.utilBatch[dest].push_back(util);
    if(m_teleQueue.utilBatch[dest].size() % batchSize == 0){
        m_teleSend[m_portType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i)
            packet->AddHeader(m_teleQueue.utilBatch[dest][i]);
        m_teleQueue.utilBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_portType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size + packet->GetSize() > m_bufferThd){
            m_bufferLoss[m_portType][dest] += batchSize;
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size += packet->GetSize();
        return true;
    }

    return false;
}

bool
SwitchNode::BatchDrop(DropHeader drop, uint8_t dest){
    m_teleQueue.dropBatch[dest].push_back(drop);
    if(m_teleQueue.dropBatch[dest].size() % batchSize == 0){
        m_teleSend[m_dropType][dest] += batchSize;

        Ptr<Packet> packet = CreatePacket(0);
        for(uint32_t i = 0;i < batchSize;++i)
            packet->AddHeader(m_teleQueue.dropBatch[dest][i]);
        m_teleQueue.dropBatch[dest].clear();

        TeleHeader teleHeader;
        teleHeader.SetType(m_dropType);
        teleHeader.SetDest(dest);
        teleHeader.SetSize(batchSize);
        packet->AddHeader(teleHeader);

        if(m_teleQueue.size + packet->GetSize() > m_bufferThd){
            m_bufferLoss[m_dropType][dest] += batchSize;
            return false;
        }
        m_teleQueue.packets[dest].push(packet);
        m_teleQueue.size += packet->GetSize();
        return true;
    }

    return false;
}

Ptr<Packet> 
SwitchNode::GetTelePacket(uint32_t priority, uint8_t dest)
{
    Ptr<Packet> packet = nullptr;

    if(!m_teleQueue.packets[dest].empty()){
        packet = m_teleQueue.packets[dest].front();
        m_teleQueue.packets[dest].pop();
        m_teleQueue.size -= packet->GetSize();

        SocketPriorityTag priorityTag;
        priorityTag.SetPriority(priority);
        packet->ReplacePacketTag(priorityTag);
    }

    return packet;
}

void 
SwitchNode::SendPostcard(uint8_t dest){
    Ptr<Packet> packet = GetTelePacket(0, dest);

    if(packet != nullptr){
        auto vec = m_teleForward[dest];
        uint32_t devId = vec[rand() % vec.size()];
        Ptr<NetDevice> dev = m_devices[devId];
        if(m_userSize[dev] + packet->GetSize() <= m_userThd){
            m_userSize[dev] += packet->GetSize();
            dev->Send(packet, dev->GetBroadcast(), 0x0171);
        }
        else{
            TeleHeader teleHeader;
            packet->RemoveHeader(teleHeader);
            m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
        }   
    }
}

void 
SwitchNode::GenerateUtil(){
    if(!m_generate)
        return;
    
    if(m_orbweaver || m_postcard){
        bool empty = true;
        for(auto it = m_bytes.begin();it != m_bytes.end();++it){
            if(it->second > 0){
                empty = false;
                it->second = 0;
            }
        }

        if(!empty){
            for(int i = 0;i < 64;++i){
                UtilHeader utilHeader;
                uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
                if(BatchUtil(utilHeader, dest) && m_postcard)
                    SendPostcard(dest);
            }
        }

        Simulator::Schedule(NanoSeconds(m_generateGap), &SwitchNode::GenerateUtil, this);
    }
}

void 
SwitchNode::RecordUtil(){
    if(!m_port)
        return;

    if(m_orbweaver || m_postcard){
        for(auto it = m_bytes.begin();it != m_bytes.end();++it){
            if(it->second > 0){
                UtilHeader utilHeader;
                utilHeader.SetNodeId(m_id);
                utilHeader.SetPortId(it->first);
                utilHeader.SetTime(Simulator::Now().GetMicroSeconds());
                utilHeader.SetByte(it->second);

                uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
                if(BatchUtil(utilHeader, dest) && m_postcard)
                    SendPostcard(dest);

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
                Ptr<Packet> packet = CreatePacket(2);
                if(packet != nullptr){
                    (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0170);
                }
            }
            nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &SwitchNode::GeneratePacket, this);
    }
}

void 
SwitchNode::BufferData(Ptr<Packet> packet){
    if(m_basic){
        std::cout << "Basic in Buffer?" << std::endl;
        return;
    }

    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    if(m_teleQueue.size + packet->GetSize() > m_bufferThd){
        m_bufferLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
    }
    else{
        m_teleQueue.packets[teleHeader.GetDest()].push(packet);
        m_teleQueue.size += packet->GetSize();
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
    m_bytes[devId] += packet->GetSize();

    Ptr<NetDevice> dev = m_devices[devId];

    if(m_userSize[dev] + packet->GetSize() <= m_userThd){
        m_userSize[dev] += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0800);
    }

    //std::cout << "Drop in switch" << std::endl;
    if(m_drop && (m_orbweaver || m_postcard)){
        DropHeader dropHeader;

        dropHeader.SetSrcIP(src);
        dropHeader.SetDstIP(dst);
        dropHeader.SetSrcPort(srcPort);
        dropHeader.SetDstPort(dstPort);
        dropHeader.SetProtocol(proto);
        dropHeader.SetNodeId(m_id);

        uint8_t dest = Hash32((char*)&m_id, sizeof(m_id)) % m_collector;
        if(BatchDrop(dropHeader, dest) && m_postcard)
            SendPostcard(dest);
    }      

    return false;
}

bool 
SwitchNode::EgressPipelineUser(Ptr<Packet> packet){
    if((!m_orbweaver && !m_postcard) || !m_path)
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

        uint8_t dest = Hash32((char*)(&dst), sizeof(dst)) % m_collector;
        if(BatchPath(pathHeader, dest) && m_postcard)
            Simulator::Schedule(NanoSeconds(1), &SwitchNode::SendPostcard, this, dest);
    }

    return true;
}

bool 
SwitchNode::IngressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    if(m_basic){
        auto vec = m_teleForward[teleHeader.GetDest()];
        uint32_t devId = vec[rand() % vec.size()];
        dev = m_devices[devId];
    }
    else{
        dev = m_devices[m_devices.size() - 1];
    }

    if(m_queueSize[dev] + packet->GetSize() <= m_queueThd){
        m_queueSize[dev] += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0171);
    }

    m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
    return false;
}

bool 
SwitchNode::IngressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic){
        std::cout << "Unknown config (pull in basic/push)" << std::endl;
        return false;
    }
    return dev->Send(packet, dev->GetBroadcast(), 0x0172);
}

bool 
SwitchNode::IngressPipelinePostcard(Ptr<Packet> packet, Ptr<NetDevice> dev){
    TeleHeader teleHeader;
    packet->PeekHeader(teleHeader);

    auto vec = m_teleForward[teleHeader.GetDest()];
    uint32_t devId = vec[rand() % vec.size()];
    dev = m_devices[devId];

    if(m_userSize[dev] + packet->GetSize() <= m_userThd){
        m_userSize[dev] += packet->GetSize();
        return dev->Send(packet, dev->GetBroadcast(), 0x0171);
    }

    m_queueLoss[teleHeader.GetType()][teleHeader.GetDest()] += batchSize;
    return false;
}

Ptr<Packet>
SwitchNode::EgressPipelineSeed(Ptr<Packet> packet, Ptr<NetDevice> dev){
    DeviceProperty property = m_deviceMap[dev];

    PppHeader ppp;
    packet->RemoveHeader(ppp);
    if(m_basic){
        for(auto dest : property.collectorDst){
            if(!m_teleQueue.packets[dest].empty()){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
        }
    }
    else{
        uint8_t dest = property.collectorDst[rand() % property.collectorDst.size()];
        uint32_t bufferSize = m_teleQueue.packets[dest].size();

        if(m_pull && property.isLowerPull[dest]){
            if(bufferSize < 1 || bufferSize * m_teleQueue.packets[dest].front()->GetSize() 
                    < m_bufferThd * 0.5){
                TeleHeader teleHeader;
                teleHeader.SetDest(dest);
                teleHeader.SetSize(0);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
            return nullptr;
        }
        else if((m_final || m_push) && (property.isUpperPull[dest] || property.isLowerPull[dest])){
            if(m_push && bufferSize > 1 && bufferSize * m_teleQueue.packets[dest].front()->GetSize() 
                    > m_bufferThd * 0.95){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else{
                TeleHeader teleHeader;
                teleHeader.SetDest(dest);
                teleHeader.SetSize(bufferSize);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
        }
    }
    return nullptr;
}

Ptr<Packet>
SwitchNode::EgressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev){
    if(m_basic)
        return packet;
    else{
        PppHeader ppp;
        packet->RemoveHeader(ppp);
        BufferData(packet);
        return nullptr;
    }
}

Ptr<Packet>
SwitchNode::EgressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev){
    PppHeader ppp;
    packet->RemoveHeader(ppp);

    TeleHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    uint8_t dest = teleHeader.GetDest();
    uint16_t size = teleHeader.GetSize();

    if(m_final || m_push){
        bool isUpper = false, isLower = false;
        uint32_t bufferSize = 0;

        if(m_deviceMap.find(dev) != m_deviceMap.end()){
            DeviceProperty property = m_deviceMap[dev];
            isUpper = property.isUpperPull[dest];
            isLower = property.isLowerPull[dest];
            bufferSize = m_teleQueue.packets[dest].size();
        }

        if(isUpper){
            if(size < bufferSize){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else if(bufferSize + 1 < size){
                teleHeader.SetSize(bufferSize);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
            return nullptr;
        }
        else if(isLower){
            if(size + 1 < bufferSize){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    ppp.SetProtocol(0x171);
                    packet->AddHeader(ppp);
                }
                return packet;
            }
            else if(bufferSize < size){
                teleHeader.SetSize(bufferSize);
                packet->AddHeader(teleHeader);

                ppp.SetProtocol(0x172);
                packet->AddHeader(ppp);
                return packet;
            }
            return nullptr;
        }

        packet = GetTelePacket(1, dest);
        if(packet != nullptr){
            ppp.SetProtocol(0x171);
            packet->AddHeader(ppp);
        }
        return packet;
    }
    else if(m_pull){
        packet = GetTelePacket(1, dest);
        if(packet != nullptr){
            ppp.SetProtocol(0x171);
            packet->AddHeader(ppp);
        }
        return packet;
    }
    else
        std::cout << "EgressPipelinePull for other protocol?" << std::endl;
       
    return nullptr;
}

Ptr<Packet>
SwitchNode::EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        PppHeader ppp;
        packet->RemoveHeader(ppp);

        m_userSize[dev] -= packet->GetSize();
        if(m_userSize[dev] < 0)
            std::cout << "Error for userSize" << std::endl;
        
        if(protocol == 0x0171 || EgressPipelineUser(packet)){
            packet->AddHeader(ppp);
            return packet;
        }
        return nullptr;
    }
    else{
        PppHeader ppp;
        packet->RemoveHeader(ppp);
        if(protocol == 0x0171){
            m_queueSize[dev] -= packet->GetSize();
            if(m_queueSize[dev] < 0)
                std::cout << "Error for queueSize" << std::endl;
        }
        packet->AddHeader(ppp);

        if(protocol == 0x0170)
            return EgressPipelineSeed(packet, dev);
        else if(protocol == 0x0171)
            return EgressPipelinePush(packet, dev);
        else if(protocol == 0x0172)
            return EgressPipelinePull(packet, dev);
        else{
            std::cout << "Unknown Protocol for EgressPipeline" << std::endl;
            return nullptr;
        }
    }
}

bool
SwitchNode::IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev){
    if(priority == 0){
        if(protocol == 0x0171)
            return IngressPipelinePostcard(packet, dev);
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