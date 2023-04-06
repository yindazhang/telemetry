import subprocess
import argparse
import pandas as pd
import numpy as np

collectorDic = {}
switchDic = {}

def parse_switch_file(path_file):
    df = pd.read_csv(path_file, header=None)
    totalLoss = df[1].sum() + df[2].sum()
    totalSend = df[3].sum()
    print(totalLoss)
    print(totalSend)
    print(totalSend - totalLoss)
    print(totalLoss / totalSend)
    print()


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-f', dest='file', action='store', help="Specify the fct file.")
    args = parser.parse_args()

    common = "s_Topo1_ECMP1_Orb"
    back = []
    for i in ["3", "5", "9"]:
        back.append(common + i + ".switch.util")

    for b in back:
        parse_switch_file(args.file + b)