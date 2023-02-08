#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "topology.h"

FILE *fct_output;
std::ifstream flow_input;

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
		BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress(serverAddress[flow.dst], DEFAULT_PORT));
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
	if(record){
		std::string file_name = "scratch/" + flow_file + "s_ECMP" + std::to_string(ecmpConfig);


		if(intSize > 0){
			file_name += "_INT" + std::to_string(intSize);
			fct_output = fopen(("scratch/" + flow_file + "s_INT" + std::to_string(intSize) + ".fct").c_str(), "w");
		}
		else if(OrbWeaver > 0){
			file_name += "_Orb" + std::to_string(OrbWeaver);
		}

		file_name += ".fct";
		fct_output = fopen(file_name.c_str(), "w");
	}
    	
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