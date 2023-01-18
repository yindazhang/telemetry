import subprocess
import argparse
import pandas as pd
import numpy as np

import plotly.graph_objects as go

def parse_tr_file(tr_file):
	total = 0
	f = open(tr_file, "r")
	text = f.read()
	lines = text.split('\n')

	for line in lines:
		numbers = line.split(' ')
		if len(numbers) == 4:
			src = int(numbers[0])
			dst = int(numbers[1])
			src_rack = src // 16
			dst_rack = dst // 16
			if src_rack == dst_rack:
				total += 2
			else:
				total += 6
	
	print("Total telemetry data: " + str(total))

def read_fct_file(fct_file):
	f = open(fct_file, "r")
	text = f.read()
	lines = text.split('\n')

	packet_number = 0
	total_packet_size = 0
	ret = [[], [], []]
	for line in lines:
		numbers = line.split(' ')
		if len(numbers) == 2:
			flow_size = int(numbers[0])
			packet_number += (flow_size + 1439) // 1440
			total_packet_size += flow_size
			if flow_size < 1e5:
				ret[0].append(int(numbers[1]))
			elif flow_size < 1e6:
				ret[1].append(int(numbers[1]))
			else:
				ret[2].append(int(numbers[1]))

	for i in range(len(ret)):
		ret[i] = sorted(ret[i])

	print("Total packet number: " + str(packet_number))
	print("Avg packet size: " + str(total_packet_size / packet_number))

	return ret

if __name__=="__main__":
	parser = argparse.ArgumentParser(description='')
	parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
	args = parser.parse_args()

	parse_tr_file(args.file + ".tr")

	fct_files = []
	fct_files.append(args.file + "s.fct")
	# fct_files.append(args.file + "s_INT4.fct")
	fct_files.append(args.file + "s_Orb1.fct")
	fct_files.append(args.file + "s_Orb3.fct")
	fct_files.append(args.file + "s_Orb7.fct")
	fct_files.append(args.file + "s_Orb15.fct")

	names = ['Original', 'Orbweaver', '+Batching', '+Efficient', '+Random']
	xnames = ['<100K','100K~1M','>1M']

	dic = {'id' : [], 'Mean' : [], '95%' : [], '99%' : []}

	fig = go.Figure()

	for index in range(len(fct_files)):
		y = read_fct_file(fct_files[index])
		fig.add_trace(go.Box(y=y, x=xnames, name=names[index]))

		q1 = [y[0][int(len(y[0])*0.05)], y[1][int(len(y[1])*0.05)], y[2][int(len(y[2])*0.05)]]
		median = [y[0][int(len(y[0])*0.5)], y[1][int(len(y[1])*0.5)], y[2][int(len(y[2])*0.5)]]
		q3 = [y[0][int(len(y[0])*0.95)], y[1][int(len(y[1])*0.95)], y[2][int(len(y[2])*0.95)]]
		lowerfence = [y[0][int(len(y[0])*0.01)], y[1][int(len(y[1])*0.01)], y[2][int(len(y[2])*0.01)]]
		upperfence = [y[0][int(len(y[0])*0.99)], y[1][int(len(y[1])*0.99)], y[2][int(len(y[2])*0.99)]]
		mean = [sum(y[0])/len(y[0]), sum(y[1])/len(y[1]), sum(y[2])/len(y[2])]

		fig.update_traces(q1=q1, median=median, q3=q3, lowerfence=lowerfence,
                  upperfence=upperfence, mean=mean)

		for j in range(len(xnames)):
			dic['id'].append(names[index] + "_" + xnames[j])
			dic['Mean'].append(int(mean[j]))
			dic['95%'].append(int(q3[j]))
			dic['99%'].append(int(upperfence[j]))
	
	df = pd.DataFrame.from_dict(dic)
	df.to_csv('csv/' + args.file + ".csv", index=False)

	print("Finish CSV")

	fig.update_yaxes(type='log', range=[3.5,8])
	fig.update_layout(xaxis_title="Size Range", yaxis_title="FCT (ns)", boxmode='group')
	
	fig.write_image("images/" + args.file + "_fct.pdf")