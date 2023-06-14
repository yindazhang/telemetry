#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include "ns3/node.h"
#include "ns3/ipv4-header.h"
#include "ns3/path-header.h"
#include "ns3/util-header.h"
#include "ns3/drop-header.h"

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
    virtual ~SwitchNode();

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
    void AddTeleRouteTo(uint8_t dest, uint32_t devId);

    struct DeviceProperty{
      std::vector<uint8_t> collectorDst;

      std::unordered_map<uint8_t, bool> isUpperPull;
      std::unordered_map<uint8_t, bool> isLowerPull;

      uint32_t devId;
      uint32_t generateGap;

      int64_t m_lastTime;

      DeviceProperty(){
        devId = 0;
        generateGap = 0x7fffffff;
        m_lastTime = 0;
      }
    };

    uint32_t GetBufferSize();

    void SetDeviceGenerateGap(uint32_t devId, uint32_t generateGap);

    void SetDeviceUpperPull(uint8_t dest, uint32_t devId);
    void SetDeviceLowerPull(uint8_t dest, uint32_t devId);

    void SetUtilGap(uint32_t utilGap);
    void SetHashSeed(uint32_t hashSeed);
    void SetEcmp(uint32_t ecmp);
    void SetRecord(uint32_t record);
    void SetOrbWeaver(uint32_t OrbWeaver);
    void SetOutput(std::string output);

    void SetCollector(uint32_t number);

    void SetPath(int8_t pathType);
    void SetPort(int8_t portType);
    void SetDrop(int8_t dropType);
    void SetGenerate(int64_t bandwidth);

    bool IngressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipeline(Ptr<Packet> packet, uint32_t priority, uint16_t protocol, Ptr<NetDevice> dev);

    bool m_orbweaver = false;

    struct TeleQueue{
      std::vector<PathHeader> pathBatch[3];
      std::vector<UtilHeader> utilBatch[3];
      std::vector<DropHeader> dropBatch[3];
      std::queue<Ptr<Packet>> packets[3];
      uint32_t size = 0;
    };

  protected:
    TeleQueue m_teleQueue;

    bool BatchPath(PathHeader path, uint8_t dest);
    bool BatchUtil(UtilHeader util, uint8_t dest);
    bool BatchDrop(DropHeader drop, uint8_t dest);

    Ptr<Packet> GetTelePacket(uint32_t priority, uint8_t dest);

    int64_t m_lastTime = 2200000000;

    const uint32_t batchSize = 4;
    const uint32_t arrSize = 65537;

    int32_t m_userThd = 128*1024;
    std::unordered_map<Ptr<NetDevice>, int32_t> m_userSize;

    int32_t m_queueThd = 2*1024;
    std::unordered_map<Ptr<NetDevice>, int32_t> m_queueSize;

    int32_t m_bufferThd = 30*1024;

    std::unordered_map<uint8_t, uint32_t> m_teleSend[5];
    std::unordered_map<uint8_t, uint32_t> m_bufferLoss[5];
    std::unordered_map<uint8_t, uint32_t> m_queueLoss[5];

    std::string output_file;

    uint32_t m_collector = 1;
    int m_ecmp;

    uint32_t m_utilGap = 10000;
    uint32_t m_hashSeed = 0;

    bool m_path = false;
    bool m_port = false;
    bool m_drop = false;
    bool m_generate = false;
    double m_generateGap = 1e5;

    int8_t m_pathType = 1;
    int8_t m_portType = 2;
    int8_t m_dropType = 4;

    bool m_record = false;
    
    bool m_postcard = false;
    bool m_basic = false;
    bool m_pull = false;
    bool m_final = false;

    std::unordered_map<uint32_t, uint32_t> m_bytes;

    std::unordered_map<uint8_t, std::vector<uint32_t>> m_teleForward;
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_routeForward;

    std::unordered_map<Ptr<NetDevice>, DeviceProperty> m_deviceMap;

    std::vector<PathHeader> m_table;
    std::set<PathHeader> m_paths;

    void SetQueueThd();

    void GeneratePacket();
    void RecordUtil();
    void GenerateUtil();

    Ptr<Packet> CreatePacket(uint8_t priority);
    void SendPostcard(uint8_t dest);

    void BufferData(Ptr<Packet> packet);

    bool IngressPipelineUser(Ptr<Packet> packet);
    bool EgressPipelineUser(Ptr<Packet> packet);

    bool IngressPipelinePostcard(Ptr<Packet> packet, Ptr<NetDevice> dev);

    bool IngressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev);
    bool IngressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev);

    Ptr<Packet> EgressPipelineSeed(Ptr<Packet> packet, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipelinePush(Ptr<Packet> packet, Ptr<NetDevice> dev);
    Ptr<Packet> EgressPipelinePull(Ptr<Packet> packet, Ptr<NetDevice> dev);

};

} // namespace ns3

#endif /* SWITCH_NODE_H */