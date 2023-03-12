#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "topology.h"

FILE *fct_output;
std::ifstream flow_input;

std::unordered_map<uint32_t, uint16_t> connectionPort;

struct Flows{
    uint32_t index, total_size;
    uint32_t src, dst, bytes, start_time;
};
Flows flow = {0};

void record_fct(FILE* fout, int64_t size, int64_t fct){
	if(record){
		fprintf(fout, "%ld %ld\n", size, fct);
    	fflush(fout);
	}
}

void ReadFlowInput(){
	if(flow.index < flow.total_size){
		flow_input >> flow.src >> flow.dst >> flow.bytes >> flow.start_time;
	}
}

void ScheduleFlowInputs(){
	while(flow.index < flow.total_size && NanoSeconds(flow.start_time) == Simulator::Now()){
		uint32_t port = 0;
		if(connectionPort.find(flow.src) == connectionPort.end())
			connectionPort[flow.src] = port;
		else{
			port = (connectionPort[flow.src] + 1) % 50000;
			connectionPort[flow.src] = port;
		}

		BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress(serverAddress[flow.dst], DEFAULT_PORT));
		source.SetAttribute("Local", AddressValue(InetSocketAddress(serverAddress[flow.src], port + 10000)));
		source.SetAttribute("MaxBytes", UintegerValue(flow.bytes));

		ApplicationContainer sourceApps = source.Install(servers[flow.src]);
		sourceApps.Get(0)->TraceConnectWithoutContext("BulkEnd", MakeBoundCallback(record_fct, fct_output));
		sourceApps.Start(Time(0));
		
		flow.index++;
		ReadFlowInput();
	}

	if (flow.index < flow.total_size)
		Simulator::Schedule(NanoSeconds(flow.start_time) - Simulator::Now(), ScheduleFlowInputs);
	else
		flow_input.close();
}

void schedule_flow(std::string flow_file){
	if(record)
		fct_output = fopen((file_name + ".fct").c_str(), "w");
    	
	flow_input.open("scratch/" + flow_file + ".tr");
	if(!flow_input.is_open()){
		std::cout << "Cannot open flow file" << std::endl;
		exit(1);
	}

	flow_input >> flow.total_size;
	std::cout << "Total flow number: " << flow.total_size << std::endl;

	if(flow.total_size > 0){
		ReadFlowInput();
		Simulator::Schedule(NanoSeconds(flow.start_time), ScheduleFlowInputs);
	}
}


#endif 