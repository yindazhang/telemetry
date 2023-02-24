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
    void AddHostRoutePolling(uint32_t devId);
    void AddHostRoutePushing(uint32_t devId);

    void SetEcmp(uint32_t Ecmp);
    void SetOrbWeaver(uint32_t OrbWeaver);
    void SetFinalSwitch();
    void SetCollectorGap(uint32_t CollectorGap);

    bool IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);
    bool EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol);

  protected:

    int64_t m_lastTime = 2200000000;

    const uint32_t batchSize = 4;
    const uint32_t arrSize = 65537;
    const uint32_t queueSize = 1024;

    int m_ecmp;
    int m_collectorGap;

    bool m_orbweaver = false;
    bool m_finalSwitch = false;

    std::queue<PathHeader> m_queue;
    std::vector<PathHeader> m_array;

    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;
    std::vector<uint32_t> m_routeCollector;
    std::vector<uint32_t> m_routePolling;
    std::vector<uint32_t> m_routePushing;

    // std::unordered_map<uint32_t, bool> m_mask;
    // std::unordered_map<uint32_t, uint32_t> m_mask_counter;

    void CollectorSend();
    Ptr<Packet> GeneratePacket(uint8_t number);

    bool AddPathHeader(Ptr<Packet> packet);

    void CacheInfo(Ptr<Packet> packet);
    void CacheInfo(PathHeader pathHeader);

    // void ClearOrbWeaverMask(uint32_t devId);
    // void SetOrbWeaverMask(Ptr<Packet> packet, uint32_t devId);

    bool IngressPipelineUser(Ptr<Packet> packet);
    bool IngressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev);

    bool EgressPipelineUser(Ptr<Packet> packet);
    bool EgressPipelineIdle(Ptr<Packet> packet);

};

} // namespace ns3

#endif /* SWITCH_NODE_H */