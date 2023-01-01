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

void
SwitchNode::OrbWeaverSend(){
    if(m_orbweaver > 0){
        Simulator::Schedule(NanoSeconds(150), &SwitchNode::OrbWeaverSend, this);

        for(auto it = m_mask.begin();it != m_mask.end();++it){
            if(it->second){
                m_mask[it->first] = false;
            }
            else{
                if(!m_queue.empty()){
                    Entry entry = m_queue.front();
                    m_queue.pop();

                    Ptr<Packet> packet = Create<Packet>((uint8_t*)(&entry), sizeof(Entry));

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

                    SocketPriorityTag priorityTag;
                    priorityTag.SetPriority(1);
                    packet->ReplacePacketTag(priorityTag);

                    Ptr<NetDevice> dev = m_devices[it->first];
                    dev->Send(packet, dev->GetBroadcast(), 0x0800);

                    m_mask[it->first] = true;
                }   
            }
        }
    }
}

void 
SwitchNode::CacheInfo(Entry entry){
    uint32_t hash = Hash32((char*)(&entry), sizeof(Entry));
    uint32_t arrIndex = hash % arrSize;

    if(m_array[arrIndex].Empty() || !m_array[arrIndex].equal(entry)){
        m_array[arrIndex] = entry;
        if(m_queue.size() > arrIndex){
            m_queue.pop();
            std::cout << "Information Loss" << std::endl;
        }
        m_queue.push(entry);
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

    ipHeader.SetTtl(ipHeader.GetTtl() - 1);
    if(ipHeader.GetTtl() == 0){
        std::cout << "Error: TTL = 0" << std::endl;
        return false;
    }

    uint32_t proto = ipHeader.GetProtocol();
    uint64_t src = ipHeader.GetSource().Get();
    uint64_t dst = ipHeader.GetDestination().Get();
    uint8_t ttl = ipHeader.GetTtl();

    uint64_t srcDst = (dst | (src << 32));
    uint32_t devId = 0;

    if(proto == 6){
        TcpHeader tcpHeader;
        packet->RemoveHeader(tcpHeader);

        uint32_t srcPort = tcpHeader.GetSourcePort();
        uint32_t dstPort = tcpHeader.GetDestinationPort();
        uint32_t srcDstPort = (dstPort | (srcPort << 16));

        auto vec = m_routeForward[dst];

        if(vec.size() <= 0){
            std::cout << "Unknown Destination for Routing" << std::endl;
            return false;
        }

        uint32_t hash = Hash32((char*)(&srcDst), 8) * 
                        (Hash32((char*)(&srcDstPort), 4) + 131);
        devId = vec[hash % vec.size()];

        if(m_orbweaver > 0){
            CacheInfo(Entry(srcDst, m_id, ttl));
        }

        packet->AddHeader(tcpHeader);
    }
    else if(proto == 17){
        auto vec = m_routeForward[dst];

        if(vec.size() <= 0){
            std::cout << "Unknown Destination for Routing" << std::endl;
            return false;
        }

        uint32_t hash = Hash32((char*)(&srcDst), 8);
        devId = vec[hash % vec.size()];
    }
    else{
        std::cout << "Unknown Protocol" << std::endl;
        return false;
    }

    m_mask[devId] = true;
    Ptr<NetDevice> dev = m_devices[devId];
    packet->AddHeader(ipHeader);
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeForward[dest.Get()].push_back(devId);
}

void
SwitchNode::AddHostRouteOther(Ipv4Address dest, uint32_t devId)
{
    m_routeOther[dest.Get()].push_back(devId);
    m_mask[devId] = false;
}


} // namespace ns3
