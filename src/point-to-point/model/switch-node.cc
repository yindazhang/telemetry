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

SwitchNode::SwitchNode() : Node() {}

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
    
    uint64_t src = ipHeader.GetSource().Get();
    uint64_t dst = ipHeader.GetDestination().Get();
    uint64_t srcDst = (dst | (src << 32));

    auto vec = m_routeTable[dst];

    if(vec.size() <= 0){
        std::cout << "Unknown Destination for Routing" << std::endl;
        return false;
    }

    uint32_t hash = Hash32((char*)(&srcDst), 8);
    uint32_t devId = vec[hash % vec.size()];

    packet->AddHeader(ipHeader);
    Ptr<NetDevice> dev = m_devices[devId];
    
    return dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

void
SwitchNode::AddHostRouteTo(Ipv4Address dest, uint32_t devId)
{
    m_routeTable[dest.Get()].push_back(devId);
}

} // namespace ns3
