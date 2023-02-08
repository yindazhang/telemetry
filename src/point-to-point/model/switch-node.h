#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include "ns3/node.h"
#include "ns3/ipv4-header.h"
#include "ns3/path-header.h"

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

    void AddHostRouteCollector(uint32_t devId);
    void AddHostRouteOrbWeaver(uint32_t devId);

    void SetFinalHop();
    void SetEcmp(uint32_t Ecmp);
    void SetOrbWeaver(uint32_t OrbWeaver);
    void SetCollectorGap(uint32_t CollectorGap);

    void CacheInfo(PathHeader pathHeader);

    void OrbWeaverSend();
    void CollectorSend();

    Ptr<Packet> GeneratePacket();
    void AddUdpIpHeader(Ptr<Packet> packet);
    bool AddPathHeader(Ptr<Packet> packet, uint32_t batchSize);

  protected:

    const uint32_t arrSize = 65537;

    int m_ecmp;
    int m_collectorGap;

    bool m_orbweaver;
    bool m_finalHop;
    bool m_removeHeader;
    bool m_localBatch;
    bool m_randomWalk;

    std::queue<PathHeader> m_queue;
    std::vector<PathHeader> m_array;

    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;
    std::vector<uint32_t> m_routeCollector;
    std::vector<uint32_t> m_routeOrbWeaver;

    std::unordered_map<uint32_t, bool> m_mask;
    std::unordered_map<uint32_t, uint32_t> m_mask_counter;

    bool ReceiveFromDeviceUser(Ptr<Packet> packet);
    bool ReceiveFromDeviceIdle(Ptr<Packet> packet, uint16_t protocol);

};

} // namespace ns3

#endif /* SWITCH_NODE_H */