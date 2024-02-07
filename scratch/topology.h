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

// Leaf-spine
std::vector<Ptr<SwitchNode>> leaves;
std::vector<Ptr<SwitchNode>> spines;

// Fat-tree
std::vector<Ptr<SwitchNode>> cores;
std::vector<Ptr<SwitchNode>> edges;
std::vector<Ptr<SwitchNode>> aggregations;

//All switches
std::vector<Ptr<SwitchNode>> switches;
std::vector<Ptr<CollectorNode>> collectors;
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
	//Config::SetDefault("ns3::TcpSocket::DataRetries", UintegerValue(UINT32_MAX));
	//Config::SetDefault("ns3::TcpSocket::ConnCount", UintegerValue(UINT32_MAX));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1440 - intSize));
	Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(MicroSeconds(2000)));
	Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(MicroSeconds(200)));
	Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(MicroSeconds(800)));
	Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(MicroSeconds(10)));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));
}

/*
void build_leaf_spine_routing(
	uint32_t SERVER_PER_LEAF, 
    uint32_t NUM_LEAF, 
    uint32_t NUM_SPINE){

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	std::cout << "Start Leaf Spine Routing" << std::endl;

	for(uint32_t i = 0;i < NUM_LEAF * SERVER_PER_LEAF - 1;++i){
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
		}
	}

	for(uint32_t i = 0;i < NUM_LEAF;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / SERVER_PER_LEAF;
			if(rack_id != i){
				for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
					leaves[i]->AddHostRouteTo(serverAddress[k], SERVER_PER_LEAF + spineId + 1);
				}
			}
			else{
				leaves[i]->AddHostRouteTo(serverAddress[k], k % SERVER_PER_LEAF + 1);
			}
		}
	}

	collectors[0]->SetDeviceGenerateGap(1, 20);
	if((OrbWeaver == 0x2) || (OrbWeaver & 0x3) == 0x3){
		for(uint32_t i = 0;i < NUM_SPINE;++i){
			spines[i]->SetDeviceGenerateGap(NUM_LEAF, 50);
			spines[i]->AddTeleRouteTo(0, NUM_LEAF);
		}
		for(uint32_t i = 0;i < NUM_LEAF - 1;++i){
			for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
				leaves[i]->SetDeviceGenerateGap(SERVER_PER_LEAF + spineId + 1, 100);
				leaves[i]->AddTeleRouteTo(0, SERVER_PER_LEAF + spineId + 1);
			}
		}
		leaves[NUM_LEAF - 1]->SetDeviceGenerateGap(SERVER_PER_LEAF, 20);
		leaves[NUM_LEAF - 1]->AddTeleRouteTo(0, SERVER_PER_LEAF);
	}
	else if((OrbWeaver & 0x9) == 0x9){
		for(uint32_t i = 0;i < NUM_SPINE;++i){
			for(uint32_t k = 1;k < NUM_LEAF;++k){
				spines[i]->SetDeviceGenerateGap(k, 100);
				spines[i]->SetDeviceLowerPull(0, k);
			}
		}
		for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
			leaves[NUM_LEAF - 1]->SetDeviceGenerateGap(SERVER_PER_LEAF + spineId + 1, 50);
			leaves[NUM_LEAF - 1]->SetDeviceLowerPull(0, SERVER_PER_LEAF + spineId + 1);
		}
	}
	else if((OrbWeaver & 0x11) == 0x11){
		for(uint32_t i = 0;i < NUM_SPINE;++i){
			for(uint32_t k = 1;k < NUM_LEAF;++k){
				spines[i]->SetDeviceGenerateGap(k, 100);
				spines[i]->SetDeviceLowerPull(0, k);
			}
			spines[i]->SetDeviceGenerateGap(NUM_LEAF, 100);
			spines[i]->SetDeviceUpperPull(0, NUM_LEAF);
		}
		for(uint32_t i = 0;i < NUM_LEAF - 1;++i){
			for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
				leaves[i]->SetDeviceGenerateGap(SERVER_PER_LEAF + spineId + 1, 100);
				leaves[i]->SetDeviceUpperPull(0, SERVER_PER_LEAF + spineId + 1);
			}
		}
		for(uint32_t spineId = 0;spineId < NUM_SPINE;++spineId){
			leaves[NUM_LEAF - 1]->SetDeviceGenerateGap(SERVER_PER_LEAF + spineId + 1, 50);
			leaves[NUM_LEAF - 1]->SetDeviceLowerPull(0, SERVER_PER_LEAF + spineId + 1);
		}
	}
}

void build_leaf_spine(
    uint32_t SERVER_PER_LEAF = 36, 
    uint32_t NUM_LEAF = 4, 
    uint32_t NUM_SPINE = 3){

	// Initilize node
	serverAddress.resize(SERVER_PER_LEAF * NUM_LEAF);
	servers.resize(SERVER_PER_LEAF * NUM_LEAF - 1);
	collectors.resize(1);

	leaves.resize(NUM_LEAF);
	spines.resize(NUM_SPINE);

	for(uint32_t i = 0;i < NUM_LEAF * SERVER_PER_LEAF - 1;++i)
		servers[i] = CreateObject<Node>();

	collectors[0] = CreateObject<CollectorNode>();
	collectors[0]->SetRecord(recordConfig);
	collectors[0]->SetOutput(file_name);
	collectors[0]->SetDest(0);
	collectors[0]->SetOrbWeaver(OrbWeaver);

	for(uint32_t i = 0;i < NUM_LEAF;++i){
		leaves[i] = CreateObject<SwitchNode>();
		leaves[i]->SetOrbWeaver(OrbWeaver);
		leaves[i]->SetEcmp(ecmpConfig);
		leaves[i]->SetHashSeed(1);

		if(taskId & 1)
			leaves[i]->SetPath(1);
		if(taskId & 2)
			leaves[i]->SetPort(2);

		leaves[i]->SetRecord(recordConfig);
		leaves[i]->SetOutput(file_name);
		leaves[i]->SetUtilGap(utilGap);

		switches.push_back(leaves[i]);
	}
	for(uint32_t i = 0;i < NUM_SPINE;++i){
		spines[i] = CreateObject<SwitchNode>();
		spines[i]->SetOrbWeaver(OrbWeaver);
		spines[i]->SetEcmp(ecmpConfig);
		spines[i]->SetHashSeed(2);

		if(taskId & 1)
			spines[i]->SetPath(1);
		if(taskId & 2)
			spines[i]->SetPort(2);

		spines[i]->SetRecord(recordConfig);
		spines[i]->SetOutput(file_name);
		spines[i]->SetUtilGap(utilGap);

		switches.push_back(spines[i]);
	}

	InternetStackHelper internet;
    internet.InstallAll();

	// Initilize link
	PointToPointHelper pp_server_switch;
	pp_server_switch.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	pp_server_switch.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_server_switch.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_switch_switch;
	pp_switch_switch.SetDeviceAttribute("DataRate", StringValue("40Gbps"));
	pp_switch_switch.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_switch_switch.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_recirculate;
	pp_recirculate.SetDeviceAttribute("DataRate", StringValue("40Gbps"));
	pp_recirculate.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_recirculate.SetChannelAttribute("Delay", StringValue("1us"));

	TrafficControlHelper tch;
	tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", 
						QueueSizeValue(QueueSize("4MiB")));

	Ipv4AddressHelper ipv4;
	for(uint32_t i = 0;i < NUM_LEAF;++i){
		for(uint32_t j = 0;j < SERVER_PER_LEAF;++j){
			uint32_t server_id = i * SERVER_PER_LEAF + j;

			NetDeviceContainer netDev;
			if(server_id == (NUM_LEAF * SERVER_PER_LEAF - 1))
				netDev = pp_server_switch.Install(collectors[0], leaves[i]);
			else
				netDev = pp_server_switch.Install(servers[server_id], leaves[i]);

			tch.Install(netDev);

			std::string ipBase = "10." + std::to_string(i) + "." + std::to_string(j) + ".0";
			ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
			Ipv4InterfaceContainer addrContainer = ipv4.Assign(netDev);
			serverAddress[server_id] = addrContainer.GetAddress(0);
		}
	}
	
	for(uint32_t i = 0;i < NUM_SPINE;++i){
		for(uint32_t j = 0;j < NUM_LEAF;++j){
			NetDeviceContainer netDev;

			if(failConfig && ((i == NUM_SPINE - 1 && j == NUM_LEAF - 1) || (i == 0 && j == 0)))
			 	netDev = pp_server_switch.Install(leaves[j], spines[i]);
			else
				netDev = pp_switch_switch.Install(leaves[j], spines[i]);

			std::string ipBase = "100." + std::to_string(i) + "." + std::to_string(j) + ".0";
			ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
			ipv4.Assign(netDev);
		}
	}

	for(uint32_t i = 0;i < NUM_LEAF;++i){
		NetDeviceContainer netDev;
		netDev = pp_recirculate.Install(leaves[i], leaves[i]);
		std::string ipBase = "63." + std::to_string(i) + "." + std::to_string(i) + ".0";
		ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
		ipv4.Assign(netDev);
	}
	for(uint32_t i = 0;i < NUM_SPINE;++i){
		NetDeviceContainer netDev;
		netDev = pp_recirculate.Install(spines[i], spines[i]);
		std::string ipBase = "127." + std::to_string(i) + "." + std::to_string(i) + ".0";
		ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
		ipv4.Assign(netDev);
	}


	build_leaf_spine_routing(SERVER_PER_LEAF, NUM_LEAF, NUM_SPINE);
}
*/

