#ifndef BUFFER_SWITCH_H
#define BUFFER_SWITCH_H

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

class BufferSwitch : public Node
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    BufferSwitch();

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

    struct DeviceProperty{
      bool isCollector;
      bool isPoll;
      bool isPush;

      uint32_t devId;
      uint32_t generateGap;

      int64_t m_lastTime;

      DeviceProperty(){
        isCollector = isPoll = isPush = false;
        devId = 0;
        generateGap = 0x7fffffff;
        m_lastTime = 0;
      }
    }

    void SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap);

    void SetDeviceCollector(uint32_t devId);

    void SetEcmp(uint32_t Ecmp);
    void SetOrbWeaver(uint32_t OrbWeaver);

    bool IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);
    bool EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);

  protected:

    int64_t m_lastTime = 2200000000;

    const uint32_t batchSize = 4;
    const uint32_t arrSize = 65537;
    const uint32_t queueSize = 1024;

    int m_ecmp;

    bool m_orbweaver = false;

    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;
    std::unordered_map<Ptr<NetDevice>, DeviceProperty> m_deviceMap;

    std::queue<PathHeader> m_buffer;
    std::vector<PathHeader> m_table;

    void GeneratePacket();
    Ptr<Packet> CreatePacket(uint8_t type);

    bool AddPathHeader(Ptr<Packet> packet);
    void BufferData(Ptr<Packet> packet);

    bool IngressPipelineUser(Ptr<Packet> packet);
    bool IngressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev);

    bool EgressPipelineUser(Ptr<Packet> packet);
    bool EgressPipelineIdle(Ptr<Packet> packet, Ptr<NetDevice> dev);

};

} // namespace ns3

#endif /* BUFFER_SWITCH_H */