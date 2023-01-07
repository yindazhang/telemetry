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
    m_array.resize(arrSize);
    Simulator::Schedule(Seconds(2), &SwitchNode::OrbWeaverSend, this);
}

void
SwitchNode::SetOrbWeaver(uint32_t OrbWeaver){
    m_orbweaver = OrbWeaver;
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

uint32_t 
SwitchNode::HashFlow(PathHeader pathHeader)
{
    Hasher hasher;
    uint32_t hash = hasher.GetHash32((char*)(&pathHeader), sizeof(PathHeader));
    return hash;
}

void 
SwitchNode::AddUdpIpHeader(Ptr<Packet> packet){
    UdpHeader udpHeader;
    udpHeader.SetDestinationPort(1000);
    udpHeader.SetSourcePort(1000);
    packet->AddHeader(udpHeader);

    Ipv4Header ipHeader;
    ipHeader.SetSource(Ipv4Address("11.11.11.11"));
    ipHeader.SetDestination(Ipv4Address("10.8.15.1"));
    ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
    ipHeader.SetPayloadSize(packet->GetSize());
    ipHeader.SetTtl(64);
    ipHeader.SetTos(0);
    ipHeader.SetMayFragment();
    packet->AddHeader(ipHeader);
}

Ptr<Packet> 
SwitchNode::GeneratePacket()
{
    if(!m_queue.empty()){
        Ptr<Packet> packet = Create<Packet>(0);

        PathHeader pathHeader = m_queue.front();
        m_queue.pop();
        packet->AddHeader(pathHeader);

        TelemetryHeader teleHeader;
        teleHeader.SetNumber(1);
        packet->AddHeader(teleHeader);

        AddUdpIpHeader(packet);

        SocketPriorityTag priorityTag;
        priorityTag.SetPriority(1);
        packet->ReplacePacketTag(priorityTag);

        return packet;
    }
    else{
        return nullptr;
    }
}

void
SwitchNode::OrbWeaverSend(){
    if((m_orbweaver & 3) == 3){
        Simulator::Schedule(NanoSeconds(300), &SwitchNode::OrbWeaverSend, this);

        for(auto it = m_mask.begin();it != m_mask.end();++it){
            if(!m_mask[it->first]){
                Ptr<Packet> packet = GeneratePacket();
                if(packet != nullptr){
                    Ptr<NetDevice> dev = m_devices[it->first];
                    dev->Send(packet, dev->GetBroadcast(), 0x0800);
                }
            }
            m_mask_counter[it->first] = 0;
            m_mask[it->first] = false;
        }
    }
    else if(m_orbweaver & 1){
        Simulator::Schedule(NanoSeconds(150), &SwitchNode::OrbWeaverSend, this);

        for(auto it = m_mask.begin();it != m_mask.end();++it){
            if(it->second){
                m_mask[it->first] = false;
            }
            else{
                Ptr<Packet> packet = GeneratePacket();
                if(packet != nullptr){
                    Ptr<NetDevice> dev = m_devices[it->first];
                    dev->Send(packet, dev->GetBroadcast(), 0x0800);
                    m_mask[it->first] = true;
                } 
            }
        }
    }
}

void 
SwitchNode::CacheInfo(PathHeader pathHeader){
    uint32_t hash = Hash32((char*)(&pathHeader), sizeof(PathHeader));
    uint32_t arrIndex = hash % m_array.size();

    if(m_array[arrIndex].Empty() || !(m_array[arrIndex] == pathHeader)){
        m_array[arrIndex] = pathHeader;
        if(m_queue.size() > arrIndex){
            m_queue.pop();
            std::cout << "Information Loss" << std::endl;
        }
        m_queue.push(pathHeader);
    }
}

bool
SwitchNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    if(protocol != 0x0800){
        std::cout << "Unknown protocol" << std::endl;
        return false;
    }
    
    Ptr<Packet> packet = p->Copy();

    Ipv4Header ipHeader;
    packet->RemoveHeader(ipHeader);
    uint32_t proto = ipHeader.GetProtocol();
    uint32_t src = ipHeader.GetSource().Get();
    uint32_t dst = ipHeader.GetDestination().Get();
    uint32_t ttl = ipHeader.GetTtl();

    auto vec = m_routeForward[dst];
    if(vec.size() <= 0){
        std::cout << "Unknown Destination for Routing" << std::endl;
        return false;
    }
    if(ttl == 0){
        std::cout << "Error: TTL = 0" << std::endl;
        return false;
    }
    if(proto != 6 && proto != 17){
        std::cout << "Unknown Protocol" << std::endl;
        return false;
    }
    ipHeader.SetTtl(ttl - 1);

    PathHeader pathHeader;
    pathHeader.SetSrcIP(src);
    pathHeader.SetDstIP(dst);

    uint32_t devId = 0;
    if(proto == 6){
        TcpHeader tcpHeader;
        packet->RemoveHeader(tcpHeader);

        uint32_t srcPort = tcpHeader.GetSourcePort();
        uint32_t dstPort = tcpHeader.GetDestinationPort();
        pathHeader.SetSrcPort(srcPort);
        pathHeader.SetDstPort(dstPort);

        uint32_t hash = HashFlow(pathHeader);
        devId = vec[hash % vec.size()];

        if(m_orbweaver & 1){
            pathHeader.SetNodeID(m_id);
            pathHeader.SetTTL(ttl);
            CacheInfo(pathHeader);
        }

        packet->AddHeader(tcpHeader);
    }
    else if(proto == 17){
        UdpHeader udpHeader;
        packet->RemoveHeader(udpHeader);

        uint32_t srcPort = udpHeader.GetSourcePort();
        uint32_t dstPort = udpHeader.GetDestinationPort();
        pathHeader.SetSrcPort(srcPort);
        pathHeader.SetDstPort(dstPort);

        if((m_orbweaver & 5) == 5){
            std::vector<uint32_t> potentialPorts;
            for(uint32_t dev : vec){
                if(m_mask.find(dev) == m_mask.end())
                    std::cout << "Cannot find Dev " << dev << " in Mask of Node " << m_id << std::endl;
                if(m_mask[dev] == false)
                    potentialPorts.push_back(dev);
            }

            if(potentialPorts.size() <= 0){
                for(auto it = m_mask.begin();it != m_mask.end();++it){
                    if(it->second == false){
                        potentialPorts.push_back(it->first);
                    }
                }

                if(potentialPorts.size() <= 0){
                    uint32_t hash = HashFlow(pathHeader);
                    potentialPorts.push_back(vec[hash % vec.size()]);
                }
            }
            devId = potentialPorts[rand() % potentialPorts.size()];
        }
        else if(m_orbweaver & 1){
            uint32_t hash = HashFlow(pathHeader);
            devId = vec[hash % vec.size()];
        }
        else{
            std::cout << "Unknown OrbWeaver Setting." << std::endl;
        }

        packet->AddHeader(udpHeader);
    }

    packet->AddHeader(ipHeader);
    if((m_orbweaver & 3) == 3){
        if(m_mask.find(devId) != m_mask.end()){
            m_mask_counter[devId] += packet->GetSize();
            if(m_mask_counter[devId] > 1400)
                m_mask[devId] = true;
        }
    }
    else if(m_orbweaver & 1){
        if(m_mask.find(devId) != m_mask.end())
            m_mask[devId] = true;
    }

    Ptr<NetDevice> dev = m_devices[devId];
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void
SwitchNode::AddHostRouteOther(Ipv4Address dest, uint32_t devId, bool set)
{
    m_routeOther[dest.Get()].push_back(devId);
    if(set){
        m_mask[devId] = false; 
        m_mask_counter[devId] = 0; 
    }
}


} // namespace ns3
