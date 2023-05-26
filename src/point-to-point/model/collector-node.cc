#include "collector-node.h"

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

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CollectorNode");

NS_OBJECT_ENSURE_REGISTERED(CollectorNode);


TypeId
CollectorNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CollectorNode")
            .SetParent<Node>()
            .SetGroupName("PointToPoint")
            .AddConstructor<CollectorNode>();
    return tid;
}

CollectorNode::CollectorNode() : Node() {
    m_orbweaver = false;
    Simulator::Schedule(Seconds(2), &CollectorNode::GeneratePacket, this);
}

CollectorNode::~CollectorNode(){
    if(m_types.find(1) != m_types.end()){
        if(m_record){
            std::cout << "Receive entries: " << m_paths.size() << std::endl;
            std::cout << "Number of duplicates: " << m_duplicates << std::endl;

            FILE* fout = fopen((output_file + ".collector.path").c_str(), "a");
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
    else if(m_types.find(2) != m_types.end()){
        std::cout << "Receive entries: " << m_duplicates << std::endl;
    }
}

void 
CollectorNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
CollectorNode::SetOutput(std::string output){
    output_file = output;
    FILE* fout = fopen((output_file + ".collector.path").c_str(), "w");
    fclose(fout);
}

uint32_t
CollectorNode::AddDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    uint32_t index = m_devices.size();
    m_devices.push_back(device);
    device->SetNode(this);
    device->SetIfIndex(index);
    device->SetReceiveCallback(MakeCallback(&CollectorNode::ReceiveFromDevice, this));
    Simulator::ScheduleWithContext(GetId(), Seconds(0.0), &NetDevice::Initialize, device);
    NotifyDeviceAdded(device);
    return index;
}

void
CollectorNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = ((OrbWeaver & 0x1) == 0x1);
    m_postcard = (OrbWeaver == 0x2);
    m_basic = ((OrbWeaver & 0x3) == 0x3);
    m_pull = ((OrbWeaver & 0x9) == 0x9);
    m_final = ((OrbWeaver & 0x11) == 0x11);
}

void 
CollectorNode::SetPriority(uint8_t dest, uint16_t priority){
    m_priority[dest] = priority;
}

void 
CollectorNode::SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap){
    DeviceProperty tmp;
    tmp.devId = devId;
    tmp.generateGap = generateGap;
    m_deviceMap[m_devices[devId]] = tmp;
}

Ptr<Packet> 
CollectorNode::CreatePacket(uint8_t priority)
{
    Ptr<Packet> packet = Create<Packet>();
    SocketPriorityTag priorityTag;
    priorityTag.SetPriority(priority);
    packet->ReplacePacketTag(priorityTag);
    return packet;
}

void
CollectorNode::GeneratePacket(){
    if(m_pull || m_final){
        int64_t nsNow = Simulator::Now().GetNanoSeconds();
        int64_t nextTime = 0xffffffffffffL;

        for(auto it = m_deviceMap.begin();it != m_deviceMap.end();++it){
            if(nsNow >= it->second.m_lastTime + it->second.generateGap){
                it->second.m_lastTime = nsNow;

                for(auto pit = m_priority.begin();pit != m_priority.end();++pit){
                    if(m_pull && pit->second > 0)
                        continue;

                    Ptr<Packet> packet = CreatePacket(2);
                
                    TeleHeader teleHeader;
                    teleHeader.SetDest(pit->first);
                    teleHeader.SetSize(pit->second);
                    packet->AddHeader(teleHeader);

                    (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0172);
                }
            }
            nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &CollectorNode::GeneratePacket, this);
    }
}

void 
CollectorNode::SendPacket(Ptr<NetDevice> dev, Ptr<Packet> packet, uint16_t protocol){
    dev->Send(packet, dev->GetBroadcast(), protocol);
}