void build_fat_tree_routing(
	uint32_t K, 
    uint32_t NUM_BLOCK ,
	uint32_t RATIO){

	int base = 1;
	if(!hG)
		base = 10;

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	std::cout << "Start Fat Tree Routing" << std::endl;

	uint32_t number_server = K * K * NUM_BLOCK * RATIO;

	for(uint32_t i = 1;i < number_server - 1;++i){
		Ptr<Ipv4> ipv4Server = servers[i-1]->GetObject<Ipv4>();
		Ptr<Ipv4StaticRouting> routeServer = ipv4RoutingHelper.GetStaticRouting(ipv4Server);
		std::string ipAddr = std::to_string(i / (K * RATIO) / K) + "." + std::to_string((i / (K * RATIO)) % K) + 
									"." + std::to_string(i % (K * RATIO)) + ".2";
		for(uint32_t k = 0;k < serverAddress.size();++k){
			if(i != k)
				routeServer->AddHostRouteTo(serverAddress[k], Ipv4Address(ipAddr.c_str()), 1);
		}
	}

	for(uint32_t i = 0;i < K * K;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / K / K / RATIO;
			cores[i]->AddHostRouteTo(serverAddress[k], rack_id + 1);
		}
	}

	for(uint32_t i = 0;i < NUM_BLOCK * K;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / K / K / RATIO;
			if(rack_id != i / K){
				for(uint32_t coreId = 1;coreId <= K;++coreId)
					aggregations[i]->AddHostRouteTo(serverAddress[k], K + coreId);
			}
			else
				aggregations[i]->AddHostRouteTo(serverAddress[k], (k / K / RATIO) % K + 1);
		}
	}

	for(uint32_t i = 0;i < NUM_BLOCK * K;++i){
		for(uint32_t k = 0;k < serverAddress.size();++k){
			uint32_t rack_id = k / K / RATIO;
			if(rack_id != i){
				for(uint32_t aggId = 1;aggId <= K;++aggId)
					edges[i]->AddHostRouteTo(serverAddress[k], K * RATIO + aggId);
			}
			else
				edges[i]->AddHostRouteTo(serverAddress[k], k % (K * RATIO) + 1);
		}
	}

	if((OrbWeaver == 0x2) || (OrbWeaver & 0x3) == 0x3){
		if(storeConfig){
			edges[0]->SetDeviceGenerateGap(1, 4 * base);
			edges[0]->AddTeleRouteTo(1, 1);
			edges[K * RATIO - 1]->SetDeviceGenerateGap(K * RATIO, 4 * base);
			edges[K * RATIO - 1]->AddTeleRouteTo(0, K * RATIO);

			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[0]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[0]->AddTeleRouteTo(0, K * RATIO + aggId);
			}
			for(uint32_t i = 1;i < K * RATIO - 1;++i){
				for(uint32_t aggId = 1;aggId <= K;++aggId){
					edges[i]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
					edges[i]->AddTeleRouteTo(0, K * RATIO + aggId);
					edges[i]->AddTeleRouteTo(1, K * RATIO + aggId);
				}
			}
			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[K*RATIO-1]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[K*RATIO-1]->AddTeleRouteTo(1, K * RATIO + aggId);
			}

			for(uint32_t j = 0;j < K;++j){
				for(uint32_t coreId = 1;coreId <= K;++coreId){
					aggregations[j]->SetDeviceGenerateGap(K + coreId, 4 * base);
					aggregations[j]->AddTeleRouteTo(0, K + coreId);
				}
				aggregations[j]->SetDeviceGenerateGap(1, 4 * base);
				aggregations[j]->AddTeleRouteTo(1, 1);
			}
			for(uint32_t i = 1;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t coreId = 1;coreId <= K;++coreId){
						aggregations[i*K+j]->SetDeviceGenerateGap(K + coreId, 4 * base);
						aggregations[i*K+j]->AddTeleRouteTo(0, K + coreId);
						aggregations[i*K+j]->AddTeleRouteTo(1, K + coreId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				for(uint32_t coreId = 1;coreId <= K;++coreId){
					aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(K + coreId, 4 * base);
					aggregations[(NUM_BLOCK-1)*K+j]->AddTeleRouteTo(1, K + coreId);
				}
				aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(K, 4 * base);
				aggregations[(NUM_BLOCK-1)*K+j]->AddTeleRouteTo(0, K);
			}

			for(uint32_t i = 0;i < K * K;++i){
				cores[i]->SetDeviceGenerateGap(NUM_BLOCK, 4 * base);
				cores[i]->AddTeleRouteTo(0, NUM_BLOCK);
				cores[i]->SetDeviceGenerateGap(1, 4 * base);
				cores[i]->AddTeleRouteTo(1, 1);
			}
		}
		else{
			edges[K * RATIO - 1]->SetDeviceGenerateGap(K * RATIO, 4 * base);
			edges[K * RATIO - 1]->AddTeleRouteTo(0, K * RATIO);

			for(uint32_t i = 0;i < K * RATIO - 1;++i){
				for(uint32_t aggId = 1;aggId <= K;++aggId){
					edges[i]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
					edges[i]->AddTeleRouteTo(0, K * RATIO + aggId);
				}
			}

			for(uint32_t i = 0;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t coreId = 1;coreId <= K;++coreId){
						aggregations[i*K+j]->SetDeviceGenerateGap(K + coreId, 4 * base);
						aggregations[i*K+j]->AddTeleRouteTo(0, K + coreId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(K, 4 * base);
				aggregations[(NUM_BLOCK-1)*K+j]->AddTeleRouteTo(0, K);
			}

			for(uint32_t i = 0;i < K * K;++i){
				cores[i]->SetDeviceGenerateGap(NUM_BLOCK, 4 * base);
				cores[i]->AddTeleRouteTo(0, NUM_BLOCK);
			}
		}
	}
	else if((OrbWeaver & 0x9) == 0x9){
		collectors[0]->SetDeviceGenerateGap(1, 4 * base);
		
		if(storeConfig){
			collectors[1]->SetDeviceGenerateGap(1, 4 * base);
			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[K*RATIO-1]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[K*RATIO-1]->SetDeviceLowerPull(0, K * RATIO + aggId);
			}
			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[0]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[0]->SetDeviceLowerPull(1, K * RATIO + aggId);
			}

			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					aggregations[j]->SetDeviceGenerateGap(k, 4 * base);
					if(k != 1)
						aggregations[j]->SetDeviceLowerPull(1, k);
					if(k <= K)
						aggregations[j]->SetDeviceLowerPull(0, k);
				}
			}
			for(uint32_t i = 1;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t edgeId = 1;edgeId <= K;++edgeId){
						aggregations[i*K+j]->SetDeviceGenerateGap(edgeId, 4 * base);
						aggregations[i*K+j]->SetDeviceLowerPull(0, edgeId);
						aggregations[i*K+j]->SetDeviceLowerPull(1, edgeId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(k, 4 * base);
					if(k != K)
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(0, k);
					if(k <= K)
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(1, k);
				}
			}
			
			for(uint32_t i = 0;i < K * K;++i){
				for(uint32_t j = 1;j <= NUM_BLOCK;++j){
					cores[i]->SetDeviceGenerateGap(j, 4 * base);
					if(j != NUM_BLOCK)
						cores[i]->SetDeviceLowerPull(0, j);
					if(j != 1)
						cores[i]->SetDeviceLowerPull(1, j);
				}
			}
		}
		else{
			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[K * RATIO - 1]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[K * RATIO - 1]->SetDeviceLowerPull(0, K * RATIO + aggId);
			}

			for(uint32_t i = 0;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t edgeId = 1;edgeId <= K;++edgeId){
						aggregations[i*K+j]->SetDeviceGenerateGap(edgeId, 4 * base);
						aggregations[i*K+j]->SetDeviceLowerPull(0, edgeId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					if(k != K){
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(k, 4 * base);
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(0, k);
					}
				}
			}
			
			for(uint32_t i = 0;i < K * K;++i){
				for(uint32_t j = 1;j < NUM_BLOCK;++j){
					cores[i]->SetDeviceGenerateGap(j, 4 * base);
					cores[i]->SetDeviceLowerPull(0, j);
				}
			}
		}
	}
	else if((OrbWeaver & 0x11) == 0x11 || (OrbWeaver & 0x21) == 0x21){
		collectors[0]->SetDeviceGenerateGap(1, 4 * base);
		
		if(storeConfig){
			collectors[1]->SetDeviceGenerateGap(1, 4 * base);

			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[0]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[0]->SetDeviceUpperPull(0, K * RATIO + aggId);
				edges[0]->SetDeviceLowerPull(1, K * RATIO + aggId);
			}
			for(uint32_t i = 1;i < K * RATIO - 1;++i){
				for(uint32_t aggId = 1;aggId <= K;++aggId){
					edges[i]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
					edges[i]->SetDeviceUpperPull(0, K * RATIO + aggId);
					edges[i]->SetDeviceUpperPull(1, K * RATIO + aggId);
				}
			}
			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[K * RATIO - 1]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[K * RATIO - 1]->SetDeviceLowerPull(0, K * RATIO + aggId);
				edges[K * RATIO - 1]->SetDeviceUpperPull(1, K * RATIO + aggId);
			}

			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					aggregations[j]->SetDeviceGenerateGap(k, 4 * base);
					if(k != 1)
						aggregations[j]->SetDeviceLowerPull(1, k);
					else
						aggregations[j]->SetDeviceUpperPull(1, k);
					if(k <= K)
						aggregations[j]->SetDeviceLowerPull(0, k);
					else
						aggregations[j]->SetDeviceUpperPull(0, k);
				}
			}
			for(uint32_t i = 1;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t edgeId = 1;edgeId <= K;++edgeId){
						aggregations[i*K+j]->SetDeviceGenerateGap(edgeId, 4 * base);
						aggregations[i*K+j]->SetDeviceLowerPull(0, edgeId);
						aggregations[i*K+j]->SetDeviceLowerPull(1, edgeId);
					}
					for(uint32_t coreId = 1;coreId <= K;++coreId){
						aggregations[i*K+j]->SetDeviceGenerateGap(K+coreId, 4 * base);
						aggregations[i*K+j]->SetDeviceUpperPull(0, K+coreId);
						aggregations[i*K+j]->SetDeviceUpperPull(1, K+coreId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(k, 4 * base);
					if(k != K)
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(0, k);
					else
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceUpperPull(0, k);
					if(k <= K)
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(1, k);
					else
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceUpperPull(1, k);
				}
			}
			
			for(uint32_t i = 0;i < K * K;++i){
				for(uint32_t j = 1;j <= NUM_BLOCK;++j)
					cores[i]->SetDeviceGenerateGap(j, 4 * base);
				cores[i]->SetDeviceUpperPull(1, 1);
				for(uint32_t j = 2;j <= NUM_BLOCK;++j)
					cores[i]->SetDeviceLowerPull(1, j);
				for(uint32_t j = 1;j < NUM_BLOCK;++j)
					cores[i]->SetDeviceLowerPull(0, j);
				cores[i]->SetDeviceUpperPull(0, NUM_BLOCK);
			}	
		}
		else{
			if(tempConfig){
				collectors[1]->SetDeviceGenerateGap(1, 4 * base);
				edges[0]->SetDeviceGenerateGap(1, 4 * base);
				edges[0]->SetDeviceLowerPull(0, 1);
			}

			for(uint32_t aggId = 1;aggId <= K;++aggId){
				edges[K * RATIO - 1]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
				edges[K * RATIO - 1]->SetDeviceLowerPull(0, K * RATIO + aggId);
			}

			for(uint32_t i = 0;i < K * RATIO - 1;++i){
				for(uint32_t aggId = 1;aggId <= K;++aggId){
					edges[i]->SetDeviceGenerateGap(K * RATIO + aggId, 4 * base);
					edges[i]->SetDeviceUpperPull(0, K * RATIO + aggId);
				}
			}

			for(uint32_t i = 0;i < NUM_BLOCK - 1;++i){
				for(uint32_t j = 0;j < K;++j){
					for(uint32_t edgeId = 1;edgeId <= K;++edgeId){
						aggregations[i*K+j]->SetDeviceGenerateGap(edgeId, 4 * base);
						aggregations[i*K+j]->SetDeviceLowerPull(0, edgeId);
					}
					for(uint32_t coreId = 1;coreId <= K;++coreId){
						aggregations[i*K+j]->SetDeviceGenerateGap(K+coreId, 4 * base);
						aggregations[i*K+j]->SetDeviceUpperPull(0, K+coreId);
					}
				}
			}
			for(uint32_t j = 0;j < K;++j){
				for(uint32_t k = 1; k <= 2*K;++k){
					if(k != K){
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(k, 4 * base);
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceLowerPull(0, k);
					}
					else{
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceGenerateGap(k, 4 * base);
						aggregations[(NUM_BLOCK-1)*K+j]->SetDeviceUpperPull(0, k);
					}
				}
			}
			
			for(uint32_t i = 0;i < K * K;++i){
				for(uint32_t j = 1;j < NUM_BLOCK;++j){
					cores[i]->SetDeviceGenerateGap(j, 4 * base);
					cores[i]->SetDeviceLowerPull(0, j);
				}
				cores[i]->SetDeviceGenerateGap(NUM_BLOCK, 4 * base);
				cores[i]->SetDeviceUpperPull(0, NUM_BLOCK);
			}				
		}
	}
}

