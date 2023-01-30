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

#include "telemetry-header.h"
#include "path-header.h"

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

CollectorNode::CollectorNode() : Node() {}

CollectorNode::~CollectorNode(){
    if(m_paths.size() > 0){
        std::cout << "Receive entries: " << m_paths.size() << std::endl;
        std::cout << "Number of duplicates: " << m_duplicates << std::endl;
    }
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

bool
CollectorNode::ReceiveFromDevice(Ptr<NetDevice> device,
                                  Ptr<const Packet> p,
                                  uint16_t protocol,
                                  const Address& from)
{
    if(protocol != 0x0700 && protocol != 0x0800){
        std::cout << "Unknown protocol in ReceiveFromDevice" << std::endl;
        return false;
    }
    
    Ptr<Packet> packet = p->Copy();

    if(protocol == 0x0800){
        Ipv4Header ipHeader;
        packet->RemoveHeader(ipHeader);
        uint8_t proto = ipHeader.GetProtocol();

        if(proto != 17){
            std::cout << "Not UDP protocol for collector" << std::endl;
            return false;
        }

        UdpHeader udpHeader;
        packet->RemoveHeader(udpHeader);
    }

    TelemetryHeader teleHeader;
    packet->RemoveHeader(teleHeader);

    uint32_t number = teleHeader.GetNumber();
    for(uint32_t i = 0;i < number;++i){
        PathHeader pathHeader;
        packet->RemoveHeader(pathHeader);

        /*
        std::cout << "PathHeader: " << pathHeader.GetSrcIP() << " "
            << pathHeader.GetDstIP() << " "
            << pathHeader.GetSrcPort() << " "
            << pathHeader.GetDstPort() << " "
            << pathHeader.GetNodeId() << " "
            << int(pathHeader.GetTTL()) << " "
            << std::endl;
        */
        if(m_paths.find(pathHeader) == m_paths.end()){
            m_paths.insert(pathHeader);
        }
        else{
            m_duplicates += 1;
        }
        
        if(m_paths.size() % 10000 == 9999){
            std::cout << "Receive entries: " << m_paths.size() << std::endl;
            std::cout << "Number of duplicates: " << m_duplicates << std::endl;
        }
    }

    return true;
}

} // namespace ns3