bool
CollectorNode::MainCollect(Ptr<Packet> packet, TeleHeader teleHeader){
    m_types.insert(teleHeader.GetType());
    switch(teleHeader.GetType()){
        case 1 :
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                PathHeader pathHeader;
                packet->RemoveHeader(pathHeader);

                if(m_record && m_paths.find(pathHeader) == m_paths.end()){
                    m_paths.insert(pathHeader);
                    if(m_paths.size() % 10000 == 9999){
                        std::cout << "Receive entries: " << m_paths.size() << std::endl;
                        std::cout << "Number of duplicates: " << m_duplicates << std::endl;
                    }

                    /*  
                    std::cout << "PathHeader: " << (int)teleHeader.GetDest() << " " 
                        << pathHeader.GetSrcIP() << " "
                        << pathHeader.GetDstIP() << " "
                        << pathHeader.GetSrcPort() << " "
                        << pathHeader.GetDstPort() << " "
                        << pathHeader.GetNodeId() << " "
                        << int(pathHeader.GetTTL()) << " "
                        << std::endl;
                    */
                }
                else
                    m_duplicates += 1;
            }
            break;
        case 2 :
            for(uint32_t i = 0;i < teleHeader.GetSize();++i){
                UtilHeader utilHeader;
                packet->RemoveHeader(utilHeader);

                m_duplicates += 1;
                        
                if(m_duplicates % 10000 == 9999)
                    std::cout << "Receive entries: " << m_duplicates << std::endl;

                /* 
                std::cout << "UtilHeader: " << (int)teleHeader.GetDest() << " " 
                        << utilHeader.GetNodeId() << " "
                        << utilHeader.GetPortId() << " "
                        << utilHeader.GetTime() << " "
                        << utilHeader.GetByte() << " "
                        << std::endl;
                */
            }
            break;
        default : std::cout << "Unknown task " << teleHeader.GetType() << std::endl; break;
    }
    return true;
}

void 
CollectorNode::BufferData(Ptr<Packet> packet, TeleHeader teleHeader){
    packet->AddHeader(teleHeader);
    m_teleQueue.packets[teleHeader.GetDest()].push(packet);
    m_teleQueue.size += packet->GetSize();
}

Ptr<Packet> 
CollectorNode::GetTelePacket(uint32_t priority, uint8_t dest)
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

bool 
CollectorNode::TempStore(Ptr<Packet> packet, TeleHeader teleHeader, uint16_t protocol, Ptr<NetDevice> dev){
    uint8_t dest;
    uint16_t size;
    uint32_t bufferSize;

    switch(protocol){
        case 0x0171 :
            BufferData(packet, teleHeader);
            return true;
        case 0x0172 :
            dest = teleHeader.GetDest();
            size = teleHeader.GetSize();

            bufferSize = m_teleQueue.packets[dest].size();
            if(bufferSize > 0 && size < m_priority[dest]){
                packet = GetTelePacket(1, dest);
                if(packet != nullptr){
                    Simulator::Schedule(NanoSeconds(1), &CollectorNode::SendPacket, this, dev, packet, 0x0171);
                }
                return true;
            }

            if(m_priority[dest] < size){
                teleHeader.SetSize(m_priority[dest]);
                packet->AddHeader(teleHeader);
                Simulator::Schedule(NanoSeconds(1), &CollectorNode::SendPacket, this, dev, packet, 0x0172);
            }
            return true;
        default : 
            std::cout << "Unknown protocol for colllector" << std::endl;
            return false;
    }
}

bool
CollectorNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    Ptr<Packet> packet = p->Copy();
    TeleHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    if(m_priority.find(teleHeader.GetDest()) == m_priority.end())
        std::cout << "Collector Dest Error" << std::endl;

    if(m_priority[teleHeader.GetDest()] == 0){
        if(protocol == 0x0171)
            return MainCollect(packet, teleHeader);
        std::cout << "Unknown propotol for main collector" << std::endl;
        return false;
    }
    else{
        if(protocol == 0x0171 || protocol == 0x0172){
            if(m_final)
                return TempStore(packet, teleHeader, protocol, device);
            return true;
        }
        std::cout << "Unknown propotol for temp storage" << std::endl;
        return false;
    }
}

} // namespace ns3
