import subprocess
import argparse
import pandas as pd
import numpy as np

if __name__=="__main__":
	parser = argparse.ArgumentParser(description='')
	parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
	args = parser.parse_args()

	fct_file = args.file + ".fct"
	f = open(fct_file, "r")
	text = f.read()
	lines = text.split('\n')

	dic = {'Flow Size': [], 'Completion Time': []}
	df = pd.DataFrame(dic)

	for line in lines:
		numbers = line.split(' ')
		if len(numbers) == 2:
			dic['Flow Size'].append(int(numbers[0]))
			dic['Completion Time'].append(int(numbers[1]))
	
	df = pd.DataFrame(dic)

	fcts = []
	fcts.append(df.where((df['Flow Size'] < 1e5)).dropna())
	# fcts.append(df.where((df['Flow Size'] >= 1e4) & (df['Flow Size'] < 1e5)).dropna())
	fcts.append(df.where((df['Flow Size'] >= 1e5) & (df['Flow Size'] < 1e6)).dropna())
	fcts.append(df.where((df['Flow Size'] >= 1e6)).dropna())

	for fct in fcts:
		print(fct.shape)
		print("Mean FCT: " + str(fct['Completion Time'].mean()))
		print("95% FCT: " + str(fct['Completion Time'].quantile(q = 0.95)))
		print("99% FCT: " + str(fct['Completion Time'].quantile(q = 0.99)))