import subprocess
import argparse
import pandas as pd
import numpy as np

'''
def read_fct_file(fct_file):
	ret = {'Mean' : [], '99%' : [], '99.9%': []}
	dfs = []

	dfs.append(df[df[0] < 100000][1].sort_values())
	dfs.append(df[(df[0] >= 100000) & (df[0] <= 1000000)][1].sort_values())
	dfs.append(df[df[0] > 1000000][1].sort_values())
	dfs.append(df[1].sort_values())

	for tmp in dfs:
		ret['Mean'].append(tmp.mean())
		ret['99%'].append(tmp.iloc[int(0.99 * len(tmp))])
		ret['99.9%'].append(tmp.iloc[int(0.999 * len(tmp))])

	return ret
'''

if __name__=="__main__":
	parser = argparse.ArgumentParser(description='')
	parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
	args = parser.parse_args()

	names = ['Original', 'Baseline', 'Ours1', 'Ours2', 'Ours3']
	stats = ['Number', 'Mean', '99%', '99.9%']

	#appends = ["s_ECMP1.fct", "s_ECMP1_Orb9.fct", "s_ECMP1_Orb5.fct", \
	#			 "s_ECMP1_Orb1.fct", "s_ECMP1_Orb3.fct"]
	appends = ["s_ECMP1_Fail.fct", "s_ECMP1_Fail_Orb9.fct", "s_ECMP1_Fail_Orb5.fct", \
				 "s_ECMP1_Fail_Orb1.fct", "s_ECMP1_Fail_Orb3.fct"]

	if len(appends) != len(names):
		print("Error in length")

	dfs = {}
	for i in range(len(appends)):
		dfs[names[i]] = pd.read_csv(args.file + appends[i], header=None, delimiter=r"\s+")

	dic = {'Time' : []}
	for name in names:
		for stat in stats:
			dic[name + "_" + stat] = []

	for t in range(2000000000, 3500000000, 10000000):
		dic['Time'].append(t)
		for name in names:
			df = dfs[name]
			df = df[df[2] < t][1].sort_values()
			size = len(df)
			if size != 0:
				dic[name + "_Number"].append(size)
				dic[name + "_Mean"].append(df.mean())
				dic[name + "_99%"].append(df.iloc[int(0.99 * size)])
				dic[name + "_99.9%"].append(df.iloc[int(0.999 * size)])
			else:
				dic[name + "_Number"].append(0)
				dic[name + "_Mean"].append(0)
				dic[name + "_99%"].append(0)
				dic[name + "_99.9%"].append(0)

	df = pd.DataFrame.from_dict(dic)
	df.to_csv('csv/' + args.file + ".csv", index=False)

	print("Finish CSV")