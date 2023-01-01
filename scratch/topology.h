#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-static-routing-helper.h"

#include "my_config.h"

using namespace ns3;

std::vector<Ptr<Node>> servers;
std::vector<Ptr<SwitchNode>> leaves;
std::vector<Ptr<SwitchNode>> spines;

std::vector<Ipv4Address> serverAddress;

void print_addr(Ipv4Address addr){
	uint32_t number = addr.Get();
	std::cout << "Addr: " << 
		((number >> 24) & 0xff) << "." << 
		((number >> 16) & 0xff) << "." << 
		((number >> 8) & 0xff) << "." << 
		((number >> 0) & 0xff) << std::endl;
}

void build_dctcp(){
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpDctcp"));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1440 - intSize));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
	Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MicroSeconds(100)));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));
}

void build_leaf_spine_routing(
	uint32_t SERVER_PER_LEAF, 
    uint32_t NUM_LEAF, 
    uint32_t NUM_SPINE){
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	std::cout << "Start routing" << std::endl;

	for(uint32_t i = 0;i < NUM_LEAF * SERVER_PER_LEAF;++i){
		Ptr<Ipv4> ipv4Server = servers[i]->GetObject<Ipv4>();
		Ptr<Ipv4StaticRouting> routeServer = ipv4RoutingHelper.GetStaticRouting(ipv4Server);
		std::string ipAddr = "10." + std::to_string(i / SERVER_PER_LEAF)
						+ "." + std::to_string(i % SERVER_PER_LEAF) + ".2";

		for(uint32_t k = 0;k < serverAddress.size();++k){
			if(i != k)
				routeServer->AddHostRouteTo(serverAddress[k], Ipv4Address(ipAddr.c_str()), 1);
		}
	}

	for(uint32_t i = 0;i < NUM_SPINE;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / SERVER_PER_LEAF;
			spines[i]->AddHostRouteTo(serverAddress[k], rack_id + 1);
			for(uint32_t leafId = 0;leafId < NUM_LEAF;++leafId){
				spines[i]->AddHostRouteOther(serverAddress[k], leafId + 1);
			}
		}
	}

	for(uint32_t i = 0;i < NUM_LEAF;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / SERVER_PER_LEAF;
			if(rack_id != i){
				for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
					leaves[i]->AddHostRouteTo(serverAddress[k], SERVER_PER_LEAF + spineId + 1);
					leaves[i]->AddHostRouteOther(serverAddress[k], SERVER_PER_LEAF + spineId + 1);
				}
			}
			else{
				leaves[i]->AddHostRouteTo(serverAddress[k], k % SERVER_PER_LEAF + 1);
				leaves[i]->AddHostRouteOther(serverAddress[k], k % SERVER_PER_LEAF + 1);
			}
		}
	}
}

void build_leaf_spine(
    uint32_t SERVER_PER_LEAF = 16, 
    uint32_t NUM_LEAF = 9, 
    uint32_t NUM_SPINE = 4){

	// Initilize node
	serverAddress.resize(SERVER_PER_LEAF * NUM_LEAF);
	servers.resize(SERVER_PER_LEAF * NUM_LEAF);
	leaves.resize(NUM_LEAF);
	spines.resize(NUM_SPINE);

	for(uint32_t i = 0;i < NUM_LEAF * SERVER_PER_LEAF;++i)
		servers[i] = CreateObject<Node>();
	for(uint32_t i = 0;i < NUM_LEAF;++i){
		leaves[i] = CreateObject<SwitchNode>();
		leaves[i]->SetOrbWeaver(OrbWeaver);
	}
	for(uint32_t i = 0;i < NUM_SPINE;++i){
		spines[i] = CreateObject<SwitchNode>();
		spines[i]->SetOrbWeaver(OrbWeaver);
	}

	InternetStackHelper internet;
    internet.InstallAll();

	// Initilize link
	PointToPointHelper pp_server_leaf;
	pp_server_leaf.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	pp_server_leaf.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_server_leaf.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_leaf_spine;
	pp_leaf_spine.SetDeviceAttribute("DataRate", StringValue("40Gbps"));
	pp_leaf_spine.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_leaf_spine.SetChannelAttribute("Delay", StringValue("1us"));

	TrafficControlHelper tch;
	tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", 
						QueueSizeValue(QueueSize("4MiB")));

	Ipv4AddressHelper ipv4;
	for(uint32_t i = 0;i < NUM_LEAF;++i){
		for(uint32_t j = 0;j < SERVER_PER_LEAF;++j){
			uint32_t server_id = i * SERVER_PER_LEAF + j;
			NetDeviceContainer netDev = pp_server_leaf.Install(servers[server_id], leaves[i]);
			tch.Install(netDev);

			std::string ipBase = "10." + std::to_string(i) + "." + std::to_string(j) + ".0";
			ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
			Ipv4InterfaceContainer addrContainer = ipv4.Assign(netDev);
			serverAddress[server_id] = addrContainer.GetAddress(0);
		}
	}

	
	for(uint32_t i = 0;i < NUM_SPINE;++i){
		for(uint32_t j = 0;j < NUM_LEAF;++j){
			NetDeviceContainer netDev = pp_leaf_spine.Install(leaves[j], spines[i]);

			std::string ipBase = "100." + std::to_string(i) + "." + std::to_string(j) + ".0";
			ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
			ipv4.Assign(netDev);
		}
	}

	build_leaf_spine_routing(SERVER_PER_LEAF, NUM_LEAF, NUM_SPINE);
}

void start_sink_app(){
	for(uint32_t i = 0;i < servers.size() - 1;++i){
		PacketSinkHelper sink("ns3::TcpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), DEFAULT_PORT));
  		ApplicationContainer sinkApps = sink.Install(servers[i]);
		sinkApps.Start(Seconds(start_time - 1));
  		sinkApps.Stop(Seconds(start_time + duration + 2));
	}

	PacketSinkHelper sink("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), COLLECT_PORT));
  	ApplicationContainer sinkApps = sink.Install(servers[servers.size() - 1]);
	sinkApps.Start(Seconds(start_time - 1));
	sinkApps.Stop(Seconds(start_time + duration + 2));
}

#endif 