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

            FILE* fout = fopen((output_file + ".collector.path").c_str(), "w");
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
}

void 
CollectorNode::SetDest(uint8_t dest){
    m_dest = dest;
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
                Ptr<Packet> packet = CreatePacket(2);
                
                TeleHeader teleHeader;
                teleHeader.SetDest(m_dest);
                teleHeader.SetSize(0);
                packet->AddHeader(teleHeader);

                (it->first)->Send(packet, (it->first)->GetBroadcast(), 0x0172);
            }
            nextTime = std::min(nextTime, it->second.m_lastTime + it->second.generateGap);
        }
        Simulator::Schedule(NanoSeconds(nextTime - nsNow), &CollectorNode::GeneratePacket, this);
    }
}

bool
CollectorNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    if(protocol != 0x0171){
        std::cout << "Unknown prorocol for colllector" << std::endl;
        return false;
    }

    Ptr<Packet> packet = p->Copy();

    TeleHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    m_types.insert(teleHeader.GetType());

    if(teleHeader.GetDest() != m_dest)
        std::cout << "Collector Dest Error" << std::endl;

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
                    
                    std::cout << "PathHeader: " << pathHeader.GetSrcIP() << " "
                        << pathHeader.GetDstIP() << " "
                        << pathHeader.GetSrcPort() << " "
                        << pathHeader.GetDstPort() << " "
                        << pathHeader.GetNodeId() << " "
                        << int(pathHeader.GetTTL()) << " "
                        << std::endl;

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
                
                std::cout << "UtilHeader: " << utilHeader.GetNodeId() << " "
                        << utilHeader.GetPortId() << " "
                        << utilHeader.GetTime() << " "
                        << utilHeader.GetByte() << " "
                        << std::endl;
                
            }
            break;
        default : std::cout << "Unknown task" << std::endl; break;
    }

    return true;
}

} // namespace ns3
