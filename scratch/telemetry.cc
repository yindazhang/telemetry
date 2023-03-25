#undef PGO_TRAINING
#define PATH_TO_PGO_CONFIG "path_to_pgo_config"

#include <iostream>
#include <iomanip> 
#include <fstream>
#include <unordered_map>
#include <chrono>

#include "schedule.h"

NS_LOG_COMPONENT_DEFINE("Telemetry");


int main(int argc, char *argv[])
{
	std::string flow_file = "Hadoop_143_0.5_10G_0.2";

	CommandLine cmd(__FILE__);
	cmd.AddValue("time", "the total run time (s), by default 0.5", duration);
	cmd.AddValue("flow", "the flow file", flow_file);
	cmd.AddValue("fctRecord", "record fct", fct_record);
	cmd.AddValue("record", "record telemetry", recordConfig);
	cmd.AddValue("INT", "the size of INT header", intSize);
	cmd.AddValue("taskId", "ID of task: (1) path tracing (2) port utilization", taskId);
	cmd.AddValue("utilGap", "Generate gap of port utilization", utilGap);
	cmd.AddValue("Collector bandwitdh (Mbps)", "Bandwidth to the collector", collectorMbps);
	cmd.AddValue("OrbWeaver", "Start OrbWeaver", OrbWeaver);
	cmd.AddValue("ECMP", "configuration of ecmp: (0) 5-tuple (1) dst IP", ecmpConfig);
	cmd.AddValue("Failure", "Link failure", failConfig);
    cmd.Parse(argc, argv);
	
	std::cout << "Run Telemetry." << std::endl;

	file_name = "scratch/" + flow_file + "s_ECMP" + std::to_string(ecmpConfig);
	if(failConfig)
		file_name += "_Fail";
	if(intSize > 0)
		file_name += "_INT" + std::to_string(intSize);
	else if(OrbWeaver > 0)
		file_name += "_Orb" + std::to_string(OrbWeaver);

	build_dctcp();
	build_leaf_spine();
	start_sink_app();
	std::cout << "Finish Topology." << std::endl;

	auto start = std::chrono::system_clock::now();

	schedule_flow(flow_file);

	Simulator::Stop(Seconds(start_time + duration + 5));
	Simulator::Run();
	Simulator::Destroy();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << "Used time: " << diff.count() << "s." << std::endl;

	flow_input.close();
	if(fct_record)
		fclose(fct_output);
}