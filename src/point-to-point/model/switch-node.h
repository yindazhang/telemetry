#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include "ns3/node.h"
#include "ns3/ipv4-header.h"

#include <queue>
#include <unordered_map>

namespace ns3
{

class Application;
class Packet;
class Address;
class Time;

class SwitchNode : public Node
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    SwitchNode();

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

    void AddHostRouteTo(Ipv4Address dest, uint32_t devId);
    void AddHostRouteOther(Ipv4Address dest, uint32_t devId);

    void SetOrbWeaver(uint32_t OrbWeaver);

    struct Entry{
      uint64_t srcDst;
      uint32_t srcDstPort;
      uint16_t nodeId;
      uint8_t ttl;

      Entry(uint64_t _srcDst = 0, uint32_t _srcDstPort = 0, 
              uint32_t _nodeId = 0, uint8_t _ttl = 0):
        srcDst(_srcDst), srcDstPort(_srcDstPort), nodeId(_nodeId), ttl(_ttl){}

      bool Empty(){
        return (srcDst == 0) && (ttl == 0);
      }

      bool equal(const Entry& other){
        return (srcDst == other.srcDst) && 
                (srcDstPort == other.srcDstPort) &&
                (nodeId == other.nodeId) && 
                (ttl == other.ttl);
      }
    };

    void CacheInfo(Entry entry);

    void OrbWeaverSend();

  protected:

    const uint32_t arrSize = 65537;

    uint32_t m_orbweaver;

    std::queue<Entry> m_queue;
    std::vector<Entry> m_array;

    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeOther;
    std::unordered_map<uint32_t, bool> m_mask;

};

} // namespace ns3

#endif /* SWITCH_NODE_H */