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
	cmd.AddValue("time", "the total run time (s), by default 1", duration);
	cmd.AddValue("flow", "the flow file", flow_file);
	cmd.AddValue("record", "record fct", record);
	cmd.AddValue("INT", "the size of INT header", intSize);
	cmd.AddValue("OrbWeaver", "Start OrbWeaver", OrbWeaver);
    cmd.Parse(argc, argv);
	
	std::cout << "Run Telemetry." << std::endl;
	auto start = std::chrono::system_clock::now();

	build_dctcp();
	build_leaf_spine();
	start_sink_app();
	std::cout << "Finish Topology." << std::endl;
	schedule_flow(flow_file);

	Simulator::Stop(Seconds(start_time + duration + 10));
	Simulator::Run();
	Simulator::Destroy();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << "Used time: " << diff.count() << "s." << std::endl;

	flow_input.close();
	if(record)
		fclose(fct_output);
}