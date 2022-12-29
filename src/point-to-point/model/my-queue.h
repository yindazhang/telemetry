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

    std::vector<Ptr<DropTailQueue<Packet>>> m_queues;
};

} // namespace ns3

#endif /* MYQUEUE_H */