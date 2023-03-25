#ifndef COLLECTOR_NODE_H
#define COLLECTOR_NODE_H

#include "ns3/node.h"
#include "ns3/ipv4-header.h"
#include "ns3/path-header.h"
#include "ns3/util-header.h"

#include <queue>
#include <unordered_map>

namespace ns3
{

class Application;
class Packet;
class Address;
class Time;

class CollectorNode : public Node
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    CollectorNode();
    virtual ~CollectorNode();

    /**
     * \brief Associate a NetDevice to this node.
     *
     * \param device NetDevice to associate to this node.
     * \returns the index of the NetDevice into the Node's list of
     *          NetDevice.
     */
    uint32_t AddDevice(Ptr<NetDevice> device) override;

    /**
     * \brief Receive a packet from a device.
     * \param device the device
     * \param packet the packet
     * \param protocol the protocol
     * \param from the sender
     * \returns true if the packet has been delivered to a protocol handler.
     */
    bool ReceiveFromDevice(Ptr<NetDevice> device,
                                Ptr<const Packet> packet,
                                uint16_t protocol,
                                const Address& from);

    void SetOutput(std::string output);
    void SetRecord(uint32_t record);
    void SetTask(uint32_t task);

  protected:

    std::set<PathHeader> m_paths;
    std::vector<UtilHeader> m_utils;

    uint64_t m_duplicates = 0;
    
    std::string output_file;

    int m_task;

    bool m_record = false;

};

} // namespace ns3

#endif /* COLLECTOR_NODE_H */