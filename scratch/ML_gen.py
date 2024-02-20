import sys
import random
import math
import heapq
from optparse import OptionParser
from custom_rand import CustomRand

def translate_bandwidth(b):
	if b == None:
		return None
	if type(b)!=str:
		return None
	if b[-1] == 'G':
		return float(b[:-1])*1e9
	if b[-1] == 'M':
		return float(b[:-1])*1e6
	if b[-1] == 'K':
		return float(b[:-1])*1e3
	return float(b)

def poisson(lam):
	return -math.log(1-random.random())*lam

S_TO_NS = 1e9

if __name__ == "__main__":
	parser = OptionParser()
	parser.add_option("-n", "--nhost", dest = "nhost", help = "number of hosts", default = "142")
	parser.add_option("-l", "--load", dest = "load", help = "the percentage of the traffic load to the network capacity, by default 0.5", default = "0.5")
	parser.add_option("-b", "--bandwidth", dest = "bandwidth", help = "the bandwidth of host link (G/M/K), by default 25G", default = "25G")
	parser.add_option("-t", "--time", dest = "time", help = "the total run time (s), by default 0.3", default = "0.3")
	options,args = parser.parse_args()

	if not options.nhost:
		print("please use -n to enter number of hosts")
		sys.exit(0)

	nhost = int(options.nhost)
	load = float(options.load)
	bandwidth = translate_bandwidth(options.bandwidth)
	time = float(options.time) * S_TO_NS

	output = "ML_" + options.nhost + "_" + options.load + "_" + \
			options.bandwidth + "_" + options.time + ".tr"

	if bandwidth == None:
		print("bandwidth format incorrect")
		sys.exit(0)

	base_t = 2 * S_TO_NS

	num_of_byte = 1000000
	cdf = [[num_of_byte, 0], [num_of_byte+1, 100]]

	# create a custom random generator, which takes a cdf, and generate number according to the cdf
	customRand = CustomRand()
	if not customRand.setCdf(cdf):
		print("Error: Not valid cdf")
		sys.exit(0)

	
	ofile = open(output, "w")

	# generate flows
	avg = num_of_byte
	avg_inter_arrival = (8*S_TO_NS*avg) / (bandwidth*load)

	n_flow_estimate = int(time / avg_inter_arrival * nhost)
	n_flow = 0
	total_size = 0
	ofile.write("%d \n"%n_flow_estimate)
	print("Estimated number of flow is " + str(n_flow_estimate))
	print("Estimated ratio to collector is " + str(11/142))

	host_pair = [i for i in range(nhost)]
	random.shuffle(host_pair)

	GROUP_NUMBER = 8

	host_list = [(base_t + int(poisson(avg_inter_arrival * (GROUP_NUMBER - 1))), i) for i in range(nhost//GROUP_NUMBER)]
	heapq.heapify(host_list)
	n_flow_collector = 0

	while len(host_list) > 0:
		t,src = host_list[0]
		inter_t = avg_inter_arrival * (GROUP_NUMBER - 1)
		if (t + inter_t > time + base_t):
			heapq.heappop(host_list)
		else:
			size = int(customRand.rand())
			if size <= 0:
				size = 1
			n_flow += GROUP_NUMBER*(GROUP_NUMBER-1)
			total_size += GROUP_NUMBER*(GROUP_NUMBER-1)*size

			for x in range(GROUP_NUMBER):
				for y in range(GROUP_NUMBER):
					if x == y:
						continue
					ofile.write("%d %d %d %d\n"%(host_pair[src*GROUP_NUMBER+x]+1,\
						host_pair[src*GROUP_NUMBER+y] + 1, size, t))
					if host_pair[src*GROUP_NUMBER+y] > 130:
						n_flow_collector += 1
			heapq.heapreplace(host_list, (t + inter_t, src))

	ofile.seek(0)
	ofile.write("%d"%n_flow)
	ofile.close()
	print("Real average size is " + str(total_size / n_flow))
	print("Real number of flow is " + str(n_flow))
	print("Estimated ratio to collector is " + str(n_flow_collector / n_flow))