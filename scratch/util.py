import subprocess
import argparse
import pandas as pd
import numpy as np

collectorDic = {}
switchDic = {}

def parse_switch_file(path_file):
    df = pd.read_csv(path_file, header=None)
    totalLoss = df[3].sum() + df[4].sum()
    totalSend = df[5].sum()
    print(totalLoss)
    print(totalSend)
    print(totalSend - totalLoss)
    print(totalLoss / totalSend)
    print()


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()

    parse_switch_file(args.file)