void build_fat_tree(
    uint32_t K = 3, 
    uint32_t NUM_BLOCK = 4,
	uint32_t RATIO = 4){

	// Initilize node
	uint32_t number_server = K * K * NUM_BLOCK * RATIO;
	uint32_t number_collector = 2;

	serverAddress.resize(number_server);
	servers.resize(number_server - number_collector);
	collectors.resize(number_collector);

	edges.resize(K * NUM_BLOCK);
	aggregations.resize(K * NUM_BLOCK);
	cores.resize(K * K);

	for(uint32_t i = 0;i < number_server - number_collector;++i)
		servers[i] = CreateObject<Node>();
	
	for(uint32_t i = 0;i < number_collector;++i){
		collectors[i] = CreateObject<CollectorNode>();
		collectors[i]->SetRecord(recordConfig);
		collectors[i]->SetOutput(file_name);
		collectors[i]->SetOrbWeaver(OrbWeaver);
	}
	collectors[0]->SetPriority(0, 0);
	if(storeConfig)
		collectors[1]->SetPriority(1, 0);
	if(tempConfig)
		collectors[1]->SetPriority(0, 10000);

	for(uint32_t i = 0;i < K * NUM_BLOCK;++i){
		edges[i] = CreateObject<SwitchNode>();
		edges[i]->SetOrbWeaver(OrbWeaver);
		edges[i]->SetEcmp(ecmpConfig);
		edges[i]->SetHashSeed(1);

		if(taskId == 1)
			edges[i]->SetPath(1);
		else if(taskId == 2)
			edges[i]->SetPort(2);
		else if(taskId == 3)
			edges[i]->SetGenerate(generateBps);
		else if(taskId == 4)
			edges[i]->SetDrop(4);
		else if(taskId == 7 || taskId == 15){
			edges[i]->SetPath(1);
			edges[i]->SetPort(2);
			edges[i]->SetDrop(4);
		}
		
		
		edges[i]->SetRecord(recordConfig);
		edges[i]->SetOutput(file_name);
		edges[i]->SetUtilGap(utilGap);
		edges[i]->SetThd(thd);
		edges[i]->SetTime(measureStart, measureEnd);

		if(storeConfig)
			edges[i]->SetCollector(2);
		else
			edges[i]->SetCollector(1);

		switches.push_back(edges[i]);
	}
	for(uint32_t i = 0;i < K * NUM_BLOCK;++i){
		aggregations[i] = CreateObject<SwitchNode>();
		aggregations[i]->SetOrbWeaver(OrbWeaver);
		aggregations[i]->SetEcmp(ecmpConfig);
		aggregations[i]->SetHashSeed(2);

		if(taskId == 1)
			aggregations[i]->SetPath(1);
		else if(taskId == 2)
			aggregations[i]->SetPort(2);
		else if(taskId == 3)
			aggregations[i]->SetGenerate(generateBps);
		else if(taskId == 4)
			aggregations[i]->SetDrop(4);
		else if(taskId == 7 || taskId == 15){
			aggregations[i]->SetPath(1);
			aggregations[i]->SetPort(2);
			aggregations[i]->SetDrop(4);
		}

		aggregations[i]->SetRecord(recordConfig);
		aggregations[i]->SetOutput(file_name);
		aggregations[i]->SetUtilGap(utilGap);
		aggregations[i]->SetThd(thd);
		aggregations[i]->SetTime(measureStart, measureEnd);

		if(storeConfig)
			aggregations[i]->SetCollector(2);
		else
			aggregations[i]->SetCollector(1);

		switches.push_back(aggregations[i]);
	}
	for(uint32_t i = 0;i < K * K;++i){
		cores[i] = CreateObject<SwitchNode>();
		cores[i]->SetOrbWeaver(OrbWeaver);
		cores[i]->SetEcmp(ecmpConfig);
		cores[i]->SetHashSeed(3);

		if(taskId == 1)
			cores[i]->SetPath(1);
		else if(taskId == 2)
			cores[i]->SetPort(2);
		else if(taskId == 3)
			cores[i]->SetGenerate(generateBps);
		else if(taskId == 4)
			cores[i]->SetDrop(4);
		else if(taskId == 7){
			cores[i]->SetPath(1);
			cores[i]->SetPort(2);
			cores[i]->SetDrop(4);
		}
		else if(taskId == 15){
			cores[i]->SetPath(1);
			cores[i]->SetPort(2);
			cores[i]->SetDrop(4);
			cores[i]->SetCount(8);
		}

		cores[i]->SetRecord(recordConfig);
		cores[i]->SetOutput(file_name);
		cores[i]->SetUtilGap(utilGap);
		cores[i]->SetThd(thd);
		cores[i]->SetTime(measureStart, measureEnd);

		if(storeConfig)
			cores[i]->SetCollector(2);
		else
			cores[i]->SetCollector(1);

		switches.push_back(cores[i]);
	}

	InternetStackHelper internet;
    internet.InstallAll();

	// Initilize link
	
	PointToPointHelper pp_server_switch;
	pp_server_switch.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	pp_server_switch.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_server_switch.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_switch_switch;
	pp_switch_switch.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	pp_switch_switch.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_switch_switch.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_recirculate;
	pp_recirculate.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	pp_recirculate.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_recirculate.SetChannelAttribute("Delay", StringValue("1us"));

	PointToPointHelper pp_fail;
	pp_recirculate.SetDeviceAttribute("DataRate", StringValue("5Gbps"));
	pp_recirculate.SetDeviceAttribute("INT", UintegerValue(intSize));
	pp_recirculate.SetChannelAttribute("Delay", StringValue("1us"));

	if(hG){
		pp_server_switch.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
		pp_switch_switch.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
		pp_recirculate.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
		pp_fail.SetDeviceAttribute("DataRate", StringValue("50Gbps"));
	}

	TrafficControlHelper tch;
	tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", 
						QueueSizeValue(QueueSize("16MiB")));

	Ipv4AddressHelper ipv4;
	for(uint32_t i = 0;i < K * NUM_BLOCK;++i){
		for(uint32_t j = 0;j < K * RATIO;++j){
			uint32_t server_id = i * K * RATIO + j;

			NetDeviceContainer netDev;
			if(server_id == 0)
				netDev = pp_server_switch.Install(collectors[1], edges[i]);
			else if(server_id == (number_server - 1))
				netDev = pp_server_switch.Install(collectors[0], edges[i]);
			else
				netDev = pp_server_switch.Install(servers[server_id - 1], edges[i]);

			tch.Install(netDev);
			std::string ipBase = std::to_string(i / K) + "." + std::to_string(i % K) + 
									"." + std::to_string(j) + ".0";
			ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
			Ipv4InterfaceContainer addrContainer = ipv4.Assign(netDev);
			serverAddress[server_id] = addrContainer.GetAddress(0);
		}
	}
	
	for(uint32_t i = 0;i < NUM_BLOCK;++i){
		for(uint32_t j = 0;j < K;++j){
			for(uint32_t k = 0;k < K;++k){
				NetDeviceContainer netDev;

				if(failConfig && (i == NUM_BLOCK - 1 && j == K - 1 && k == K - 1))
					netDev = pp_fail.Install(edges[i*K+j], aggregations[i*K+k]);
				else
					netDev = pp_switch_switch.Install(edges[i*K+j], aggregations[i*K+k]);

				std::string ipBase = std::to_string(i + NUM_BLOCK) + "." + std::to_string(j) + 
									"." + std::to_string(k) + ".0";
				ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
				ipv4.Assign(netDev);
			}
		}
	}

	for(uint32_t i = 0;i < NUM_BLOCK;++i){
		for(uint32_t j = 0;j < K;++j){
			for(uint32_t k = 0;k < K;++k){
				NetDeviceContainer netDev;

				if(failConfig && (i == NUM_BLOCK - 1 && j == 0 && k == 0))
					netDev = pp_fail.Install(aggregations[i*K+j], cores[j*K+k]);
				else
					netDev = pp_switch_switch.Install(aggregations[i*K+j], cores[j*K+k]);

				std::string ipBase = std::to_string(i + 2*NUM_BLOCK) + "." + std::to_string(j) + 
									"." + std::to_string(k) + ".0";
				ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
				ipv4.Assign(netDev);
			}
		}
	}

	for(uint32_t i = 0;i < K * NUM_BLOCK;++i){
		NetDeviceContainer netDev;
		netDev = pp_recirculate.Install(edges[i], edges[i]);
		std::string ipBase = "31." + std::to_string(i) + "." + std::to_string(i) + ".0";
		ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
		ipv4.Assign(netDev);
	}
	for(uint32_t i = 0;i < K * NUM_BLOCK;++i){
		NetDeviceContainer netDev;
		netDev = pp_recirculate.Install(aggregations[i], aggregations[i]);
		std::string ipBase = "63." + std::to_string(i) + "." + std::to_string(i) + ".0";
		ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
		ipv4.Assign(netDev);
	}
	for(uint32_t i = 0;i < K * K;++i){
		NetDeviceContainer netDev;
		netDev = pp_recirculate.Install(cores[i], cores[i]);
		std::string ipBase = "127." + std::to_string(i) + "." + std::to_string(i) + ".0";
		ipv4.SetBase(ipBase.c_str(), "255.255.255.0");
		ipv4.Assign(netDev);
	}

	build_fat_tree_routing(K, NUM_BLOCK, RATIO);
}

void start_sink_app(){
	for(uint32_t i = 0;i < servers.size();++i){
		PacketSinkHelper sink("ns3::TcpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), DEFAULT_PORT));
  		ApplicationContainer sinkApps = sink.Install(servers[i]);
		sinkApps.Start(Seconds(start_time - 1));
  		sinkApps.Stop(Seconds(start_time + duration + 4));
	}
}

#endif 