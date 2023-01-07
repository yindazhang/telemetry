#include "my-queue.h"

#include "ns3/abort.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include "ns3/socket.h"
#include "ns3/ipv4-header.h"
#include "ns3/ppp-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MyQueue");

NS_OBJECT_ENSURE_REGISTERED(MyQueue);

TypeId
MyQueue::GetTypeId()
{
    static TypeId tid = 
        TypeId("ns3::MyQueue")
            .SetParent<Queue<Packet>>()
            .SetGroupName("PointToPoint")
            .AddConstructor<MyQueue>();
    return tid;
}

MyQueue::MyQueue()
{
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues.push_back(CreateObject<DropTailQueue<Packet>>());
    m_queues[0]->SetMaxSize(QueueSize("1MiB"));
    m_queues[1]->SetMaxSize(QueueSize("1MiB"));
}

MyQueue::~MyQueue() {}

bool
MyQueue::Enqueue(Ptr<Packet> item)
{
    if(GetNBytes() > 200000){
        return false;
    }

    PppHeader ppp;
    Ipv4Header ipHeader;
    item->RemoveHeader(ppp);
    item->RemoveHeader(ipHeader);

    uint32_t priority = 0;
    SocketPriorityTag priorityTag;
    if(item->PeekPacketTag(priorityTag)){
        priority = (priorityTag.GetPriority() & 0x1);
    }

    if(priority == 0){
        if(GetNBytes() > 1500 * 30){
            if(ipHeader.GetEcn() == Ipv4Header::ECN_ECT1 || 
                    ipHeader.GetEcn() == Ipv4Header::ECN_ECT0){
                ipHeader.SetEcn(Ipv4Header::ECN_CE);
            }
        }
    }

    item->AddHeader(ipHeader);
    item->AddHeader(ppp);
    return m_queues[priority]->Enqueue(item);
}

Ptr<Packet>
MyQueue::Dequeue()
{
    Ptr<Packet> ret = nullptr;
    for(auto queue : m_queues){
        ret = queue->Dequeue();
        if(ret != nullptr)
            return ret;
    }
    return nullptr;
}

Ptr<Packet>
MyQueue::Remove()
{
    std::cout << "Remove in MyQueue is not implemented now." << std::endl;
    return nullptr;
}

Ptr<const Packet>
MyQueue::Peek() const
{
    std::cout << "Peek in MyQueue is not implemented now." << std::endl;
    return nullptr;
}

bool 
MyQueue::IsEmpty() const
{
    return (m_queues[0]->IsEmpty() && m_queues[1]->IsEmpty());
}

uint32_t
MyQueue::GetNBytes() const
{
    return (m_queues[0]->GetNBytes() + m_queues[1]->GetNBytes());
}

} // namespace ns3