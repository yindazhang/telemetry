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
    if(m_task == 1){
        if(m_record){
            std::cout << "Receive entries: " << m_paths.size() << std::endl;
            std::cout << "Number of duplicates: " << m_duplicates << std::endl;

            FILE* fout = fopen(output_file.c_str(), "w");
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
    else if(m_task == 2){
        std::cout << "Receive entries: " << m_duplicates << std::endl;
    }
    else{
        std::cout << "Unknown task " << m_task << std::endl;
    }
}

void 
CollectorNode::SetTask(uint32_t task){
    m_task = task;
}

void 
CollectorNode::SetRecord(uint32_t record){
    m_record = record;
}

void 
CollectorNode::SetOutput(std::string output){
    if(m_task == 1)
        output_file = output + ".collector.path";
    else if(m_task == 2)
        output_file = output + ".collector.util";
    else
        std::cout << "Unknown task" << std::endl;
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
    if(protocol != 0x0171){
        std::cout << "Unknown prorocol for colllector" << std::endl;
        return false;
    }

    Ptr<Packet> packet = p->Copy();

    if(m_task == 1){
        PathHeader pathHeader;
        packet->RemoveHeader(pathHeader);

        if(m_record && m_paths.find(pathHeader) == m_paths.end()){
            m_paths.insert(pathHeader);
            if(m_paths.size() % 10000 == 9999){
                std::cout << "Receive entries: " << m_paths.size() << std::endl;
                std::cout << "Number of duplicates: " << m_duplicates << std::endl;
            }
            /*
            std::cout << "PathHeader: " << pathHeader.GetSrcIP() << " "
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
    else if(m_task == 2){
        UtilHeader utilHeader;
        packet->RemoveHeader(utilHeader);

        m_duplicates += 1;
        if(m_record)
            m_utils.push_back(utilHeader);
        
        if(m_duplicates % 10000 == 9999)
            std::cout << "Receive entries: " << m_duplicates << std::endl;
        
        /*
        std::cout << "UtilHeader: " << utilHeader.GetNodeId() << " "
                << utilHeader.GetPortId() << " "
                << utilHeader.GetTime() << " "
                << utilHeader.GetByte() << " "
                << std::endl;
        */
    }
    else{
        std::cout << "Unknown task" << std::endl;
    }


    return true;
}

} // namespace ns3
