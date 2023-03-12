#ifndef MYQUEUE_H
#define MYQUEUE_H

#include "ns3/drop-tail-queue.h"

#include <vector>

namespace ns3
{

class MyQueue : public Queue<Packet>
{
public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief MyQueue Constructor
     */
    MyQueue();

    ~MyQueue() override;

    bool Enqueue(Ptr<Packet> packet) override;
    Ptr<Packet> Dequeue() override;
    Ptr<Packet> Remove() override;
    Ptr<const Packet> Peek() const override;

    bool IsEmpty() const override;
    uint32_t GetNBytes() const override;

protected:
    std::vector<Ptr<DropTailQueue<Packet>>> m_queues;
    uint32_t m_maxSize;
    uint32_t m_teleSize;
    uint32_t m_ecnThreshold;

    uint64_t totalUserPacket;
    uint64_t dropUserPacket;
    uint64_t totalIDLEPacket;
    uint64_t dropIDLEPacket;
};

} // namespace ns3

#endif /* MYQUEUE_H */